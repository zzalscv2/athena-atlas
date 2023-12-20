/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAMTESTALG_H
#define MUONCALIBSTREAMTESTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/RpcPadContainer.h"
#include "xAODEventInfo/EventInfo.h"

//  Test Algorithm to dumps EventInfo, MdtPrepDataContainers, RpcPrepDataContainer, TgcPrepDataContainer

class MuonCalibStreamTestAlg : public AthReentrantAlgorithm {
    
    public:
        //! Constructor.
        MuonCalibStreamTestAlg(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;

        virtual StatusCode finalize() override;
        
        //! Destructur
        ~MuonCalibStreamTestAlg() = default;

    private:

        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "MuonCalibStreamEventInfo", "EventInfo"};
        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_MdtPrepDataKey {this, "MdtPrepData", "MDT_DriftCircles"};
        SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_RpcPrepDataKey {this, "RpcPrepData", "RPC_Measurements"};
        SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_TgcPrepDataKey {this, "TgcPrepData", "TGC_Measurements"};
};
#endif
