#include "core_sub_thread.h"

#include "base/threading/thread_restrictions.h"
#include "net/url_request/url_fetcher.h"

CoreSubThread::CoreSubThread(CoreThread::ID identifier)
    : CoreThreadImpl(identifier) {
}

CoreSubThread::~CoreSubThread() {
  Stop();
}

void CoreSubThread::Init() {
  CoreThreadImpl::Init();
  
  if (CoreThread::CurrentlyOn(CoreThread::IO)) {
    // Though this thread is called the "IO" thread, it actually just routes
    // messages around; it shouldn't be allowed to perform any blocking disk
    // I/O.
    base::ThreadRestrictions::SetIOAllowed(false);
    base::ThreadRestrictions::DisallowWaiting();
  }
}

void CoreSubThread::CleanUp() {
  if (CoreThread::CurrentlyOn(CoreThread::IO))
    IOThreadPreCleanUp();
  
  CoreThreadImpl::CleanUp();
}

void CoreSubThread::IOThreadPreCleanUp() {
  // Kill all things that might be holding onto
  // net::URLRequest/net::URLRequestContexts.

  // Destroy all URLRequests started by URLFetchers.
  net::URLFetcher::CancelAll();
}
