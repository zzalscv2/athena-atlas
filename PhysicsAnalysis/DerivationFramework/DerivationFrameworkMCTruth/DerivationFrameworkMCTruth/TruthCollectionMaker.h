/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthParticleContainer.h"
// Handles and keys
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
// Exprssion evaluator
#include "ExpressionEvaluation/ExpressionParserUser.h"

// Forward declarations
class StoreGateSvc;

namespace DerivationFramework {

  class TruthCollectionMaker : public ExpressionParserUser<AthAlgTool>, public IAugmentationTool {
    public: 
      TruthCollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthCollectionMaker();
      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      mutable std::atomic<unsigned int> m_ntotpart, m_npasspart;
      // R/W handles
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
         {this, "ParticlesKey", "TruthParticles", "ReadHandleKey for input TruthParticleContainer"};
      SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outputParticlesKey
         {this, "NewCollectionName", "", "WriteHandleKey for new TruthParticleContainer"};
      // Non-handle properties
      Gaudi::Property<std::string> m_partString
         {this, "ParticleSelectionString", "", "ExpressionEvaluation string for particle selection"};
      Gaudi::Property<bool> m_do_compress
         {this, "Do_Compress", false, "Removes particles with the same pdgId in a decay chain (but keeps first and last)"};
      Gaudi::Property<bool> m_do_sherpa 
         {this, "Do_Sherpa", false, "Checks if there are truth W bosons in the current record.  If not, tries to combine W daughters to create one"};
      Gaudi::Property<bool> m_keep_navigation_info
         {this, "KeepNavigationInfo", true, "m_do_sherpa currently only works for W+jets"}; 
      // Decor handles
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_linkDecoratorKey
         {this, "originalTruthParticle", "TruthParticles.originalTruthParticle", "Name of the decoration linking to the orgiginal truth particle"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
         {this, "classifierParticleOrigin", "TruthParticles.classifierParticleOrigin", "Name of the decoration which records the particle origin as determined by the MCTruthClassifier"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey
         {this, "classifierParticleType", "TruthParticles.classifierParticleType", "Name of the decoration which records the particle type as determined by the MCTruthClassifier"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey
         {this, "classifierParticleOutCome", "TruthParticles.classifierParticleOutCome", "Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey
         {this, "Classification", "TruthParticles.Classification", "Name of the decoration which records the particle classification as determined by the MCTruthClassifier"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_motherIDDecoratorKey
         {this, "motherID", "TruthParticles.motherID", "Name of the decoration which records the ID of the particle's mother"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_daughterIDDecoratorKey
         {this, "daughterID", "TruthParticles.daughterID", "Name of the decoration which records the ID of the particle's daughter"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_hadronOriginDecoratorKey
         {this, "TopHadronOriginFlag", "TruthParticles.TopHadronOriginFlag", "Name of the decoration which records the origin of hadrons from top decays"};

      ServiceHandle<StoreGateSvc> m_metaStore; //!< Handle on the metadata store for init
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKER_H
