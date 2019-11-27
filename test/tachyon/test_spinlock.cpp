#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <tachyon/spinlock.h>

template <class Spinlock>
class spinlock_test: public ::testing::Test
{};

using spinlock_impls = ::testing::Types<
  tachyon::spinlock<tachyon::SpinlockPolicyTAS>,
  tachyon::spinlock<tachyon::SpinlockPolicyCAS>
>;

TYPED_TEST_SUITE(spinlock_test, spinlock_impls);

template <class Spinlock>
void test_spinlock_thread_safety(int num_writers, int num_readers, int num_iters)
{
  struct trivially_copyable_object
  {
      int i = 0;
      double d = 0.;
  };

  Spinlock lock;
  trivially_copyable_object shared_value;

  auto write_routine = [&](int writer_idx, int num_iters)
  {
    for (int start = writer_idx * num_iters, end = start + num_iters, i = start ; i < end; ++i)
    {
      std::lock_guard<Spinlock> guard(lock); // guard
      shared_value = {i, static_cast<double>(i)};
    }
  };

  auto read_routine = [&](int num_iters)
  {
      trivially_copyable_object value;
      for (int i = 0; i < num_iters; ++i)
      {
        lock.lock();   // lock
        value = shared_value;
        lock.unlock(); // unlock

        EXPECT_DOUBLE_EQ(value.d, value.i);
      }
  };

  std::thread writers[num_writers];
  for (int i = 0; i < num_writers; ++i)
  {
    writers[i] = std::thread(write_routine, i, num_iters);
  }

  std::thread readers[num_readers];
  for (int i = 0; i < num_readers; ++i)
  {
    readers[i] = std::thread(read_routine, num_iters);
  }

  for (auto& writer: writers)
  {
    writer.join();
  }

  for (auto& reader: readers)
  {
    reader.join();
  }
}

TYPED_TEST(spinlock_test, thread_safety)
{
  for (int num_writers = 1; num_writers <= 4; num_writers <<= 1)
  {
    for (int num_readers = 1; num_readers <= std::thread::hardware_concurrency(); num_readers <<= 1)
    {
      test_spinlock_thread_safety<TypeParam>(num_writers, num_readers, 1000000 / num_readers);
    }
  }
}

TYPED_TEST(spinlock_test, is_locked)
{
  TypeParam lock;
  ASSERT_FALSE(lock.is_locked());

  {
    std::lock_guard<TypeParam> guard(lock);
    ASSERT_TRUE(lock.is_locked());

    std::thread t(
      [&lock]() {
        ASSERT_TRUE(lock.is_locked());
      });

    t.join();
  }

  ASSERT_FALSE(lock.is_locked());
}

TYPED_TEST(spinlock_test, lock_unlock)
{
  TypeParam lock;
  ASSERT_FALSE(lock.is_locked());
  lock.lock();
  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock());

  std::thread t(
    [&lock]() {
      ASSERT_FALSE(lock.try_lock());
      ASSERT_TRUE(lock.is_locked());
    });

  t.join();

  lock.unlock();
  ASSERT_FALSE(lock.is_locked());
}

TYPED_TEST(spinlock_test, try_lock)
{
  TypeParam lock;
  ASSERT_FALSE(lock.is_locked());
  ASSERT_TRUE(lock.try_lock());
  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock());

  std::thread t(
    [&lock]() {
      ASSERT_FALSE(lock.try_lock());
      ASSERT_TRUE(lock.is_locked());
    });

  t.join();

  lock.unlock();
  ASSERT_FALSE(lock.is_locked());
  ASSERT_TRUE(lock.try_lock());
  ASSERT_TRUE(lock.is_locked());
}

TYPED_TEST(spinlock_test, try_lock_until)
{
  TypeParam lock;
  ASSERT_FALSE(lock.is_locked());
  ASSERT_TRUE(lock.try_lock_until(std::chrono::steady_clock::now()));
  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));

  std::thread t1(
    [&lock]() {
      ASSERT_FALSE(lock.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
      ASSERT_TRUE(lock.is_locked());
    });

  t1.join();

  std::thread t2(
    [&lock]() {
      ASSERT_TRUE(lock.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
      ASSERT_TRUE(lock.is_locked());
    });

  std::this_thread::sleep_for(std::chrono::nanoseconds(300));
  lock.unlock();
  t2.join();

  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  ASSERT_TRUE(lock.is_locked());
}

TYPED_TEST(spinlock_test, try_lock_for)
{
  TypeParam lock;
  ASSERT_FALSE(lock.is_locked());
  ASSERT_TRUE(lock.try_lock_for(std::chrono::milliseconds(1)));
  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock_for(std::chrono::milliseconds(1)));

  std::thread t1(
    [&lock]() {
      ASSERT_FALSE(lock.try_lock_for(std::chrono::milliseconds(1)));
      ASSERT_TRUE(lock.is_locked());
    });

  t1.join();

  std::thread t2(
    [&lock]() {
      ASSERT_TRUE(lock.try_lock_for(std::chrono::milliseconds(1)));
      ASSERT_TRUE(lock.is_locked());
    });

  std::this_thread::sleep_for(std::chrono::nanoseconds(300));
  lock.unlock();
  t2.join();

  ASSERT_TRUE(lock.is_locked());
  ASSERT_FALSE(lock.try_lock_for(std::chrono::milliseconds(1)));
  ASSERT_TRUE(lock.is_locked());
}
