#ifndef DHTFETCHER_FETCHER_MAIN_RUNNER_H_
#define DHTFETCHER_FETCHER_MAIN_RUNNER_H_

#include "base/at_exit.h"

#if defined(OS_MACOSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#endif

class CoreMainRunner;
class FetcherMainRunner {
 public:
  FetcherMainRunner();
  ~FetcherMainRunner();
  
  void SetCommandLine(int argc, const char** argv);
  
  int Initialize();
  int Run();
  void Shutdown();
  
 private:
  bool is_initialized_;
  bool is_shutdown_;
  
  scoped_ptr<CoreMainRunner> syncbox_runner_;
  scoped_ptr<base::AtExitManager> exit_manager_;
#if defined(OS_MACOSX)
  scoped_ptr<base::mac::ScopedNSAutoreleasePool> autorelease_pool_;
#endif
 
  DISALLOW_COPY_AND_ASSIGN(FetcherMainRunner);
};

#endif
