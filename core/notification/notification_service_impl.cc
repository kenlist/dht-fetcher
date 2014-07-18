#include "notification_service_impl.h"

#include "base/threading/thread_local.h"
#include "dht-fetcher/core/notification/notification_observer.h"
#include "dht-fetcher/common/fetcher_notification_types.h"

#define NOTIFY_THREAD_ID CoreThread::SYNC

NotificationService* NotificationService::GetInstance() {
  return NotificationServiceImpl::GetInstance();
}

NotificationServiceImpl* NotificationServiceImpl::GetInstance() {
  return Singleton<NotificationServiceImpl>::get();
}

bool NotificationServiceImpl::HasKey(const NotificationSourceMap &map,
                                     const NotificationSource &source) {
  return map.find(source.map_key()) != map.end();
}

NotificationServiceImpl::NotificationServiceImpl() {
}

CoreThread::ID NotificationServiceImpl::GetNotifyThreadId() {
  return NOTIFY_THREAD_ID;
}

void NotificationServiceImpl::AddObserverOnThread(NotificationObserver* observer,
                                                  int type,
                                                  const NotificationSource &source) {
  DCHECK(CoreThread::CurrentlyOn(NOTIFY_THREAD_ID));
  CHECK(observer);
  
  NotificationObserverList* observer_list;
  if (HasKey(observers_[type], source)) {
    observer_list = observers_[type][source.map_key()];
  } else {
    observer_list = new NotificationObserverList;
    observers_[type][source.map_key()] = observer_list;
  }
  
  observer_list->AddObserver(observer);
#ifndef NDEBUG
  ++observer_counts_[type];
#endif
}

void NotificationServiceImpl::RemoveObserverOnThread(NotificationObserver *observer,
                                             int type,
                                             const NotificationSource &source) {
  DCHECK(CoreThread::CurrentlyOn(NOTIFY_THREAD_ID));
  CHECK(HasKey(observers_[type], source));
  
  NotificationObserverList* observer_list = observers_[type][source.map_key()];
  if (observer_list) {
    observer_list->RemoveObserver(observer);
    if (!observer_list->might_have_observers()) {
      observers_[type].erase(source.map_key());
      delete observer_list;
    }
#ifndef NDEBUG
    --observer_counts_[type];
#endif
  }
}

void NotificationServiceImpl::NotifyOnThread(int type,
                                             NotificationSource source,
                                             NotificationDetails details) {
  DCHECK(CoreThread::CurrentlyOn(NOTIFY_THREAD_ID));
  CHECK_GT(type, NOTIFICATION_ALL) <<
      "Allowed for observing, but not posting.";
  
  // Notify observers of all types and all sources
  if (HasKey(observers_[NOTIFICATION_ALL], AllSources()) &&
      source != AllSources()) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[NOTIFICATION_ALL][source.map_key()],
                      Observe(type, source, details));
  }
  
  // Notify observers of all types and the give source
  if (HasKey(observers_[NOTIFICATION_ALL], source)) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[type][source.map_key()],
                      Observe(type, source, details));
  }
  
  // Notify observers of the given type and all sources
  if (HasKey(observers_[type], AllSources()) &&
      source != AllSources()) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[type][AllSources().map_key()],
                      Observe(type, source, details));
  }
  
  // Notifiy observers of the given type and the given source
  if (HasKey(observers_[type], source)) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[type][source.map_key()],
                      Observe(type, source, details));
  }
}

void NotificationServiceImpl::Notify(int type,
                                     const NotificationSource &source,
                                     const NotificationDetails &details) {
  if (CoreThread::CurrentlyOn(NOTIFY_THREAD_ID)) {
    NotifyOnThread(type, source, details);
  } else {
    CoreThread::PostTask(
        CoreThread::SYNC,
        FROM_HERE,
        base::Bind(&NotificationServiceImpl::NotifyOnThread,
                   base::Unretained(this),
                   type,
                   source,
                   details));
  }
}

NotificationServiceImpl::~NotificationServiceImpl() {
#ifndef NDEBUG
  for (int i = 0; i < static_cast<int>(observer_counts_.size()); i++) {
    if (observer_counts_[i] > 0) {
      VLOG(1) << observer_counts_[i] << " notification observer(s) leaked "
                 "of notification type " << i;
    }
  }
#endif

  for (int i = 0; i < static_cast<int>(observers_.size()); i++) {
    NotificationSourceMap omap = observers_[i];
    for (NotificationSourceMap::iterator it = omap.begin();
         it != omap.end(); ++it) {
      delete it->second;
    }
  }
}



