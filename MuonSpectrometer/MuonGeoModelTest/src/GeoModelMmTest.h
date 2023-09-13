/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTEST_GEOMODELMMTEST_H
#define MUONGEOMODELTEST_GEOMODELMMTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonGM {

class GeoModelMmTest : public AthHistogramAlgorithm {
   public:
    GeoModelMmTest(const std::string& name, ISvcLocator* pSvcLocator);
    
    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

   private:
     
    /// MuonDetectorManager from the conditions store
    // ReadCondHandleKey is a class template is used for handling and managing conditions.
    //this declaration is creating an instance of ReadCondHandleKey specialized for MuonGM::MuonDetectorManager 
    //and associating it with "DetectorManagerKey" in the current class. 
    //The property "DetectorManagerKey"  will be used to access condition data related to the MuonDetectorManager.
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

    // handling (like data access) for the service of related to handling muon identifiers (like MicroMegas)
     ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   
    StatusCode dumpToTree(const EventContext& ctx, const MuonGM::MMReadoutElement* detEl);
    Identifier gasGapId(const MuonGM::MMReadoutElement* detEl, int gasGap) const;

    MuonVal::MuonTesterTree m_tree{"MmGeoModelTree", "GEOMODELTESTER"};

    MuonVal::ScalarBranch<unsigned short>& m_stationEta{m_tree.newScalar<unsigned short>("stationEta")};
    MuonVal::ScalarBranch<unsigned short>& m_stationPhi{m_tree.newScalar<unsigned short>("stationPhi")};
    MuonVal::ScalarBranch<int>& m_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<int>& m_multilayer{m_tree.newScalar<int>("multilayer")};
    MuonVal::VectorBranch<int>& m_gasGap{m_tree.newVector<int>("gasGap")};
    MuonVal::VectorBranch<int>& m_channel{m_tree.newVector<int>("channel")};
    MuonVal::VectorBranch<float>& m_stripLength{m_tree.newVector<float>("stripLength")};
    MuonVal::VectorBranch<float>& m_stripActiveLength{m_tree.newVector<float>("stripActiveLength")};
    MuonVal::VectorBranch<float>& m_stripActiveLengthLeft{m_tree.newVector<float>("stripActiveLengthLeft")};
    MuonVal::VectorBranch<float>& m_stripActiveLengthRight{m_tree.newVector<float>("stripActiveLengthRight")};
    MuonVal::ThreeVectorBranch m_stripCenter{m_tree, "stripCenter"};
    MuonVal::ThreeVectorBranch m_stripLeftEdge{m_tree, "stripLeftEdge"};
    MuonVal::ThreeVectorBranch m_stripRightEdge{m_tree, "stripRightEdge"};

     
};

}
#endif