#ifndef DHTFETCHER_CORE_CORE_SUB_THREAD_H_
#define DHTFETCHER_CORE_CORE_SUB_THREAD_H_

#include "base/basictypes.h"
#include "thread/core_thread_impl.h"

class NotificationService;
class CoreSubThread : public CoreThreadImpl {
 public:
  explicit CoreSubThread(CoreThread::ID identifier);
  virtual ~CoreSubThread();
  
 protected:
  virtual void Init() OVERRIDE;
  virtual void CleanUp() OVERRIDE;
  
 private:
  // These methods encapsulate cleanup that needs to happen on the IO thread
  // before we call the embedder's CleanUp function.
  void IOThreadPreCleanUp();

  DISALLOW_COPY_AND_ASSIGN(CoreSubThread);
};

#endif
