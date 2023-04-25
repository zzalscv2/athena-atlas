/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_ACTSDETALIGNCONDALG_H
#define ACTSGEOMETRY_ACTSDETALIGNCONDALG_H

#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/RawGeomAlignStore.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
/**
 *  The ActsDetAlignCondAlg loads the rigid alignment transformations and pipes them into
 *  through the ActsDetectorElements associated to 1 subdetector. The detector elements then
 *  cache all internal transformations needed to build the readoutgeometry of the measurement layers.
 *  The created cache object is then published to the ConditionsStore to be pickedup by the ActsGeometryContext
 */

class ActsDetAlignCondAlg : public AthReentrantAlgorithm {
public:
    /// Standard constructor
    ActsDetAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual ~ActsDetAlignCondAlg();

    StatusCode initialize() override final;

    StatusCode execute(const EventContext& ctx) const override final;
    /// Switch off reentrancy to avoid condition clashes
    bool isReEntrant() const override final { return false; }

private:
    /// Key to the alignment transformations for the detector volumes
    SG::ReadCondHandleKey<GeoAlignmentStore> m_inputKey{this, "InputTransforms", ""};
    /// Key to the alignment transformations written by the alg
    SG::WriteCondHandleKey<ActsTrk::RawGeomAlignStore> m_outputKey{this, "ActsTransforms", ""};
    /// ServiceHandle to the ActsTrackingGeometry
    ServiceHandle<IActsTrackingGeometrySvc> m_trackingGeoSvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
    /// Flag determining the subdetector. Needs to be static castable to ActsTrk::DetectorType
    Gaudi::Property<int> m_detType{this, "DetectorType", static_cast<int>(ActsTrk::DetectorType::UnDefined)};
    /// Flag toggling whether the GeoAlignmentStore is whiped before written to storgate
    Gaudi::Property<bool> m_whipeGeoStore{this, "WhipeGeoStore", true};
    /// Static cast of >DetectorType< property
    ActsTrk::DetectorType m_Type{ActsTrk::DetectorType::UnDefined};
};
#endif