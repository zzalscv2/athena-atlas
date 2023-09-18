/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELSTGCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELSTGCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonGM {

class GeoModelsTgcTest : public AthHistogramAlgorithm {
   public:
    GeoModelsTgcTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
     
     StatusCode dumpToTree(const EventContext& ctx, const sTgcReadoutElement* readoutEle);

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

    MuonVal::MuonTesterTree m_tree{"sTgcGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<short>& m_stIndex{m_tree.newScalar<short>("stationIndex")}; // 57(S) or 58(L)
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")}; // [-3, 3]
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")}; // [1, 8]
    MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("stationMultilayer")}; // {1, 2}

    //// Chamber Details

    MuonVal::ScalarBranch<short>& m_numLayers{m_tree.newScalar<short>("numLayers")}; // 4
    MuonVal::ScalarBranch<float>& m_yCutout{m_tree.newScalar<float>("yCutout")}; // yCutoutCathode
    MuonVal::ScalarBranch<float>& m_activeHeight{m_tree.newScalar<float>("activeHeight")}; // Length - ylFrame - ysFrame   
    MuonVal::ScalarBranch<float>& m_gasTck{m_tree.newScalar<float>("gasTck")}; // gasTck 2.85mm

    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::ThreeVectorBranch m_readoutTransform{m_tree, "GeoModelTransform"};

    //// Wire Dimensions
    MuonVal::VectorBranch<uint>& m_numWires{m_tree.newVector<uint>("numWires")}; // nWires 
    MuonVal::VectorBranch<short>& m_firstWireGroupWidth{m_tree.newVector<short>("firstWireGroupWidth")}; // firstWireGroup <= 20
    MuonVal::VectorBranch<short>& m_numWireGroups{m_tree.newVector<short>("numWireGroups")}; // nWireGroups >19
    MuonVal::VectorBranch<float>& m_wireCutout{m_tree.newVector<float>("wireCutout")}; // wireCutout ~ 800mm
    MuonVal::ScalarBranch<float>& m_wirePitch{m_tree.newScalar<float>("wirePitch")}; // wirePitch 1.8mm
    MuonVal::ScalarBranch<float>& m_wireWidth{m_tree.newScalar<float>("wireWidth")}; // wireWidth 0.015mm
    MuonVal::ScalarBranch<short>& m_wireGroupWidth{m_tree.newScalar<short>("wireGroupWidth")}; // wireGroupWidth 20
    MuonVal::ThreeVectorBranch m_globalWireGroupPositions{m_tree, "globalWireGroupPositions"};
    MuonVal::VectorBranch<uint8_t>& m_wireGroupNum{m_tree.newVector<uint8_t>("wireGroupNumber")}; // wire Group number
    MuonVal::VectorBranch<uint8_t>& m_wireGroupGasGap{m_tree.newVector<uint8_t>("wireGroupGasGap")}; // gas gap number

    /// Strip dimensions 
    MuonVal::ScalarBranch<uint>& m_numStrips{m_tree.newScalar<uint>("numStrips")}; // nStrips
    MuonVal::ScalarBranch<float>& m_stripPitch{m_tree.newScalar<float>("stripPitch")}; // stripPitch 3.2mm
    MuonVal::ScalarBranch<float>& m_stripWidth{m_tree.newScalar<float>("stripWidth")}; // stripWidth 2.7mm
    MuonVal::VectorBranch<float>& m_firstStripPitch{m_tree.newVector<float>("firstStripPitch")}; // firstStripWidth 1.6/3.2mm
    MuonVal::ThreeVectorBranch m_globalStripPositions{m_tree, "globalStripPositions"};
    MuonVal::VectorBranch<uint>& m_stripNum{m_tree.newVector<uint>("stripNumber")}; // strip number
    MuonVal::VectorBranch<uint8_t>& m_stripGasGap{m_tree.newVector<uint8_t>("stripGasGap")}; // gas gap number
    MuonVal::VectorBranch<float>& m_stripLengths{m_tree.newVector<float>("stripLengths")}; // Length of each strip

   /// Pad dimensions 
    MuonVal::VectorBranch<uint>& m_numPads{m_tree.newVector<uint>("numPads")};
    MuonVal::VectorBranch<uint>& m_numPadEta{m_tree.newVector<uint>("numPadEta")};
    MuonVal::VectorBranch<uint>& m_numPadPhi{m_tree.newVector<uint>("numPadPhi")};

    MuonVal::ThreeVectorBranch m_globalPadCornerBL{m_tree, "globalPadCornerBottomLeft"};
    MuonVal::ThreeVectorBranch m_globalPadCornerBR{m_tree, "globalPadCornerBottomRight"};
    MuonVal::ThreeVectorBranch m_globalPadCornerTL{m_tree, "globalPadCornerTopLeft"};
    MuonVal::ThreeVectorBranch m_globalPadCornerTR{m_tree, "globalPadCornerTopRight"};
    MuonVal::ThreeVectorBranch m_globalPadPositions{m_tree, "globalPadPositions"};
    MuonVal::VectorBranch<uint8_t>& m_padGasGap{m_tree.newVector<uint8_t>("padGasGap")}; // gas gap number
    MuonVal::VectorBranch<uint>& m_padEta{m_tree.newVector<uint>("padEtaNumber")}; // pad number in eta direction
    MuonVal::VectorBranch<uint>& m_padPhi{m_tree.newVector<uint>("padPhiNumber")}; // pad number in phi direction
    
};

}
#endif