/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDTOOLS_MUONTRACKTAGTESTTOOL_H
#define MUONCOMBINEDTOOLS_MUONTRACKTAGTESTTOOL_H

#include "MuonCombinedToolInterfaces/IMuonTrackTagTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"

#include <mutex>
#include <string>

namespace Trk {
  class TrackingGeometry;
  class TrackingVolume;
}

namespace MuonCombined {

class MuonTrackTagTestTool : public AthAlgTool, virtual public IMuonTrackTagTool {

  public:
    MuonTrackTagTestTool(const std::string& type, const std::string& name, const IInterface* parent);
    ~MuonTrackTagTestTool()=default;

    StatusCode initialize();

    double chi2(const Trk::TrackParameters& idParsAtEntry, const Trk::TrackParameters& msParsAtEntry) const;
    double chi2(const Trk::Track& id, const Trk::Track& ms) const;

  private:
    ToolHandle<Trk::IExtrapolator> m_extrapolator{this,"ExtrapolatorTool","Trk::Extrapolator/AtlasExtrapolator",};
    mutable ServiceHandle<Trk::ITrackingGeometrySvc> m_trackingGeometrySvc ATLAS_THREAD_SAFE {this,"TrackingGeometrySvc","AtlasTrackingGeometrySvc"};  // Services are assumed to be thread-safe

    mutable const Trk::TrackingGeometry* m_trackingGeometry ATLAS_THREAD_SAFE;  // Initialized with call_once, then used read-only
    mutable const Trk::TrackingVolume* m_msEntrance ATLAS_THREAD_SAFE;  // Initialized with call_once, then used read-only
    mutable std::once_flag m_trackingOnceFlag ATLAS_THREAD_SAFE;

    double m_chi2cut;
#ifdef MUONCOMBDEBUG
    bool m_truth;
#endif
};

}  // namespace MuonCombined


#endif
