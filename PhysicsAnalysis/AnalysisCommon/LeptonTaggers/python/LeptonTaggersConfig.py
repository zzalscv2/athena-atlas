# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
lep_tag_log = logging.getLogger('LeptonTaggersConfig')

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# TO DELETE:
#
# How LeptonTagger was called in MUON5:
#
# import LeptonTaggers.LeptonTaggersConfig as LepTagConfig
# if not hasattr(MUON5Seq,"Muons_decoratePromptLepton"):
#     LepTagConfig.ConfigureAntiKt4PV0TrackJets(MUON5Seq,"MUON1")
#     MUON5Seq += LepTagConfig.GetDecoratePromptLeptonAlgs()
#     MUON5Seq += LepTagConfig.GetDecorateImprovedPromptLeptonAlgs()
#
# MUON5SlimmingHelper.ExtraVariables += LepTagConfig.GetExtraPromptVariablesForDxAOD(onlyBDT=False)
# MUON5SlimmingHelper.ExtraVariables += LepTagConfig.GetExtraImprovedPromptVariablesForDxAOD()
# MUON5SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent


def VtxFittingToolCfg(ConfigFlags, **kwargs) -> ComponentAccumulator:
    """
    Generate a vertex fitting tool.
    """
    acc = ComponentAccumulator()

    kwargs.setdefault("doSeedVertexFit", False)

    kwargs.setdefault("DistToPriVtxName", "distToPriVtx")
    kwargs.setdefault("NormDistToPriVtxName", "normDistToPriVtx")
    kwargs.setdefault("DistToRefittedPriVtxName", "distToRefittedPriVtx")
    kwargs.setdefault("NormDistToRefittedPriVtxName", "normDistToRefittedPriVtx")
    kwargs.setdefault("DistToRefittedRmLepPriVtxName", "distToRefittedRmLepPriVtx")
    kwargs.setdefault("NormDistToRefittedRmLepPriVtxName",
                      "normDistToRefittedRmLepPriVtx")

    from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
    fast_vtx_fitter = acc.popToolsAndMerge(FastVertexFitterCfg(ConfigFlags))

    kwargs.setdefault("vertexFitterTool", fast_vtx_fitter)
    kwargs.setdefault("seedVertexFitterTool", fast_vtx_fitter)

    vertex_fitting_tool = CompFactory.Prompt.VertexFittingTool(
        name="VertexFittingTool", **kwargs
    )

    acc.setPrivateTools(vertex_fitting_tool)

    return acc


def DecorateReFitPrimaryVertexCfg(
    ConfigFlags, name="PrimaryVertexReFitter",
    lepton_type="", **kwargs
) -> ComponentAccumulator:
    """
    CA to run the PrimaryVertexReFitter algorithm.
    """
    if lepton_type not in ['Electrons', 'Muons']:
        raise ValueError(f'DecorateReFitPrimaryVertex - unknown lepton type: "{lepton_type}"')

    acc = ComponentAccumulator()

    kwargs.setdefault("ReFitPriVtxName", f"RefittedPriVtx_{lepton_type}")
    kwargs.setdefault("LeptonContainerName", lepton_type)

    kwargs.setdefault("DistToRefittedPriVtxName", "distToRefittedPriVtx")
    kwargs.setdefault("NormDistToRefittedPriVtxName", "normDistToRefittedPriVtx")
    kwargs.setdefault("RefittedVtxLinkName", "RefittedPriVtxLink")
    kwargs.setdefault("RefittedVtxWithoutLeptonLinkName",
                      f"RefittedPriVtxWithoutLepton_{lepton_type}")

    alg_name = f'PrimaryVertexReFitter_{lepton_type}_decoratePriVtx'

    kwargs.setdefault("VertexFittingTool", acc.popToolsAndMerge(
        VtxFittingToolCfg(ConfigFlags)
    ))
    the_alg = CompFactory.Prompt.PrimaryVertexReFitter(alg_name, **kwargs)

    acc.addEventAlgo(the_alg, primary = True)
    return acc


def VtxItrMergingToolCfg(flags, name="VertexIterativeMergingTool",
                         lepton_name="", **kwargs) -> ComponentAccumulator:
    """
    Generate a VtxItrMergingTool configuration.
    """
    acc = ComponentAccumulator()

    kwargs.setdefault("minFitProb", 0.03)

    tool_name = f'VtxIterFitMergingTool_{lepton_name}'

    vtxItrMergingTool = CompFactory.Prompt.VertexIterativeFitMergingTool(
        name=tool_name, **kwargs
    )

    acc.setPrivateTools(vtxItrMergingTool)

    return acc


