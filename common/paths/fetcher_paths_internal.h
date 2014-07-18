#ifndef FETCHER_PATHS_INTERNAL_H_
#define FETCHER_PATHS_INTERNAL_H_

#include <string>

#include "build/build_config.h"

#if defined(OS_MACOSX)
#if defined(__OBJC__)
@class NSBundle;
#else
class NSBundle;
#endif
#endif

namespace base {
class FilePath;
}

namespace dhtfetcher {

// Get the path to the user's data directory, regardless of whether
// DIR_USER_DATA has been overridden by a command-line option.
bool GetDefaultUserDataDirectory(base::FilePath* result);

}

#endif
