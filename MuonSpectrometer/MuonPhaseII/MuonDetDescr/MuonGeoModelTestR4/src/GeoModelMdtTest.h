/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELMDTTEST_H
#define MUONGEOMODELTESTR4_GEOMODELMDTTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <set>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/IActsTrackingGeometryTool.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
namespace MuonGMR4{

class GeoModelMdtTest : public AthHistogramAlgorithm{
    public:
        GeoModelMdtTest(const std::string& name, ISvcLocator* pSvcLocator);

        ~GeoModelMdtTest() = default;

        StatusCode execute() override;
        StatusCode initialize() override;
        unsigned int cardinality() const override final {return 1;}

    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

         /// Set of stations to be tested
       std::set<Identifier> m_testStations{};
    
       /// String should be formated like <stationName><stationEta><A/C><stationPhi>
       Gaudi::Property<std::vector<std::string>> m_selectStat{this, "TestStations", {"BIL1A3"}};
    
       Gaudi::Property<std::string> m_outputTxt{this, "DumpTxtFile", "MdtGeoDump.txt", 
       "Dump the basic informations from the Readout geometry into a txt file" };
        const MuonDetectorManager* m_detMgr{nullptr};
};

}
#endif