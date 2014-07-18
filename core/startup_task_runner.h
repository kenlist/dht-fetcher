#ifndef DHTFETCHER_CORE_STARTUP_TASK_RUNNER_H_
#define DHTFETCHER_CORE_STARTUP_TASK_RUNNER_H_

#include <list>

#include "base/callback.h"
#include "base/single_thread_task_runner.h"

#include "build/build_config.h"

typedef base::Callback<bool(void)> StartupTask;

class StartupTaskRunner {
 public:
  // Constructor: Note that |startup_complete_callback| is optional. If it is
  // not null it will be called once all the startup tasks have run.
  StartupTaskRunner(base::Callback<void()> startup_complete_callback,
                    scoped_refptr<base::SingleThreadTaskRunner> proxy);

  ~StartupTaskRunner();
  
  // Add a task to the queue of startup tasks to be run.
  void AddTask(StartupTask& callback);

  // Start running the tasks asynchronously.
  void StartRunningTasksAsync();

  // Run all tasks, or all remaining tasks, synchronously
  void RunAllTasksNow();

 private:
  friend class base::RefCounted<StartupTaskRunner>;

  std::list<StartupTask> task_list_;
  void WrappedTask();

  base::Callback<void()> startup_complete_callback_;
  scoped_refptr<base::SingleThreadTaskRunner> proxy_;

  DISALLOW_COPY_AND_ASSIGN(StartupTaskRunner);
};

#endif
