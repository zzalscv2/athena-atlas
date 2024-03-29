/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetCaloClusterThinning.h, (c) ATLAS Detector software
// @author Danilo E. Ferreira de Lima <dferreir@cern.ch>
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORKCALO_JETCALOCLUSTERTHINNING_H
#define DERIVATIONFRAMEWORKCALO_JETCALOCLUSTERTHINNING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include <atomic>
#include <string>

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

class JetCaloClusterThinning
  : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool>
{
public:
  JetCaloClusterThinning(const std::string& t,
                         const std::string& n,
                         const IInterface* p);
  virtual ~JetCaloClusterThinning();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode doThinning() const override;

private:
  mutable std::atomic<unsigned int> m_ntotTopo,
    m_npassTopo; //, m_ntotFrwd, m_npassFrwd;

  StringProperty m_streamName{ this,
                               "StreamName",
                               "",
                               "Name of the stream being thinned" };
  SG::ThinningHandleKey<xAOD::CaloClusterContainer>
    m_TopoClSGKey{ this, "TopoClCollectionSGKey", "CaloCalTopoCluster", "" };
  SG::ReadHandleKey<xAOD::JetContainer>
    m_sgKey{ this, "SGKey", "", "SG key of jet container to thin" };

  Gaudi::Property<std::vector<std::string>> m_addClusterSGKey{ this, "AdditionalClustersKey", {}, ""};
  SG::ThinningHandleKey<xAOD::CaloClusterContainer> m_tmpAddClusterKey{ this, "TmpAddClustersKey","LCOriginTopoClusters",""};
  std::vector<SG::ThinningHandleKey<xAOD::CaloClusterContainer>> m_addClusterKeys;

  std::string m_selectionString;

  void setJetClustersMask(std::vector<bool>&, const xAOD::JetContainer*&) const;
  void setJetClustersMask(std::vector<bool>&, std::vector<const xAOD::Jet*>&) const;

};

}

#endif // DERIVATIONFRAMEWORK_JETCALOCLUSTERTHINNING_H
