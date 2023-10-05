/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_STGCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonPrepRawData/MuonPrepRawData/MdtPrepData.h

class sTgcStrip_v1 : public UncalibratedMeasurement_v1 {

 public:
  /// Default constructor
  sTgcStrip_v1() = default;
  /// Virtual destructor
  virtual ~sTgcStrip_v1() = default;

  /// Returns the type of the Tgc strip as a simple enumeration
  xAOD::UncalibMeasType type() const override final {
    return xAOD::UncalibMeasType::sTgcStripType;
  }
  unsigned int numDimensions() const override final { return 1; }

  /** @brief Returns the hash of the measurement channel (tube (x) layer) */
  IdentifierHash measurementHash() const;

  /** @brief Returns the bcBitMap of this PRD
    bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
  uint16_t bcBitMap() const;

  /** @brief Returns the time  (ns). */
  uint16_t time() const;

  /** @brief Returns the charge*/
  uint32_t charge() const;

  void setBcBitMap(uint16_t);
  void setTime(uint16_t value);
  void setCharge(uint32_t value);
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::sTgcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif