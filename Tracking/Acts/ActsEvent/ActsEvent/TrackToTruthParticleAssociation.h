#ifndef TRACKTOGENPARTICLEASSOCIATION_H
#define TRACKTOGENPARTICLEASSOCIATION_H
#include "boost/container/small_vector.hpp"
#include "xAODTruth/TruthParticle.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include <vector>
#include <array>
#include <cstdint>
#include <utility>

namespace ActsTrk
{
   constexpr unsigned int NHitCounter = static_cast< std::underlying_type<xAOD::UncalibMeasType>::type >(xAOD::UncalibMeasType::sTgcStripType)+1u;
   constexpr unsigned int NTruthParticlesPerTrack = 5;  // a tiny fraction of measurements will have more than
                                                        // 6 associated GenParticles
   using HitCounterArray = std::array<uint8_t,  NHitCounter>;
   using TruthParticleHitCountVector = boost::container::small_vector<std::pair<const xAOD::TruthParticle *,
                                                                                HitCounterArray >, NTruthParticlesPerTrack>;
   using TrackToTruthParticleAssociation = std::vector<TruthParticleHitCountVector> ;
}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TrackToTruthParticleAssociation, 101405904, 1 )

#endif
