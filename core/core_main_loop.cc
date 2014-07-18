#include "core_main_loop.h"

#include "crypto/nss_util.h"
#include "base/debug/trace_event.h"
#include "thread/core_thread.h"
#include "base/threading/thread_restrictions.h"
#include "net/base/network_change_notifier.h"

#include "dht-fetcher/core/core_main_parts.h"
#include "dht-fetcher/core/main_function_params.h"
#include "dht-fetcher/core/startup_task_runner.h"
#include "dht-fetcher/core/core_sub_thread.h"
#include "dht-fetcher/core/notification/notification_service.h"
#include "dht-fetcher/common/fetcher_switches.h"

CoreMainLoop* g_current_core_main_loop = NULL;

CoreMainLoop* CoreMainLoop::GetIntance() {
  DCHECK(CoreThread::CurrentlyOn(CoreThread::UI));
  return g_current_core_main_loop;
}

CoreMainLoop::CoreMainLoop(const MainFunctionParams& parameters)
    : parameters_(parameters),
      parsed_command_line_(parameters.command_line),
      created_threads_(false),
      is_succeed_(true),
      is_tracing_startup_(
          parameters.command_line.HasSwitch(switches::kTraceStartup)) {
  DCHECK(!g_current_core_main_loop);
  g_current_core_main_loop = this;
}

CoreMainLoop::~CoreMainLoop() {
  DCHECK_EQ(this, g_current_core_main_loop);
  g_current_core_main_loop = NULL;
}

void CoreMainLoop::Init() {
  TRACE_EVENT0("startup", "CoreMainLoop::Init")
  
  parts_.reset(CoreMainParts::Create(parameters_));
}

// CoreMainLoop stages ==================================================

void CoreMainLoop::EarlyInitialization() {
  TRACE_EVENT0("startup", "CoreMainLoop::EarlyInitialization");
  
  if (parts_)
    parts_->PreEarlyInitialization();
  
#if !defined(USE_OPENSSL)
  // We want to be sure to init NSPR on the main thread.
  crypto::EnsureNSPRInit();
#endif

  if (parts_)
    parts_->PostEarlyInitialization();
}

void CoreMainLoop::MainMessageLoopStart() {
  TRACE_EVENT0("startup", "CoreMainLoop::MainMessageLoopStart")
  if (parts_) {
    TRACE_EVENT0("startup",
        "CoreMainLoop::MainMessageLoopStart:PreMainMessageLoopStart");
    parts_->PreMainMessageLoopStart();
  }

  // Create a MessageLoop if one does not already exist for the current thread.
  if (!base::MessageLoop::current())
    main_message_loop_.reset(new base::MessageLoopForUI);

  InitializeMainThread();

  {
    TRACE_EVENT0("startup", "CoreMainLoop::Subsystem:NetworkChangeNotifier")
    network_change_notifier_.reset(net::NetworkChangeNotifier::Create());
  }

  if (parts_)
    parts_->PostMainMessageLoopStart();
}

