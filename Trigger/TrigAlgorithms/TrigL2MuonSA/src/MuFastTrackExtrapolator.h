/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MUFASTTRACKEXTRAPOLATOR_H
#define  TRIGL2MUONSA_MUFASTTRACKEXTRAPOLATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrackData.h"

#include "TrigMuonToolInterfaces/ITrigMuonBackExtrapolator.h"

namespace TrigL2MuonSA {
  
  class MuFastTrackExtrapolator: public AthAlgTool
  {
  public:
    MuFastTrackExtrapolator(const std::string& type, 
			    const std::string& name,
			    const IInterface*  parent);
    
    void setExtrapolatorTool(ToolHandle<ITrigMuonBackExtrapolator>* backExtrapolator) {m_backExtrapolatorTool = backExtrapolator;};

    StatusCode extrapolateTrack(std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns,
				double winPt) const;
    
    double getMuFastRes(const double pt, const int add, const double eta, const double phi) const;

  private:
    const ToolHandle<ITrigMuonBackExtrapolator>* m_backExtrapolatorTool {nullptr};

  };

} // namespace TrigL2MuonSA

#endif  // MUFASTTRACKEXTRAPOLATOR_H
