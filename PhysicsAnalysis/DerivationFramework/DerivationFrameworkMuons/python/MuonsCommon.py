# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# MuonsCommon.py 
# Schedules all tools needed for muon object selection and writes
# results into SG. These may then be accessed along the train   
#********************************************************************
from __future__ import print_function
def makeMuonsDFCommon(postfix=""):

   from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkJob
   from DerivationFrameworkMuons import DFCommonMuonsConfig
   from AthenaCommon.AppMgr import ToolSvc
   from AthenaCommon import CfgMgr 
   DFCommonMuonsTrtCutOff = DFCommonMuonsConfig.TrtCutOff
   
   #====================================================================
   # MCP GROUP TOOLS 
   #====================================================================
   
   #====================================================================
   # AUGMENTATION TOOLS 
   #====================================================================
   from MuonSelectorTools.MuonSelectorToolsConf import CP__MuonSelectionTool
   from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__AsgSelectionToolWrapper
   DFCommonMuonToolWrapperTools = []

   MuonCollection="Muons"
   if "LRT" in postfix:
    MuonCollection="MuonsLRT"
   
   ### IDHits
   from AthenaConfiguration.AllConfigFlags import ConfigFlags
   from AthenaConfiguration.Enums import LHCPeriod
   isRun3 = False
   if ConfigFlags.GeoModel.Run == LHCPeriod.Run3: isRun3 = True
   DFCommonMuonsSelector = CP__MuonSelectionTool(name = "DFCommonMuonsSelector"+postfix)
   DFCommonMuonsSelector.MaxEta = 3.
   DFCommonMuonsSelector.MuQuality = 3
   DFCommonMuonsSelector.IsRun3Geo = isRun3
   # turn of the momentum correction which is not needed for IDHits cut and Preselection
   DFCommonMuonsSelector.TurnOffMomCorr = True
   
   if DFCommonMuonsTrtCutOff is not None: DFCommonMuonsSelector.TrtCutOff = DFCommonMuonsTrtCutOff
   ToolSvc += DFCommonMuonsSelector
   print (DFCommonMuonsSelector)
   
   DFCommonMuonToolWrapperIDCuts = DerivationFramework__AsgSelectionToolWrapper( name = "DFCommonMuonToolWrapperIDCuts"+postfix,
                                                                           AsgSelectionTool = DFCommonMuonsSelector,
                                                                           CutType = "IDHits",
                                                                           StoreGateEntryName = "DFCommonMuonPassIDCuts",
                                                                           ContainerName = MuonCollection)
   #preselection
   ToolSvc += DFCommonMuonToolWrapperIDCuts
   print (DFCommonMuonToolWrapperIDCuts)
   DFCommonMuonToolWrapperTools.append(DFCommonMuonToolWrapperIDCuts)
   
   DFCommonMuonToolWrapperPreselection = DerivationFramework__AsgSelectionToolWrapper( name = "DFCommonMuonToolWrapperPreselection"+postfix,
                                                                           AsgSelectionTool = DFCommonMuonsSelector,
                                                                           CutType = "Preselection",
                                                                           StoreGateEntryName = "DFCommonMuonPassPreselection",
                                                                           ContainerName = MuonCollection)
   
   ToolSvc += DFCommonMuonToolWrapperPreselection
   print (DFCommonMuonToolWrapperPreselection)
   DFCommonMuonToolWrapperTools.append(DFCommonMuonToolWrapperPreselection)
   ### Decoration of the muon objects with the ID track covariances
   #from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import DerivationFramework__MuonIDCovMatrixDecorator
   #DFCommonMuonIDCovMatrixDecorator = DerivationFramework__MuonIDCovMatrixDecorator( name = "DFCommonMuonIDCovMatrixDecorator")
   #ToolSvc += DFCommonMuonIDCovMatrixDecorator
   #print (DFCommonMuonIDCovMatrixDecorator)
   #DFCommonMuonToolWrapperTools.append(DFCommonMuonIDCovMatrixDecorator)
   
   #############
   #  Add tools
   #############
   DerivationFrameworkJob += CfgMgr.DerivationFramework__CommonAugmentation("DFCommonMuons"+postfix+"Kernel",
                                                                            AugmentationTools = DFCommonMuonToolWrapperTools
                                                                           )
   
   from IsolationAlgs.IsoUpdatedTrackCones import GetUpdatedIsoTrackCones
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderLoose1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Loose")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNominal1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nominal")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderTight1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Tight")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderPrompt_D0Sig1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Prompt_D0Sig")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_Hard_D0Sig1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_Hard_D0Sig")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_Medium_D0Sig1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_Medium_D0Sig")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_All_D0Sig1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_All_D0Sig")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderPrompt_MaxWeight1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Prompt_MaxWeight")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_Hard_MaxWeight1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_Hard_MaxWeight")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_Medium_MaxWeight1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_Medium_MaxWeight")
   if not hasattr(DerivationFrameworkJob,"IsolationBuilderNonprompt_All_MaxWeight1000"+postfix):
       DerivationFrameworkJob += GetUpdatedIsoTrackCones(postfix=postfix, WP="Nonprompt_All_MaxWeight")

   if "LRT" in postfix and not hasattr(DerivationFrameworkJob, 'LRTMuonCaloIsolationBuilder'):
        from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        
        from IsolationAlgs.IsolationSteeringDerivConfig import (
            LRTMuonIsolationSteeringDerivCfg )
            
        CAtoGlobalWrapper(LRTMuonIsolationSteeringDerivCfg, ConfigFlags)
