/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_TGCSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_TGCSTRIPAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class TgcStripAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    TgcStripAuxContainer_v1();

   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<Identifier::value_type> m_identifier;
    std::vector<IdentifierHash::value_type> m_identifierHash;
    std::vector<PosAccessor<1>::element_type> m_localPosition;
    std::vector<CovAccessor<1>::element_type> m_localCovariance;

    std::vector<uint16_t> m_bcBitMap{};
 
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TgcStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif
