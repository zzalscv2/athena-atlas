# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# EGAM10.py
# This defines DAOD_EGAM10, a skimmed DAOD format for Run 3.
# Inclusive photon reduction - for e/gamma photon studies 
# (migrated from r21 STDM2)
# It requires the flag EGAM10 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent )

electronRequirements = ' && '.join(['(Electrons.pt > 15*GeV)',
                                    '(abs(Electrons.eta) < 2.5)', 
                                    '(Electrons.DFCommonElectronsLHLoose)'])
photonRequirements = ' && '.join(['(DFCommonPhotons_et >= 15*GeV)',
                                  '(abs(DFCommonPhotons_eta) < 2.5)'])


def PhotonPointingToolCfg(ConfigFlags):
    acc = ComponentAccumulator()
    acc.setPrivateTools( CompFactory.CP.PhotonPointingTool(
        name = 'EGAM10_PhotonPointingTool',
        isSimulation = ConfigFlags.Input.isMC  ) )
    return acc


def PhotonVertexSelectionWrapperCfg(ConfigFlags, **kwargs):
    acc = ComponentAccumulator()
    if "PhotonPointingTool" not in kwargs:
        photonPointingTool = acc.popToolsAndMerge( 
            PhotonPointingToolCfg(ConfigFlags) )
        kwargs.setdefault("PhotonPointingTool", photonPointingTool)
    acc.setPrivateTools( 
        CompFactory.DerivationFramework.PhotonVertexSelectionWrapper( **kwargs )
    )
    return acc


def EGAM10SkimmingToolCfg(flags):
    '''Configure the EGAM10 skimming tool'''
    acc = ComponentAccumulator()

    # off-line based selection
    photonSelection = '(count(' + photonRequirements + ') >= 1)'
    print('EGAM10 offline skimming expression: ', photonSelection)
    EGAM10_OfflineSkimmingTool = \
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = 'EGAM10_OfflineSkimmingTool',
            expression = photonSelection)

    # trigger-based selection

    # 2015 data triggers. unprescaled list validated with 
    # https://twiki.cern.ch/twiki/bin/viewauth/Atlas/LowestUnprescaled#Egamma_MET.
    # full list avaiable here: https://svnweb.cern.ch/trac/atlasoff/browser/Trigger/TriggerCommon/TrigMenuRulebook/trunk/python/Physics_pp_v5_rules.py (good luck)
    singlePhotonTriggers = ['HLT_g10_loose',
                            'HLT_g15_loose_L1EM7',
                            'HLT_g20_loose_L1EM12',
                            'HLT_g25_loose_L1EM15 ',
                            'HLT_g35_loose_L1EM15',
                            'HLT_g25_loose',
                            'HLT_g25_medium',
                            'HLT_g35_loose',
                            'HLT_g35_medium',
                            'HLT_g40_loose_L1EM15',
                            'HLT_g45_loose_L1EM15',
                            'HLT_g50_loose_L1EM15',
                            'HLT_g50_loose',
                            'HLT_g60_loose',
                            'HLT_g70_loose',
                            'HLT_g80_loose',
                            'HLT_g100_loose',
                            'HLT_g120_loose',
                            'HLT_g140_loose',
                            'HLT_g140_loose_HLTCalo',
                            'HLT_g200_etcut',
                            'HLT_g10_etcut',
                            'HLT_g20_etcut_L1EM12',
                            'HLT_g300_etcut_L1EM24VHIM',
                            'HLT_g22_tight',
                            'HLT_g25_medium_L1EM22VHI',
                            'HLT_g35_loose_L1EM22VHI',
                            'HLT_g45_tight_L1EM22VHI',
                            'HLT_g35_loose_L1EM24VHI',
                            'HLT_g35_loose_L1EM26VHI',
                            'HLT_g10_medium',
                            'HLT_g15_loose_L1EM3',
                            'HLT_g15_loose',
                            'HLT_g20_loose',
                            'HLT_g20_tight',
                            'HLT_g40_tight',
                            'HLT_g45_tight',
                            'HLT_g60_loose_L1EM15VH ',
                            'HLT_g180_loose',
                            'HLT_g60_loose_L1EM24VHI',
                            'HLT_g70_loose_L1EM24VHI',
                            'HLT_g80_loose_L1EM24VHI',
                            'HLT_g100_loose_L1EM24VHI',
                            'HLT_g120_loose_L1EM24VHI',
                            'HLT_g60_loose_L1EM26VHI',
                            'HLT_g70_loose_L1EM26VHI',
                            'HLT_g80_loose_L1EM26VHI',
                            'HLT_g100_loose_L1EM26VHI',
                            'HLT_g120_loose_L1EM26VHI',
                            'HLT_g140_loose_L1EM26VHI',
                            'HLT_g160_loose_L1EM26VHI',
                            'HLT_g180_loose_L1EM26VHI',
                            'HLT_200_loose_L1EM26VHI',
                            'HLT_g20_loose_L1EM18VH',
                            'HLT_g24_loose',
                            'HLT_g35_medium_L1EM22VHI',
                            'HLT_g35_medium_L1EM24VHI',
                            'HLT_g10_loose_L1EM3',
                            'HLT_g10_medium_L1EM3',
                            'HLT_g140_tight_L1EM24VHIM',
                            'HLT_g200_loose_L1EM24VHIM',
                            'HLT_g20_tight_L1EM15VHI',
                            'HLT_g20_tight_icalovloose_L1EM15VHI',
                            'HLT_g20_tight_icalotight_L1EM15VHI',
                            'HLT_g22_tight_L1EM15VHI',
                            'HLT_g22_tight_icalovloose_L1EM15VHI',
                            'HLT_g22_tight_icalotight_L1EM15VHI',
                            'HLT_g25_loose_L1EM20VH',
                            'HLT_g12_loose',
                            'HLT_g12_medium',
                            'HLT_g70_loose_L1EN24VHIM',
                            'HLT_g80_loose_L1EM24VHIM',
                            'HLT_g80_loose_icalovloose_L1EM24VHIM',
                            'HLT_g60_loose_L1EM24VHIM',
                            'HLT_g100_loose_L1EM24VHIM',
                            'HLT_g120_loose_L1EM24VHIM',
                            'HLT_g140_loose_L1EM24VHIM',
                            'HLT_g160_loose_L1EM24VHIM',
                            'HLT_g180_loose_L1EM24VHIM',
                            'HLT_g35_loose_L1EM24VHIM',
                            'HLT_g35_tight_icalotight_L1EM24VHIM',
                            'HLT_g40_tight_icalotight_L1EM24VHIM',
                            'HLT_g85_tight_L1EM24VHIM',
                            'HLT_g85_tight_icalovloose_L1EM24VHIM',
                            'HLT_g100_tight_L1EM24VHIM',
                            'HLT_g100_tight_icalovloose_L1EM24',
                            'HLT_g45_tight_L1EM24VHI',
                            'HLT_g300_etcut_L1EM24VHI',
                            'HLT_g85_tight_L1EM24VHI',
                            'HLT_g100_tight',
                            'HLT_g100_tight_L1EM24VHI',
                            'HLT_g100_tight_icalovloose_L1EM24VHIM',
                            'HLT_g70_loose_L1EM24VHIM',
                            'HLT_g85_tight',
                            'HLT_g6_loose',
                            'HLT_g6_tight_icalotight',
                            'HLT_g25_tight_L1EM20VH',
                            'HLT_g15_loose_L1EM8VH',
                            'HLT_g50_loose_L1EM20VH',
                            'HLT_g60_loose_L1EM20VH',
                            'HLT_g25_medium_L1EM20VH',
                            'HLT_g35_medium_L1EM20VH',
                            'HLT_g35_loose_L1EM20VH',
                            'HLT_g22_tight_icalovloose',
                            'HLT_g22_tight_icalotight',
                            'HLT_g35_tight_icalotight_L1EM24VHI',
                            'HLT_g40_tight_icalotight_L1EM24VHI',
                            'HLT_g85_tight_icalovloose_L1EM24VHI',
                            'HLT_g100_tight_icalovloose_L1EM24VHI',
                            'HLT_g35_medium_icalovloose',
                            'HLT_g35_medium_icalotight',
                            'HLT_g15_etcut_L1EM7',
                            'HLT_g20_medium_L1EM15',
                            'HLT_g20_tight_L1EM15',
                            'HLT_g20_etcut_L1EM15',
                            'HLT_g20_medium',
                            'HLT_g20_etcut',
                            'HLT_g25_medium_L1EM15',
                            'HLT_g25_tight_L1EM15',
                            'HLT_g25_etcut_L1EM15',
                            'HLT_g30_loose_L1EM15',
                            'HLT_g30_etcut_L1EM15']


    diPhotonTriggers = ['HLT_2g20_loose_L12EM15',
                        'HLT_2g20_loose',
                        'HLT_2g20_tight',
                        'HLT_2g22_tight',
                        'HLT_2g25_tight',
                        'HLT_g35_loose_g25_loose',
                        'HLT_g35_medium_HLTCalo_g25_medium_HLTCalo',
                        'HLT_g35_loose_L1EM15_g25_loose_L1EM15',
                        'HLT_g35_loose_L1EM15VH_g25_loose_L1EM15VH',
                        'HLT_g35_medium_g25_medium',
                        'HLT_2g50_loose',
                        'HLT_2g60_loose_L12EM15VH ',
                        'HLT_2g10_loose',
                        'HLT_2g50_loose_L12EM18VH',
                        'HLT_2g60_loose_L12EM18VH',
                        'HLT_2g50_loose_L12EM20VH',
                        'HLT_g50_loose_L12EM18VH',
                        'HLT_g60_loose_L12EM18VH',
                        'HLT_g50_loose_L12EM20VH',
                        'HLT_g60_loose_L12EM20VH',
                        'HLT_2g25_tight_L12EM20VH',
                        'HLT_g35_loose_g25_loose_L12EM18VH',
                        'HLT_g35_loose_g25_loose_L12EM20VH ',
                        'HLT_g35_medium_g25_medium_L12EM18VH',
                        'HLT_g35_medium_g25_medium_L12EM20VH',
                        'HLT_2g20_tight_L12EM15VHI',
                        'HLT_2g20_tight_icalovloose_L12EM15VHI',
                        'HLT_2g20_tight_icalotight_L12EM15VHI',
                        'HLT_2g22_tight_L12EM15VHI',
                        'HLT_2g22_tight_icalovloose_L12EM15VHI',
                        'HLT_2g22_tight_icalotight_L12EM15VHI',
                        'HLT_2g60_loose_L12EM20VH',
                        'HLT_2g3_loose_dPhi15_L12EM3_VTE50',
                        'HLT_2g3_loose_L12EM3_VTE50',
                        'HLT_2g3_medium_dPhi15_L12EM3_VTE50',
                        'HLT_2g22_tight_icalovloose',
                        'HLT_2g22_tight_icalotight',
                        'HLT_2g10_loose_L12EM7',
                        'HLT_2g15_loose_L12EM7']


    triPhotonTriggers = ['HLT_3g15_loose',
                         'HLT_g20_loose_2g15_loose_L12EM13VH',
                         'HLT_2g20_loose_g15_loose',
                         'HLT_3g20_loose',
                         'HLT_3g20_loose_L12EM18VH',
                         'HLT_2g24_loose_g15_loose',
                         'HLT_2g24_g20_loose',
                         'HLT_3g24_loose_L12EM20VH',
                         'HLT_2g25_loose_g15_loose',
                         'HLT_2g25_loose_g20_loose',
                         'HLT_3g25_loose']
    
    firstDataTriggers = ['HLT_2g10_loose',
                         'HLT_g20_loose_L1EM15']



    # 2016 data triggers (preliminary)
    # full list avaiable here: https://svnweb.cern.ch/trac/atlasoff/browser/Trigger/TriggerCommon/TriggerMenu/trunk/python/menu/Physics_pp_v6.py (good luck)
    singlePhotonTriggers_2016 = ['HLT_g140_loose',
                                 'HLT_g160_loose',
                                 'HLT_g200_loose',
                                 'HLT_g140_tight',
                                 'HLT_g250_etcut',
                                 'HLT_g300_etcut']

    diPhotonTriggers_2016 = ['HLT_g35_medium_g25_medium',
                             'HLT_2g50_loose',
                             'HLT_2g60_loose_L12EM15VH',
                             'HLT_2g20_tight',
                             'HLT_2g22_tight']

    triPhotonTriggers_2016 = ['HLT_2g20_loose_g15_loose',
                              'HLT_3g20_loose']
    
    noalgTriggers = ['HLT_noalg_L1EM12',
                     'HLT_noalg_L1EM15',
                     'HLT_noalg_L1EM18VH',
                     'HLT_noalg_L1EM20VH',
                     'HLT_noalg_L1EM10',
                     'HLT_noalg_L1EM10VH',
                     'HLT_noalg_L1EM13VH',
                     'HLT_noalg_L1EM20VHI',
                     'HLT_noalg_L1EM22VHI',
                     'HLT_noalg_L1EM8VH',
                     'HLT_noalg_L1EM15VH',
                     'HLT_noalg_L12EM7',
                     'HLT_noalg_L12EM15']

    allTriggers = singlePhotonTriggers + diPhotonTriggers + triPhotonTriggers +\
                  firstDataTriggers + singlePhotonTriggers_2016 +\
                  diPhotonTriggers_2016 + triPhotonTriggers_2016 +\
                  noalgTriggers

    #remove duplicates
    allTriggers = list(set(allTriggers))

    # can't use trigger API (https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI) because we also need prescaled triggers
    print('EGAM10 trigger skimming list (OR): ', allTriggers)
    EGAM10_TriggerSkimmingTool = \
        CompFactory.DerivationFramework.TriggerSkimmingTool(
            name = 'EGAM10_TriggerSkimmingTool',
            TriggerListOR = allTriggers)

    # do the AND of trigger-based and offline-based selection
    print('EGAM10 skimming is logical AND of previous selections')
    EGAM10_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(
        name = 'EGAM10_SkimmingTool',
        FilterList=[EGAM10_OfflineSkimmingTool, EGAM10_TriggerSkimmingTool])

    acc.addPublicTool(EGAM10_OfflineSkimmingTool)
    acc.addPublicTool(EGAM10_TriggerSkimmingTool)    
    acc.addPublicTool(EGAM10_SkimmingTool, primary = True)
    
    return acc


