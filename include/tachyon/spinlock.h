/**
 * https://en.wikipedia.org/wiki/Spinlock
 * todo try_lock, try_lock_until, try_lock_for: http://austria.sourceforge.net/dox/html/classSpinLock.html
 * https://geidav.wordpress.com/2016/12/03/scalable-spinlocks-1-array-based/
 * https://github.com/geidav/spinlocks-bench
 * http://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf
 *
 * @note Most programs should use mutexes instead of spin locks.
 *       Spin locks are primarily useful in conjunction with real-time scheduling policies.
 */

#pragma once

#include <atomic>
#include <chrono>

#include "def.h"

namespace tachyon
{

enum spinlock_type
{
  SPINLOCK_TAS,       // test-and-set
  SPINLOCK_CAS,       // compare-and-swap
  SPINLOCK_ANDERSON,
//  SPINLOCK_TBB,
};

/**
 * Spinlock class.
 * Specializations of this template class should provide the following contract.
 * This class can be used in combination with std::lock_guard or std::unique_lock.
 */
template<spinlock_type Type>
class spinlock
{
  //void lock() {}

  //void unlock() {}

  //bool try_lock() {}

  //template <class Clock, class Duration>
  //bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t) {}

  //template <class Rep, class Period>
  //bool try_lock_for(const std::chrono::duration<Rep, Period>& d) {}

  //bool is_locked() const {}
};

/**
 * Specialization of spinlock based on test-and-test atomic operation.
 */
template<>
class spinlock<SPINLOCK_TAS>
{
public:
  ALWAYS_INLINE void lock()
  {
    while (locked_.exchange(true, std::memory_order_acquire)); // spin until the lock is acquired
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
    auto deadline = std::chrono::time_point_cast<Clock::duration, Clock, Duration>(t);
    while (locked_.exchange(true, std::memory_order_acquire)) // spin until the lock is acquired
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

  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

private:
//    std::atomic_flag locked_ = ATOMIC_FLAG_INIT;
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Specialization of spinlock based on Anderson's array-based queuing lock.
 */
template<>
class spinlock<SPINLOCK_ANDERSON>
{
public:
  ALWAYS_INLINE void lock()
  {
  }

  ALWAYS_INLINE void unlock()
  {
  }

//  ALWAYS_INLINE bool try_lock()
//  {
//  }
//
//  template <class Clock, class Duration>
//  ALWAYS_INLINE bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
//  {
//  }
//
//  template <class Rep, class Period>
//  ALWAYS_INLINE bool try_lock_for(const std::chrono::duration<Rep, Period>& d)
//  {
//  }
//
//  ALWAYS_INLINE bool is_locked() const
//  {
//  }

private:
};

} // namespace tachyon
