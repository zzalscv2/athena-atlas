/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKEVENTPRIMITIVES_TRKOBJECTCOUNTER_H
#define TRKEVENTPRIMITIVES_TRKOBJECTCOUNTER_H
#include <atomic>
#include <cstddef>
namespace Trk {
/**
    @class ObjectCounter
    @brief Helper to enable counting number of instantiations
    in debug builds.
    @author Christos Anastopoulos.
*/
template<typename T>
class ObjectCounter
{

public:
#ifndef NDEBUG
  static inline std::atomic_size_t s_numberOfInstantiations = 0;
#endif

  static std::size_t numberOfInstantiations()
  {
#ifndef NDEBUG
    return s_numberOfInstantiations.load();
#endif
    return 0;
  }

protected:
// cases where we want different behaviour for release/debug
#ifndef NDEBUG
  ObjectCounter()
  {
    s_numberOfInstantiations.fetch_add(1, std::memory_order_relaxed);
  }
  ObjectCounter(const ObjectCounter&)
  {
    s_numberOfInstantiations.fetch_add(1, std::memory_order_relaxed);
  }
  ~ObjectCounter()
  {
    s_numberOfInstantiations.fetch_sub(1, std::memory_order_relaxed);
  }
#else
  ObjectCounter() = default;
  ObjectCounter(const ObjectCounter&) = default;
  ~ObjectCounter() = default;
#endif
  // The rest
  ObjectCounter& operator=(const ObjectCounter&) = default;
  ObjectCounter(ObjectCounter&&) = default;
  ObjectCounter& operator=(ObjectCounter&&) = default;
};

}
#endif
