#ifndef DHTFETCHER_CORE_THREAD_CORE_THREAD_MESSAGE_LOOP_PROXY_H_
#define DHTFETCHER_CORE_THREAD_CORE_THREAD_MESSAGE_LOOP_PROXY_H_

#include "dht-fetcher/core/thread/core_thread.h"

// An implementation of MessageLoopProxy to be used in conjunction
// with CoreThread
class CoreThreadMessageLoopProxy : public base::MessageLoopProxy {
 public:
  explicit CoreThreadMessageLoopProxy(CoreThread::ID identifier)
      : id_(identifier) {}
  
  // MessageLoopProxy implementation.
  virtual bool PostDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task, base::TimeDelta delay) OVERRIDE;
  
  virtual bool PostNonNestableDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay) OVERRIDE;
  
  virtual bool RunsTasksOnCurrentThread() const OVERRIDE;
  
 protected:
  virtual ~CoreThreadMessageLoopProxy() {}

 private:
  CoreThread::ID id_;
  DISALLOW_COPY_AND_ASSIGN(CoreThreadMessageLoopProxy);
};

#endif
