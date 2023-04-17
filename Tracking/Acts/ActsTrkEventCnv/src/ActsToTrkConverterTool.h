/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENTCNV_ActsToTrkConverterTool_H
#define ACTSTRKEVENTCNV_ActsToTrkConverterTool_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/EventContext.h"
#include "TrkParameters/TrackParameters.h" //typedef, cannot fwd declare


// PACKAGE
#include "ActsTrkEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

#include "Acts/EventData/TrackParameters.hpp"

namespace ActsTrk {
class ActsToTrkConverterTool : public extends<AthAlgTool, IActsToTrkConverterTool>
{

public:
  virtual StatusCode initialize() override;

  ActsToTrkConverterTool(const std::string& type, const std::string& name,
	           const IInterface* parent);


  /// Find the ATLAS surface corresponding to the Acts surface 
  /// Only work if the Acts surface has an associated detector element
  /// (Pixel and SCT)
  virtual 
  const Trk::Surface&
  actsSurfaceToTrkSurface(const Acts::Surface &actsSurface) const override;

  /// Find the Acts surface corresponding to the ATLAS surface 
  /// Use a map associating ATLAS ID to Acts surfaces
  /// (Pixel and SCT)
  virtual
  const Acts::Surface&
  trkSurfaceToActsSurface(const Trk::Surface &atlasSurface) const override;

  /// Create an SourceLink from an ATLAS measurment
  /// Works for 1 and 2D measurmenent. 
  /// A pointer to the measurment is kept in the SourceLink
  virtual 
  const ATLASSourceLink
  trkMeasurementToSourceLink(const Acts::GeometryContext& gctx, const Trk::MeasurementBase &measurement,
			       std::vector<ATLASSourceLink::ElementsType>& Collection) const override;

  /// Create an SourceLink from an ATLAS uncalibrated measurment
  /// Works for 1 and 2D measurmenent.
  /// A pointer to the measurment is kept in the SourceLink
  virtual
  const ATLASUncalibSourceLink
  uncalibratedTrkMeasurementToSourceLink(const InDetDD::SiDetectorElementCollection &detectorElements, 
				      const xAOD::UncalibratedMeasurement *measurement,
				      std::vector<ATLASUncalibSourceLink::ElementsType>& Collection) const override;

  /// Transform an ATLAS track into a vector of SourceLink to be use in the avts tracking
  /// Transform both measurement and outliers.
  virtual 
  const std::vector<ATLASSourceLink>
  trkTrackToSourceLinks(const Acts::GeometryContext& gctx, 
			 const Trk::Track &track,
			 std::vector<ATLASSourceLink::ElementsType>& collection) const override;

  /// Create Acts TrackParameter from ATLAS one.
  /// Take care of unit conversion between the two.  
  virtual
  const Acts::BoundTrackParameters
  trkTrackParametersToActsParameters(const Trk::TrackParameters &atlasParameter) const override;

  /// Create ATLAS TrackParameter from Acts one.
  /// Take care of unit conversion between the two.  
  virtual
  std::unique_ptr<const Trk::TrackParameters>
  actsTrackParametersToTrkParameters(const Acts::BoundTrackParameters &actsParameter, const Acts::GeometryContext& gctx) const override;

  virtual
  const IActsTrackingGeometryTool*
  trackingGeometryTool() const override
  {
    return m_trackingGeometryTool.get();
  };

private:
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
  std::map<Identifier, const Acts::Surface*> m_actsSurfaceMap;

  Gaudi::Property<bool> m_visualDebugOutput{this, "VisualDebugOutput", 
    false, "Print additional output for debug plots"};
};

}; // namespace ActsTrk

#endif
