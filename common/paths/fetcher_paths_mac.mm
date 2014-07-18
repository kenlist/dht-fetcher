#include "fetcher_paths_internal.h"

#import <Foundation/Foundation.h>
#include <string.h>

#include <string>

#include "base/base_paths.h"
#include "base/logging.h"
#import "base/mac/foundation_util.h"
#import "base/mac/scoped_nsautorelease_pool.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "chrome/common/chrome_constants.h"

#if !defined(OS_IOS)
#import "base/mac/mac_util.h"
#endif

namespace {

bool GetDefaultUserDataDirectoryForProduct(const std::string& product_dir,
                                           base::FilePath* result) {
  bool success = false;
  if (result && PathService::Get(base::DIR_APP_DATA, result)) {
    *result = result->Append(product_dir);
    success = true;
  }
  return success;
}

}  // namespace

namespace dhtfetcher {

bool GetDefaultUserDataDirectory(base::FilePath* result) {
  return GetDefaultUserDataDirectoryForProduct("dht-fetcher", result);
}

}
