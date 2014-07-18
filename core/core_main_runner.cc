#include "core_main_runner.h"

#include "base/debug/trace_event.h"
#include "base/run_loop.h"
#include "dht-fetcher/core/notification/notification_service_impl.h"
#include "dht-fetcher/core/core_main_loop.h"

class CoreMainRunnerImpl : public CoreMainRunner {
 public:
  CoreMainRunnerImpl()
      : initialization_started_(false)
      , is_shutdown_(false) {}
  
  virtual ~CoreMainRunnerImpl() {
    if (initialization_started_ && !is_shutdown_)
      Shutdown();
  }
  
  virtual bool Initialize(const MainFunctionParams& parameters) OVERRIDE {
    TRACE_EVENT0("startup", "CoreMainRunnerImpl::Initialize");
    
    // On Android we normally initialize the syncbox in a series of UI thread
    // tasks. While this is happening a second request can come from the OS or
    // another application to start the syncbox. If this happens then we must
    // not run these parts of initialization twice.
    if (!initialization_started_) {
      initialization_started_ = true;
      
      notification_service_.reset(new NotificationServiceImpl);

      main_loop_.reset(new CoreMainLoop(parameters));

      main_loop_->Init();

      main_loop_->EarlyInitialization();

      main_loop_->MainMessageLoopStart();
    }
    
    main_loop_->CreateStartupTasks();
    
    return main_loop_->is_succeed();
  }

  virtual bool Run() OVERRIDE {
    DCHECK(initialization_started_);
    DCHECK(!is_shutdown_);
    
    if (main_loop_->is_succeed()) {
      base::RunLoop run_loop;
      run_loop.Run();
      return true;
    }
    return false;
  }
  
  virtual void Shutdown() OVERRIDE {
    DCHECK(initialization_started_);
    DCHECK(!is_shutdown_);
  }

 protected:
  bool initialization_started_;
  bool is_shutdown_;
  
  scoped_ptr<NotificationServiceImpl> notification_service_;
  scoped_ptr<CoreMainLoop> main_loop_;
  
  DISALLOW_COPY_AND_ASSIGN(CoreMainRunnerImpl);
};

// static
CoreMainRunner* CoreMainRunner::Create() {
  return new CoreMainRunnerImpl();
}
