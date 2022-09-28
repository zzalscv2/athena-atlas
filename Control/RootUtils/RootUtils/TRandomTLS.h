/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file RootUtils/TRandomTLS.h
 * @author Frank Winklmeier
 * @date Sept, 2022
 * @brief Thread-local TRandom generator.
 */

#ifndef ROOTUTILS_TRANDOMTLS_H
#define ROOTUTILS_TRANDOMTLS_H

#include <atomic>

#include "Rtypes.h"
#include "boost/thread/tss.hpp"

namespace RootUtils {

  /**
   * Thread-local TRandom generator.
   *
   * This class provides a thread-local TRandom instance of type T.
   * For each new instance/thread the seed is incremented to ensure
   * independent random numbers.
   */
  template <class T>
  class TRandomTLS {
  public:
    /**
     * Constructor
     *
     * @param seed The seed of the first TRandom instance. Additional instances
     *             are created with the seed incremented by one. If the seed is 0,
     *             it will not be incremented as ROOT then uses a time-based seed.
     */
    TRandomTLS(UInt_t seed = 4357) : m_seed(seed) {}

    /// Destructor
    ~TRandomTLS() = default;

    /// Get thread-specific TRandom
    T* get() const;
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

  private:
    /// Thread-local TRandom
    mutable boost::thread_specific_ptr<T> m_rand_tls;

    /// TRandom seed (incremented for each new instance/thread)
    mutable std::atomic<UInt_t> m_seed;
  };


  //
  // Inline methods
  //

  template <class T>
  inline T* TRandomTLS<T>::get() const
  {
    T* random = m_rand_tls.get();
    if (!random) {
      random = new T(m_seed > 0 ? m_seed++ : 0);
      m_rand_tls.reset(random);
    }
    return random;
  }

} // namespace RootUtils

#endif
