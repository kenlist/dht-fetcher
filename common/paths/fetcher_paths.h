#ifndef DHTFETCHER_COMMON_PATHS_CORE_PATHS_H_
#define DHTFETCHER_COMMON_PATHS_CORE_PATHS_H_

#include "build/build_config.h"

namespace dhtfetcher {

enum {
  PATH_START = 1000,

  DIR_APP = PATH_START,         // Directory where dlls and data reside.
  DIR_LOGS,                     // Directory where logs should be written.
  DIR_USER_DATA,                // Directory where user data can be written.
  
  PATH_END
};

// Call once to register the provider for the path keys defined above.
void RegisterPathProvider();

}

#endif
