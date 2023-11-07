/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// =============================================================================
#include "CaloRingerElectronsReader.h"

#include <algorithm>

#include "AthenaKernel/getMessageSvc.h"
#include "PATCore/AcceptData.h"
#include "StoreGate/ReadHandle.h"

namespace Ringer {

// =============================================================================
CaloRingerElectronsReader::CaloRingerElectronsReader(const std::string& type,
                                 const std::string& name,
                                 const ::IInterface* parent) :
  CaloRingerInputReader(type, name, parent),
  m_clRingsBuilderElectronFctor(nullptr)
{

  // declare interface
  declareInterface<ICaloRingerElectronsReader>(this);

}

// =============================================================================
CaloRingerElectronsReader::~CaloRingerElectronsReader()
{
  delete m_clRingsBuilderElectronFctor;
}

// =============================================================================
StatusCode CaloRingerElectronsReader::initialize()
{

  ATH_CHECK( CaloRingerInputReader::initialize() );

  ATH_CHECK(m_inputElectronContainerKey.initialize());

  if ( m_builderAvailable ) {
    // Initialize our fctor
    m_clRingsBuilderElectronFctor =
      new BuildCaloRingsFctor<xAOD::ElectronContainer>(
        m_inputElectronContainerKey.key(),
        m_crBuilder,
        Athena::getMessageSvc(),
        this
      );
    ATH_CHECK( m_clRingsBuilderElectronFctor->initialize() );
  }

  return StatusCode::SUCCESS;

}

// =============================================================================
StatusCode CaloRingerElectronsReader::finalize()
{
  return StatusCode::SUCCESS;
}

// =============================================================================
StatusCode CaloRingerElectronsReader::execute()
{

  ATH_MSG_DEBUG("Entering " << name() << " execute, m_builderAvailable = " << m_builderAvailable);

   // Retrieve electrons
  SG::ReadHandle<xAOD::ElectronContainer> electrons(m_inputElectronContainerKey);
  // check is only used for serial running; remove when MT scheduler used
  if(!electrons.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve "<< m_inputElectronContainerKey.key());
    return StatusCode::FAILURE;
  }

  // Check if requested to run CaloRings Builder:
  if ( m_builderAvailable ) {
    ATH_CHECK( m_clRingsBuilderElectronFctor->prepareToLoopFor(electrons->size()) );

    // loop over our particles:
    for ( const auto electron : *electrons ){
      m_clRingsBuilderElectronFctor->operator()( electron );
    }

    m_clRingsBuilderElectronFctor->checkRelease();
  }

  return StatusCode::SUCCESS;
}

} // namespace Ringer


