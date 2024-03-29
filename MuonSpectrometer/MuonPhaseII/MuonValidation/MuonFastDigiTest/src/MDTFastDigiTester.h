/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFASTDIGITEST_MUONVALR4_MDTFastDigiTester_H
#define MUONFASTDIGITEST_MUONVALR4_MDTFastDigiTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

// EDM includes 
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"

// muon includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"


///  @brief Lightweight algorithm to read xAOD MDT sim hits and 
///  (fast-digitised) drift circles from SG and fill a 
///  validation NTuple with identifier and drift circle info.

namespace MuonValR4{

  class MDTFastDigiTester : public AthHistogramAlgorithm {
  public:
    MDTFastDigiTester(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MDTFastDigiTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    // MDT sim hits in xAOD format 
    SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_inSimHitKey {this, "SimHitKey", "xMdtSimHits",
                                                          "xAOD SimHit collection"};
    // drift circles in xAOD format 
    SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_inDriftCircleKey{
    this, "DriftCircleKey", "xAODMdtCircles", "mdt circle container"};
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"FastSmearingResults","MuonFastDigiTest"}; 
    MuonVal::ScalarBranch<int>& m_out_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<int>& m_out_stationEta{m_tree.newScalar<int>("stationEta")};
    MuonVal::ScalarBranch<int>& m_out_stationPhi{m_tree.newScalar<int>("stationPhi")};
    MuonVal::ScalarBranch<int>& m_out_multilayer{m_tree.newScalar<int>("multilayer")};
    MuonVal::ScalarBranch<int>& m_out_tubeLayer{m_tree.newScalar<int>("tubeLayer")};
    MuonVal::ScalarBranch<int>& m_out_tube{m_tree.newScalar<int>("tube")}; 
    MuonVal::ScalarBranch<float>& m_out_barcode{m_tree.newScalar<float>("barcode")};   
    MuonVal::ScalarBranch<float>& m_out_simDriftRadius{m_tree.newScalar<float>("sim_driftRadius")}; 
    MuonVal::ScalarBranch<bool>& m_out_hasDigi{m_tree.newScalar<bool>("hasDigi", false)}; 
    MuonVal::ScalarBranch<float>& m_out_digiDriftRadius{m_tree.newScalar<float>("digi_driftRadius", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_digiDriftRadiusCov{m_tree.newScalar<float>("digi_driftRadiusCov", 0.0)}; 
  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MDTFastDigiTester_H
