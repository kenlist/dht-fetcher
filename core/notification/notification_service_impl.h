#ifndef DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SERVICE_IMPL_H_
#define DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SERVICE_IMPL_H_

#include <map>

#include "base/memory/singleton.h"
#include "base/observer_list.h"
#include "dht-fetcher/core/notification/notification_service.h"
#include "dht-fetcher/core/thread/core_thread.h"

class NotificationObserver;
class NotificationRegistrar;

class NotificationServiceImpl : public NotificationService {
 public:
  static NotificationServiceImpl* GetInstance();
  
  // Normally instantiated when the thread is created. Not all threads have
  // a NotificationService. Only one instance should be created per thread.
  NotificationServiceImpl();
  virtual ~NotificationServiceImpl();

  virtual void Notify(int type,
                      const NotificationSource& source,
                      const NotificationDetails& details) OVERRIDE;

 private:
  friend class NotificationRegistrar;
  friend struct DefaultSingletonTraits<NotificationServiceImpl>;

  typedef ObserverList<NotificationObserver> NotificationObserverList;
  typedef std::map<uintptr_t, NotificationObserverList*> NotificationSourceMap;
  typedef std::map<int, NotificationSourceMap> NotificationObserverMap;
  typedef std::map<int, int> NotificationObserverCount;

  // Convenience function to determine whether a source has a
  // NotificationOserverList in the given map.
  static bool HasKey(const NotificationSourceMap& map,
                     const NotificationSource& source);

  void AddObserverOnThread(NotificationObserver* observer,
                           int type,
                           const NotificationSource& source);

  void RemoveObserverOnThread(NotificationObserver* observer,
                              int type,
                              const NotificationSource& source);

  void NotifyOnThread(int type,
                      NotificationSource source,
                      NotificationDetails details);

  static CoreThread::ID GetNotifyThreadId();

 private:
  NotificationObserverMap observers_;

#ifndef NDEBUG
  // Used to check to see that AddObserver and RemoveObserver calls are
  // balanced.
  NotificationObserverCount observer_counts_;
#endif

  DISALLOW_COPY_AND_ASSIGN(NotificationServiceImpl);
};

#endif
