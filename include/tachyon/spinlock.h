#pragma once

#include <atomic>
#include <chrono>

#include "def.h"

namespace tachyon
{

/**
 * Spinlock classes.
 * All classes can be used in combination with std::lock_guard or std::unique_lock.
 */

/**
 * Naive spinlock that utilizes atomic test-and-set operation (unfair).
 */
class spinlock_tas
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE void lock()
  {
    while (!try_lock()); // spin until the lock is acquired
  }

  ALWAYS_INLINE void unlock()
  {
    locked_.store(false, std::memory_order_release);
  }

  ALWAYS_INLINE bool try_lock()
  {
    return !locked_.exchange(true, std::memory_order_acquire);
  }

  template <class Clock, class Duration>
  ALWAYS_INLINE bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
  {
    auto deadline = std::chrono::time_point_cast<typename Clock::duration, Clock, Duration>(t);
    while (!try_lock()) // spin until the lock is acquired
    {
      if (Clock::now() > deadline)
      {
        return false; // time is over
      }
    }
    return true;
  }

  template <class Rep, class Period>
  ALWAYS_INLINE bool try_lock_for(const std::chrono::duration<Rep, Period>& d)
  {
    return try_lock_until(std::chrono::steady_clock::now() + d);
  }

private:
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Optimized version of spinlock_tas that spins on reading the lock before calling test-and-set ([test-and-]test-and-set).
 */
class spinlock_tatas
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE void lock()
  {
    while (!try_lock()) // spin until the lock is acquired
    {
      while (locked_.load(std::memory_order_relaxed));
    }
  }

  ALWAYS_INLINE void unlock()
  {
    locked_.store(false, std::memory_order_release);
  }

  ALWAYS_INLINE bool try_lock()
  {
    return !locked_.exchange(true, std::memory_order_acquire);
  }

  template <class Clock, class Duration>
  ALWAYS_INLINE bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
  {
    auto deadline = std::chrono::time_point_cast<typename Clock::duration, Clock, Duration>(t);
    while (!try_lock()) // spin until the lock is acquired
    {
      while (locked_.load(std::memory_order_relaxed))
      {
        if (Clock::now() > deadline)
        {
          return false; // time is over
        }
      }
    }
    return true;
  }

  template <class Rep, class Period>
  ALWAYS_INLINE bool try_lock_for(const std::chrono::duration<Rep, Period>& d)
  {
    return try_lock_until(std::chrono::steady_clock::now() + d);
  }

private:
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Naive spinlock that utilizes atomic compare-and-swap operation (unfair).
 */
class spinlock_cas
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE void lock()
  {
    while (!try_lock()); // spin until the lock is acquired
  }

  ALWAYS_INLINE void unlock()
  {
    locked_.store(false, std::memory_order_release);
  }

  ALWAYS_INLINE bool try_lock()
  {
    bool expected = false;
    return locked_.compare_exchange_strong(expected, true, std::memory_order_acquire);
  }

  template <class Clock, class Duration>
  ALWAYS_INLINE bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
  {
    auto deadline = std::chrono::time_point_cast<typename Clock::duration, Clock, Duration>(t);
    while (!try_lock()) // spin until the lock is acquired
    {
      if (Clock::now() > deadline)
      {
        return false; // time is over
      }
    }
    return true;
  }

  template <class Rep, class Period>
  ALWAYS_INLINE bool try_lock_for(const std::chrono::duration<Rep, Period>& d)
  {
    return try_lock_until(std::chrono::steady_clock::now() + d);
  }

private:
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Spinlock based on Anderson's array-based queuing lock (fair).
 */
// ...

} // namespace tachyon
