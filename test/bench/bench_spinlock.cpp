#include <thread>

#include <benchmark/benchmark.h>

#include <tachyon/spinlock.h>

static const auto MAX_NUM_THREADS = std::thread::hardware_concurrency();
static const auto MIN_TIME = 10; // in seconds

template <class Spinlock>
class bench_spinlock: public benchmark::Fixture
{
protected:
  Spinlock lock;
};

#define DEFINE_AND_REGISTER_LOCK_RATE_BM(BaseClass, Method, LockType, ...)                                    \
BENCHMARK_TEMPLATE_DEFINE_F(BaseClass, Method, LockType)(benchmark::State& state)                             \
{                                                                                                             \
  for (auto _ : state)                                                                                        \
  {                                                                                                           \
    std::lock_guard<LockType> guard(lock);                                                                    \
  }                                                                                                           \
  state.SetItemsProcessed(state.iterations());                                                                \
}                                                                                                             \
BENCHMARK_REGISTER_F(BaseClass, Method)->ThreadRange(1, MAX_NUM_THREADS)->MinTime(MIN_TIME)->UseRealTime();   \
BENCHMARK_REGISTER_F(BaseClass, Method)->ThreadRange(1, MAX_NUM_THREADS)->MinTime(MIN_TIME);

DEFINE_AND_REGISTER_LOCK_RATE_BM(bench_spinlock, rate_tas, tachyon::spinlock_tas);
DEFINE_AND_REGISTER_LOCK_RATE_BM(bench_spinlock, rate_tatas, tachyon::spinlock_tatas);
DEFINE_AND_REGISTER_LOCK_RATE_BM(bench_spinlock, rate_tas_eb, tachyon::spinlock_tas_eb);
DEFINE_AND_REGISTER_LOCK_RATE_BM(bench_spinlock, rate_cas, tachyon::spinlock_cas);

#undef DEFINE_AND_REGISTER_LOCK_RATE_BM

BENCHMARK_MAIN();