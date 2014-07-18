#ifndef DHTFETCHER_CORE_CORE_MAIN_LOOP_H_
#define DHTFETCHER_CORE_CORE_MAIN_LOOP_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "dht-fetcher/core/core_sub_thread.h"

namespace base {
class CommandLine;
class FilePath;
class MessageLoop;
}

namespace net {
class NetworkChangeNotifier;
}

class CoreMainParts;
class CoreShutdownImpl;
class CoreThreadImpl;
class CoreURLFetcherFactory;
class StartupTaskRunner;
struct MainFunctionParams;

class CoreMainLoop {
 public:
  static CoreMainLoop* GetIntance();
  
  explicit CoreMainLoop(const MainFunctionParams& paramters);
  virtual ~CoreMainLoop();
  
  void Init();
  
  void EarlyInitialization();
  void MainMessageLoopStart();
  
  // Create and start running the tasks we need to complete startup. Note that
  // this can be called more than once (currently only on Android) if we get a
  // request for synchronous startup while the tasks created by asynchronous
  // startup are still running.
  void CreateStartupTasks();

  // Performs the shutdown sequence, starting with PostMainMessageLoopRun
  // through stopping threads to PostDestroyThreads.
  void ShutdownThreadsAndCleanUp();

  bool is_succeed() const { return is_succeed_; }
  
  bool is_tracing_startup() const { return is_tracing_startup_; }

 private:
  // Initialize main thread.
  void InitializeMainThread();
 
  // Called just before creating the threads.
  bool PreCreateThreads();

  // Create all secondary threads.
  bool CreateThreads();

  // Called right after the threads have been started.
  bool CoreThreadsStarted();

 private:
  // Members initialized on construction ---------------------------------------
  const MainFunctionParams& parameters_;
  const base::CommandLine& parsed_command_line_;
  bool created_threads_;
  bool is_succeed_;

  // Members initialized in |MainMessageLoopStart()| ---------------------------
  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_ptr<net::NetworkChangeNotifier> network_change_notifier_;
  // The startup task runner is created by CreateStartupTasks()
  scoped_ptr<StartupTaskRunner> startup_task_runner_;

  // Destroy parts_ before main_message_loop_ (required) and before other
  // classes constructed in content (but after main_thread_).
  scoped_ptr<CoreMainParts> parts_;
  
  // Members initialized in |InitializeMainThread()| ---------------------------
  // This must get destroyed before other threads that are created in parts_.
  scoped_ptr<CoreThreadImpl> main_thread_;
  
  // Members initialized in |CreateThreads()| ------------------------
  scoped_ptr<CoreSubThread> sync_thread_;
  scoped_ptr<CoreSubThread> file_user_blocking_thread_;
  scoped_ptr<CoreSubThread> file_thread_;
  scoped_ptr<CoreSubThread> io_thread_;
  
  // Members initialized in |CoreThreadsStarted()| ------------------------

  bool is_tracing_startup_;

  DISALLOW_COPY_AND_ASSIGN(CoreMainLoop);
};

#endif
