/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELMDTTEST_H
#define MUONGEOMODELTESTR4_GEOMODELMDTTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonCondData/MdtCondDbData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonGM {

class GeoModelMdtTest : public AthHistogramAlgorithm {
   public:
    GeoModelMdtTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
    const MdtCondDbData* retrieveDeadChannels(const EventContext& ctx ) const;
    StatusCode dumpToTree(const EventContext& ctx, const MdtReadoutElement* readoutEle);
  
    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};


    /// Conditions object to exclude all tubes that are not
    SG::ReadCondHandleKey<MdtCondDbData> m_deadChanKey{this, "ReadKey", "MdtCondDbData", "Key of MdtCondDbData"};


    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    /// Set of stations to be tested
    std::set<Identifier> m_testStations{};

    /// String should be formated like
    /// <stationName><stationEta><A/C><stationPhi>
    Gaudi::Property<std::vector<std::string>> m_selectStat{
        this, "TestStations", {"BIL1A3"}};

    Gaudi::Property<bool> m_dumpSurfaces{this, "dumpSurfaces", false, "Adds the bounds and surfaces of each tube to the dump"};

   
    /// Write a TTree for validation purposes
    MuonVal::MuonTesterTree m_tree{"MdtGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("stationMultiLayer")};

    MuonVal::ScalarBranch<double>& m_tubeRad{m_tree.newScalar<double>("tubeRadius")};
    MuonVal::ScalarBranch<double>& m_tubePitch{m_tree.newScalar<double>("tubePitch")};


    /// Transformation of the underlying GeoModel element (Translation, ColX, ColY, ColZ)
    MuonVal::ThreeVectorBranch m_readoutTransform{m_tree, "GeoModelTransform"};   
    /// Number of tubes per layer
    MuonVal::ScalarBranch<unsigned short>& m_numTubes{m_tree.newScalar<unsigned short>("numTubes")};
    /// Number of tubes per layer
    MuonVal::ScalarBranch<unsigned short>& m_numLayers{m_tree.newScalar<unsigned short>("numLayers")};

    /// Readout each tube specifically
    MuonVal::VectorBranch<unsigned short>& m_tubeLay{m_tree.newVector<unsigned short>("tubeLayer")};
    MuonVal::VectorBranch<unsigned short>& m_tubeNum{m_tree.newVector<unsigned short>("tubeNumber")};
    /// Ideal transformations to the tube rest frame

    MuonVal::ThreeVectorBranch m_tubeTransformTran{m_tree, "tubeTransformTranslation"};
    MuonVal::ThreeVectorBranch m_tubeTransformColX{m_tree, "tubeTransformCol0"};
    MuonVal::ThreeVectorBranch m_tubeTransformColY{m_tree, "tubeTransformCol1"};
    MuonVal::ThreeVectorBranch m_tubeTransformColZ{m_tree, "tubeTransformCol2"};
   

    MuonVal::VectorBranch<double>& m_tubeLength{m_tree.newVector<double>("tubeLength")};
    MuonVal::VectorBranch<double>& m_activeTubeLength{m_tree.newVector<double>("activeTubeLength")};
    MuonVal::VectorBranch<double>& m_wireLength{m_tree.newVector<double>("wireLength")};
    
    /// Position of the readout
    MuonVal::ThreeVectorBranch m_roPos{m_tree, "readOutPos"};

    /// Distorted transformations
    MuonVal::VectorBranch<unsigned short>& m_layDistTubeLay{m_tree.newVector<unsigned short>("DistTubeLayer")};
    MuonVal::VectorBranch<unsigned short>& m_layDistTubeNum{m_tree.newVector<unsigned short>("DistTubeNumber")};
    MuonVal::VectorBranch<double>& m_layDistPosAlongWire{m_tree.newVector<double>("DistPosAlongTube")};
    MuonVal::ThreeVectorBranch m_layDistCenter{m_tree, "DistLayerCenter"};
    MuonVal::ThreeVectorBranch m_layDistColX{m_tree, "DistLayerLinearCol1"};
    MuonVal::ThreeVectorBranch m_layDistColY{m_tree, "DistLayerLinearCol2"};
    MuonVal::ThreeVectorBranch m_layDistColZ{m_tree, "DistLayerLinearCol3"};

    /// Alignment parameters
    MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};

    /// B Line chamber defomrations
    MuonVal::ScalarBranch<float>& m_BLineBz{m_tree.newScalar<float>("BLineBz", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineBp{m_tree.newScalar<float>("BLineBp", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineBn{m_tree.newScalar<float>("BLineBn", 0.)};    
    MuonVal::ScalarBranch<float>& m_BLineSp{m_tree.newScalar<float>("BLineSp", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineSn{m_tree.newScalar<float>("BLineSn", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineTw{m_tree.newScalar<float>("BLineTw", 0.)};
    MuonVal::ScalarBranch<float>& m_BLinePg{m_tree.newScalar<float>("BLinePg", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineTr{m_tree.newScalar<float>("BLineTr", 0.)};    
    MuonVal::ScalarBranch<float>& m_BLineEg{m_tree.newScalar<float>("BLineEg", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineEp{m_tree.newScalar<float>("BLineEp", 0.)};
    MuonVal::ScalarBranch<float>& m_BLineEn{m_tree.newScalar<float>("BLineEn", 0.)};
    /// AS-built parameters
    MuonVal::ScalarBranch<float>& m_asBuiltPosY0{m_tree.newScalar<float>("AsBuiltPosY0", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltPosZ0{m_tree.newScalar<float>("AsBuiltPosZ0", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltPosAlpha{m_tree.newScalar<float>("AsBuiltPosAlpha", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltPosPitchY{m_tree.newScalar<float>("AsBuiltPosPitchY", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltPosPitchZ{m_tree.newScalar<float>("AsBuiltPosPitchZ", 0.)};
    MuonVal::ScalarBranch<int>  & m_asBuiltPosStagg{m_tree.newScalar<int>("AsBuiltPosStagg",0)};

    MuonVal::ScalarBranch<float>& m_asBuiltNegY0{m_tree.newScalar<float>("AsBuiltNegY0", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltNegZ0{m_tree.newScalar<float>("AsBuiltNegZ0", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltNegAlpha{m_tree.newScalar<float>("AsBuiltNegAlpha", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltNegPitchY{m_tree.newScalar<float>("AsBuiltNegPitchY", 0.)};
    MuonVal::ScalarBranch<float>& m_asBuiltNegPitchZ{m_tree.newScalar<float>("AsBuiltNegPitchZ", 0.)};
    MuonVal::ScalarBranch<int>  & m_asBuiltNegStagg{m_tree.newScalar<int>("AsBuiltNegStagg",0)};

};

}  // namespace MuonGM
#endif