void CoreMainLoop::CreateStartupTasks() {
  TRACE_EVENT0("startup", "CoreMainLoop::CreateStartupTasks");

  // First time through, we really want to create all the tasks
  if (!startup_task_runner_.get()) {
#if defined(OS_ANDROID)
    startup_task_runner_ = make_scoped_ptr(new StartupTaskRunner(
        base::Bind(&CoreStartupComplete),
        base::MessageLoop::current()->message_loop_proxy()));
#else
    startup_task_runner_ = make_scoped_ptr(new StartupTaskRunner(
        base::Callback<void()>(),
        base::MessageLoop::current()->message_loop_proxy()));
#endif
    StartupTask pre_create_threads =
        base::Bind(&CoreMainLoop::PreCreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(pre_create_threads);

    StartupTask create_threads =
        base::Bind(&CoreMainLoop::CreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(create_threads);

    StartupTask browser_thread_started = base::Bind(
        &CoreMainLoop::CoreThreadsStarted, base::Unretained(this));
    startup_task_runner_->AddTask(browser_thread_started);

#if defined(OS_ANDROID)
    if (CoreMayStartAsynchronously()) {
      startup_task_runner_->StartRunningTasksAsync();
    }
#endif
  }
#if defined(OS_ANDROID)
  if (!CoreMayStartAsynchronously()) {
    // A second request for asynchronous startup can be ignored, so
    // StartupRunningTasksAsync is only called first time through. If, however,
    // this is a request for synchronous startup then it must override any
    // previous call for async startup, so we call RunAllTasksNow()
    // unconditionally.
    startup_task_runner_->RunAllTasksNow();
  }
#else
  startup_task_runner_->RunAllTasksNow();
#endif
}

void CoreMainLoop::InitializeMainThread() {
  TRACE_EVENT0("startup", "CoreMainLoop::InitializeMainThread")
  const char* kThreadName = "CoreMain";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_)
    main_message_loop_->set_thread_name(kThreadName);

  // Register the main thread by instantiating it, but don't call any methods.
  main_thread_.reset(
      new CoreThreadImpl(CoreThread::UI, base::MessageLoop::current()));
}

// Called just before creating the threads.
bool CoreMainLoop::PreCreateThreads() {
  if (parts_) {
    TRACE_EVENT0("startup",
        "CoreMainLoop::CreateThreads:PreCreateThreads");
    is_succeed_ = parts_->PreCreateThreads();
  }
  
  // If the UI thread blocks, the whole UI is unresponsive.
  // Do not allow disk IO from the UI thread.
  base::ThreadRestrictions::SetIOAllowed(false);
  base::ThreadRestrictions::DisallowWaiting();

  return is_succeed_;
}

// Create all secondary threads.
bool CoreMainLoop::CreateThreads() {
  TRACE_EVENT0("startup", "CoreMainLoop::CreateThreads");

  base::Thread::Options default_options;
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
  base::Thread::Options ui_message_loop_options;
  ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;

  // Start threads in the order they occur in the CoreThread::ID
  // enumeration, except for CoreThread::UI which is the main
  // thread.
  //
  // Must be size_t so we can increment it.
  for (size_t thread_id = CoreThread::UI + 1;
       thread_id < CoreThread::ID_COUNT;
       ++thread_id) {
    scoped_ptr<CoreSubThread>* thread_to_start = NULL;
    base::Thread::Options* options = &default_options;

    switch (thread_id) {
      case CoreThread::FILE_USER_BLOCKING:
        TRACE_EVENT_BEGIN1("startup",
            "CoreMainLoop::CreateThreads:start",
            "Thread", "CoreThread::FILE_USER_BLOCKING");
        thread_to_start = &file_user_blocking_thread_;
        break;
      case CoreThread::FILE:
        TRACE_EVENT_BEGIN1("startup",
            "CoreMainLoop::CreateThreads:start",
            "Thread", "CoreThread::FILE");
        thread_to_start = &file_thread_;
        options = &io_message_loop_options;
        break;
      case CoreThread::SYNC:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "CoreMainLoop::SYNC");
        thread_to_start = &sync_thread_;
        options = &io_message_loop_options;
        break;
      case CoreThread::IO:
        TRACE_EVENT_BEGIN1("startup",
            "CoreMainLoop::CreateThreads:start",
            "Thread", "CoreThread::IO");
        thread_to_start = &io_thread_;
        options = &io_message_loop_options;
        break;
      case CoreThread::UI:
      case CoreThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
    }

    CoreThread::ID id = static_cast<CoreThread::ID>(thread_id);

    if (thread_to_start) {
      (*thread_to_start).reset(new CoreSubThread(id));
      (*thread_to_start)->StartWithOptions(*options);
    } else {
      NOTREACHED();
    }

    TRACE_EVENT_END0("startup", "CoreMainLoop::CreateThreads:start");

  }
  created_threads_ = true;
  return is_succeed_;
}

// Called right after the browser threads have been started.
bool CoreMainLoop::CoreThreadsStarted() {
  TRACE_EVENT0("startup", "CoreMainLoop::CoreThreadsStarted")
  
  // Initialize NotificationService
  NotificationService::GetInstance();

  return is_succeed_;
}

