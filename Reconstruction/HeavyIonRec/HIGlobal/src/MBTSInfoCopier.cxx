/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "xAODTrigMinBias/TrigT2MbtsBitsAuxContainer.h"
#include "xAODTrigMinBias/TrigT2MbtsBits.h"

#include "MBTSInfoCopier.h"

MBTSInfoCopier::MBTSInfoCopier(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

MBTSInfoCopier::~MBTSInfoCopier()
{
}

StatusCode MBTSInfoCopier::initialize()
{
  ATH_CHECK(m_mbtsCellContainerKey.initialize());
  ATH_CHECK(m_MbtsBitsKey.initialize());
  ATH_CHECK( detStore()->retrieve(m_tileTBID) );
  return StatusCode::SUCCESS;
}

StatusCode MBTSInfoCopier::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode MBTSInfoCopier::execute(const EventContext& context) const
{
  SG::ReadHandle<TileCellContainer> tileCellsHandle (m_mbtsCellContainerKey, context);
  static const unsigned int MAX_MBTS_COUNTER{32};  
  std::vector<float> energies(MAX_MBTS_COUNTER);
  std::vector<float> times(MAX_MBTS_COUNTER);

  for (const TileCell* cell : *tileCellsHandle) {
    // code from: TileCalorimeter/TileMonitoring/src/TileMBTSMonitorAlgorithm.cxx
    Identifier id = cell->ID();
    int counter = m_tileTBID->phi(id) + 8 * m_tileTBID->eta(id);
    if (m_tileTBID->side(id) < 0) counter += 16;// EBC side
    ATH_CHECK(counter<static_cast<int>(MAX_MBTS_COUNTER));
    energies[counter] = cell->energy();
    times[counter] = cell->time();
  }
    ATH_MSG_DEBUG("energies " << energies);
    ATH_MSG_DEBUG("times " << times);


  SG::WriteHandle<xAOD::TrigT2MbtsBitsContainer> mbtsBitsHandle (m_MbtsBitsKey, context);
  auto mbtsBitsContainer = std::make_unique< xAOD::TrigT2MbtsBitsContainer>();
  auto mbtsBitsAuxContainer = std::make_unique< xAOD::TrigT2MbtsBitsAuxContainer>();
  mbtsBitsContainer->setStore(mbtsBitsAuxContainer.get());

  xAOD::TrigT2MbtsBits * mbtsObj = new xAOD::TrigT2MbtsBits();
  mbtsBitsContainer->push_back(mbtsObj);

  mbtsObj->setTriggerEnergies(energies);
  mbtsObj->setTriggerTimes(times);

  ATH_CHECK(mbtsBitsHandle.record( std::move(mbtsBitsContainer), std::move( mbtsBitsAuxContainer ) ) );



  return StatusCode::SUCCESS;
}

