/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELRTGCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELRTGCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonGM {

class GeoModelTgcTest : public AthHistogramAlgorithm {
   public:
    GeoModelTgcTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
     
     StatusCode dumpToTree(const EventContext& ctx, const TgcReadoutElement* readoutEle);

    /// MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

     ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    /// Set of stations to be tested
    std::set<Identifier> m_testStations{};

    /// String should be formated like
    /// <stationName><stationEta><A/C><stationPhi>
    Gaudi::Property<std::vector<std::string>> m_selectStat{
        this, "TestStations", {}, "Constrain the stations to be tested"};

     /// Write a TTree for validation purposes
    MuonVal::MuonTesterTree m_tree{"TgcGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
  
    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::ThreeVectorBranch m_readoutTransform{m_tree, "ElementTransform"};   
    
    /// Alignment parameters
    MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};

    
    MuonVal::ThreeVectorBranch m_stripCenter{m_tree,"stripCenter"};
    MuonVal::VectorBranch<uint8_t>& m_stripGasGap{m_tree.newVector<uint8_t>("stripGasGap")};
    MuonVal::VectorBranch<unsigned int>& m_stripNum{m_tree.newVector<unsigned int>("stripNumber")};
    
    MuonVal::ThreeVectorBranch m_gangCenter{m_tree, "gangCenter"};
    MuonVal::VectorBranch<uint8_t>& m_gangGasGap{m_tree.newVector<uint8_t>("gangGasGap")};
    MuonVal::VectorBranch<unsigned int>& m_gangNum{m_tree.newVector<unsigned int>("gangNumber")};
    MuonVal::VectorBranch<uint8_t>& m_gangNumWires{m_tree.newVector<uint8_t>("gangNumWires")};
    MuonVal::VectorBranch<float>& m_gangLength{m_tree.newVector<float>("gangLength")};
   

      
    MuonVal::ThreeVectorBranch m_layCenter{m_tree,"layerCenter"};
    MuonVal::ThreeVectorBranch m_layTransColX{m_tree, "layerLinearCol1"};
    MuonVal::ThreeVectorBranch m_layTransColY{m_tree, "layerLinearCol2"};
    MuonVal::ThreeVectorBranch m_layTransColZ{m_tree, "layerLinearCol3"};
    MuonVal::VectorBranch<bool>& m_layMeasPhi{m_tree.newVector<bool>("layerMeasPhi")};
    MuonVal::VectorBranch<uint8_t>& m_layNumber{m_tree.newVector<uint8_t>("layerNumber")};

};

}
#endif