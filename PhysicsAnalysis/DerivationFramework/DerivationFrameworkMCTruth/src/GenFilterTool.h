/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file GenFilterTool.h
 * @author TJ Khoo
 * @date July 2015
 * @brief tool to decorate EventInfo with quantities needed to disentangle generator filtered samples
*/

#ifndef DerivationFrameworkMCTruth_GenFilterTool_H
#define DerivationFrameworkMCTruth_GenFilterTool_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/SystemOfUnits.h"

// EDM include -- typedef, so has to be included
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
// Defs for the particle origin
#include "MCTruthClassifier/MCTruthClassifierDefs.h"

class IMCTruthClassifier;

namespace DerivationFramework {

  class GenFilterTool : public AthAlgTool, public IAugmentationTool {

  public:
    GenFilterTool(const std::string& t, const std::string& n, const IInterface* p);
    ~GenFilterTool();
    virtual StatusCode addBranches() const override final;
    virtual StatusCode initialize() override final;

  private:
    StatusCode getGenFiltVars(const EventContext& ctx, float& genFiltHT, float& genFiltHTinclNu, float& genFiltMET, float& genFiltPTZ, float& genFiltFatJ) const;

    bool isPrompt( const xAOD::TruthParticle* tp ) const;


    SG::ReadHandleKey<xAOD::EventInfo>m_eventInfoKey{this,"EventInfoName" , "EventInfo"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer>m_mcKey{this, "MCCollectionName", "TruthParticles"};
    SG::ReadHandleKey<xAOD::JetContainer>m_truthJetsKey{this, "TruthJetCollectionName", "AntiKt4TruthWZJets"};
    SG::ReadHandleKey<xAOD::JetContainer>m_truthFatJetsKey{this, "TruthFatJetCollectionName", "AntiKt10TruthJets"};

    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_mcReadDecor{this, "TruthClassKey",""};
    

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_decorKeys{this, "DecorationKeys", {} , "Decorations added to the eventinfo"};
    Gaudi::Property<float> m_MinJetPt{this, "MinJetPt", 35.* Gaudi::Units::GeV};  //!< Min pT for the truth jets
    Gaudi::Property<float> m_MaxJetEta{this, "MaxJetEta", 2.5}; //!< Max eta for the truth jets
    Gaudi::Property<float> m_MinLepPt{this,"MinLeptonPt", 25.*Gaudi::Units::GeV};  //!< Min pT for the truth leptons
    Gaudi::Property<float> m_MaxLepEta{this, "MaxLeptonEta", 2.5}; //!< Max eta for the truth leptons

    PublicToolHandle<IMCTruthClassifier> m_classif{this, "TruthClassifier", "MCTruthClassifier/DFCommonTruthClassifier"};
  }; /// class

} /// namespace

#endif
