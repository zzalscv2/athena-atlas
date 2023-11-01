/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CsvMuonSimHitDumperMuonCnv.h"

#include <MuonReadoutGeometry/MuonReadoutElement.h>
#include <MuonReadoutGeometry/GlobalUtilities.h>

#include<fstream>

CsvMuonSimHitDumperMuonCnv::CsvMuonSimHitDumperMuonCnv(const std::string& name, ISvcLocator* pSvcLocator):
   AthAlgorithm{name, pSvcLocator} {}

StatusCode CsvMuonSimHitDumperMuonCnv::initialize() {
   ATH_CHECK(m_inSimHitKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   ATH_CHECK(m_surfaceProvTool.retrieve());
   ATH_CHECK(detStore()->retrieve(m_r4DetMgr));

   return StatusCode::SUCCESS;

}

StatusCode CsvMuonSimHitDumperMuonCnv::execute(){
    
   const ActsGeometryContext gctx{};
   
   const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};

   const EventContext & context = Gaudi::Hive::currentContext();
   
   SG::ReadHandle<xAOD::MuonSimHitContainer> readSimHits(m_inSimHitKey,
                                                              context);

   std::ofstream file{"MuonSimHit_" + std::to_string(++m_event)+".csv"};
   
   file<<"pdgId"<<"\t";
   file<<"StationName"<<"\t";
   file<<"StationEta"<<"\t";
   file<<"StationPhi"<<"\t";
   file<<"LocalPositionExtrx"<<"\t";
   file<<"LocalPositionExtry"<<"\t";
   file<<"LocalPositionExtrz"<<"\t";
   file<<"LocalDirectionx"<<"\t";
   file<<"LocalDirectiony"<<"\t";
   file<<"LocalDirectionz"<<std::endl;


   std::set<Identifier> usedStations{};
   for (const xAOD::MuonSimHit* simHit : *readSimHits) {

      //The sim hit collection contains non-muon hits, while the fast digi only writes out muon 
      // deposits. Here remove sim hits that are not expected to be accounted for in the digi. 

      if (std::abs(simHit->pdgId()) != 13) continue;
      const Identifier ID = simHit->identify();
      if (!usedStations.insert(m_idHelperSvc->chamberId(ID)).second) continue;

      ATH_MSG_ALWAYS("Dump simulation hit for " <<m_idHelperSvc->toString(ID));
      const MuonGMR4::MdtReadoutElement* reElement = m_r4DetMgr->getMdtReadoutElement(ID);
      
      //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
      const Amg::Transform3D toChamber = m_surfaceProvTool->globalToChambCenter(gctx, ID) *
                                         reElement->localToGlobalTrans(gctx, reElement->measurementHash(ID));

      const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
      const Amg::Vector3D localDir{toChamber.linear() * xAOD::toEigen(simHit->localDirection())};

    
      const std::optional<double> lamda = Amg::intersect<3>(localPos, localDir, Amg::Vector3D::UnitY(), 0.);
      const Amg::Vector3D localPosExtr = localPos + (*lamda)*localDir;


      file<<simHit->pdgId()<<"\t";
      file<<mdtHelper.stationName(ID)<<"\t";
      file<<mdtHelper.stationEta(ID)<<"\t";
      file<<mdtHelper.stationPhi(ID)<<"\t";
   
      file<<localPosExtr.x()<<"\t";
      file<<localPosExtr.y()<<"\t";
      file<<localPosExtr.z()<<"\t";

      file<<localDir.x()<<"\t";
      file<<localDir.y()<<"\t";
      file<<localDir.z()<<std::endl;   


}

return StatusCode::SUCCESS;

}


