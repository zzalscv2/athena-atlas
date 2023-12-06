/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CsvMdtDriftCircleDumperMuonCnv.h"

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include<fstream>
#include<TString.h>


CsvMdtDriftCircleDumperMuonCnv::CsvMdtDriftCircleDumperMuonCnv(const std::string& name, ISvcLocator* pSvcLocator):
 AthAlgorithm{name, pSvcLocator} {}

 StatusCode CsvMdtDriftCircleDumperMuonCnv::initialize(){
   ATH_CHECK(m_inDriftCircleKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());

   return StatusCode::SUCCESS;
 }

 StatusCode CsvMdtDriftCircleDumperMuonCnv::execute(){

   const EventContext & context = Gaudi::Hive::currentContext();
   const std::string delim = ",";
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"MuonDriftCircle.csv"};
   
    file<<"driftRadius"<<delim;
    file<<"tubePositionx"<<delim;
    file<<"tubePositiony"<<delim;
    file<<"tubePositionz"<<delim;
    file<<"stationName"<<delim;
    file<<"stationEta"<<delim;
    file<<"stationPhi"<<delim;
    file<<"multilayer"<<delim;
    file<<"tubelayer"<<delim;
    file<<"tube"<<std::endl;


   SG::ReadHandle<xAOD::MdtDriftCircleContainer> readDriftCircles{m_inDriftCircleKey, context};
   ATH_CHECK(readDriftCircles.isPresent());
   const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};

   for(const xAOD::MdtDriftCircle* driftCircle : *readDriftCircles){
      const Amg::Vector3D tubePos{xAOD::toEigen(driftCircle->tubePosInStation())};
      const Identifier tubeId{(Identifier::value_type)driftCircle->identifier()};
      file<<driftCircle->driftRadius()<<delim;
      file<<tubePos.x()<<delim;
      file<<tubePos.y()<<delim;
      file<<tubePos.z()<<delim;
      file<<m_idHelperSvc->stationName(tubeId)<<delim;
      file<<m_idHelperSvc->stationEta(tubeId)<<delim;
      file<<m_idHelperSvc->stationPhi(tubeId)<<delim;
      file<<mdtHelper.multilayer(tubeId)<<delim;
      file<<mdtHelper.tubeLayer(tubeId)<<delim;
      file<<mdtHelper.tube(tubeId)<<std::endl;

   }

   return StatusCode::SUCCESS;


 }



