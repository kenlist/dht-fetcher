#ifndef DHTFETCHER_CORE_THREAD_CORE_THREAD_DELEGATE_H_
#define DHTFETCHER_CORE_THREAD_CORE_THREAD_DELEGATE_H_

class CoreThreadDelegate {
 public:
  virtual ~CoreThreadDelegate() {}
  
  // Called prior to starting the message loop
  virtual void Init() = 0;
  
  // Called as the first task on the thread's message loop.
  virtual void InitAsync() = 0;

  // Called just after the message loop ends.
  virtual void CleanUp() = 0;
};

#endif
