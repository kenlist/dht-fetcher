#ifndef DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SOURCE_H_
#define DHTFETCHER_CORE_NOTIFICATION_NOTIFICATION_SOURCE_H_

class NotificationSource {
 public:
  NotificationSource(const NotificationSource& other) : ptr_(other.ptr_) {}
  ~NotificationSource() {}

  uintptr_t map_key() const { return reinterpret_cast<uintptr_t>(ptr_); }

  bool operator!=(const NotificationSource& other) const {
    return ptr_ != other.ptr_;
  }
  bool operator==(const NotificationSource& other) const {
    return ptr_ == other.ptr_;
  }

 protected:
  explicit NotificationSource(const void* ptr) : ptr_(ptr) {}

  const void* ptr_;
};

template <class T>
class Source : public NotificationSource {
 public:
  Source(const T* ptr) : NotificationSource(ptr) {}
  Source(const NotificationSource& other)
    : NotificationSource(other) {}

  T* operator->() const { return ptr(); }
  T* ptr() const { return static_cast<T*>(const_cast<void*>(ptr_)); }
};

#endif
