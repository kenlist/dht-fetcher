#ifndef DHTFETCHER_CORE_SYNCBOX_MAIN_PARTS_H_
#define DHTFETCHER_CORE_SYNCBOX_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "dht-fetcher/core/main_function_params.h"

class CoreMainParts {
 public:
  static CoreMainParts* Create(
      const MainFunctionParams& paramters);
  
  virtual ~CoreMainParts();

  virtual void PreEarlyInitialization();
  virtual void PostEarlyInitialization();
  virtual void PreMainMessageLoopStart();
  virtual void PostMainMessageLoopStart();
  virtual bool PreCreateThreads();

 protected:
  explicit CoreMainParts(
      const MainFunctionParams& paramters);
  
  const MainFunctionParams& parameters() const {
    return parameters_;
  }
  const base::CommandLine& paresd_command_line() const {
    return parsed_command_line_;
  }
  
 private:  
  const MainFunctionParams parameters_;
  const base::CommandLine& parsed_command_line_;
  
  DISALLOW_COPY_AND_ASSIGN(CoreMainParts);
};

#endif
