#include "core_thread_impl.h"

#include <string>

#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_restrictions.h"
#include "base/lazy_instance.h"
#include "base/atomicops.h"

#include "dht-fetcher/core/thread/core_thread_delegate.h"
#include "dht-fetcher/core/thread/core_thread_message_loop_proxy.h"

namespace {

static const char* g_core_thread_names[CoreThread::ID_COUNT] = {
  "",  // UI
  "Core_Sync",   //SYNC
  "Core_FileThread",  // FILE
  "Core_FileUserBlockingThread",  // FILE_USER_BLOCKING
  "Core_IOThread",  // IO
};

struct CoreThreadGlobals {
  CoreThreadGlobals()
      : blocking_pool(new base::SequencedWorkerPool(3, "KanboxBlocking")) {
    
  }

  // This lock protects |threads|. Do not read or modify that array
  // without holding this lock. Do not block while holding this lock.
  base::Lock lock;
  
  // This array is protected by |lock|. The threads are not owned by this
  // array. Typically, the threads are owned on the UI thread by
  // MainLoop. CoreThreadImpl objects remove themselves from this
  // array upon destruction.
  CoreThreadImpl* threads[CoreThread::ID_COUNT];
  
  // Only atomic operations are used on this array. The delegates are not owned
  // by this array, rather by whoever calls CoreThread::SetDelegate.
  CoreThreadDelegate* thread_delegates[CoreThread::ID_COUNT];
  
  const scoped_refptr<base::SequencedWorkerPool> blocking_pool;
};

base::LazyInstance<CoreThreadGlobals>::Leaky
    g_globals = LAZY_INSTANCE_INITIALIZER;

};

CoreThreadImpl::CoreThreadImpl(ID identifier)
    : Thread(g_core_thread_names[identifier]),
      identifier_(identifier) {
  Initialize();
}

CoreThreadImpl::CoreThreadImpl(ID identifier,
                                     base::MessageLoop* message_loop)
    : Thread(message_loop->thread_name().c_str()), identifier_(identifier) {
  set_message_loop(message_loop);
  Initialize();
}

CoreThreadImpl::~CoreThreadImpl() {
  // All Thread subclasses must call Stop() in the destructor. This is
  // doubly important here as various bits of code check they are on
  // the right CoreThread.
  Stop();
  
  CoreThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  globals.threads[identifier_] = NULL;
#ifndef NDEBUG
  // Double check that the threads are ordered correctly in the enumeration.
  for (int i = identifier_ + 1; i < ID_COUNT; ++i) {
    DCHECK(!globals.threads[i]) <<
        "Threads must be listed in the reverse order that they die";
  }
#endif
}

void CoreThreadImpl::Init() {
  CoreThreadGlobals& globals = g_globals.Get();
  
  using base::subtle::AtomicWord;
  AtomicWord* storage =
      reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  CoreThreadDelegate* delegate =
      reinterpret_cast<CoreThreadDelegate*>(stored_pointer);
  
  if (delegate) {
    delegate->Init();
    message_loop()->PostTask(FROM_HERE,
                             base::Bind(&CoreThreadDelegate::InitAsync,
                                        // Delegate is expected to exist for the
                                        // duration of the thread's lifetime
                                        base::Unretained(delegate)));
  }
}

void CoreThreadImpl::Run(base::MessageLoop *message_loop) {
  CoreThread::ID thread_id = ID_COUNT;
  if (!GetCurrentThreadIdentifier(&thread_id))
    return Thread::Run(message_loop);
  
  switch (thread_id) {
    case CoreThread::UI:
      return UIThreadRun(message_loop);
    case CoreThread::FILE:
      return FileThreadRun(message_loop);
    case CoreThread::FILE_USER_BLOCKING:
      return FileUserBlockingThreadRun(message_loop);
    case CoreThread::IO:
      return IOThreadRun(message_loop);
    case CoreThread::ID_COUNT:
      CHECK(false);
      break;
    default:
      break;
  }
  Thread::Run(message_loop);
}

