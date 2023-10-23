/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsVolumeMappingTool.h"

// ATHENA
#include "GaudiKernel/IInterface.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsInterop/Logger.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

// ACTS
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

// STL
#include <iostream>
#include <memory>


ActsVolumeMappingTool::ActsVolumeMappingTool(const std::string& type, const std::string& name,
    const IInterface* parent)
  : base_class(type, name, parent)
{
}

StatusCode
ActsVolumeMappingTool::initialize()
{
  ATH_MSG_INFO("Initializing ACTS Volume Mapper");

  ATH_CHECK( m_trackingGeometryTool.retrieve() );

  m_trackingGeometry = m_trackingGeometryTool->trackingGeometry();

  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometry } );
  // Make stepper and propagator
  SlStepper stepper;
  StraightLinePropagator propagator = StraightLinePropagator(std::move(stepper), std::move(navigator));

  /// The material mapper
  Acts::VolumeMaterialMapper::Config smmConfig;
  smmConfig.mappingStep = 10;
  m_mapper = std::make_shared<Acts::VolumeMaterialMapper>(
      smmConfig,
      std::move(propagator),
      makeActsAthenaLogger(this, "VolumeMaterialMapper"));

  m_geoContext = m_trackingGeometryTool->getNominalGeometryContext().context();

  ATH_MSG_INFO("ACTS Surface Mapper successfully initialized");
  return StatusCode::SUCCESS;
}

Acts::VolumeMaterialMapper::State
ActsVolumeMappingTool::mappingState() const
{
  auto mappingState = m_mapper->createState(
    m_geoContext, m_magFieldContext, *m_trackingGeometry);

  return mappingState;
}
