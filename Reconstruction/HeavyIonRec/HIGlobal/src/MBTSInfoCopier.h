/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HIGLOBAL_MBTSINFOCOPIER_H
#define HIGLOBAL_MBTSINFOCOPIER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include <string>
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "TileEvent/TileContainer.h"
#include "CaloIdentifier/TileTBID.h"

#include "xAODTrigMinBias/TrigT2MbtsBitsContainer.h"

/**
 * @class MBTSInfoCopier
 * @brief 
 **/
class MBTSInfoCopier : public AthReentrantAlgorithm {
public:
  MBTSInfoCopier(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~MBTSInfoCopier() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKey<TileCellContainer> m_mbtsCellContainerKey{this, "MBTSContainer", "MBTSContainer"};
  SG::WriteHandleKey<xAOD::TrigT2MbtsBitsContainer> m_MbtsBitsKey{this, "MbtsBitsKey", "MBTSBits"};
  const TileTBID* m_tileTBID{nullptr};

};

#endif // HIGLOBAL_MBTSINFOCOPIER_H
