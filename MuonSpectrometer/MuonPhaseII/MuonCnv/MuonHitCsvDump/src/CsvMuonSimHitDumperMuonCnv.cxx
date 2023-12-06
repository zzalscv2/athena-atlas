/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CsvMuonSimHitDumperMuonCnv.h"

#include <MuonReadoutGeometry/MuonReadoutElement.h>
#include <MuonReadoutGeometry/GlobalUtilities.h>

#include<fstream>
#include <TString.h>

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
   
   SG::ReadHandle<xAOD::MuonSimHitContainer> readSimHits{m_inSimHitKey, context};
   ATH_CHECK(readSimHits.isPresent());
   // these are the conventions by ACTS
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"MuonSimHit.csv"};
   const std::string delim = ",";
   file<<"pdgId"<<delim;
   file<<"StationName"<<delim;
   file<<"StationEta"<<delim;
   file<<"StationPhi"<<delim;
   file<<"LocalPositionExtrx"<<delim;
   file<<"LocalPositionExtry"<<delim;
   file<<"LocalPositionExtrz"<<delim;
   file<<"LocalDirectionx"<<delim;
   file<<"LocalDirectiony"<<delim;
   file<<"LocalDirectionz"<<std::endl;


   std::set<Identifier> usedStations{};
   for (const xAOD::MuonSimHit* simHit : *readSimHits) {

      //The sim hit collection contains non-muon hits, while the fast digi only writes out muon 
      // deposits. Here remove sim hits that are not expected to be accounted for in the digi. 

      if (std::abs(simHit->pdgId()) != 13) continue;
      const Identifier ID = simHit->identify();
      if (!usedStations.insert(m_idHelperSvc->chamberId(ID)).second) continue;

      ATH_MSG_VERBOSE("Dump simulation hit for " <<m_idHelperSvc->toString(ID));
      const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(ID);
      
      //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
      const Amg::Transform3D toChamber = m_surfaceProvTool->globalToChambCenter(gctx, ID) *
                                         reElement->localToGlobalTrans(gctx, reElement->measurementHash(ID));

      const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
      const Amg::Vector3D localDir{toChamber.linear() * xAOD::toEigen(simHit->localDirection())};

      const std::optional<double> lambda = Amg::intersect<3>(localPos, localDir, Amg::Vector3D::UnitZ(), 0.);
      const Amg::Vector3D localPosExtr = localPos + (*lambda)*localDir;


      file<<simHit->pdgId()<<delim;
      file<<mdtHelper.stationName(ID)<<delim;
      file<<mdtHelper.stationEta(ID)<<delim;
      file<<mdtHelper.stationPhi(ID)<<delim;
   
      file<<localPosExtr.x()<<delim;
      file<<localPosExtr.y()<<delim;
      file<<localPosExtr.z()<<delim;

      file<<localDir.x()<<delim;
      file<<localDir.y()<<delim;
      file<<localDir.z()<<std::endl;   


}

return StatusCode::SUCCESS;

}


