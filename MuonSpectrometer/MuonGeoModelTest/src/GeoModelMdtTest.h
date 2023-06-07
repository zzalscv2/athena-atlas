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
    void dumpToFile(const EventContext& ctx, const MdtReadoutElement* readoutEle, std::ostream& sstr);

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

    Gaudi::Property<std::string> m_outputTxt{
        this, "DumpTxtFile", "MdtGeoDump.txt",
        "Dump the basic informations from the Readout geometry into a txt "
        "file"};
    Gaudi::Property<bool> m_dumpSurfaces{this, "dumpSurfaces", false, "Adds the bounds and surfaces of each tube to the dump"};

   
    /// Write a TTree for validation purposes
    Gaudi::Property<bool> m_dumpTree{this, "writeTTree", true};
    MuonVal::MuonTesterTree m_tree{"MdtGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("stationMultiLayer")};

    MuonVal::ScalarBranch<double>& m_tubeRad{m_tree.newScalar<double>("tubeRadius")};
    MuonVal::ScalarBranch<double>& m_tubePitch{m_tree.newScalar<double>("tubePitch")};


    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::ThreeVectorBranch m_readoutTransform{m_tree, "ElementTransform"};   
    /// Number of tubes per layer
    MuonVal::ScalarBranch<unsigned short>& m_numTubes{m_tree.newScalar<unsigned short>("numTubes")};
    /// Number of tubes per layer
    MuonVal::ScalarBranch<unsigned short>& m_numLayers{m_tree.newScalar<unsigned short>("numLayers")};

    /// Transforatmations to the first tube in a layer
    MuonVal::VectorBranch<unsigned short>& m_layTransNumber{m_tree.newVector<unsigned short>("layerNumber")};
    /// Ideal transformations
    MuonVal::ThreeVectorBranch m_layCenter{m_tree,"LayerCenter"};
    MuonVal::ThreeVectorBranch m_layTransColX{m_tree, "LayerLinearCol1"};
    MuonVal::ThreeVectorBranch m_layTransColY{m_tree, "LayerLinearCol2"};
    MuonVal::ThreeVectorBranch m_layTransColZ{m_tree, "LayerLinearCol3"};
   

    /// Readout each tube specifically
    MuonVal::VectorBranch<unsigned short>& m_tubeLay{m_tree.newVector<unsigned short>("tubeLayer")};
    MuonVal::VectorBranch<unsigned short>& m_tubeNum{m_tree.newVector<unsigned short>("tubeNumber")};
    MuonVal::VectorBranch<double>& m_tubeLength{m_tree.newVector<double>("tubeLength")};
    MuonVal::VectorBranch<double>& m_activeTubeLength{m_tree.newVector<double>("activeTubeLength")};
    MuonVal::VectorBranch<double>& m_wireLength{m_tree.newVector<double>("wireLength")};

    /// Center of the tube
    MuonVal::ThreeVectorBranch m_tubePos{m_tree,"tubePos"};
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

};

}  // namespace MuonGM
#endif