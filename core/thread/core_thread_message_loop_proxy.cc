#include "core_thread_message_loop_proxy.h"

bool CoreThreadMessageLoopProxy::PostDelayedTask(
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay) {
  return CoreThread::PostDelayedTask(id_, from_here, task, delay);
}

bool CoreThreadMessageLoopProxy::PostNonNestableDelayedTask(
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay) {
  return CoreThread::PostNonNestableDelayedTask(
      id_, from_here, task, delay);
}

bool CoreThreadMessageLoopProxy::RunsTasksOnCurrentThread() const {
  return CoreThread::CurrentlyOn(id_);
}
