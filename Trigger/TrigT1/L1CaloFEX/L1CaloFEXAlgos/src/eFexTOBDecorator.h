/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTOBDecorator  -  description:
//       This algorithm decorates the eFEX TOBs with the recalculated isolation variables
//                              -------------------
//     begin                : 13 02 2023
//     email                : paul.daniel.thompson@cern.ch
//***************************************************************************/

#ifndef EFEXTOBDECORATORTOOL_H
#define EFEXTOBDECORATORTOOL_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"


#include "L1CaloFEXToolInterfaces/IeFEXTOBEtTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {
    
  class eFexTOBDecorator : public AthAlgorithm{
  public:
    eFexTOBDecorator(const std::string& name, ISvcLocator* svc);

    // Function initialising the algorithm
    virtual StatusCode initialize();
    // Function executing the algorithm
    virtual StatusCode execute();
    	
  private:
    const std::string m_ReadKeyEM_name = "L1_eEMRoI";
    const std::string m_ReadKeyTau_name = "L1_eTauRoI";

    // Readhandles for eFEX TOBs
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFEXegEDMContainerKey{this,"eFexEMRoIContainer",m_ReadKeyEM_name,"SG key of the input eFex RoI container"};
    SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFEXtauEDMContainerKey{this,"eFexTauRoIContainer",m_ReadKeyTau_name,"SG key of the input eFex Tau RoI container"};
    
    // WriteDecor handles for the EM RoI decorations
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RetaCoreDec { this, "RetaCoreDecDecorKey"  , m_ReadKeyEM_name+".RetaCoreDec"  , "Recalculated EM RetaCore" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RetaEnvDec  { this, "RetaEnvDecDecorKey"   , m_ReadKeyEM_name+".RetaEnvDec"   , "Recalculated EM RetaEnv"  };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RhadEMDec   { this, "RetaEMDecDecorKey"    , m_ReadKeyEM_name+".RhadEMDec"    , "Recalculated EM RetaEM"   };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RhadHadDec  { this, "RhadHadDecDecorKey"   , m_ReadKeyEM_name+".RhadHadDec"   , "Recalculated EM RhadHad"  };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_WstotDenDec { this, "WstotDenDecDecorKey"  , m_ReadKeyEM_name+".WstotDenDec"  , "Recalculated EM WstotDen" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_WstotNumDec { this, "WstotNumDecDecorKey"  , m_ReadKeyEM_name+".WstotNumDec"  , "Recalculated EM WstotNum" };

    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_ClusterEtSumPSDec { this, "ClusterEtSumPSDecorKey"  , m_ReadKeyEM_name+".ClusterEtSumPS"  , "Cluster ET sum PS PS" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_ClusterEtSumL1Dec { this, "ClusterEtSumL1DecorKey"  , m_ReadKeyEM_name+".ClusterEtSumL1"  , "Cluster ET sum PSL1" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_ClusterEtSumL2Dec { this, "ClusterEtSumL2DecorKey"  , m_ReadKeyEM_name+".ClusterEtSumL2"  , "Cluster ET sum PSL2" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_ClusterEtSumL3Dec { this, "ClusterEtSumL3DecorKey"  , m_ReadKeyEM_name+".ClusterEtSumL3"  , "Cluster ET sum PSL3" };


    // WriteDecor handles for the Tau RoI decorations
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_RCoreDec   { this, "RCoreDecorKey"   , m_ReadKeyTau_name+".RCoreDec"   , "Recalculated Tau RCore" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REnvDec    { this, "REnvDecorKey"    , m_ReadKeyTau_name+".REnvDec"    , "Recalculated Tau REnv" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REMCoreDec { this, "REMCoreDecorKey" , m_ReadKeyTau_name+".REMCoreDec" , "Recalculated Tau REMCore" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REMHadDec  { this, "REMHadDecorKey"  , m_ReadKeyTau_name+".REMHadDec"  , "Recalculated Tau REMHad" };

    ToolHandle<IeFEXTOBEtTool> m_eFEXTOBEtTool {this, "eFEXTOBEtTool", "LVL1::eFEXTOBEtTool", "Tool for reconstructing TOB ET sums"};
  };
}
#endif
