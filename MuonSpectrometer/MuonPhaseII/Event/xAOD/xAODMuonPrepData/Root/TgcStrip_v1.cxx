/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
// #include "MuonReadoutGeometryR4/TgcReadoutElement.h" FIXME - add back when available
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/TgcStrip_v1.h"

namespace {
    static const std::string preFixStr{"TGC_"};
}
#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE TgcStrip_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void TgcStrip_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }
                                                                          
namespace xAOD {

IMPLEMENT_SETTER_GETTER(uint16_t, bcBitMap, setBcBitMap)

IdentifierHash TgcStrip_v1::measurementHash() const {
   //  return MuonGMR4::TgcReadoutElement::measurementHash(tubeLayer(), // FIXME - check what we need to get the hash for TGCs
   //                                                      driftTube());
   return IdentifierHash();
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