def DecorateNonPromptVertexCfg(flags, name="DecorateNonPromptVertex",
                               lepton_name="", **kwargs) -> ComponentAccumulator:
    """
    Configure the non-prompt vertex decorator.
    """
    if lepton_name not in ['Electrons', 'Muons']:
        raise ValueError(f'DecorateNonPromptVertex - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    vertex_fitting_tool = acc.popToolsAndMerge(VtxFittingToolCfg(flags))

    kwargs.setdefault("VertexFittingTool",vertex_fitting_tool)

    kwargs.setdefault(
        "VertexMergingTool",
        acc.popToolsAndMerge(VtxItrMergingToolCfg(
            flags, lepton_name=lepton_name,
            VertexFittingTool=vertex_fitting_tool
        ))
    )

    kwargs.setdefault("LeptonContainerName", lepton_name)
    kwargs.setdefault("ReFitPriVtxContainerName", f"RefittedPriVtx_{lepton_name}")
    kwargs.setdefault("SVContainerName", f"SecVtxContainer_{lepton_name}")

    kwargs.setdefault("SecVtxLinksName", "SecVtxLinks")
    kwargs.setdefault("DeepMergedSecVtxLinksName", "DeepMergedSecVtxLinks")
    kwargs.setdefault("NoLeptonPriVtxLinkName", f"RefittedPriVtxWithoutLepton_{lepton_name}")
    kwargs.setdefault("IndexVectorName", "PromptLeptonInput_SecondaryVertexIndexVector")

    alg = CompFactory.Prompt.NonPromptLeptonVertexingAlg(
        name=f'NonPromptVtx_decorate_{lepton_name}',
        **kwargs
    )

    acc.addEventAlgo(alg, primary=True)

    return acc


def RNNToolCfg(flags, name="RNNTool",
               RNN_name="RNN_name", lepton_name="",
               **kwargs) -> ComponentAccumulator:
    """
    Configure the RNN tool.
    """
    if lepton_name not in ["Electrons", "Muons"]:
        raise ValueError(f'RNNTool - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    #
    # Read configuration from AFS for this initial merge request, will switch to cvmfs with second request
    #
    if lepton_name == 'Electrons':
        kwargs.setdefault("configRNNVersion", 'InputData-2020-02-25/RNN/Electron')
        kwargs.setdefault("configRNNJsonFile",
                          'elecs_feb20_fullrun2_linear_ptraw_ntk5_model_ndense10_nhidden50_nepoch10_nbatch256_use_weights_nn-config.json')

    elif lepton_name == 'Muons':
        kwargs.setdefault("configRNNVersion", 'InputData-2020-02-25/RNN/Muon')
        kwargs.setdefault("configRNNJsonFile",
                          'muons_feb19_fullrun2_linear_ptraw_ntk5_model_ndense10_nhidden50_nepoch10_nbatch256_use_weights_nn-config.json')

    tool_rnn = CompFactory.Prompt.RNNTool(
        name=f'{RNN_name}_{lepton_name}_RNNTool', **kwargs
    )
    acc.setPrivateTools(tool_rnn)

    return acc


def DecoratePromptLeptonRNNCfg(flags, RNN_name="RNN_name", lepton_name="",
                               name="DecoratePromptLeptonRNN",
                               **kwargs) -> ComponentAccumulator:
    """
    Configure the prompt lepton RNN decorator.
    """
    if lepton_name not in ["Electrons", "Muons"]:
        raise ValueError(f'DecorateNonPromptVertex - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    #
    # Prepare DecoratePromptLepton alg
    #
    kwargs.setdefault("inputContainerLepton", lepton_name)
    kwargs.setdefault("inputContainerTrack", 'InDetTrackParticles')
    kwargs.setdefault("inputContainerTrackJet", 'AntiKtVR30Rmax4Rmin02PV0TrackJets')
    kwargs.setdefault("inputContainerPrimaryVertices", 'PrimaryVertices')

    kwargs.setdefault("decorationPrefixRNN", 'PromptLeptonRNN_')

    kwargs.setdefault("debug", False)
    kwargs.setdefault("outputStream", 'out')

    kwargs.setdefault("toolRNN", acc.popToolsAndMerge(
        RNNToolCfg(flags, RNN_name=RNN_name, lepton_name=lepton_name)
    ))

    alg = CompFactory.Prompt.DecoratePromptLeptonRNN(
        name=f'{lepton_name}_decorate_RNN_{RNN_name}', **kwargs
    )
    acc.addEventAlgo(alg, primary=True)

    lep_tag_log.info('Decorate%s - prepared %s algorithm for: %s',
                     RNN_name, RNN_name, lepton_name)

    return acc


def getStringIntVars(BDT_name):
    """
    Get the integer variables for the BDT (I think)
    """
    int_vars = []

    if BDT_name == 'PromptLeptonImprovedVeto':
        int_vars += ['MVAXBin']
    elif (BDT_name == 'PromptLeptonImprovedVetoBARR'
          or BDT_name == 'PromptLeptonImprovedVetoECAP'):
        int_vars += ['MVAXBin', 'TrackJetNTrack']
    else:
        raise ValueError(f'getStringIntVars - unknown alg: "{BDT_name}"')

    return int_vars


def getStringFloatVars(BDT_name, part_type=''):
    """
    Get the float variables for the BDT (I think)
    """
    float_vars = []

    if BDT_name == 'PromptLeptonImprovedVeto':
        float_vars += ['Topoetcone30rel',
                       'Ptvarcone30_TightTTVA_pt500rel',
                       'PromptLeptonRNN_prompt',
                       'PtFrac',
                       'DRlj',
                       'CaloClusterERel',
                       'CandVertex_normDistToPriVtxLongitudinalBest']
    elif (BDT_name == 'PromptLeptonImprovedVetoBARR'
          or BDT_name == 'PromptLeptonImprovedVetoECAP'):
        float_vars += ['Topoetcone30rel',
                       'Ptvarcone30rel',
                       'PromptLeptonRNN_prompt',
                       'PtFrac',
                       'DRlj',
                       'CaloClusterSumEtRel',
                       'PtRel',
                       'CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx']
    else:
        raise ValueError(f'getStringFloatVars - unknown alg: "{BDT_name}"')

    return float_vars


def DecoratePromptLeptonImprovedCfg(
    flags, BDT_name="", lepton_name="", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets",
    **kwargs
) -> ComponentAccumulator:
    """
    Configure the PromptLeptonImproved decorator.
    """
    #
    # Check track jet container is correct
    #
    if track_jet_name != 'AntiKtVR30Rmax4Rmin02PV0TrackJets':
        raise ValueError(f'Decorate{BDT_name} - unknown track jet collection: "{track_jet_name}"')

    acc = ComponentAccumulator()

    #
    # Prepare DecoratePromptLepton alg
    #
    kwargs.setdefault("LeptonContainerName", lepton_name)

    kwargs.setdefault("TrackJetContainerName", track_jet_name)
    kwargs.setdefault("PrimaryVertexContainerName", 'PrimaryVertices')
    kwargs.setdefault("ClusterContainerName", 'CaloCalTopoClusters')

    kwargs.setdefault("ConfigFileVersion", '')
    kwargs.setdefault("BDTName", BDT_name)
    kwargs.setdefault("InputVarDecoratePrefix", 'PromptLeptonImprovedInput_')
    kwargs.setdefault("PrintTime", False)
    kwargs.setdefault("OutputLevel", 3)


    #
    # Read configuration from AFS for this initial merge request, will switch to cvmfs with second request
    #
    if lepton_name == 'Electrons':
        kwargs.setdefault("MethodTitleMVA", f'BDT_Electron_{BDT_name}')
        kwargs.setdefault("ConfigFileVersion",
                          f'InputData-2020-02-25/BDT/Electron/{BDT_name}')
        kwargs.setdefault("accessorRNNVars", ['PromptLeptonRNN_prompt'])
    elif lepton_name == 'Muons':
        kwargs.setdefault("MethodTitleMVA", f'BDT_Muon_{BDT_name}')
        kwargs.setdefault("ConfigFileVersion",
                          f'InputData-2020-02-25/BDT/Muon/{BDT_name}')
        kwargs.setdefault("accessorRNNVars", ['PromptLeptonRNN_prompt'])
    else:
        raise ValueError(f'Decorate{BDT_name} - unknown lepton type: "{lepton_name}"')

    kwargs.setdefault("stringIntVars", getStringIntVars(BDT_name))
    kwargs.setdefault("stringFloatVars", getStringFloatVars(BDT_name))
    kwargs.setdefault("extraDecoratorFloatVars", ['RawPt'])
    kwargs.setdefault("extraDecoratorShortVars", ['CandVertex_NPassVtx'])
    kwargs.setdefault("vetoDecoratorFloatVars", ['PromptLeptonRNN_prompt'])
    kwargs.setdefault("vetoDecoratorShortVars", [])

    kwargs.setdefault("leptonPtBinsVector", [10.0e3, 15.0e3, 20.0e3, 25.0e3, 32.0e3, 43.0e3, 100.0e3])


    #
    # Secondary vertex selection for the PromptLeptonImproved
    #
    kwargs.setdefault("VertexLinkName", 'DeepMergedSecVtxLinks')

    alg = CompFactory.Prompt.DecoratePromptLeptonImproved(
        f'{lepton_name}_decorate{BDT_name}', **kwargs
    )
    acc.addEventAlgo(alg, primary=True)

    lep_tag_log.info(
        'Decorate%s - prepared %s algorithm for: %s, %s',
        BDT_name, BDT_name, lepton_name, track_jet_name
    )

    return acc


def DecorateImprovedPromptLeptonAlgsCfg(
    ConfigFlags, name="DecorateImprovedPromptLeptonAlgs",
    lepton_type="", **kwargs
) -> ComponentAccumulator:
    """
    CA to decorate with PLIV input algorithms
    """
    valid_lepton_types = ["", "Electrons", "Muons"]
    if lepton_type not in valid_lepton_types:
        lep_tag_log.error("Requested lepton type: %s", lepton_type)
        lep_tag_log.error("Allowed lepton types: %s", valid_lepton_types)
        raise ValueError('DecorateImprovedPromptLeptonAlgsCfg - '
                         + f'unknown lepton type: "{lepton_type}"')

    acc = ComponentAccumulator()

    if lepton_type in ["", "Electrons"]:
        acc.merge(DecorateReFitPrimaryVertexCfg(ConfigFlags, lepton_type="Electrons"))
        acc.merge(DecorateNonPromptVertexCfg(ConfigFlags, lepton_name="Electrons"))
        acc.merge(DecoratePromptLeptonRNNCfg(ConfigFlags,
                                             RNN_name='PromptLeptonRNN',
                                             lepton_name='Electrons'))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVetoBARR",
            lepton_name="Electrons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVetoECAP",
            lepton_name="Electrons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))

    if lepton_type in ["", "Muons"]:
        acc.merge(DecorateReFitPrimaryVertexCfg(ConfigFlags, lepton_type="Muons"))
        acc.merge(DecorateNonPromptVertexCfg(ConfigFlags, lepton_name="Muons"))
        acc.merge(DecoratePromptLeptonRNNCfg(ConfigFlags,
                                             RNN_name='PromptLeptonRNN',
                                             lepton_name='Muons'))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVeto",
            lepton_name="Muons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))

    return acc


# Script to run for testing the config
# from https://atlassoftwaredocs.web.cern.ch/guides/ca_configuration/ca/
if __name__ == "__main__":
    # import the flags and set them
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    FLAGS = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    FLAGS.Common.ProductionStep = ProductionStep.Derivation

    FLAGS.Exec.MaxEvents = 3

    # use one of the predefined files
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    FLAGS.Input.Files = defaultTestFiles.AOD_RUN3_MC
    FLAGS.fillFromArgs() # make the job understand command line options
    # lock the flags
    FLAGS.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    ACC = MainServicesCfg(FLAGS)

    # Pool file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    ACC.merge(PoolReadCfg(FLAGS))

    # --------------------
    # Common augmentations (needed for PLIV inputs)
    # --------------------
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    trigger_lists_helper = TriggerListsHelper(FLAGS)

    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    ACC.merge(PhysCommonAugmentationsCfg(FLAGS, TriggerListsHelper=trigger_lists_helper))

    # from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg
    # FTagJetColl = ['AntiKtVR30Rmax4Rmin02TrackJets']
    # ACC.merge(FtagJetCollectionsCfg(FLAGS,FTagJetColl))
    # ACC.merge(METCommonCfg(FLAGS))

    # add the algorithm to the configuration
    ACC.merge(DecorateImprovedPromptLeptonAlgsCfg(FLAGS))

    # debug printout
    ACC.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = ACC.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
