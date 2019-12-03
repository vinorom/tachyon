#include <benchmark/benchmark.h>

void bench_spinlock()
{}

static void BM_spinlock(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    bench_spinlock();
  }
}
// Register the function as a benchmark
BENCHMARK(BM_spinlock);
// Run the benchmark
BENCHMARK_MAIN();