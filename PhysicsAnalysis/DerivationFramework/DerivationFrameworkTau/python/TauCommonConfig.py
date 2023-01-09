# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddTauAugmentationCfg(ConfigFlags, **kwargs):

    prefix = kwargs["prefix"]
    kwargs.setdefault("doVeryLoose", False)
    kwargs.setdefault("doLoose",     False)
    kwargs.setdefault("doMedium",    False)
    kwargs.setdefault("doTight",     False)

    acc = ComponentAccumulator()

    # tau selection relies on RNN electron veto, we must decorate the fixed eveto WPs before applying tau selection
    acc.merge(AddTauWPDecorationCfg(ConfigFlags, evetoFix=True))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg

    TauAugmentationTools = []

    if kwargs["doVeryLoose"]:
        TauSelectorVeryLoose = acc.popToolsAndMerge(TauSelectionToolCfg(ConfigFlags,
                                                                        name = 'TauSelectorVeryLoose',
                                                                        ConfigPath = 'TauAnalysisAlgorithms/tau_selection_veryloose.conf'))
        acc.addPublicTool(TauSelectorVeryLoose)

        TauVeryLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(ConfigFlags,
                                                                                name               = "TauVeryLooseWrapper",
                                                                                AsgSelectionTool   = TauSelectorVeryLoose,
                                                                                StoreGateEntryName = "DFTauVeryLoose",
                                                                                ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauVeryLooseWrapper)

    if kwargs["doLoose"]:
        TauSelectorLoose = acc.popToolsAndMerge(TauSelectionToolCfg(ConfigFlags,
                                                                    name = 'TauSelectorLoose',
                                                                    ConfigPath = 'TauAnalysisAlgorithms/tau_selection_loose.conf'))
        acc.addPublicTool(TauSelectorLoose)

        TauLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(ConfigFlags,
                                                                            name               = "TauLooseWrapper",
                                                                            AsgSelectionTool   = TauSelectorLoose,
                                                                            StoreGateEntryName = "DFTauLoose",
                                                                            ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauLooseWrapper)

    if kwargs["doMedium"]:
        TauSelectorMedium = acc.popToolsAndMerge(TauSelectionToolCfg(ConfigFlags,
                                                                     name = 'TauSelectorMedium',
                                                                     ConfigPath = 'TauAnalysisAlgorithms/tau_selection_medium.conf'))
        acc.addPublicTool(TauSelectorMedium)

        TauMediumWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(ConfigFlags,
                                                                             name               = "TauMediumWrapper",
                                                                             AsgSelectionTool   = TauSelectorMedium,
                                                                             StoreGateEntryName = "DFTauMedium",
                                                                             ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauMediumWrapper)

    if kwargs["doTight"]:
        TauSelectorTight = acc.popToolsAndMerge(TauSelectionToolCfg(ConfigFlags,
                                                                    name = 'TauSelectorTight',
                                                                    ConfigPath = 'TauAnalysisAlgorithms/tau_selection_tight.conf'))
        acc.addPublicTool(TauSelectorTight)

        TauTightWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(ConfigFlags,
                                                                            name               = "TauTightWrapper",
                                                                            AsgSelectionTool   = TauSelectorTight,
                                                                            StoreGateEntryName = "DFTauTight",
                                                                            ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauTightWrapper)

    if TauAugmentationTools:
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation(f"{prefix}_TauAugmentationKernel", AugmentationTools = TauAugmentationTools))

    return acc


# Low pT di-taus
def AddDiTauLowPtCfg(ConfigFlags, **kwargs):
    """Configure the low-pt di-tau building"""

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    AntiKt10EMPFlow = AntiKt10LCTopo.clone(inputdef = cst.GPFlow)
    acc.merge(JetRecCfg(ConfigFlags,AntiKt10EMPFlow))

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderLowPtCfg
    acc.merge(DiTauBuilderLowPtCfg(ConfigFlags, name="DiTauLowPtBuilder"))

    return acc


