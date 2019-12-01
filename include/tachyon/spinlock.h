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
 * Optimized version of unfair spinlock_tas that spins on reading the lock before calling expensive test-and-set ([test-and-]test-and-set).
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
 * Optimized version of unfair spinlock_tas that uses exponential back-off while trying to acquire the lock.
 */
class spinlock_tas_eb
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE void lock()
  {
    backoff_t backoff = BACKOFF_INITIALIZER;

    while (!try_lock()) // spin until the lock is acquired
    {
      backoff_eb(backoff);
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
    backoff_t backoff = BACKOFF_INITIALIZER;
    auto deadline = std::chrono::time_point_cast<typename Clock::duration, Clock, Duration>(t);

    while (!try_lock()) // spin until the lock is acquired
    {
      if (Clock::now() > deadline)
      {
        return false; // time is over
      }

      backoff_eb(backoff);
    }

    return true;
  }

  template <class Rep, class Period>
  ALWAYS_INLINE bool try_lock_for(const std::chrono::duration<Rep, Period>& d)
  {
    return try_lock_until(std::chrono::steady_clock::now() + d);
  }

private:
    using backoff_t = unsigned int;

    /**
     * Exponential back-off implementation.
     */
    ALWAYS_INLINE static void backoff_eb(backoff_t& c)
    {
      backoff_t ceiling = c;
      for (backoff_t i = 0; i < ceiling; i++)
      {
        std::atomic_thread_fence(std::memory_order_acq_rel);
        //__asm__ __volatile__("" ::: "memory");
      }

      c = ceiling <<= ceiling < BACKOFF_CEILING;
      return;
    }

private:
  static const backoff_t BACKOFF_INITIALIZER = (1 << 9);
  static const backoff_t BACKOFF_CEILING = ((1 << 20) - 1);

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
