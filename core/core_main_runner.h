#ifndef DHTFETCHER_CORE_CORE_MAIN_RUNNER_H_
#define DHTFETCHER_CORE_CORE_MAIN_RUNNER_H_

#include "base/basictypes.h"

struct MainFunctionParams;

class CoreMainRunner {
 public:
  virtual ~CoreMainRunner() {}
  
  static CoreMainRunner* Create();
  
  //! 初始化Core
  virtual bool Initialize(const MainFunctionParams& parameters) = 0;
  
  //! 开始运行Core
  virtual bool Run() = 0;
  
  //! 关闭Core
  virtual void Shutdown() = 0;
  
 protected:
  CoreMainRunner() {}
  
  DISALLOW_COPY_AND_ASSIGN(CoreMainRunner);
};

#endif
