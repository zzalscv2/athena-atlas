/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "sTGCHitsTestTool.h"

#include "Identifier/Identifier.h"

#include "GeoAdaptors/GeoMuonHits.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "MuonSimEvent/sTgcHitIdHelper.h"

#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "MuonSimEvent/sTGCSimHit.h"

#include "GeneratorObjects/McEventCollection.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/INTupleSvc.h"

#include "TH2D.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"

using namespace MuonGM;
using namespace std;


StatusCode sTGCHitsTestTool::processEvent() {
  CHECK(executeCheckEventInfo());

   if (m_DosTGCTest) {
    const sTGCSimHitCollection* p_collection = nullptr;
    CHECK(evtStore()->retrieve(p_collection,"sTGC_Hits"));
    for (const sTGCSimHit& hit : *p_collection) {
      Amg::Vector3D u = hit.globalPosition();
      CHECK(executeFillHistos(u));
      
      
      const sTgcHitIdHelper* hitHelper = sTgcHitIdHelper::GetHelper();
      int simId = hit.sTGCId();
      std::string sim_stationName = hitHelper->GetStationName(simId);

      static const std::string QS1C("QS1C");
      static const std::string QS2C("QS2C");
      static const std::string QS3C("QS3C");
      static const std::string QL1P("QL1P");
      static const std::string QL2P("QL2P");
      static const std::string QL3P("QL3P");

      if (sim_stationName==QS1C && u.z()>0){
        m_sTgc_TransverseView_QS1C_posZ->Fill(u.x(),u.y());
      }


      if (sim_stationName==QS2C && u.z()>0){
        m_sTgc_TransverseView_QS2C_posZ->Fill(u.x(),u.y());
      }


      if (sim_stationName==QS3C && u.z()>0){
        m_sTgc_TransverseView_QS3C_posZ->Fill(u.x(),u.y());
      }

      if (sim_stationName==QL1P && u.z()>0){
        m_sTgc_TransverseView_QL1P_posZ->Fill(u.x(),u.y());
      }


      if (sim_stationName==QL2P && u.z()>0){
        m_sTgc_TransverseView_QL2P_posZ->Fill(u.x(),u.y());
      }

      if (sim_stationName==QL3P && u.z()>0){
        m_sTgc_TransverseView_QL3P_posZ->Fill(u.x(),u.y());
      }

      double r_sTGc = sqrt(u.x()*u.x()+u.y()*u.y());

      if (u.z() > 0){
        m_sTgc_rZview_positiveZ->Fill(u.z(), r_sTGc);
      }


      // GeoMMHit ghit(hit);
      //       if (!ghit) continue;
      //       Amg::Vector3D u = ghit.getGlobalPosition();
      //       CHECK(executeFillHistos(u));
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode sTGCHitsTestTool::initialize() {
  CHECK(MuonHitTestToolBase::initialize());
  _TH2D( m_sTgc_TransverseView_QS1C_posZ,"sTGC_TransverseView_QS1C_posZ",1200,-6000.,6000.,1200,-6000.,6000.);
  _TH2D( m_sTgc_TransverseView_QS2C_posZ,"sTGC_TransverseView_QS2C_posZ",1200,-6000.,6000.,1200,-6000.,6000.);
  _TH2D( m_sTgc_TransverseView_QS3C_posZ,"sTGC_TransverseView_QS3C_posZ",1200,-6000.,6000.,1200,-6000.,6000.);

  _TH2D( m_sTgc_TransverseView_QL1P_posZ,"sTGC_TransverseView_QL1P_posZ",1200,-6000.,6000.,1200,-6000.,6000.);
  _TH2D( m_sTgc_TransverseView_QL2P_posZ,"sTGC_TransverseView_QL2P_posZ",1200,-6000.,6000.,1200,-6000.,6000.);
  _TH2D( m_sTgc_TransverseView_QL3P_posZ,"sTGC_TransverseView_QL3P_posZ",1200,-6000.,6000.,1200,-6000.,6000.);

  _TH2D(m_sTgc_rZview_positiveZ,"sTGC_rZView_posZ",2000,6500.,8500.,5000,0.,5000.);
  return StatusCode::SUCCESS;
}
