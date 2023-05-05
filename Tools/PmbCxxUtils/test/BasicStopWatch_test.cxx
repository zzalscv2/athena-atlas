/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "PmbCxxUtils/BasicStopWatch.h"

int main() {
  // Map to hold the results
  PMonUtils::BasicStopWatchResultMap_t results;

  // First code block
  {
    PMonUtils::BasicStopWatch Benchmark("FOO", results);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Second code block
  {
    PMonUtils::BasicStopWatch Benchmark("BAR", results);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Third code block
  {
    PMonUtils::BasicStopWatch Benchmark("FOO", results);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Print some  information and perform the sanity checks
  for (const auto& [key, value] : results) {
    std::cout << key << " " << value << " ms " << std::endl;
    if (key == "FOO") {
      assert(std::abs(value - 2000.) < 10);
    }
    else if (key == "BAR") {
      assert(std::abs(value - 1000.) < 10);
    }
  }

  return 0;
}
