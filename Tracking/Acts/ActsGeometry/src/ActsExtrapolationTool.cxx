/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsExtrapolationTool.h"

// ATHENA
#include "GaudiKernel/IInterface.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"

// PACKAGE
#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsInterop/Logger.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"

// ACTS
#include "Acts/Extrapolation/ExtrapolationCell.hpp" // for excell and ecode
#include "Acts/Extrapolation/IExtrapolationEngine.hpp" // for the parameters
#include "Acts/Extrapolation/ExtrapolationEngine.hpp"
#include "Acts/Extrapolation/RungeKuttaEngine.hpp"
#include "Acts/Extrapolation/MaterialEffectsEngine.hpp"
#include "Acts/Extrapolation/StaticNavigationEngine.hpp"
#include "Acts/Extrapolation/StaticEngine.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/BoundaryCheck.hpp"
#include "Acts/Extrapolator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"


// STL
#include <iostream>
#include <memory>

ActsExtrapolationTool::ActsExtrapolationTool(const std::string& type, const std::string& name,
    const IInterface* parent) 
  : AthAlgTool(type, name, parent),
    m_fieldServiceHandle("AtlasFieldSvc", name)
{

}
  
StatusCode 
ActsExtrapolationTool::initialize()
{
  using namespace std::literals::string_literals;


  ATH_MSG_INFO("Initializing ACTS extrapolation");

  ATH_CHECK( m_trackingGeometryTool.retrieve() );
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry 
    = m_trackingGeometryTool->trackingGeometry();

  Acts::Navigator navigator(trackingGeometry);

  if (m_fieldMode == "ATLAS"s) {
    // we need the field service
    ATH_CHECK( m_fieldServiceHandle.retrieve() );
    ATH_MSG_INFO("Using ATLAS magnetic field service");
    using BField_t = ATLASMagneticFieldWrapper;
    BField_t bField(m_fieldServiceHandle.get());
    auto stepper = Acts::EigenStepper<BField_t>(std::move(bField));
    auto propagator = Acts::Propagator<decltype(stepper), Acts::Navigator>(std::move(stepper), 
                                                                      std::move(navigator));
    m_varProp = std::make_unique<VariantPropagator>(propagator);
  }
  else if (m_fieldMode == "Constant") {
    std::vector<double> constantFieldVector = m_constantFieldVector;
    double Bx = constantFieldVector.at(0);
    double By = constantFieldVector.at(1);
    double Bz = constantFieldVector.at(2);
    ATH_MSG_INFO("Using constant magnetic field: (Bx, By, Bz) = (" << Bx << ", " << By << ", " << Bz << ")");
    using BField_t = Acts::ConstantBField;
    BField_t bField(Bx, By, Bz);
    auto stepper = Acts::EigenStepper<BField_t>(std::move(bField));
    auto propagator = Acts::Propagator<decltype(stepper), Acts::Navigator>(std::move(stepper), 
                                                                      std::move(navigator));
    m_varProp = std::make_unique<VariantPropagator>(propagator);
  }

  ATH_MSG_INFO("ACTS extrapolation successfully initialized");
  return StatusCode::SUCCESS;
}

Acts::ExtrapolationCode
ActsExtrapolationTool::extrapolate(Acts::ExCellCharged&       ecCharged,
              const Acts::Surface*       sf,
              const Acts::BoundaryCheck& bcheck) const 
{
  return m_exEngine->extrapolate(ecCharged, sf, bcheck);
}



Acts::ExtrapolationCode
ActsExtrapolationTool::extrapolate(Acts::ExCellNeutral&       ecNeutral,
              const Acts::Surface*       sf,
              const Acts::BoundaryCheck& bcheck) const 
{
  return m_exEngine->extrapolate(ecNeutral, sf, bcheck);
}

std::shared_ptr<Acts::IExtrapolationEngine>
ActsExtrapolationTool::extrapolationEngine() const 
{
  return std::dynamic_pointer_cast<Acts::IExtrapolationEngine>(m_exEngine);
}

void
ActsExtrapolationTool::prepareAlignment() const 
{
  m_trackingGeometryTool->prepareAlignment();
}
