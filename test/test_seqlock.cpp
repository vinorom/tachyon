#include <tachyon/seqlock.h>

#include <thread>
#include <cmath>
#include <iostream>

struct PODObject
{
    int i;
    double d;
};

int main()
{
  static const int num_writers = 1;
  static const int num_readers = 1;
  static const int num_iters = 1;

  auto write_routine = [](int start)
  {
    tachyon::seqlock<PODObject> lock;

    for (int i = start; i < start + num_iters; ++i)
    {
      PODObject value{i, static_cast<double>(i)};
      std::cout << "{" << value.i << "," << value.d << "}\n";
      lock.write(value);
    }
  };

  PODObject read_values[num_readers][num_iters];
  auto read_routine = [&read_values](int reader_idx)
  {
    tachyon::seqlock<PODObject> lock;

    for (int i = 0; i < num_iters; ++i)
    {
      auto value = lock.read();
      read_values[reader_idx][i] = value;
    }
  };

  std::thread writers[num_writers];
  for (int i = 0; i < num_writers; ++i)
  {
    writers[i] = std::thread(write_routine, i);
  }

  std::thread readers[num_readers];
  for (int i = 0; i < num_readers; ++i)
  {
    readers[i] = std::thread(read_routine, i);
  }

  for (auto& writer: writers)
  {
    writer.join();
  }
  for (auto& reader: readers)
  {
    reader.join();
  }

  for (int i = 0; i < num_readers; ++i)
  {
    for (int j = 0; j < num_iters; ++j)
    {
      auto value = read_values[i][j];
      auto diff = std::fabs(static_cast<double>(value.i) - value.d);
      if (diff > 1.e-5)
      {
        std::cout << "assert: {" << value.i << ", " << value.d << "}" << std::endl;
      }
    }
  }

  return 0;
}