#ifndef MEASUREMENTTOGENPARTICLEASSOCIATION_H
#define MEASUREMENTTOGENPARTICLEASSOCIATION_H
#include "boost/container/small_vector.hpp"
#include "xAODTruth/TruthParticle.h"
#include <vector>

namespace ActsTrk
{
   constexpr unsigned int NTruthParticlesPerMeasurement = 5;  // chosen to make the small_vector fit into 164 bytes
                                                            // a tiny fraction of measurements will have more than
                                                            // 5 associated GenParticles
   using ParticleVector = boost::container::small_vector<const xAOD::TruthParticle *, NTruthParticlesPerMeasurement>;
   using MeasurementToTruthParticleAssociation = std::vector<ParticleVector> ;
}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::MeasurementToTruthParticleAssociation , 151157774 , 1 )

#endif
