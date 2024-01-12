// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXCUDA_TRACKPARTICLECONTAINER_H
#define ATHEXCUDA_TRACKPARTICLECONTAINER_H

// VecMem include(s).
#include <vecmem/edm/container.hpp>

namespace AthCUDAExamples {

/// Interface for the VecMem based GPU friendly TrackParticleContainer.
template <typename BASE>
struct TrackParticleInterface : public BASE {

  /// Inherit the base class's constructor(s)
  using BASE::BASE;

  /// Get the polar angles of the tracks (const)
  VECMEM_HOST_AND_DEVICE
  const auto& theta() const { return BASE::template get<0>(); }
  /// Get the polar angles of the tracks (non-const)
  VECMEM_HOST_AND_DEVICE
  auto& theta() { return BASE::template get<0>(); }

  /// Get the azimuthal angles of the tracks (const)
  VECMEM_HOST_AND_DEVICE
  const auto& phi() const { return BASE::template get<1>(); }
  /// Get the azimuthal angles of the tracks (non-const)
  VECMEM_HOST_AND_DEVICE
  auto& phi() { return BASE::template get<1>(); }

  /// Get the inverse momenta of the tracks (const)
  VECMEM_HOST_AND_DEVICE
  const auto& qOverP() const { return BASE::template get<2>(); }
  /// Get the inverse momenta of the tracks (non-const)
  VECMEM_HOST_AND_DEVICE
  auto& qOverP() { return BASE::template get<2>(); }

};  // struct TrackParticleInterface

/// SoA, GPU friendly TrackParticleContainer.
using TrackParticleContainer = vecmem::edm::container<
    TrackParticleInterface, vecmem::edm::type::vector<float>,
    vecmem::edm::type::vector<float>, vecmem::edm::type::vector<float>>;

}  // namespace AthCUDAExamples

#endif  // ATHEXCUDA_TRACKPARTICLECONTAINER_H
