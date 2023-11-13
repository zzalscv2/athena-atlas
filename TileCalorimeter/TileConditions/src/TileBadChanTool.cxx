/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileConditions/TileBadChanTool.h"
#include "TileIdentifier/TileHWID.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Calo includes
#include "CaloDetDescr/CaloDetDescrElement.h"

// Athena incldues
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadCondHandle.h"

// Gaudi includes
#include "GaudiKernel/EventContext.h"

#include <string>
#include <algorithm>


//
//____________________________________________________________________
TileBadChanTool::TileBadChanTool(const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent)
    , m_tileMgr(nullptr)
    , m_tileHWID(nullptr)
    , m_defaultStatus(TileBchStatus())
    , m_defaultTripsProbs(TileCalibUtils::MAX_DRAWER, 0.0F)
{

}

//
//____________________________________________________________________
TileBadChanTool::~TileBadChanTool() {

}


//
//____________________________________________________________________
StatusCode TileBadChanTool::initialize() {
  ATH_MSG_DEBUG( "in initialize()" );

  //=== Initialize bad channels key
  ATH_CHECK( m_badChannelsKey.initialize() );

  //=== TileDetDescrManager
  ATH_CHECK( detStore()->retrieve(m_tileMgr) );

  //=== TileDetDescrManager
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  m_roses.resize(TileCalibUtils::MAX_DRAWERIDX);
  m_drawers.resize(TileCalibUtils::MAX_DRAWERIDX);

  for (unsigned int ros = 0; ros < TileCalibUtils::MAX_ROS; ++ros) {
    for (unsigned int drawer = 0; drawer < TileCalibUtils::getMaxDrawer(ros); ++drawer) {
      unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
      m_roses[drawerIdx] = ros;
      m_drawers[drawerIdx] = drawer;
    }
  }


  return StatusCode::SUCCESS;
}

//
//____________________________________________________________________
StatusCode TileBadChanTool::finalize() {

  ATH_MSG_DEBUG( "finalize called" );
  return StatusCode::SUCCESS;

}

//
//____________________________________________________________________
CaloBadChannel
TileBadChanTool::caloStatus(const EventContext& ctx, Identifier cell_id) const
{

  CaloBadChannel::BitWord res = 0;

  //=== get the TileHWIDs of the two channels connected to the caloCellnxs id
  const CaloDetDescrElement* elem = m_tileMgr->get_cell_element(cell_id);

  if (!elem) {
    //=== this should never happen
    ATH_MSG_ERROR( "CaloMgr returns NULL CaloDetDescrElement" );
    std::abort();
  }

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey,ctx);

  IdentifierHash hash1_id(elem->onl1());
  IdentifierHash hash2_id(elem->onl2());

  //=== status of 1. connected channel
  TileBchStatus channel1_status = (hash1_id != TileHWID::NOT_VALID_HASH) ?
    badChannels->getChannelStatus(m_tileHWID->channel_id(hash1_id)) : m_defaultStatus;

  //=== status of 2. connected channel
  TileBchStatus channel2_status = (hash2_id != TileHWID::NOT_VALID_HASH) ?
    badChannels->getChannelStatus(m_tileHWID->channel_id(hash2_id)) : m_defaultStatus;

  //=== set cell status depending on channel status
  if (channel1_status.isBad() && channel2_status.isBad()) {
    CaloBadChannel::setBit(CaloBadChannel::deadBit, res);
  } else if (channel1_status.isAffected() || channel2_status.isAffected()) {
    CaloBadChannel::setBit(CaloBadChannel::affectedBit, res);
  }

  return CaloBadChannel(res);
}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getAdcStatus(const HWIdentifier& adc_id, const EventContext& ctx) const {

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  return badChannels->getAdcStatus(adc_id);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getAdcStatus(const HWIdentifier& adc_id) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getAdcStatus(adc_id, ctx);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getAdcStatus(IdentifierHash hash_id, unsigned int adc) const {

  if (hash_id != TileHWID::NOT_VALID_HASH) {
    HWIdentifier adc_id = m_tileHWID->adc_id(hash_id, adc);

    SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey);
    return badChannels->getAdcStatus(adc_id);
  } else {
    return m_defaultStatus;
  }

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getChannelStatus(IdentifierHash hash_id) const {

  if (hash_id != TileHWID::NOT_VALID_HASH) {
    HWIdentifier channel_id = m_tileHWID->channel_id(hash_id);

    SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey);
    return badChannels->getChannelStatus(channel_id);
  } else {
    return m_defaultStatus;
  }

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getChannelStatus(const HWIdentifier& channel_id, const EventContext& ctx) const {

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  return badChannels->getChannelStatus(channel_id);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getChannelStatus(const HWIdentifier& channel_id) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getChannelStatus(channel_id, ctx);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getChannelStatus(unsigned int drawerIdx, unsigned int channel, const EventContext& ctx) const {

  HWIdentifier channel_id = m_tileHWID->channel_id(m_roses[drawerIdx], m_drawers[drawerIdx], channel);

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  return badChannels->getChannelStatus(channel_id);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getChannelStatus(unsigned int drawerIdx, unsigned int channel) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getChannelStatus(drawerIdx, channel, ctx);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getAdcStatus(unsigned int drawerIdx, unsigned int channel, unsigned int adc, const EventContext& ctx) const {

  HWIdentifier adc_id =  m_tileHWID->adc_id(m_roses[drawerIdx], m_drawers[drawerIdx], channel, adc);

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  return badChannels->getAdcStatus(adc_id);

}

//
//____________________________________________________________________
const TileBchStatus&
TileBadChanTool::getAdcStatus(unsigned int drawerIdx, unsigned int channel, unsigned int adc) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getAdcStatus(drawerIdx, channel, adc, ctx);

}


uint32_t TileBadChanTool::encodeStatus(const TileBchStatus& status) const {
  return TileBadChannels::encodeStatus(status);
}

const std::vector<float>& TileBadChanTool::getTripsProbabilities(unsigned int ros, const EventContext& ctx) const {

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);

  const std::vector<std::vector<float>>& tripsProbs = badChannels->getTripsProbabilities();

  if (!tripsProbs.empty()) {
    return tripsProbs.at(ros - 1);
  }

  return m_defaultTripsProbs;
}

const std::vector<float>& TileBadChanTool::getTripsProbabilities(unsigned int ros) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getTripsProbabilities(ros, ctx);

}


bool TileBadChanTool::isDrawerMasked(unsigned int frag_id, const EventContext& ctx) const {

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  const std::vector<int>& maskedDrawers = badChannels->getMaskedDrawers();

  return std::binary_search (maskedDrawers.begin(),
                             maskedDrawers.end(),
                             frag_id);
}

bool TileBadChanTool::isDrawerMasked(unsigned int frag_id) const {

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return isDrawerMasked(frag_id, ctx);

}
