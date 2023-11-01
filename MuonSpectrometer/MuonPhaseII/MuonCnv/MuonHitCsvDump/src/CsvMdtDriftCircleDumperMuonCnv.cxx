/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CsvMdtDriftCircleDumperMuonCnv.h"

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include<fstream>


CsvMdtDriftCircleDumperMuonCnv::CsvMdtDriftCircleDumperMuonCnv(const std::string& name, ISvcLocator* pSvcLocator):
 AthAlgorithm{name, pSvcLocator} {}

 StatusCode CsvMdtDriftCircleDumperMuonCnv::initialize(){
   ATH_CHECK(m_inDriftCircleKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());

   return StatusCode::SUCCESS;
 }

 StatusCode CsvMdtDriftCircleDumperMuonCnv::execute(){

   const EventContext & context = Gaudi::Hive::currentContext();

   std::ofstream file{"MuonDriftCircle__" + std::to_string(++m_event)+".csv"};
   
    file<<"driftRadius"<<"\t";
    file<<"tubePositionx"<<"\t";
    file<<"tubePositiony"<<"\t";
    file<<"tubePositionz"<<"\t";
    file<<"stationName"<<"\t";
    file<<"stationEta"<<"\t";
    file<<"stationPhi"<<"\t";
    file<<"multilayer"<<"\t";
    file<<"tubelayer"<<"\t";
    file<<"tube"<<std::endl;


   SG::ReadHandle<xAOD::MdtDriftCircleContainer> readDriftCircles(
            m_inDriftCircleKey, context);

   const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};

   for(const xAOD::MdtDriftCircle* driftCircle : *readDriftCircles){
      const Amg::Vector3D tubePos{xAOD::toEigen(driftCircle->tubePosInStation())};
      const Identifier tubeId{(Identifier::value_type)driftCircle->identifier()};
      file<<driftCircle->driftRadius()<<" \t";
      file<<tubePos.x()<<"\t";
      file<<tubePos.y()<<"\t";
      file<<tubePos.z()<<"\t";
      file<<m_idHelperSvc->stationName(tubeId)<<"\t";
      file<<m_idHelperSvc->stationEta(tubeId)<<"\t";
      file<<m_idHelperSvc->stationPhi(tubeId)<<"\t";
      file<<mdtHelper.multilayer(tubeId)<<"\t";
      file<<mdtHelper.tubeLayer(tubeId)<<"\t";
      file<<mdtHelper.tube(tubeId)<<std::endl;

   }

   return StatusCode::SUCCESS;


 }



