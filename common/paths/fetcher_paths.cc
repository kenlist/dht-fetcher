#include "fetcher_paths.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/threading/thread_restrictions.h"

#if defined(OS_MACOSX)
#include "base/mac/bundle_locations.h"
#include "base/mac/foundation_util.h"
#endif

#include "dht-fetcher/common/paths/fetcher_paths_internal.h"
#include "dht-fetcher/common/fetcher_constants.h"

namespace dhtfetcher {
  
bool PathProvider(int key, base::FilePath* result) {
  // Some keys are just aliases...
  switch (key) {
    case dhtfetcher::DIR_APP:
      return PathService::Get(base::DIR_MODULE, result);
    case dhtfetcher::DIR_LOGS:
#ifdef NDEBUG
      // Release builds write to the data dir
      return PathService::Get(kanbox::DIR_USER_DATA, result);
#else
      // Debug builds write next to the binary (in the build tree)
#if defined(OS_MACOSX)
      if (!PathService::Get(base::DIR_EXE, result))
        return false;
      if (base::mac::AmIBundled()) {
        // exe_dir gave us .../Kanbox.app/Contents/MacOS/Chromium.
        *result = result->DirName();
        *result = result->DirName();
        *result = result->DirName();
      }
      return true;
#else
      return PathService::Get(base::DIR_EXE, result);
#endif  // defined(OS_MACOSX)
#endif  // NDEBUG
  }
  
  // Assume that we will not need to create the directory if it does not exist.
  // This flag can be set to true for the cases where we want to create it.
  bool create_dir = false;

  base::FilePath cur;
  switch (key) {
    case dhtfetcher::DIR_USER_DATA:
      if (!GetDefaultUserDataDirectory(&cur)) {
        NOTREACHED();
        return false;
      }
      create_dir = true;
      break;
    default:
      return false;
  }

  // TODO(bauerb): http://crbug.com/259796
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  if (create_dir && !base::PathExists(cur) &&
      !base::CreateDirectory(cur))
    return false;

  *result = cur;
  return true;
}

// This cannot be done as a static initializer sadly since Visual Studio will
// eliminate this object file if there is no direct entry point into it.
void RegisterPathProvider() {
  PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
}
  
}
