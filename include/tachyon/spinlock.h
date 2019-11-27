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

/**
 * LockingPolicy should defines atomic locking logic and provide the following contract.
 */
//class LockingPolicy
//{
//public:
//  bool is_locked() const;
//  bool try_lock();
//  void unlock();
//};

/**
 * Spinlock class.
 * This class can be used in combination with std::lock_guard or std::unique_lock.
 * @tparam LockingPolicy Policy that defines atomic locking logic.
 */
template <class LockingPolicy>
class spinlock: protected LockingPolicy
{
public:
  using LockingPolicy::is_locked;
  using LockingPolicy::try_lock;
  using LockingPolicy::unlock;

  ALWAYS_INLINE void lock()
  {
    while (!LockingPolicy::try_lock()); // spin until the lock is acquired
  }

  template <class Clock, class Duration>
  ALWAYS_INLINE bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
  {
    auto deadline = std::chrono::time_point_cast<typename Clock::duration, Clock, Duration>(t);
    while (!LockingPolicy::try_lock()) // spin until the lock is acquired
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
};

/**
 * Unfair spinlock policy based on atomic test-and-test operation.
 */
class SpinlockPolicyTAS
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE bool try_lock()
  {
    return !locked_.exchange(true, std::memory_order_acquire);
  }

  ALWAYS_INLINE void unlock()
  {
    locked_.store(false, std::memory_order_release);
  }

private:
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Unfair spinlock policy based on atomic compare-and-swap operation.
 */
class SpinlockPolicyCAS
{
public:
  ALWAYS_INLINE bool is_locked() const
  {
    return locked_.load(std::memory_order_consume);
  }

  ALWAYS_INLINE bool try_lock()
  {
    bool expected = false;
    return locked_.compare_exchange_strong(expected, true, std::memory_order_acquire);
  }

  ALWAYS_INLINE void unlock()
  {
    locked_.store(false, std::memory_order_release);
  }

private:
  std::atomic_bool locked_ = {false};
  static_assert(std::atomic_bool::is_always_lock_free);
};

/**
 * Fair spinlock policy based on Anderson's array-based queuing lock.
 */
class SpinlockPolicyAnderson
{
public:
//  ALWAYS_INLINE bool is_locked() const
//  {
//  }
//
//  ALWAYS_INLINE bool try_lock()
//  {
//  }
//
//  ALWAYS_INLINE void unlock()
//  {
//  }
};

} // namespace tachyon