def EGAM10KernelCfg(ConfigFlags, name='EGAM10Kernel', **kwargs):
    '''Configure the derivation framework driving algorithm (kernel) 
       for EGAM10'''
    acc = ComponentAccumulator()


    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import ( 
        PhysCommonAugmentationsCfg )
    acc.merge( PhysCommonAugmentationsCfg(
        ConfigFlags,
        TriggerListsHelper = kwargs['TriggerListsHelper'] ) )
    

    # EGAM10 augmentations
    augmentationTools = []

    #====================================================================
    # Max Cell energy and time
    #====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        MaxCellDecoratorCfg )
    MaxCellDecorator = acc.popToolsAndMerge(MaxCellDecoratorCfg(ConfigFlags))
    acc.addPublicTool(MaxCellDecorator)
    augmentationTools.append(MaxCellDecorator)  
   


    #====================================================================
    # PhotonVertexSelectionWrapper decoration tool - needs PhotonPointing tool
    #====================================================================
    EGAM10_PhotonPointingTool = acc.popToolsAndMerge(
        PhotonPointingToolCfg(ConfigFlags))
            
    EGAM10_PhotonVertexSelectionWrapper = acc.popToolsAndMerge(
        PhotonVertexSelectionWrapperCfg(
            ConfigFlags,
            name = 'EGAM10_PhotonVertexSelectionWrapper',
            PhotonPointingTool = EGAM10_PhotonPointingTool,
            DecorationPrefix = 'EGAM10',
            PhotonContainer = 'Photons',
            VertexContainer = 'PrimaryVertices') )
    acc.addPublicTool(EGAM10_PhotonVertexSelectionWrapper)
    augmentationTools += [EGAM10_PhotonVertexSelectionWrapper]


    # thinning tools
    thinningTools = []

    # Track thinning
    if ConfigFlags.Derivation.Egamma.doTrackThinning:

        streamName = kwargs['StreamName']

        TrackThinningKeepElectronTracks = True
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepAllElectronTracks = True

        # Tracks associated with high-pT Electrons (deltaR=0.6)
        if (TrackThinningKeepElectronTracks):
            EGAM10ElectronTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM10ElectronTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Electrons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = electronRequirements,
                    BestMatchOnly = True,
                    ConeSize = 0.6)
            acc.addPublicTool(EGAM10ElectronTPThinningTool)
            thinningTools.append(EGAM10ElectronTPThinningTool)

        # Tracks associated with Photons
        if (TrackThinningKeepPhotonTracks):
            EGAM10PhotonTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM10PhotonTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Photons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = photonRequirements,
                    BestMatchOnly = False,
                    ConeSize = 0.6)
            acc.addPublicTool(EGAM10PhotonTPThinningTool)
            thinningTools.append(EGAM10PhotonTPThinningTool)

        # Tracks associated with all Electrons (for ambiguity resolver tool)
        if (TrackThinningKeepAllElectronTracks):
            EGAM10ElectronTPThinningToolAR = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM10ElectronTPThinningToolAR',
                    StreamName = streamName,
                    SGKey = 'Electrons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = electronRequirements,
                    BestMatchOnly = True)
            acc.addPublicTool(EGAM10ElectronTPThinningToolAR)
            thinningTools.append(EGAM10ElectronTPThinningToolAR)


    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM10SkimmingToolCfg(ConfigFlags))


    # setup the kernel
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,
                                      SkimmingTools = [skimmingTool],
                                      AugmentationTools = augmentationTools,
                                      ThinningTools = thinningTools) )

    return acc



