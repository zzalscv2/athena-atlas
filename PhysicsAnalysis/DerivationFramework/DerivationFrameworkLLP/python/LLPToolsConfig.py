# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for LLP Derivations
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Thinning for VSI tracks
def VSITrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    VSITrackParticleThinning = CompFactory.DerivationFramework.VSITrackParticleThinning
    acc.addPublicTool(VSITrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Thinning for Std tracks in jets
def JetTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    JetTrackParticleThinning = CompFactory.DerivationFramework.JetTrackParticleThinning
    acc.addPublicTool(JetTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Thinning for LRT tracks in jets
def JetLargeD0TrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    JetLargeD0TrackParticleThinning = CompFactory.DerivationFramework.JetLargeD0TrackParticleThinning
    acc.addPublicTool(JetLargeD0TrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc


def LLP1TriggerSkimmingToolCfg(ConfigFlags, name, **kwargs):

    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType

    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(ConfigFlags)
    trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
    trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
    trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
    trig_elmu = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
    trig_mug = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.g,  livefraction=0.8)

    trig_VBF_2018 =["HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490", "HLT_j45_gsc55_bmv2c1070_split_2j45_320eta490_L1J25.0ETA23_2J15.31ETA49", "HLT_j80_0eta240_j60_j45_320eta490_AND_2j35_gsc45_bmv2c1070_split", "HLT_ht300_2j40_0eta490_invm700_L1HT150-J20s5.ETA31_MJJ-400-CF_AND_2j35_gsc45_bmv2c1070_split", "HLT_j70_j50_0eta490_invm1100j70_dphi20_deta40_L1MJJ-500-NFF"]

    triggers = trig_el + trig_mu + trig_g + trig_elmu + trig_mug + trig_VBF_2018
    #remove duplicates
    triggers = sorted(list(set(triggers)))

    acc = ComponentAccumulator()
    TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
    acc.addPublicTool(TriggerSkimmingTool(name, 
                                          TriggerListAND = [],
                                          TriggerListOR  = triggers,
                                          **kwargs),
                                          primary = True)
    return acc


def LLP1TriggerMatchingToolRun2Cfg(ConfigFlags, name, **kwargs):
    """Configure the common trigger matching for run 2 DAODs"""

    triggerList = kwargs['TriggerList']
    outputContainerPrefix = kwargs['OutputContainerPrefix']
    
    kwargs.setdefault('InputElectrons', 'LRTElectrons')
    kwargs.setdefault('InputMuons', 'MuonsLRT')
    kwargs.setdefault('DRThreshold', None) 

    acc = ComponentAccumulator()

    # Create trigger matching decorations
    from DerivationFrameworkTrigger.TriggerMatchingToolConfig import TriggerMatchingToolCfg
    if kwargs['DRThreshold'] is None:
        PhysCommonTriggerMatchingTool = acc.getPrimaryAndMerge(TriggerMatchingToolCfg(
            ConfigFlags,
            name=name,
            ChainNames = triggerList,
            OutputContainerPrefix = outputContainerPrefix,
            InputElectrons = kwargs['InputElectrons'],
            InputMuons =  kwargs['InputMuons'])) 
    else:
        PhysCommonTriggerMatchingTool = acc.getPrimaryAndMerge(TriggerMatchingToolCfg(
            ConfigFlags,
            name=name,
            ChainNames = triggerList,
            OutputContainerPrefix = outputContainerPrefix,  
            DRThreshold = kwargs['DRThreshold'],
            InputElectrons = kwargs['InputElectrons'],
            InputMuons =  kwargs['InputMuons'])) 
    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation(f"{outputContainerPrefix}TriggerMatchingKernel",
                                        AugmentationTools=[PhysCommonTriggerMatchingTool]))
    return(acc)

def LRTMuonMergerAlg(ConfigFlags, name="LLP1_MuonLRTMergingAlg", **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.CP.MuonLRTMergingAlg(name, **kwargs)
    acc.addEventAlgo(alg, primary=True)
    return acc

def LRTElectronMergerAlg(ConfigFlags, name="LLP1_ElectronLRTMergingAlg", **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.CP.ElectronLRTMergingAlg(name, **kwargs)
    acc.addEventAlgo(alg, primary=True)
    return acc

# Electron LLH setup for LLP1
def LRTElectronLHSelectorsCfg(ConfigFlags):

    acc = ComponentAccumulator()

    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu
    from ROOT import LikeEnum

    lhMenu = electronLHmenu.offlineMC21
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as geoFlags
    if geoFlags.Run() == "RUN2" :
        lhMenu = electronLHmenu.offlineMC20

    ElectronLHSelectorVeryLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        ConfigFlags,
        name="ElectronLHSelectorVeryLooseNoPix",
        quality=LikeEnum.VeryLooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorVeryLooseNoPix.primaryVertexContainer = "PrimaryVertices"
    
    ElectronLHSelectorLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        ConfigFlags,
        name="ElectronLHSelectorLooseNoPix",
        quality=LikeEnum.LooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorLooseNoPix.primaryVertexContainer = "PrimaryVertices"

    ElectronLHSelectorMediumNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        ConfigFlags,
        name="ElectronLHSelectorMediumNoPix",
        quality=LikeEnum.MediumLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorMediumNoPix.primaryVertexContainer = "PrimaryVertices"

    ElectronLHSelectorTightNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        ConfigFlags,
        name="ElectronLHSelectorTightNoPix",
        quality=LikeEnum.TightLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorTightNoPix.primaryVertexContainer = "PrimaryVertices"

    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronLikelihoodToolWrapperCfg

    # decorate electrons with the output of LH very loose
    ElectronPassLHVeryLooseNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHVeryLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=True))

    ElectronPassLHVeryLooseNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHVeryLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=True))

    # decorate electrons with the output of LH loose
    ElectronPassLHLooseNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,   
        name="ElectronPassLHLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHLooseNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,   
        name="ElectronPassLHLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    # decorate electrons with the output of LH medium
    ElectronPassLHMediumNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHMediumNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHMediumNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHMediumNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    # decorate electrons with the output of LH tight
    ElectronPassLHTightNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHTightNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHTightNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        ConfigFlags,
        name="ElectronPassLHTightNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    LRTEGAugmentationTools = [ElectronPassLHVeryLooseNoPix,
                              ElectronPassLHVeryLooseNoPixLRT,
                              ElectronPassLHLooseNoPix,
                              ElectronPassLHLooseNoPixLRT,
                              ElectronPassLHMediumNoPix,
                              ElectronPassLHMediumNoPixLRT,
                              ElectronPassLHTightNoPix,
                              ElectronPassLHTightNoPixLRT]

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        "LLP1EGammaLRTKernel",
        AugmentationTools=LRTEGAugmentationTools
    ))

    return acc
