#ifndef DHTFETCHER_CORE_MAIN_FUNCTION_PARAMS_H_
#define DHTFETCHER_CORE_MAIN_FUNCTION_PARAMS_H_

#include "base/callback_forward.h"
#include "base/command_line.h"

#if defined(OS_MACOSX)
namespace base {
namespace mac {
class ScopedNSAutoreleasePool;
}
}
#endif

struct MainFunctionParams {
  explicit MainFunctionParams(const base::CommandLine& cl)
      : command_line(cl),
#if defined(OS_MACOSX)
        autorelease_pool(NULL)
#endif
  {}
  
  const base::CommandLine& command_line;
  
#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool* autorelease_pool;
#endif
};

#endif