void CoreThreadImpl::CleanUp() {
  CoreThreadGlobals& globals = g_globals.Get();
  
  using base::subtle::AtomicWord;
  AtomicWord* storage =
      reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  CoreThreadDelegate* delegate =
      reinterpret_cast<CoreThreadDelegate*>(stored_pointer);
  
  if (delegate)
    delegate->CleanUp();
}

// We disable optimizations for this block of functions so the compiler doesn't
// merge them all together.
MSVC_DISABLE_OPTIMIZE()
MSVC_PUSH_DISABLE_WARNING(4748)

NOINLINE void CoreThreadImpl::UIThreadRun(
    base::MessageLoop* message_loop) {
  volatile int line_number = __LINE__;
  Thread::Run(message_loop);
  CHECK_GT(line_number, 0);
}

NOINLINE void CoreThreadImpl::FileThreadRun(
    base::MessageLoop* message_loop) {
  volatile int line_number = __LINE__;
  Thread::Run(message_loop);
  CHECK_GT(line_number, 0);
}

NOINLINE void CoreThreadImpl::FileUserBlockingThreadRun(
    base::MessageLoop* message_loop) {
  volatile int line_number = __LINE__;
  Thread::Run(message_loop);
  CHECK_GT(line_number, 0);
}

NOINLINE void CoreThreadImpl::IOThreadRun(
    base::MessageLoop* message_loop) {
  volatile int line_number = __LINE__;
  Thread::Run(message_loop);
  CHECK_GT(line_number, 0);
}

MSVC_POP_WARNING()
MSVC_ENABLE_OPTIMIZE();

// static
bool CoreThreadImpl::PostTaskHelper(
    CoreThread::ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay,
    bool nestable) {
  DCHECK(identifier >= 0 && identifier < ID_COUNT);
  // Optimization: to avoid unnecessary locks, we listed the ID enumeration in
  // order of lifetime.  So no need to lock if we know that the target thread
  // outlives current thread.
  // Note: since the array is so small, ok to loop instead of creating a map,
  // which would require a lock because std::map isn't thread safe, defeating
  // the whole purpose of this optimization.
  CoreThread::ID current_thread = ID_COUNT;
  bool target_thread_outlives_current =
      GetCurrentThreadIdentifier(&current_thread) &&
      current_thread >= identifier;
  
  CoreThreadGlobals& globals = g_globals.Get();
  if (!target_thread_outlives_current)
    globals.lock.Acquire();
  
  base::MessageLoop* message_loop =
      globals.threads[identifier] ? globals.threads[identifier]->message_loop()
                                  : NULL;

  if (message_loop) {
    if (nestable) {
      message_loop->PostDelayedTask(from_here, task, delay);
    } else {
      message_loop->PostNonNestableDelayedTask(from_here, task, delay);
    }
  }
  
  if (!target_thread_outlives_current)
    globals.lock.Release();
  
  return !!message_loop;
}

void CoreThreadImpl::Initialize() {
  CoreThreadGlobals& globals = g_globals.Get();
  
  base::AutoLock lock(globals.lock);
  DCHECK(identifier_ >= 0 && identifier_ < ID_COUNT);
  DCHECK(globals.threads[identifier_] == NULL);
  globals.threads[identifier_] = this;
}

//-----CoreThread static methods' implementaions

bool CoreThread::PostTask(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task) {
  return CoreThreadImpl::PostTaskHelper(
      identifier, from_here, task, base::TimeDelta(), true);
}

bool CoreThread::PostDelayedTask(
    ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure &task,
    base::TimeDelta delay) {
  return CoreThreadImpl::PostTaskHelper(
      identifier, from_here, task, delay, true);
}

bool CoreThread::PostNonNestableTask(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task) {
  return CoreThreadImpl::PostTaskHelper(
      identifier, from_here, task, base::TimeDelta(), false);
}

bool CoreThread::PostNonNestableDelayedTask(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay) {
    return CoreThreadImpl::PostTaskHelper(
        identifier, from_here, task, delay, false);
}

