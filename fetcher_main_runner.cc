#include "fetcher_main_runner.h"

#include "base/command_line.h"
#include "base/process/memory.h"
#include "base/debug/trace_event.h"
#include "base/i18n/icu_util.h"
#include "dht-fetcher/common/fetcher_switches.h"
#include "dht-fetcher/common/paths/fetcher_paths.h"
#include "dht-fetcher/core/main_function_params.h"
#include "dht-fetcher/core/core_main_runner.h"

FetcherMainRunner::FetcherMainRunner()
    : is_initialized_(false),
      is_shutdown_(false) {
}

FetcherMainRunner::~FetcherMainRunner() {
  if (is_initialized_ && !is_shutdown_)
    Shutdown();
}

void FetcherMainRunner::SetCommandLine(int argc, const char** argv) {
  CommandLine::Init(argc, argv);
}

int FetcherMainRunner::Initialize() {

#if defined(OS_ANDROID)
    // See note at the initialization of ExitManager, below; basically,
    // only Android builds have the ctor/dtor handlers set up to use
    // TRACE_EVENT right away.
    TRACE_EVENT0("startup", "ContentMainRunnerImpl::Initialize");
#endif

  is_initialized_ = true;
  base::EnableTerminationOnHeapCorruption();
  base::EnableTerminationOnOutOfMemory();
  
#if defined(OS_MAXOSX)
  // We need this pool for all the objects created before we get to the
  // event loop, but we don't want to leave them hanging around until the
  // app quits. Each "main" needs to flush this pool right before it goes into
  // its main event loop to get rid of the cruft.
  autorelease_pool_.reset(new base::mac::ScopedNSAutoreleasePool());
#endif

  // Enable startup tracing asap to avoid early TRACE_EVENT calls being
  // ignored.
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kTraceStartup)) {
    base::debug::CategoryFilter category_filter(
        command_line.GetSwitchValueASCII(switches::kTraceStartup));
    base::debug::TraceLog::GetInstance()->SetEnabled(
        category_filter,
        base::debug::TraceLog::RECORDING_MODE,
        base::debug::TraceLog::RECORD_UNTIL_FULL);
  }

#if !defined(OS_ANDROID)
    // Android tracing started at the beginning of the method.
    // Other OSes have to wait till we get here in order for all the memory
    // management setup to be completed.
    TRACE_EVENT0("startup", "ContentMainRunnerImpl::Initialize");
#endif // !OS_ANDROID

#if defined(USE_NSS)
    crypto::EarlySetupForNSSInit();
#endif
  
   dhtfetcher::RegisterPathProvider();
  
#if !defined(UNIT_TEST)
   CHECK(base::i18n::InitializeICU());
#endif
  
  MainFunctionParams main_params(command_line);
#if defined(OS_MACOSX)
  main_params.autorelease_pool = autorelease_pool_.get();
#endif
  if (!syncbox_runner_.get()) {
    syncbox_runner_.reset(CoreMainRunner::Create());
  }
  return syncbox_runner_->Initialize(main_params);
}

int FetcherMainRunner::Run() {
  DCHECK(is_initialized_);
  DCHECK(!is_shutdown_);
  
  return syncbox_runner_->Run();
}

void FetcherMainRunner::Shutdown() {
  DCHECK(is_initialized_);
  DCHECK(!is_shutdown_);
  
#if defined(OS_MACOSX)
  autorelease_pool_.reset(NULL);
#endif

  exit_manager_.reset(NULL);
  is_shutdown_ = true;
}
