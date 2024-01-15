/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SpacePointPersistification.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthLinks/ElementLink.h"

namespace InDet {

  SpacePointPersistification::SpacePointPersistification(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode SpacePointPersistification::initialize()
  {
    ATH_MSG_INFO("Inizializing " << name() << " ...");
    ATH_CHECK(m_inSpacepoints.initialize());
    m_clusterLink = m_inSpacepoints.key() + "." + m_clusterLink.key();
    ATH_CHECK(m_clusterLink.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode SpacePointPersistification::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    SG::ReadHandle< xAOD::SpacePointContainer > spacePointHandle = SG::makeHandle( m_inSpacepoints, ctx );
    ATH_CHECK(spacePointHandle.isValid());
    const xAOD::SpacePointContainer* spacePoints = spacePointHandle.cptr();

    SG::WriteDecorHandle< xAOD::SpacePointContainer, std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer>> > clusterDecoration( m_clusterLink, ctx);
    
    for (const xAOD::SpacePoint* sp : *spacePoints) {
      const auto& meas = sp->measurements();
      std::vector< ElementLink<xAOD::UncalibratedMeasurementContainer> > els;
      for (std::size_t idx(0); idx<meas.size(); ++idx) {
	els.push_back( ElementLink<xAOD::UncalibratedMeasurementContainer>(*dynamic_cast<const xAOD::UncalibratedMeasurementContainer*>(meas[idx]->container()), meas[idx]->index()) );
      }
      clusterDecoration(*sp) = els;
    }
    
    return StatusCode::SUCCESS;
  }

}


