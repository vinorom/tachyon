#pragma once

#include <atomic>

namespace tachyon
{

class seqlock1
{
public:
    seqlock1(): seq_(0) {}

protected:
    using atomic_counter_t = std::atomic<unsigned long>;
    static_assert(atomic_counter_t::is_always_lock_free);

    atomic_counter_t seq_;
};


template <typename T>
class seqlock
{
public:
    seqlock(): seq_(0) {}

    void write(const T& value)
    {
      unsigned long seq0 = seq_;
      while ((seq0 & 1) || !seq_.compare_exchange_weak(seq0, seq0 + 1))
      {
        seq0 = seq_;
      }
      value_ = value;
      seq_ = seq0 + 2;
    }

    T read() const
    {
      unsigned long seq0, seq1;
      T value;
      do
      {
        seq0 = seq_;
        value = value_;
        seq1 = seq_;
      }
      while (seq0 != seq1 || (seq0 & 1));
      return value;
    }

protected:
    using atomic_counter_t = std::atomic<unsigned long>;
    static_assert(atomic_counter_t::is_always_lock_free);

    std::atomic<T> value_;
    atomic_counter_t seq_;
};

} // namespace tachyon
