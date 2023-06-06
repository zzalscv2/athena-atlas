/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONTRKPREDATACNV_MDTPREPDATATOXAODCNVALG_H
#define XAODMUONTRKPREDATACNV_MDTPREPDATATOXAODCNVALG_H

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"

class MdtPrepDataToxAODCnvAlg : public AthReentrantAlgorithm {
   public:
    MdtPrepDataToxAODCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

    ~MdtPrepDataToxAODCnvAlg() = default;

    StatusCode execute(const EventContext& ctx) const override;
    StatusCode initialize() override;

   private:
    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_inputKey{
        this, "InputContainer", "MDT_DriftCircles", "MDT prep data"};
    SG::WriteHandleKey<xAOD::MdtDriftCircleContainer> m_outputKey{
        this, "OutputContainer", "xAODMdtCircles", "Output container"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgr{
        this, "MuonManagerKey", "MuonDetectorManager"};
};
#endif