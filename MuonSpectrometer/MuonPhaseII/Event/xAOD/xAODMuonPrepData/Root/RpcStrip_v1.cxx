/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/RpcStrip_v1.h"

namespace {
    static const std::string preFixStr{"RPC_"};
}
#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE RpcStrip_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void RpcStrip_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }
                                                                          
namespace xAOD {

IMPLEMENT_SETTER_GETTER(float, time, setTime)
IMPLEMENT_SETTER_GETTER(uint32_t, triggerInfo, setTriggerInfo)
IMPLEMENT_SETTER_GETTER(uint8_t, ambiguityFlag, setAmbiguityFlag)
IMPLEMENT_SETTER_GETTER(float, timeOverThreshold, setTimeOverThreshold)


IdentifierHash RpcStrip_v1::measurementHash() const {
    // return MuonGMR4::RpcReadoutElement::measurementHash(Identifier(static_cast<Identifier::value_type>(identifier()))); 
    //FIXME! Not optimal, but in any case it doesn't work since we need to have a RRE instantiated to get the hash
    return IdentifierHash();
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
