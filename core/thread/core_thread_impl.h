#ifndef DHTFETCHER_CORE_THREAD_CORE_THREAD_IMPL_H_
#define DHTFETCHER_CORE_THREAD_CORE_THREAD_IMPL_H_

#include "base/threading/thread.h"
#include "dht-fetcher/core/thread/core_thread.h"

class CoreThreadImpl : public CoreThread,
                          public base::Thread {
 public:
  explicit CoreThreadImpl(CoreThread::ID identifier);
  CoreThreadImpl(CoreThread::ID identifier,
                    base::MessageLoop* message_loop);
  virtual ~CoreThreadImpl();
  
 protected:
  // Override from base::Thread
  virtual void Init() OVERRIDE;
  virtual void Run(base::MessageLoop* message_loop) OVERRIDE;
  virtual void CleanUp() OVERRIDE;
  
 private:
  // We implement all the functionality of the public CoreThread
  // functions, but state is stored in the CoreThreadImpl to keep
  // the API cleaner. Therefore make CoreThread a friend class.
  friend class CoreThread;
  
  // The following are unique function names that makes it possible to tell
  // the thread id from the callstack alone in crash dumps.
  void UIThreadRun(base::MessageLoop* message_loop);
  void FileThreadRun(base::MessageLoop* message_loop);
  void FileUserBlockingThreadRun(base::MessageLoop* message_loop);
  void IOThreadRun(base::MessageLoop* message_loop);
  
  static bool PostTaskHelper(
      CoreThread::ID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay,
      bool nestable);
      
  // Common initialization code for the constructors.
  void Initialize();
  
  // The identifier of this thread.  Only one thread can exist with a given
  // identifier at a given time.
  ID identifier_;
};

#endif
