/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
   
   #include<string>

   #include <AthenaBaseComps/AthAlgorithm.h>
   #include <MuonIdHelpers/IMuonIdHelperSvc.h>
   #include <StoreGate/ReadHandleKey.h>

   #include "xAODMuonPrepData/MdtDriftCircleContainer.h"


/** The CsvMdtDriftCircleDumper reads an Mdt Drift Circle container and dumps information to csv files**/

class CsvMdtDriftCircleDumperMuonCnv: public AthAlgorithm {

   public:

   CsvMdtDriftCircleDumperMuonCnv(const std::string& name, ISvcLocator* pSvcLocator);
    ~CsvMdtDriftCircleDumperMuonCnv() = default;

     StatusCode initialize() override;
     StatusCode execute() override;

   private:

    // drift circles in xAOD format 
    SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_inDriftCircleKey{
    this, "DriftCircleKey", "xAODMdtCircles", "mdt circle container"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

   size_t m_event = 0;

};
