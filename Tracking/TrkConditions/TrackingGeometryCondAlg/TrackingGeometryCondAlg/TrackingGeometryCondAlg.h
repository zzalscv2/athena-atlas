/*
 *   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRACKINGGEOMETRYCONDALG_H
#define TRACKINGGEOMETRYCONDALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "TrkGeometry/TrackingGeometry.h"

#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/IGeometryProcessor.h"

#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk{
class TrackingGeometryCondAlg : public AthAlgorithm
{
public:
  TrackingGeometryCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackingGeometryCondAlg() override = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;
  virtual StatusCode finalize() override final {return StatusCode::SUCCESS;}
  /** Make this algorithm clonable. */
  virtual bool isClonable() const override final{ return true; };

private:

  /// Output conditions object.
  SG::WriteCondHandleKey<TrackingGeometry> m_trackingGeometryWriteKey{
    this,
    "TrackingGeometryWriteKey",
    "AtlasTrackingGeometry",
    "Key of output of TrackingGeometry for ID"
  };
  ServiceHandle<ICondSvc> m_condSvc{ this, "CondSvc", "CondSvc" };
  ToolHandle<Trk::IGeometryBuilderCond> m_trackingGeometryBuilder{
    this,
    "GeometryBuilder",
    ""
  };
  ToolHandleArray<Trk::IGeometryProcessor>
    m_geometryProcessors{ this, "GeometryProcessors", {}, "" };
};
}
#endif //TRACKINGGEOMETRYCONDALG_H
