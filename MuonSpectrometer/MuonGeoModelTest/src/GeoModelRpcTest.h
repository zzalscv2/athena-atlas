/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELRPCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELRPCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonGM {

class GeoModelRpcTest : public AthHistogramAlgorithm {
   public:
    GeoModelRpcTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
     
     StatusCode dumpToTree(const EventContext& ctx, const RpcReadoutElement* readoutEle);

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

    Gaudi::Property<std::string> m_outputTxt{
        this, "DumpTxtFile", "",
        "Dump the basic informations from the Readout geometry into a txt "};
    
     /// Write a TTree for validation purposes
    Gaudi::Property<bool> m_dumpTree{this, "writeTTree", true};
    MuonVal::MuonTesterTree m_tree{"RpcGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<uint8_t>& m_doubletR{m_tree.newScalar<uint8_t>("stationDoubletR")};
    MuonVal::ScalarBranch<uint8_t>& m_doubletZ{m_tree.newScalar<uint8_t>("stationDoubletZ")};
    MuonVal::ScalarBranch<uint8_t>& m_doubletPhi{m_tree.newScalar<uint8_t>("stationDoubletPhi")};
 
    /// Number of strips, strip pitch in eta & phi direction
    MuonVal::ScalarBranch<uint8_t>& m_numStripsEta{m_tree.newScalar<uint8_t>("numEtaStrips")};
    MuonVal::ScalarBranch<uint8_t>& m_numStripsPhi{m_tree.newScalar<uint8_t>("numPhiStrips")};
    
    /// Strip dimensions 
    MuonVal::ScalarBranch<float>& m_stripEtaPitch{m_tree.newScalar<float>("stripEtaPitch")};
    MuonVal::ScalarBranch<float>& m_stripPhiPitch{m_tree.newScalar<float>("stripPhiPitch")};
    MuonVal::ScalarBranch<float>& m_stripEtaWidth{m_tree.newScalar<float>("stripEtaWidth")};
    MuonVal::ScalarBranch<float>& m_stripPhiWidth{m_tree.newScalar<float>("stripPhiWidth")};
    MuonVal::ScalarBranch<float>& m_stripEtaLength{m_tree.newScalar<float>("stripEtaLength")};
    MuonVal::ScalarBranch<float>& m_stripPhiLength{m_tree.newScalar<float>("stripPhiLength")};
    /// Number of eta & phi gas gaps
    MuonVal::ScalarBranch<uint8_t>& m_numGasGapsEta{m_tree.newScalar<uint8_t>("numEtaGasGaps")};
    MuonVal::ScalarBranch<uint8_t>& m_numGasGapsPhi{m_tree.newScalar<uint8_t>("numPhiGasGaps")};
    
    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::ThreeVectorBranch m_readoutTransform{m_tree, "GeoModelTransform"};
    
    /// Alignment parameters
    MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};
    
    /// Rotation matrix of the respective layers
    MuonVal::ThreeVectorBranch m_stripRotColX{m_tree, "stripRotCol0"};
    MuonVal::ThreeVectorBranch m_stripRotColY{m_tree, "stripRotCol1"};
    MuonVal::ThreeVectorBranch m_stripRotColZ{m_tree, "stripRotCol2"};
    MuonVal::VectorBranch<uint8_t>& m_stripRotGasGap{m_tree.newVector<uint8_t>("stripRotGasGap")};
    MuonVal::VectorBranch<uint8_t>& m_stripRotDblPhi{m_tree.newVector<uint8_t>("stripRotDoubletPhi")};
    MuonVal::VectorBranch<bool>& m_stripRotMeasPhi{m_tree.newVector<bool>("stripRotMeasPhi")};
    
    /// Strip positions
    MuonVal::ThreeVectorBranch m_stripPos{m_tree, "stripPos"};
    MuonVal::VectorBranch<bool>& m_stripPosMeasPhi{m_tree.newVector<bool>("stripPosMeasPhi")};
    MuonVal::VectorBranch<uint8_t>& m_stripPosGasGap{m_tree.newVector<uint8_t>("stripPosGasGap")};
    MuonVal::VectorBranch<uint8_t>& m_stripPosNum{m_tree.newVector<uint8_t>("stripPosNum")};
    MuonVal::VectorBranch<uint8_t>& m_stripDblPhi{m_tree.newVector<uint8_t>("stripPosDoubletPhi")};
    

};

}
#endif