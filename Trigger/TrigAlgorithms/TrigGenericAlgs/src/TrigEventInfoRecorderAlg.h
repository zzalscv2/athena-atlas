 // -*- C++ -*- 
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEVENTINFOALG_H
#define TRIGEVENTINFOALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEventShape/EventShape.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "LumiBlockComps/ILumiBlockMuTool.h"
#include "xAODTracking/VertexContainer.h"


class TrigEventInfoRecorderAlg : public AthReentrantAlgorithm {

    public:
    
    TrigEventInfoRecorderAlg(const std::string & name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
    // Custom functions
    StatusCode decorateWithEventInfo(const EventContext& ctx, xAOD::TrigComposite* trigEI) const;
 
    // Other functionalities may be added adding similar functions or defining a derived class

    private:
    
    // WRITE HANDLE KEY FOR OUTPUT CONTAINER
    SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_TrigEventInfoKey{this, "trigEventInfoKey","Undefined"};

    SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_rhoDecor{this, "RhoPFlowDecor", m_TrigEventInfoKey, "JetDensityEMPFlow"};
    SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_rhoEMTDecor{this, "RhoEMTopoDecor", m_TrigEventInfoKey, "JetDensityEMTopo"};
    SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_muDecor{this, "AvgMuDecor", m_TrigEventInfoKey, "AvgMu"};
    SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_numPVDecor{this, "NumPVDecor", m_TrigEventInfoKey, "NumPV"};

    // Configurations 
    Gaudi::Property<bool> m_decoratePFlowInfo {
      this, "decoratePFlowInfo", false, "Flag to enable PFlow event info decoration"};
    Gaudi::Property<bool> m_decorateEMTopoInfo {
      this, "decorateEMTopoInfo", false, "Flag to enable EMTopo event info decoration"};
    Gaudi::Property<bool> m_renounceAll {
      this, "renounceAll", false, "Flag to renounce all input dependencies and sweep up what is there."};
    // Event Info ReadHandleKeys
    ToolHandle<ILumiBlockMuTool> m_lumiBlockMuTool{this, "LuminosityTool", "LumiBlockMuTool/LumiBlockMuTool", "Luminosity Tool"};
    SG::ReadHandleKey<xAOD::EventShape> m_rhoKeyPF{this, "RhoKey_PFlow", "HLT_Kt4EMPFlowEventShape"}; // name of the density key: TOPO, PFLOW, etc
    SG::ReadHandleKey<xAOD::EventShape> m_rhoKeyEMT{this, "RhoKey_EMTopo", "HLT_Kt4EMTopoEventShape"}; // name of the density key: TOPO, PFLOW, etc
    SG::ReadHandleKey<xAOD::VertexContainer> m_PrimaryVxInputName{ this,"primaryVertexInputName","HLT_IDVertex_FS","Input Vertex Collection" };
    
    // OTHER PURPOSES to follow
     

};
#endif

