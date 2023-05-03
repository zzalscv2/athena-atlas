/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PMONUTILS_BASICSTOPWATCH_H
#define PMONUTILS_BASICSTOPWATCH_H

#include <chrono>
#include <string>

#include "tbb/concurrent_hash_map.h"

namespace PMonUtils {

  typedef tbb::concurrent_hash_map<std::string, double> BasicStopWatchResultMap_t;

  class BasicStopWatch {

    public:
      // Constructor
      BasicStopWatch(const std::string& name, BasicStopWatchResultMap_t& result) :
        m_name(name), m_result(result), m_start(std::chrono::steady_clock::now()) { }

      // Destructor
      ~BasicStopWatch() {
        std::chrono::duration<double, std::milli> total = std::chrono::steady_clock::now() - m_start;
        tbb::concurrent_hash_map<std::string, double>::accessor acc;
        m_result.insert(acc, m_name);
        acc->second += total.count();
        acc.release();
      }

    private:
      // Name of the current StopWatch
      std::string m_name;

      // Reference to the BasicStopWatchResultMap_t
      BasicStopWatchResultMap_t& m_result;

      // Start time of the current StopWatch
      std::chrono::time_point<std::chrono::steady_clock> m_start;

  }; // end class BasicStopWatch

} // end namespace PMonUtils

#endif
