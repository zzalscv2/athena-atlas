/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
// #include "MuonReadoutGeometryR4/MMReadoutElement.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/MMCluster_v1.h"

namespace {
    static const std::string preFixStr{"MM_"};
}
#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE MMCluster_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void MMCluster_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }
                                                                          
namespace xAOD {

IMPLEMENT_SETTER_GETTER(uint16_t, time, setTime)
IMPLEMENT_SETTER_GETTER(uint32_t, charge, setCharge)
IMPLEMENT_SETTER_GETTER(float, driftDist, setDriftDist)
IMPLEMENT_SETTER_GETTER(float, angle, setAngle)
IMPLEMENT_SETTER_GETTER(float, chiSqProb, setChiSqProb)
AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MMCluster_v1, unsigned int, MMCluster_v1::Author, author)
AUXSTORE_PRIMITIVE_SETTER_WITH_CAST( MMCluster_v1, unsigned int, MMCluster_v1::Author, author, setAuthor)

IdentifierHash MMCluster_v1::measurementHash() const {
    // return MuonGMR4::MMReadoutElement::measurementHash(Identifier(static_cast<Identifier::value_type>(identifier()))); 
    //FIXME! Not optimal, but in any case it doesn't work since we need to have a RRE instantiated to get the hash
    return IdentifierHash();
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
