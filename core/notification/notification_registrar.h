#ifndef DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_REGISTRAR_H_
#define DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_REGISTRAR_H_

#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "dht-fetcher/core/notification/notification_service.h"

class NotificationObserver;
class NotificationSource;

class NotificationRegistrar
    : public base::RefCountedThreadSafe<NotificationRegistrar> {
  friend class base::RefCountedThreadSafe<NotificationRegistrar>;
 public:
  NotificationRegistrar();
    
  void Add(NotificationObserver* observer,
           int type,
           const NotificationSource& source = NotificationService::AllSources());
  void Remove(NotificationObserver* observer,
              int type,
              const NotificationSource& source = NotificationService::AllSources());
  
  // Unregisters all notifications.
  void RemoveAll();

 private:
  void AddOnThread(NotificationObserver* observer,
                   int type,
                   NotificationSource source);
  void RemoveOnThread(NotificationObserver* observer,
                      int type,
                      NotificationSource source);
  void RemoveAllOnThread();

  bool IsRegistered(NotificationObserver *observer,
                    int type,
                    const NotificationSource &source);
 protected:
  virtual ~NotificationRegistrar();

 private:
  struct Record;
  typedef std::vector<Record> RecordVector;
  
  // Lists for search in wrapper.
  RecordVector registered_;
  
  DISALLOW_COPY_AND_ASSIGN(NotificationRegistrar);
};

#endif
