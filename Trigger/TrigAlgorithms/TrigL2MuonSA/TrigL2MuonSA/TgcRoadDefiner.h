/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_TGCROADDEFINER_H
#define  TRIGL2MUONSA_TGCROADDEFINER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrigMuonBackExtrapolator/ITrigMuonBackExtrapolator.h"
#include "TrigL2MuonSA/PtEndcapLUTSvc.h"
#include "TrigL2MuonSA/PtEndcapLUT.h"
#include "TrigL2MuonSA/TgcFit.h"
#include "TrigL2MuonSA/TgcData.h"
#include "TrigL2MuonSA/MuonRoad.h"
#include "TrigL2MuonSA/MdtRegion.h"
#include "TrigT1Interfaces/RecMuonRoI.h"
#include "RegionSelector/IRegSelSvc.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <string>

namespace TrigL2MuonSA {

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class TgcRoadDefiner: public AthAlgTool
{
 public:
  TgcRoadDefiner(const std::string& type, 
		 const std::string& name,
		 const IInterface*  parent);

  ~TgcRoadDefiner()=default;
  
  virtual StatusCode initialize() override;

  StatusCode defineRoad(const LVL1::RecMuonRoI*      p_roi,
                        const TrigL2MuonSA::TgcHits& tgcHits,
                        TrigL2MuonSA::MuonRoad&      muonRoad,
                        TrigL2MuonSA::TgcFitResult&  tgcFitResult);

  void setMdtGeometry(const ServiceHandle<IRegSelSvc>& regionSelector);
  void setPtLUT(const TrigL2MuonSA::PtEndcapLUTSvc* ptEndcapLUTSvc);
  void setRoadWidthForFailure(double rWidth_TGC_Failed);
  void setExtrapolatorTool(ToolHandle<ITrigMuonBackExtrapolator>* backExtrapolator);

  bool prepareTgcPoints(const TrigL2MuonSA::TgcHits& tgcHits);
  
 private:
  ToolHandle<ITrigMuonBackExtrapolator>* m_backExtrapolatorTool {nullptr};
  const ToolHandle<PtEndcapLUT>*         m_ptEndcapLUT {nullptr};

  ToolHandle<TgcFit>                     m_tgcFit {"TrigL2MuonSA::TgcFit"};

  TrigL2MuonSA::TgcFit::PointArray m_tgcStripMidPoints;  // List of TGC strip middle station points.
  TrigL2MuonSA::TgcFit::PointArray m_tgcWireMidPoints;   // List of TGC wire middle station points.
  TrigL2MuonSA::TgcFit::PointArray m_tgcStripInnPoints;  // List of TGC strip inner station points.
  TrigL2MuonSA::TgcFit::PointArray m_tgcWireInnPoints;   // List of TGC wire inner station points.

  double m_rWidth_TGC_Failed {0};
  
  ServiceHandle<IRegSelSvc> m_regionSelector;
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
}

#endif // TRIGL2MUONSA_TGCROADDEFINER_H