bool CoreThread::PostTaskAndReply(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    const base::Closure &reply) {
  return GetMessageLoopProxyForThread(identifier)->PostTaskAndReply(
      from_here, task, reply);
}

bool CoreThread::PostBlockingPoolTask(
    const tracked_objects::Location &from_here,
    const base::Closure &task) {
  return g_globals.Get().blocking_pool->PostWorkerTask(from_here, task);
}

bool CoreThread::PostBlockingPoolTaskAndReply(
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    const base::Closure &reply) {
  return g_globals.Get().blocking_pool->PostTaskAndReply(
      from_here, task, reply);
}

bool CoreThread::PostBlockingPoolSequencedTask(
    const std::string &sequence_token_name,
    const tracked_objects::Location &from_here,
    const base::Closure &task) {
  return g_globals.Get().blocking_pool->PostNamedSequencedWorkerTask(
      sequence_token_name, from_here, task);
}

base::SequencedWorkerPool* CoreThread::GetBlockingPool() {
  return g_globals.Get().blocking_pool.get();
}

bool CoreThread::IsThreadInitialized(ID identifier) {
  if (g_globals == NULL)
    return false;
  
  CoreThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  DCHECK(identifier >= 0 && identifier < ID_COUNT);
  return globals.threads[identifier] != NULL;
}

bool CoreThread::CurrentlyOn(ID identifier) {
  // We shouldn't use MessageLoop::current() since it uses LazyInstance which
  // may be deleted by ~AtExitManager when a WorkerPool thread calls this
  // function.
  // http://crbug.com/63678
  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  CoreThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  DCHECK(identifier >= 0 && identifier < ID_COUNT);
  return globals.threads[identifier] &&
         globals.threads[identifier]->message_loop() ==
             base::MessageLoop::current();
}

bool CoreThread::IsMessageLoopValid(ID identifier) {
  if (g_globals == NULL)
    return false;

  CoreThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  DCHECK(identifier >= 0 && identifier < ID_COUNT);
  return globals.threads[identifier] &&
         globals.threads[identifier]->message_loop();
}

bool CoreThread::GetCurrentThreadIdentifier(ID *identifier) {
  if (g_globals == NULL)
    return false;
  
  // We shouldn't use MessageLoop::current() since it uses LazyInstance which
  // may be deleted by ~AtExitManager when a WorkerPool thread calls this
  // function.
  // http://crbug.com/63678
  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  base::MessageLoop* cur_message_loop = base::MessageLoop::current();
  CoreThreadGlobals& globals = g_globals.Get();
  for (int i = 0; i < ID_COUNT; ++i) {
    if (globals.threads[i] &&
        globals.threads[i]->message_loop() == cur_message_loop) {
      *identifier = globals.threads[i]->identifier_;
      return true;
    }
  }
  
  return false;
}

scoped_refptr<base::MessageLoopProxy>
CoreThread::GetMessageLoopProxyForThread(ID identifier) {
  return make_scoped_refptr(new CoreThreadMessageLoopProxy(identifier));
}

base::MessageLoop* CoreThread::UnsafeGetMessageLoopForThread(ID identifier) {
  if (g_globals == NULL)
    return NULL;

  CoreThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  base::Thread* thread = globals.threads[identifier];
  DCHECK(thread);
  base::MessageLoop* loop = thread->message_loop();
  return loop;
}

void CoreThread::SetDelegate(
    ID identifier,
    CoreThreadDelegate *delegate) {
  using base::subtle::AtomicWord;
  CoreThreadGlobals& globals = g_globals.Get();
  AtomicWord* storage = reinterpret_cast<AtomicWord*>(
      &globals.thread_delegates[identifier]);
  AtomicWord old_pointer = base::subtle::NoBarrier_AtomicExchange(
      storage, reinterpret_cast<AtomicWord>(delegate));
  
  DCHECK(!delegate || !old_pointer);
}
