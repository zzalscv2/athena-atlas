/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileHid2RESrcIDCondAlg.h"
#include "TileIdentifier/TileHWID.h"

// Athena includes

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteCondHandle.h"


StatusCode TileHid2RESrcIDCondAlg::initialize() {

  ATH_MSG_DEBUG( "In initialize()" );

  // Retrieve TileHWID helper from det store
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_hid2ReSrcIdKey.initialize() );

  return StatusCode::SUCCESS;
}


StatusCode TileHid2RESrcIDCondAlg::execute(const EventContext& ctx) const {

  SG::WriteCondHandle<TileHid2RESrcID> hid2ReSrcId{m_hid2ReSrcIdKey, ctx};

  if (hid2ReSrcId.isValid()) {
    ATH_MSG_DEBUG("Found valid TileHid2RESrcID: " << hid2ReSrcId.key());
    return StatusCode::SUCCESS;
  }

  uint32_t run = ctx.eventID().run_number();
  std::unique_ptr<TileHid2RESrcID> hid2re = std::make_unique<TileHid2RESrcID>(m_tileHWID, run);

  // Set validity of TileHid2RESrcID for current run
  EventIDRange eventRange{EventIDBase{run, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, 0},
                          EventIDBase{run + 1, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, 0}};

  ATH_CHECK( hid2ReSrcId.record(eventRange, std::move(hid2re)) );

  ATH_MSG_VERBOSE("Recorded TileHid2RESrcID object with "
                  << hid2ReSrcId.key()
                  << " with EventRange " << eventRange
                  << " into Conditions Store");

  return StatusCode::SUCCESS;
}