# TauWP decoration
def AddTauWPDecorationCfg(ConfigFlags, **kwargs):
    """Recompute the tau working point decorations"""

    # the correct configurations are:
    # - TauContainerName="TauJets" and OverrideDecoration=False
    # - TauContainerName="TauJets_MuonRM" and OverrideDecoration=True
    kwargs.setdefault("evetoFix",           False)
    kwargs.setdefault("TauContainerName",   "TauJets")
    kwargs.setdefault("OverrideDecoration", False)

    acc = ComponentAccumulator()

    TauWPDecoratorWrapper = CompFactory.DerivationFramework.TauWPDecoratorWrapper
    TauWPDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

    if kwargs['evetoFix']:
        evetoTauWPDecorator = CompFactory.TauWPDecorator(name = "TauWPDecoratorEleRNN_v1",
                                                         flatteningFile1Prong = "rnneveto_mc16d_flat_1p_fix.root",
                                                         flatteningFile3Prong = "rnneveto_mc16d_flat_3p_fix.root",
                                                         DecorWPNames = [ "EleRNNLoose_v1", "EleRNNMedium_v1", "EleRNNTight_v1" ],
                                                         DecorWPCutEffs1P = [0.95, 0.90, 0.85],
                                                         DecorWPCutEffs3P = [0.98, 0.95, 0.90],
                                                         UseEleBDT = True,
                                                         ScoreName = "RNNEleScore",
                                                         NewScoreName = "RNNEleScoreSigTrans_v1",
                                                         DefineWPs = True )
        acc.addPublicTool(evetoTauWPDecorator)

        tauContainerName = kwargs['TauContainerName']
        evetoTauWPDecoratorWrapper = TauWPDecoratorWrapper(name               = f"TauWPDecorEvetoWrapper_{tauContainerName}",
                                                           TauContainerName   = tauContainerName,
                                                           TauWPDecorator     = evetoTauWPDecorator,
                                                           OverrideDecoration = kwargs["OverrideDecoration"])
        acc.addPublicTool(evetoTauWPDecoratorWrapper)
        acc.addEventAlgo(TauWPDecoratorKernel(name              = f"TauWPDecorKernel_{tauContainerName}",
                                              AugmentationTools = [evetoTauWPDecoratorWrapper]))

    return acc


# TauJets_MuonRM steering
def AddMuonRemovalTauAODReRecoAlgCfg(flags, **kwargs):
    """Configure the MuonRM AOD tau building"""

    acc = ComponentAccumulator()

    # get tools from holder
    import tauRec.TauToolHolder as tauTools
    tools_mod = []
    tools_mod.append( acc.popToolsAndMerge(tauTools.TauAODMuonRemovalCfg(flags)) )
    tools_after = []
    tools_after.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauTrackRNNClassifierCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.EnergyCalibrationLCCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauCommonCalcVarsCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauSubstructureCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ClusterCreatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ClusterScalerCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ScoreCalculatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0SelectorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauVertexVariablesCfg(flags)) )
    import PanTauAlgs.JobOptions_Main_PanTau_New as pantau
    tools_after.append( acc.popToolsAndMerge(pantau.PanTauCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauCombinedTESCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.MvaTESVariableDecoratorCfg(flags)) )
    tools_after[-1].EventShapeKey = ''
    tools_after.append( acc.popToolsAndMerge(tauTools.MvaTESEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauIDVarCalculatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauJetRNNEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorJetRNNCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauEleRNNEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauDecayModeNNClassifierCfg(flags)) )
    TauAODRunnerAlg=CompFactory.getComp("TauAODRunnerAlg")
    for tool in tools_mod:
        tool.inAOD = True
    for tool in tools_after:
        tool.inAOD = True
    myTauAODRunnerAlg = TauAODRunnerAlg(  
        name                           = "MuonRemovalTauAODReRecoAlg",
        Key_tauOutputContainer         = "TauJets_MuonRM",
        Key_pi0OutputContainer         = "TauFinalPi0s_MuonRM",
        Key_neutralPFOOutputContainer  = "TauNeutralParticleFlowObjects_MuonRM",
        Key_chargedPFOOutputContainer  = "TauChargedParticleFlowObjects_MuonRM",
        Key_hadronicPFOOutputContainer = "TauHadronicParticleFlowObjects_MuonRM",
        Key_tauTrackOutputContainer    = "TauTracks_MuonRM",
        Key_vertexOutputContainer      = "TauSecondaryVertices_MuonRM",
        modificationTools              = tools_mod,
        officialTools                  = tools_after
    )
    acc.addEventAlgo(myTauAODRunnerAlg)
    return acc
