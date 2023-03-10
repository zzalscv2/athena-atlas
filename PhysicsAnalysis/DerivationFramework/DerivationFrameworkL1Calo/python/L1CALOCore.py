#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# L1CALOCore.py
# Define the list of containers for the L1Calo derivations
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from AthenaCommon.Constants import INFO
from AthenaConfiguration.ComponentFactory import CompFactory

# 
def L1CALOCoreCfg(flags, deriv='L1CALO1', **kwargs):
    """
    Core fragment for L1CALO derivations
    """
    from AthenaCommon.Logging import logging
    log = logging.getLogger('L1CALO')
    log.info('Called L1CaloCore config for derivation %s',deriv)

    streamNameStem = "DAOD_" + deriv
    streamName =  "Stream" + streamNameStem 

    acc = ComponentAccumulator()

    # decode the legacy L1Calo information - required because flags.Trigger.doLVL1 is False
    from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
    acc.merge(LVL1CaloRun2ReadBSCfg(flags))


    # set up thinning tools
    thinningTools = []

    # set up legacy trigger tower thinning for L1CALO1
    if 'L1CALO1' in deriv:
        from TrigT1CaloCalibTools.L1CaloCalibToolsConfig import LegacyTriggerTowerThinningCfg
        LegacyTowerThinningTool = acc.getPrimaryAndMerge(LegacyTriggerTowerThinningCfg(
            flags,
            name = "L1CALOCaloThinningTool",
            StreamName = streamName,
            TriggerTowerLocation = "xAODTriggerTowers",
            MinCaloCellET = 0.8,
            MinADC = 36,
            UseRandom = True,
            MinRandom = 0.01 ) )
        thinningTools.append(LegacyTowerThinningTool)

    # set up decorators
    # Legacy Trigger Tower decorator
    from TrigT1CaloCalibTools.L1CaloCalibToolsConfig import LegacyTriggerTowerDecoratorCfg
    LegacyTriggerTowerDecoratorAlg = LegacyTriggerTowerDecoratorCfg(flags, name = 'L1CaloTriggerTowerDecorator')
    acc.getPrimaryAndMerge(LegacyTriggerTowerDecoratorAlg)
    
    #
    augmentationTools = []

    # Set up the derivation kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name = "DFL1CALO_KERN",
        AugmentationTools = augmentationTools,
        ThinningTools = thinningTools,
        OutputLevel = INFO))

    # Phase 1 setup
    # emulate/decorate the input towers 
    # first need to decode LATOME readout
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg
    acc.merge(ReadSCellFromByteStreamCfg(flags))

    # Emulate jFEX input towers
    emulatedDataTowersKey = "L1_jFexEmulatedTowers"
    if flags.Trigger.L1.dojFex:
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
        acc.merge(jFexEmulatedTowersCfg(flags,'jFexEmulatedTowerMaker',emulatedDataTowersKey))

    # Decorate any jFEX data towers
    if flags.Trigger.L1.dojFex:
        from L1CaloFEXAlgos.L1CaloFEXAlgosConfig import L1CaloFEXDecoratorCfg
        acc.merge(L1CaloFEXDecoratorCfg(flags,'jFexTower2SCellDecorator','L1_jFexDataTowers'))

    # Decorate the emulated jFEX towers
    if flags.Trigger.L1.dojFex:
        acc.merge(L1CaloFEXDecoratorCfg(flags,'jFexTower2SCellEmulatedDecorator',emulatedDataTowersKey))

    # Emulate eFEX input towers
    if flags.Trigger.L1.doeFex:
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import eFexEmulatedTowersCfg
        eFexEmulatedTool = eFexEmulatedTowersCfg(flags,'L1_eFexEmulatedTowers')
        acc.merge(eFexEmulatedTool)

    # decorate the eFEX TOBs (offline copy)
    if flags.Trigger.L1.doeFex:
        from L1CaloFEXAlgos.L1CaloFEXAlgosConfig import eFexTOBDecoratorCfg
        DecoratorAlgo = eFexTOBDecoratorCfg(flags,'eFexTOBDecorator','L1_eEMRoI_OfflineCopy','L1_eTauRoI_OfflineCopy')
        acc.merge(DecoratorAlgo)

    # Re-simulate from LATOME
    from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
    acc.merge(L1CaloFEXSimCfg(flags))

    # set up the slimming helper
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    L1CaloSlimmingHelper = SlimmingHelper("L1CaloSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    L1CaloSlimmingHelper.IncludeTriggerNavigation = True   # Trigger info is actually stored only when running on data...
    L1CaloSlimmingHelper.IncludeAdditionalTriggerContent = True 

    # Container selection based on share/L1CALO versions
    # Note: if the container is in the on-the-fly list (ContainersOnTheFly.py) then we do not have to add it to the dictionary
    # We can do smart slimming if the container is in the smart list (FullListOfSmartContainers.py)

    # Generic event info
    L1CaloSlimmingHelper.AppendToDictionary.update({"EventInfo":"xAOD::EventInfo","EventInfoAux":"xAOD::EventAuxInfo"}) 

    # We keep all of EventInfo rather than smart slim
    AllVariables += ["EventInfo","Kt4EMTopoOriginEventShape"]

    # Physics Objects 
    # Those for which there is smart slimming but not in the on the fly list
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"Muons":"xAOD::MuonContainer", "MuonsAux":"xAOD::MuonAuxContainer",
         "Photons":"xAOD::PhotonContainer", "PhotonsAux":"xAOD::PhotonAuxContainer",
         "TauJets":"xAOD::TauJetContainer", "TauJetsAux":"xAOD::TauJetAuxContainer"}
    )
    SmartCollections += ["AntiKt4EMTopoJets","Muons","Photons","TauJets"]
    
    # Use unslimmed electrons in order to use Likelihood qualities
    StaticContent += ["xAOD::ElectronContainer#Electrons"]
    StaticContent += ["xAOD::ElectronAuxContainer#ElectronsAux."]

    # Missing ET - unslimmed container
    AllVariables += ["METAssoc_AntiKt4EMTopo"]
        
    # using MET slimming as per share/L1CALOX.py
    StaticContent += ["xAOD::MissingETContainer#MET_Reference_AntiKt4EMTopo"]
    StaticContent += ["xAOD::MissingETAuxContainer#MET_Reference_AntiKt4EMTopoAux.-ConstitObjectLinks.-ConstitObjectWeights"]

    StaticContent += ["xAOD::MissingETContainer#MET_Reference_AntiKt4EMTopo"]
    StaticContent += ["xAOD::MissingETAuxContainer#MET_Core_AntiKt4EMTopoAux.name.mpx.mpy.sumet.source"]

    StaticContent += ["xAOD::MissingETContainer#MET_Track"]
    StaticContent += ["xAOD::MissingETAuxContainer#MET_TrackAux.name.mpx.mpy"]

    # Primary vertices
    StaticContent += ["xAOD::VertexContainer#PrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"]

    # Egamma CP additions
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"egammaTopoSeededClusters":"xAOD::CaloClusterContainer","egammaTopoSeededClustersAux":"xAOD::CaloClusterAuxContainer"}
    )
    AllVariables += ["egammaTopoSeededClusters"]
    
    # GSF vertices and tracks
    StaticContent += ["xAOD::VertexContainer#GSFConversionVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#GSFConversionVerticesAux."]
    # we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex"]

    trackParticleAuxExclusions="-caloExtension.-cellAssociation.-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
    StaticContent += ["xAOD::TrackParticleContainer#GSFTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#GSFTrackParticlesAux."+trackParticleAuxExclusions]

    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"egammaClusters":"xAOD::CaloClusterContainer",
         "egammaClustersAux":"xAOD::CaloClusterAuxContainer",
         "TauPi0Clusters":"xAOD::CaloClusterContainer",
         "TauPi0ClustersAux":"xAOD::CaloClusterAuxContainer",
         "CaloCalTopoClusters":"xAOD::CaloClusterContainer",
         "CaloCalTopoClustersAux":"xAOD::CaloClusterAuxContainer",
         "MuonSegments":"xAOD::MuonSegmentContainer",
         "MuonSegmentsAux":"xAOD::MuonSegmentAuxContainer"}
    )
    AllVariables += ["egammaClusters","TauPi0Clusters","CaloCalTopoClusters","MuonSegments"]

    # L1Calo information
    # Legacy RoI Containers
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"LVL1EmTauRoIs":"xAOD::EmTauRoIContainer",
         "LVL1EmTauRoIsAux":"xAOD::EmTauRoIAuxContainer",
         "LVL1EnergySumRoI":"xAOD::EnergySumRoI",
         "LVL1EnergySumRoIAux":"xAOD::EnergySumRoIAuxInfo",
         "LVL1JetEtRoI":"xAOD::JetEtRoI",
         "LVL1JetEtRoIAux":"xAOD::JetEtRoIAuxInfo",
         "LVL1JetRoIs":"xAOD::JetRoIContainer",
         "LVL1JetRoIsAux":"xAOD::JetRoIAuxContainer",
         "LVL1MuonRoIs":"xAOD::MuonRoIContainer",
         "LVL1MuonRoIsAux":"xAOD::MuonRoIAuxContainer"}
    )
    AllVariables += [ "LVL1EmTauRoIs","LVL1EnergySumRoI","LVL1JetEtRoI","LVL1JetRoIs","LVL1MuonRoIs"]

    # Legacy sub-systems
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"JEMTobRoIsRoIB":"xAOD::JEMTobRoIContainer",
         "JEMTobRoIsRoIBAux":"xAOD::JEMTobRoIAuxContainer",
         "JEMTobRoIs":"xAOD::JEMTobRoIContainer",
         "JEMTobRoIsAux":"xAOD::JEMTobRoIAuxContainer",
         "JEMEtSums":"xAOD::JEMEtSumsContainer",
         "JEMEtSumsAux":"xAOD::JEMEtSumsAuxContainer"}
    )
    AllVariables += ["JEMTobRoIsRoIB",
                     "JEMTobRoIs",
                     "JEMEtSums"]
    
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"CMXCPHits":"xAOD::CMXCPHitsContainer",
         "CMXCPHitsAux":"xAOD::CMXCPHitsAuxContainer",
         "CMXCPTobs":"xAOD::CMXCPTobContainer",
         "CMXCPTobsAux":"xAOD::CMXCPTobAuxContainer",
         "CMXEtSums":"xAOD::CMXEtSumsContainer",
         "CMXEtSumsAux":"xAOD::CMXEtSumsAuxContainer",
         "CMXJetHits":"xAOD::CMXJetHitsContainer",
         "CMXJetHitsAux":"xAOD::CMXJetHitsAuxContainer",
         "CMXJetTobs":"xAOD::CMXJetTobContainer",
         "CMXJetTobsAux":"xAOD::CMXJetTobAuxContainer",
         "CMXRoIs":"xAOD::CMXRoIContainer",
         "CMXRoIsAux":"xAOD::CMXRoIAuxContainer"}
    )
    AllVariables += ["CMXCPHits",
                     "CMXCPTobs",
                     "CMXEtSums",
                     "CMXJetHits",
                     "CMXJetTobs",
                     "CMXRoIs"]


    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"CPMTobRoIs":"xAOD::CPMTobRoIContainer",
         "CPMTobRoIsAux":"xAOD::CPMTobRoIAuxContainer",
         "CPMTobRoIsRoIB":"xAOD::CPMTobRoIContainer",
         "CPMTobRoIsRoIBAux":"xAOD::CPMTobRoIAuxContainer",
         "CPMTowers":"xAOD::CPMTowerContainer",
         "CPMTowersAux":"xAOD::CPMTowerAuxContainer",
         "CPMTowersOverlap":"xAOD::CPMTowerContainer",
         "CPMTowersOverlapAux":"xAOD::CPMTowerAuxContainer",
         "RODHeaders":"xAOD::RODHeaderContainer",
         "RODHeadersAux":"xAOD::RODHeaderAuxContainer",
         "xAODTriggerTowers":"xAOD::TriggerTowerContainer",
         "xAODTriggerTowersAux":"xAOD::TriggerTowerAuxContainer",
         "JetElements":"xAOD::JetElementContainer",
         "JetElementsAux":"xAOD::JetElementAuxContainer",
         "JetElementsOverlap":"xAOD::JetElementContainer",
         "JetElementsOverlapAux":"xAOD::JetElementAuxContainer",
         "L1TopoRawData":"xAOD::L1TopoRawDataContainer",
         "L1TopoRawDataAux":"xAOD::L1TopoRawDataAuxContainer"}
    )
    AllVariables += ["CPMTobRoIs",
                     "CPMTobRoIsRoIB",
                     "CPMTowers",
                     "CPMTowersOverlap",
                     "RODHeaders",
                     "xAODTriggerTowers",
                     "JetElements",
                     "JetElementsOverlap",
                     "L1TopoRawData"]

    
    # Phase 1
    # TOBs from HLT
    L1CaloSlimmingHelper,AllVariables = addEfexTOBs(L1CaloSlimmingHelper, AllVariables)
    L1CaloSlimmingHelper,AllVariables = addJfexTOBs(L1CaloSlimmingHelper, AllVariables)
    L1CaloSlimmingHelper,AllVariables = addGfexTOBs(L1CaloSlimmingHelper, AllVariables)
    # TOBs from reconstruction (_OfflineCopy) - will be removed once fully commissioned
    L1CaloSlimmingHelper,AllVariables = addEfexTOBs(L1CaloSlimmingHelper, AllVariables, "_OfflineCopy")
    L1CaloSlimmingHelper,AllVariables = addJfexTOBs(L1CaloSlimmingHelper, AllVariables, "_OfflineCopy")
    L1CaloSlimmingHelper,AllVariables = addGfexTOBs(L1CaloSlimmingHelper, AllVariables, "_OfflineCopy")

    # re-simulated 
    L1CaloSlimmingHelper,AllVariables = addEfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")
    L1CaloSlimmingHelper,AllVariables = addJfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")
    L1CaloSlimmingHelper,AllVariables = addGfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")


    # FEX input data towers
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_eFexDataTowers":"xAOD::eFexTowerContainer",
         "L1_eFexDataTowersAux":"xAOD::eFexTowerAuxContainer",
         "L1_jFexDataTowers":"xAOD::jFexTowerContainer",
         "L1_jFexDataTowersAux":"xAOD::jFexTowerAuxContainer",
         "L1_gFexDataTowers":"xAOD::gFexTowerContainer",
         "L1_gFexDataTowersAux":"xAOD::gFexTowerAuxContainer"}
    )
    AllVariables += ["L1_eFexDataTowers","L1_jFexDataTowers","L1_gFexDataTowers"]

    # Emulated eFEX input tower data from LATOME
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_eFexEmulatedTowers":"xAOD::eFexTowerContainer",
         "L1_eFexEmulatedTowersAux":"xAOD::eFexTowerAuxContainer"}
    )
    AllVariables += ["L1_eFexEmulatedTowers"]
    
    # Emulated jFEX input tower data from LATOME    
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_jFexEmulatedTowers":"xAOD::jFexTowerContainer",
         "L1_jFexEmulatedTowersAux":"xAOD::jFexTowerAuxContainer"}
    )    
    AllVariables += ["L1_jFexEmulatedTowers"]

    
    L1CaloSlimmingHelper.AllVariables = AllVariables
    L1CaloSlimmingHelper.StaticContent = StaticContent
    L1CaloSlimmingHelper.SmartCollections = SmartCollections
    L1CaloSlimmingHelper.ExtraVariables = ExtraVariables
    
    # Output stream    
    L1CaloItemList = L1CaloSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, streamNameStem, ItemList=L1CaloItemList, AcceptAlgs=["DFL1CALO_KERN"]))

    return acc


def addEfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of eFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_eEMRoI"+postFix : "xAOD::eFexEMRoIContainer",
         "L1_eEMRoI"+postFix+"Aux" : "xAOD::eFexEMRoIAuxContainer",
         "L1_eTauRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_cTauRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_cTauRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_eEMxRoI"+postFix : "xAOD::eFexEMRoIContainer",
         "L1_eEMxRoI"+postFix+"Aux" : "xAOD::eFexEMRoIAuxContainer",
         "L1_eTauxRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauxRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer"} )    

    allVariables += ["L1_eEMRoI" + postFix,
                     "L1_eTauRoI" + postFix,
                     "L1_cTauRoI" + postFix,
                     "L1_eEMxRoI" + postFix,
                     "L1_eTauxRoI" + postFix]
    
    return slimminghelper, allVariables

        
def addJfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of jFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_jFexMETRoI"+postFix : "xAOD::jFexMETRoIContainer",
         "L1_jFexMETRoI"+postFix+"Aux" : "xAOD::jFexMETRoIAuxContainer",
         "L1_jFexTauRoI"+postFix : "xAOD::jFexTauRoIContainer",
         "L1_jFexTauRoI"+postFix+"Aux" : "xAOD::jFexTauRoIAuxContainer",
         "L1_jFexFwdElRoI"+postFix : "xAOD::jFexFwdElRoIContainer",
         "L1_jFexFwdElRoI"+postFix+"Aux" : "xAOD::jFexFwdElRoIAuxContainer",
         "L1_jFexSRJetRoI"+postFix : "xAOD::jFexSRJetRoIContainer",
         "L1_jFexSRJetRoI"+postFix+"Aux" : "xAOD::jFexSRJetRoIAuxContainer",
         "L1_jFexLRJetRoI"+postFix : "xAOD::jFexLRJetRoIContainer",
         "L1_jFexLRJetRoI"+postFix+"Aux" : "xAOD::jFexLRJetRoIAuxContainer",
         "L1_jFexSumETRoI"+postFix : "xAOD::jFexSumETRoIContainer",
         "L1_jFexSumETRoI"+postFix+"Aux" : "xAOD::jFexSumETRoIAuxContainer",
         "L1_jFexMETxRoI"+postFix : "xAOD::jFexMETRoIContainer",
         "L1_jFexMETxRoI"+postFix+"Aux" : "xAOD::jFexMETRoIAuxContainer",
         "L1_jFexTauxRoI"+postFix : "xAOD::jFexTauRoIContainer",
         "L1_jFexTauxRoI"+postFix+"Aux" : "xAOD::jFexTauRoIAuxContainer",
         "L1_jFexFwdElxRoI"+postFix : "xAOD::jFexFwdElRoIContainer",
         "L1_jFexFwdElxRoI"+postFix+"Aux" : "xAOD::jFexFwdElRoIAuxContainer",
         "L1_jFexSRJetxRoI"+postFix : "xAOD::jFexSRJetRoIContainer",
         "L1_jFexSRJetxRoI"+postFix+"Aux" : "xAOD::jFexSRJetRoIAuxContainer",
         "L1_jFexLRJetxRoI"+postFix : "xAOD::jFexLRJetRoIContainer",
         "L1_jFexLRJetxRoI"+postFix+"Aux" : "xAOD::jFexLRJetRoIAuxContainer",
         "L1_jFexSumETxRoI"+postFix : "xAOD::jFexSumETRoIContainer",
         "L1_jFexSumETxRoI"+postFix+"Aux" : "xAOD::jFexSumETRoIAuxContainer"})

    allVariables += ["L1_jFexMETRoI" + postFix,
                     "L1_jFexTauRoI" + postFix,
                     "L1_jFexFwdElRoI" + postFix,
                     "L1_jFexSRJetRoI" + postFix,
                     "L1_jFexLRJetRoI" + postFix,
                     "L1_jFexSumETRoI" + postFix,
                     "L1_jFexMETxRoI" + postFix,
                     "L1_jFexTauxRoI" + postFix,
                     "L1_jFexFwdElxRoI" + postFix,
                     "L1_jFexSRJetxRoI" + postFix,
                     "L1_jFexLRJetxRoI" + postFix,
                     "L1_jFexSumETxRoI" + postFix]

    return slimminghelper, allVariables


def addGfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of gFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_gFexRhoRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexRhoRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gFexSRJetRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexSRJetRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gScalarEJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gScalarEJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gFexLRJetRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexLRJetRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gMETComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMETComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gMHTComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMHTComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gMSTComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMSTComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer"})

    allVariables += ["L1_gFexRhoRoI" + postFix,
                     "L1_gFexSRJetRoI" + postFix,
                     "L1_gScalarEJwoj" + postFix,
                     "L1_gFexLRJetRoI" + postFix,
                     "L1_gMETComponentsJwoj" + postFix,
                     "L1_gMHTComponentsJwoj" + postFix,
                     "L1_gMSTComponentsJwoj" + postFix]

    return slimminghelper, allVariables
