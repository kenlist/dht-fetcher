#include "notification_registrar.h"

#include <algorithm>

#include "base/logging.h"
#include "dht-fetcher/core/notification/notification_service_impl.h"

struct NotificationRegistrar::Record {
  bool operator==(const Record& other) const;
  
  NotificationObserver* observer;
  int type;
  NotificationSource source;
};

bool NotificationRegistrar::Record::operator==(const Record& other) const {
  return observer == other.observer &&
         type == other.type &&
         source == other.source;
}

NotificationRegistrar::NotificationRegistrar() {
}

NotificationRegistrar::~NotificationRegistrar() {
  RemoveAll();
}

void NotificationRegistrar::Add(NotificationObserver* observer,
                                int type,
                                const NotificationSource& source) {
  if (CoreThread::CurrentlyOn(NotificationServiceImpl::GetNotifyThreadId())) {
    AddOnThread(observer, type, source);
  } else {
    CoreThread::PostTask(CoreThread::SYNC,
                            FROM_HERE,
                            base::Bind(&NotificationRegistrar::AddOnThread,
                                       this,
                                       observer,
                                       type,
                                       source));
  }
}

void NotificationRegistrar::Remove(NotificationObserver* observer,
                                   int type,
                                   const NotificationSource& source) {
  if (CoreThread::CurrentlyOn(NotificationServiceImpl::GetNotifyThreadId())) {
    RemoveOnThread(observer, type, source);
  } else {
    CoreThread::PostTask(CoreThread::SYNC,
                            FROM_HERE,
                            base::Bind(&NotificationRegistrar::RemoveOnThread,
                                       this,
                                       observer,
                                       type,
                                       source));
  }
}

void NotificationRegistrar::RemoveAll() {
  if (CoreThread::CurrentlyOn(NotificationServiceImpl::GetNotifyThreadId())) {
    RemoveAllOnThread();
  } else {
    CoreThread::PostTask(CoreThread::SYNC,
                            FROM_HERE,
                            base::Bind(&NotificationRegistrar::RemoveAllOnThread,
                                       this));
  }
}

void NotificationRegistrar::AddOnThread(NotificationObserver* observer,
                                        int type,
                                        NotificationSource source) {
  DCHECK(!IsRegistered(observer, type, source)) << "Duplicate registration.";
  
  Record record = { observer, type, source };
  registered_.push_back(record);
  
  NotificationServiceImpl::GetInstance()->AddObserverOnThread(
      observer, type, source);
}

void NotificationRegistrar::RemoveOnThread(NotificationObserver* observer,
                                           int type,
                                           NotificationSource source) {
  Record record = { observer, type, source };
  RecordVector::iterator found = std::find(
      registered_.begin(), registered_.end(), record);
  DCHECK(found != registered_.end());
  
  registered_.erase(found);
  
  NotificationServiceImpl::GetInstance()->RemoveObserverOnThread(
      observer, type, source);
}

void NotificationRegistrar::RemoveAllOnThread() {
  if (registered_.empty())
    return;
  
  for (size_t i = 0; i < registered_.size(); i++) {
      NotificationServiceImpl::GetInstance()->RemoveObserverOnThread(
          registered_[i].observer,
          registered_[i].type,
          registered_[i].source);
  }
  registered_.clear();
}

bool NotificationRegistrar::IsRegistered(NotificationObserver *observer,
                                         int type,
                                         const NotificationSource &source) {
  DCHECK(CoreThread::CurrentlyOn(
      NotificationServiceImpl::GetNotifyThreadId()));
  Record record = { observer, type, source };
  return std::find(registered_.begin(), registered_.end(), record) !=
      registered_.end();
}
