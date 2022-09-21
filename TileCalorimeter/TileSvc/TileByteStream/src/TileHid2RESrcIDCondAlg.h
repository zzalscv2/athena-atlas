/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H
#define TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H

#include "TileByteStream/TileHid2RESrcID.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TileHWID;

/**
 * @class TileHid2RESrcIDCondAlg
 * @brief Condition algorithm to prepare TileHid2RESrcID conditions object and put it into conditions store
 */
class TileHid2RESrcIDCondAlg: public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    Gaudi::Property<bool> m_forHLT{this, "ForHLT", false, "Produce TileHid2RESrcID conditions object for HLT"};

  /**
   * @brief Name of output TileHid2RESrcID
   */
    SG::WriteCondHandleKey<TileHid2RESrcID> m_hid2ReSrcIdKey{this,
        "TileHid2RESrcID", "TileHid2RESrcID", "Output TileHid2RESrcID conditions object"};

    const TileHWID* m_tileHWID{nullptr};
};


#endif // TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H
