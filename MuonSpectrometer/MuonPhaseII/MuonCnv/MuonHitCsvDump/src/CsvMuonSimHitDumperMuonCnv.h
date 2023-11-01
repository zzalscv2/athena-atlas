/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

   #include<string>

   #include <AthenaBaseComps/AthAlgorithm.h>
   #include <MuonIdHelpers/IMuonIdHelperSvc.h>
   #include <StoreGate/ReadHandleKey.h>

   #include "xAODMuonSimHit/MuonSimHitContainer.h"
   #include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>
   #include <MuonReadoutGeometryR4/MuonDetectorManager.h>
   
   /** The CsvMuonSimHitDumper reads a Simulation Hit container for muons and dumps information to csv files**/

   class CsvMuonSimHitDumperMuonCnv: public AthAlgorithm {

   public:

   CsvMuonSimHitDumperMuonCnv(const std::string& name, ISvcLocator* pSvcLocator);
   ~CsvMuonSimHitDumperMuonCnv() = default;


    StatusCode initialize() override;
    StatusCode execute() override;

   private:

    
    SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_inSimHitKey{
    this, "MuonSimHitKey", "", "muon sim hit container"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

 /// Access to the new readout geometry
   const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};
  PublicToolHandle<MuonGMR4::IMuonStationLayerSurfaceTool> m_surfaceProvTool{this, "LayerGeoTool", ""};

  int m_event = 0;
};