def EGAM10Cfg(ConfigFlags):

    acc = ComponentAccumulator()


    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # TODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    EGAM10TriggerListsHelper = TriggerListsHelper()
    #EGAM10TriggerListsHelper.Run3TriggerNames = EGAM10TriggerListsHelper.Run3TriggerNamesNoTau
    #EGAM10TriggerListsHelper.Run3TriggerNamesTau = []
    EGAM10TriggerListsHelper.Run2TriggerNames = EGAM10TriggerListsHelper.Run2TriggerNamesNoTau
    EGAM10TriggerListsHelper.Run2TriggerNamesTau = []

    # configure skimming/thinning/augmentation tools
    acc.merge(EGAM10KernelCfg(ConfigFlags,
                             name = 'EGAM10Kernel',
                             StreamName = 'StreamDAOD_EGAM10',
                             TriggerListsHelper = EGAM10TriggerListsHelper))
    

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    EGAM10SlimmingHelper = SlimmingHelper(
        'EGAM10SlimmingHelper',
        NamesAndTypes = ConfigFlags.Input.TypedCollections )


    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM10SlimmingHelper.AllVariables =[
        'CaloCalTopoClusters']

    # and on MC we also add:
    if ConfigFlags.Input.isMC:
        EGAM10SlimmingHelper.AppendToDictionary.update(
            {"TruthIsoCentralEventShape"    : "xAOD::EventShape",
             "TruthIsoCentralEventShapeAux" : "xAOD::EventShapeAuxInfo",
             "TruthIsoForwardEventShape"    : "xAOD::EventShape",
             "TruthIsoForwardEventShapeAux" : "xAOD::EventShapeAuxInfo"}
        )
        EGAM10SlimmingHelper.AllVariables += [
            'TruthEvents',
            'TruthParticles',
            'TruthVertices',
            'TruthMuons',
            'TruthElectrons',
            'TruthPhotons',
            'TruthNeutrinos',
            'TruthTaus',
            'AntiKt4TruthJets',
            'AntiKt4TruthDressedWZJets',
            'egammaTruthParticles',
            'TruthIsoCentralEventShape',
            'TruthIsoForwardEventShape']




    # -------------------------------------------
    # containers that we slim
    # -------------------------------------------

    # first add variables from smart-slimming
    # adding only also those for which we add all variables since
    # the XXXCPContent.py files also bring in some extra variables 
    # for other collections
    # muons, tau, MET, b-tagging could be switched off if not needed
    # and use too much space
    EGAM10SlimmingHelper.SmartCollections = ['Electrons',
                                             'Photons',
                                             'InDetTrackParticles',
                                             'PrimaryVertices',
                                             'AntiKt4EMPFlowJets' ]

    if ConfigFlags.Input.isMC:
        EGAM10SlimmingHelper.SmartCollections += ['AntiKt4TruthJets',
                                                  'AntiKt4TruthDressedWZJets']

    # then add extra variables:

    # egamma clusters
    EGAM10SlimmingHelper.ExtraVariables += [
        'egammaClusters.PHI2CALOFRAME.ETA2CALOFRAME.phi_sampl']

    # photons
    EGAM10SlimmingHelper.ExtraVariables += [
        'Photons.core57cellsEnergyCorrection.topoetcone20.topoetcone30',
        'Photons.topoetcone40.ptcone20.ptcone30.ptcone40.f3.f3core',
        'Photons.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId',
        'Photons.maxEcell_x.maxEcell_y.maxEcell_z',
        'Photons.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500',
        'Photons.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000', 
        'Photons.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500']

    # electrons
    EGAM10SlimmingHelper.ExtraVariables += [
        'Electrons.topoetcone20.topoetcone30.topoetcone40.ptcone20.ptcone30',
        'Electrons.ptcone40.maxEcell_time.maxEcell_energy.maxEcell_gain',
        'Electrons.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z']

    # primary vertices
    EGAM10SlimmingHelper.ExtraVariables += [
        'PrimaryVertices.covariance.trackWeights.sumPt2.EGAM10_sumPt',
        'PrimaryVertices.EGAM10_sumPt2.EGAM10_pt.EGAM10_eta.EGAM10_phi']

    # tracks
    EGAM10SlimmingHelper.ExtraVariables += [
        'InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights']

    # photons and electrons: detailed shower shape variables and track variables
    EGAM10SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent
    
    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import (
        getGainDecorations, getClusterEnergyPerLayerDecorations )
    GainDecoratorTool = None
    ClusterEnergyPerLayerDecorators = []  
    for toolStr in acc.getEventAlgo('EGAM10Kernel').AugmentationTools:
        toolStr  = f'{toolStr}'
        splitStr = toolStr.split('/')
        tool =  acc.getPublicTool(splitStr[1])
        if splitStr[0] == 'DerivationFramework::GainDecorator':
            GainDecoratorTool = tool
        elif splitStr[0] == 'DerivationFramework::ClusterEnergyPerLayerDecorator':
            ClusterEnergyPerLayerDecorators.append( tool )

    if GainDecoratorTool : 
        EGAM10SlimmingHelper.ExtraVariables.extend(
            getGainDecorations(GainDecoratorTool) )
    for tool in ClusterEnergyPerLayerDecorators:
        EGAM10SlimmingHelper.ExtraVariables.extend(
            getClusterEnergyPerLayerDecorations( tool ) )

    # energy density
    EGAM10SlimmingHelper.ExtraVariables += [ 
        'TopoClusterIsoCentralEventShape.Density',
        'TopoClusterIsoForwardEventShape.Density',
    ]

    from DerivationFrameworkEGamma import EGammaIsoConfig
    pflowIsoVar,densityList,densityDict,acc1 = \
        EGammaIsoConfig.makeEGammaCommonIsoCfg(ConfigFlags)
    acc.merge(acc1)
    EGAM10SlimmingHelper.AppendToDictionary.update(densityDict)
    EGAM10SlimmingHelper.ExtraVariables += \
        densityList + [f'Photons{pflowIsoVar}'] 


    # truth
    if ConfigFlags.Input.isMC:
        EGAM10SlimmingHelper.ExtraVariables += [
            'Electrons.truthOrigin.truthType.truthParticleLink.truthPdgId',
            'Electrons.lastEgMotherTruthType.lastEgMotherTruthOrigin',
            'Electrons.lastEgMotherTruthParticleLink.lastEgMotherPdgId',
            'Electrons.firstEgMotherTruthType.firstEgMotherTruthOrigin',
            'Electrons.firstEgMotherTruthParticleLink.firstEgMotherPdgId']

        EGAM10SlimmingHelper.ExtraVariables += [
            'Photons.truthOrigin.truthType.truthParticleLink' ]

        EGAM10SlimmingHelper.ExtraVariables += [
            'TruthIsoCentralEventShape.DensitySigma.Density.DensityArea',
            'TruthIsoForwardEventShape.DensitySigma.Density.DensityArea']

    # Add event info
    if ConfigFlags.Derivation.Egamma.doEventInfoSlimming:
        EGAM10SlimmingHelper.SmartCollections.append('EventInfo')
    else:
        EGAM10SlimmingHelper.AllVariables += ['EventInfo']    
    
    # Add egamma trigger objects
    EGAM10SlimmingHelper.IncludeEGammaTriggerContent = True

    # Add trigger matching info
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            AddRun2TriggerMatchingToSlimmingHelper )
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper = EGAM10SlimmingHelper,
            OutputContainerPrefix = 'TrigMatch_', 
            TriggerList = EGAM10TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper = EGAM10SlimmingHelper, 
            OutputContainerPrefix = 'TrigMatch_',
            TriggerList = EGAM10TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import (
            AddRun3TrigNavSlimmingCollectionsToSlimmingHelper )
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM10SlimmingHelper)


    EGAM10ItemList = EGAM10SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags,
                              'DAOD_EGAM10',
                              ItemList = EGAM10ItemList,
                              AcceptAlgs = ['EGAM10Kernel']))

    return acc
    
