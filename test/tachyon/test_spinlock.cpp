#include <cmath>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <tachyon/spinlock.h>

struct trivially_copyable_object
{
  int i = 0;
  double d = 0.;
};

template <class Spinlock>
void test_spinlock_lock_unlock(int num_writers, int num_readers, int num_iters)
{
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

struct test_lock_unlock_params
{
  int num_writers = 1;
  int num_readers = 4;
  int num_iters = 1000000;
};

//template <class Spinlock>
class spinlock_test: public ::testing::TestWithParam<test_lock_unlock_params>
{};

/*
using spinlock_impls = ::testing::Types<
  tachyon::spinlock<tachyon::SPINLOCK_TAS>,
  tachyon::spinlock<tachyon::SPINLOCK_CAS>
  >;

TYPED_TEST_CASE(spinlock_test, spinlock_impls);
*/

TEST_P(spinlock_test, tas_lock_unlock)
{
  auto params = GetParam();
//  test_spinlock_lock_unlock<TypeParam>(params.num_writers, params.num_readers, params.num_iters);
  test_spinlock_lock_unlock<tachyon::spinlock<tachyon::SPINLOCK_TAS>>(params.num_writers, params.num_readers, params.num_iters);
}

//REGISTER_TYPED_TEST_CASE_P(spinlock_test, lock_unlock);

static std::vector<test_lock_unlock_params> test_lock_unlock_param_values{
  {1, 1,  1000000},
  {1, 4,  100000},
  {1, 8,  100000},
  {1, 16, 10000},
  {1, 32, 10000},
  {2, 1,  1000000},
  {2, 4,  100000},
  {2, 8,  100000},
  {2, 16, 10000},
  {2, 32, 10000},
  {4, 1,  100000},
  {4, 4,  100000},
  {4, 8,  100000},
  {4, 16, 10000},
  {4, 32, 10000},
};

INSTANTIATE_TEST_CASE_P(spinlock, spinlock_test, ::testing::ValuesIn(test_lock_unlock_param_values));
