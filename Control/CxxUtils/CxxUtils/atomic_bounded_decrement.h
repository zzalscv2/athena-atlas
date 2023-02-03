// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/atomic_bounded_decrement.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Atomically decrement a value with a lower bound.
 */


#ifndef CXXUTILS_ATOMIC_BOUNDED_DECREMENT_H
#define CXXUTILS_ATOMIC_BOUNDED_DECREMENT_H


#include "CxxUtils/stall.h"
#include <atomic>


namespace CxxUtils {


/**
 * @brief Atomically decrement a value with a lower bound.
 * @param a Pointer to the atomic value.
 * @param bound Lower bound for a.
 * @param decr The amount by which to decrement.
 * @param memorder Memory ordering.
 *
 * Atomically decrement A by DECR, provided the result is not less than BOUND.
 * I.e., the atomic equivalent of:
 *@code
   if (a >= bound + decr) a -= decr;
 @endcode
 *
 * Returns in any case the original value of the atomic variable.
 */
template <class T>
inline
T atomic_bounded_decrement (std::atomic<T>* a, T bound = 0, T decr = 1,
                            std::memory_order memorder = std::memory_order_seq_cst)
{
  T orig = a->load (memorder);
  // Be careful with the comparison here --- it needs to work for
  // unsigned values, so we don't want to subtract decr from orig.
  while (orig >= bound + decr &&
         !a->compare_exchange_strong (orig, orig - decr, memorder)) {
    CxxUtils::stall();
  }
  return orig;
}


} // namespace CxxUtils


#endif // not CXXUTILS_ATOMIC_BOUNDED_DECREMENT_H
