#include "core_main_parts.h"

#include "base/path_service.h"
#include "base/debug/trace_event.h"
#include "net/url_request/url_request.h"
#include "dht-fetcher/common/paths/fetcher_paths.h"

CoreMainParts* CoreMainParts::Create(
    const MainFunctionParams& paramters) {
  return new CoreMainParts(paramters);
}

CoreMainParts::CoreMainParts(
    const MainFunctionParams& parameters)
    : parameters_(parameters)
    , parsed_command_line_(parameters.command_line) {
}

CoreMainParts::~CoreMainParts() {
}
  
void CoreMainParts::PreEarlyInitialization() {
  TRACE_EVENT0("startup", "CoreMainParts::PreEarlyInitialization");
}

void CoreMainParts::PostEarlyInitialization() {
  TRACE_EVENT0("startup", "CoreMainParts::PostEarlyInitialization");
}

void CoreMainParts::PreMainMessageLoopStart() {
  TRACE_EVENT0("startup", "CoreMainParts::PreMainMessageLoopStart");
}

void CoreMainParts::PostMainMessageLoopStart() {
  TRACE_EVENT0("startup", "CoreMainParts::PostMainMessageLoopStart");
}

bool CoreMainParts::PreCreateThreads() {
  TRACE_EVENT0("startup", "CoreMainParts::PreCreateThreads")

  base::FilePath user_data_dir;
  if (!PathService::Get(dhtfetcher::DIR_USER_DATA, &user_data_dir))
    return false;
  
  return true;
}
