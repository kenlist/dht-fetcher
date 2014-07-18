#ifndef DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_OBSERVER_H_
#define DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_OBSERVER_H_

class NotificationDetails;
class NotificationSource;

class NotificationObserver {
  public:
    // Observer callback.
    virtual void Observe(int type,
                         const NotificationSource& source,
                         const NotificationDetails& details) = 0;
  
  protected:
    virtual ~NotificationObserver() {}
};

#endif
