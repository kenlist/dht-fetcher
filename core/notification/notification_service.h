#ifndef DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SERVICE_H_
#define DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SERVICE_H_

#include "notification_details.h"
#include "notification_source.h"

class NotificationService {
 public:
  static NotificationService* GetInstance();

  virtual ~NotificationService() {}

  // Synchronously posts a notification to all interested observers.
  virtual void Notify(int type,
                      const NotificationSource& source = AllSources(),
                      const NotificationDetails& details = NoDetails()) = 0;

  // Returns a NotificationSource that represents all notification sources.
  static Source<void> AllSources() { return Source<void>(NULL); }

  // Returns a NotificationDetails object that represents a lack of details
  // associated with a notification. (This is effectively a null pointer.)
  static Details<void> NoDetails() { return Details<void>(NULL); }
};

#endif
