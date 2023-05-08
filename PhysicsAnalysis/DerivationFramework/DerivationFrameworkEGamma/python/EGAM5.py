# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# EGAM5.py
# This defines DAOD_EGAM5, a skimmed DAOD format for Run 3.
# W->enu derivation
# It requires the flag EGAM5 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from AthenaCommon.SystemOfUnits import MeV, GeV

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent )

from DerivationFrameworkEGamma.TriggerContent import (
    WTnPTriggers, ExtraContainersTrigger,
    ExtraContainersElectronTrigger, ExtraContainersTriggerDataOnly )



def EGAM5SkimmingToolCfg(flags):
    '''Configure the EGAM5 skimming tool'''
    acc = ComponentAccumulator()


    # 1st selection: trigger-based (WTP triggers)
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = 'Run2'
    elif flags.Trigger.EDMVersion == 3:
        MenuType = 'Run3'
    else:
        MenuType = ''
    triggers = WTnPTriggers[MenuType]
    print('EGAM5 trigger skimming list (OR): ', triggers)
    
    EGAM5_TriggerSkimmingTool = \
        CompFactory.DerivationFramework.TriggerSkimmingTool(
            name = 'EGAM5_TriggerSkimmingTool',
            TriggerListOR = triggers)


    # 2nd selection: off-line, based on mT(enu)
    expression2 = 'count(EGAM5_ENuTransverseMass > 40*GeV)>=1'
    print('EGAM5 offline skimming expression: ', expression2)
    EGAM5_OfflineSkimmingTool = \
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = 'EGAM5_OfflineSkimmingTool',
            expression = expression2)


    # 3rd selection: mix of off-line and on-line criteria
    expression3a = ' || '. join(['HLT_e60_lhloose_xe60noL1',
                                 'HLT_e120_lhloose',
                                 'HLT_j80_xe80',
                                 'HLT_xe70',
                                 'HLT_xe80_tc_lcw_L1XE50',
                                 'HLT_xe90_mht_L1XE50',
                                 'HLT_xe90_tc_lcw_wEFMu_L1XE50',
                                 'HLT_xe90_mht_wEFMu_L1XE50'])
    expression3b = 'count(Electrons.pt > 14.5*GeV) >= 1'
    expression3 = '( ' + expression3a + ' ) && ( ' + expression3b + ' )'
    print('EGAM5 mixed offline-online skimming expression: ', expression3)

    EGAM5_OnlineOfflineSkimmingTool = \
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = 'EGAM5_OnlineOfflineSkimmingTool',
            expression = expression3)

    # do the OR of previous selections
    print('EGAM5 skimming is logical OR of previous selections')
    EGAM5_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
        name = 'EGAM5_SkimmingTool',
        FilterList=[EGAM5_OfflineSkimmingTool, 
                    EGAM5_TriggerSkimmingTool,
                    EGAM5_OnlineOfflineSkimmingTool])

    acc.addPublicTool(EGAM5_OfflineSkimmingTool)
    acc.addPublicTool(EGAM5_TriggerSkimmingTool)
    acc.addPublicTool(EGAM5_OnlineOfflineSkimmingTool)
    acc.addPublicTool(EGAM5_SkimmingTool, primary = True)

    return acc


def EGAM5enuTransverseMassToolCfg(flags):
    '''Configure the EGAM5 enu transverse mass augmentation tool'''
    acc = ComponentAccumulator()

    # ====================================================================
    # enu transverse mass
    #
    # 1 tight electron with pT>25 GeV + MET > 25 GeV
    # ====================================================================
    electronPtRequirement = '(Electrons.pt > 24.5*GeV)'
    electronQualityRequirement = '(Electrons.DFCommonElectronsLHTight)'
    requirement_el = '(' + electronQualityRequirement + \
                     '&&' + electronPtRequirement + ')'

    acc.setPrivateTools( CompFactory.DerivationFramework.EGTransverseMassTool(
        name = 'EGAM5_enuTransverseMassTool',
        ObjectRequirements = requirement_el,
        METmin = 25*GeV,
        StoreGateEntryName = 'EGAM5_ENuTransverseMass',
        ObjectMassHypothesis = 0.511*MeV,
        ObjectContainerName = 'Electrons',
        METContainerName = 'MET_Core_AntiKt4EMPFlow'
    ) )

    return acc



def EGAM5KernelCfg(ConfigFlags, name='EGAM5Kernel', **kwargs):
    '''Configure the derivation framework driving algorithm (kernel) 
       for EGAM5'''
    acc = ComponentAccumulator()


    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import (
        PhysCommonAugmentationsCfg )
    acc.merge( PhysCommonAugmentationsCfg(
        ConfigFlags,
        TriggerListsHelper = kwargs['TriggerListsHelper'] ) )
    

    # EGAM5 augmentations
    augmentationTools = []


    #====================================================================
    # enu transverse mass
    #====================================================================   
    EGAM5enuTransverseMassTool = acc.popToolsAndMerge(
        EGAM5enuTransverseMassToolCfg(ConfigFlags) )
    acc.addPublicTool(EGAM5enuTransverseMassTool)
    augmentationTools.append(EGAM5enuTransverseMassTool)

    #====================================================================
    # Max Cell energy and time
    #====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        MaxCellDecoratorCfg )
    MaxCellDecorator = acc.popToolsAndMerge(MaxCellDecoratorCfg(ConfigFlags))
    acc.addPublicTool(MaxCellDecorator)
    augmentationTools.append(MaxCellDecorator)
    
    # ====================================================================
    # Gain and cluster energies per layer decoration tool
    # ====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        GainDecoratorCfg, ClusterEnergyPerLayerDecoratorCfg )
    EGAM5_GainDecoratorTool = acc.popToolsAndMerge(
        GainDecoratorCfg(ConfigFlags, name = 'EGAM5_GainDecoratorTool' ))
    acc.addPublicTool(EGAM5_GainDecoratorTool)
    augmentationTools.append(EGAM5_GainDecoratorTool)

    cluster_sizes = (3,7), (5,5), (7,11)
    for neta, nphi in cluster_sizes:
        cename = 'EGAM5_ClusterEnergyPerLayerDecorator_%sx%s' % (neta, nphi)
        EGAM5_ClusterEnergyPerLayerDecorator = acc.popToolsAndMerge(
            ClusterEnergyPerLayerDecoratorCfg(
                ConfigFlags,
                neta = neta,
                nphi=nphi,
                name=cename ))
        acc.addPublicTool(EGAM5_ClusterEnergyPerLayerDecorator)
        augmentationTools.append(EGAM5_ClusterEnergyPerLayerDecorator)


    # thinning tools
    thinningTools = []
    streamName = kwargs['StreamName']

    # Track thinning
    if ConfigFlags.Derivation.Egamma.doTrackThinning:

        from DerivationFrameworkInDet.InDetToolsConfig import (
            TrackParticleThinningCfg, MuonTrackParticleThinningCfg,
            TauTrackParticleThinningCfg )

        TrackThinningKeepElectronTracks = True
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepAllElectronTracks = False
        TrackThinningKeepJetTracks = False
        TrackThinningKeepMuonTracks = False
        TrackThinningKeepTauTracks = False
        TrackThinningKeepPVTracks = True

        # Tracks associated with Electrons
        if (TrackThinningKeepElectronTracks):
            EGAM5ElectronTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM5ElectronTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Electrons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = 'Electrons.pt > 0*GeV',
                    BestMatchOnly = True,
                    ConeSize = 0.3)
            acc.addPublicTool(EGAM5ElectronTPThinningTool)
            thinningTools.append(EGAM5ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if (TrackThinningKeepAllElectronTracks):
            EGAM5ElectronTPThinningTool2 = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM5ElectronTPThinningTool2',
                    StreamName = streamName,
                    SGKey = 'Electrons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = 'Electrons.pt > 4*GeV',
                    BestMatchOnly = False,
                    ConeSize = 0.6)
            acc.addPublicTool(EGAM5ElectronTPThinningTool2)
            thinningTools.append(EGAM5ElectronTPThinningTool2)

        # Tracks associated with Photons
        if (TrackThinningKeepPhotonTracks):
            EGAM5PhotonTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM5PhotonTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Photons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    GSFConversionVerticesKey = 'GSFConversionVertices',
                    SelectionString = 'Photons.pt > 0*GeV',
                    BestMatchOnly = True,
                    ConeSize = 0.3)
            acc.addPublicTool(EGAM5PhotonTPThinningTool)
            thinningTools.append(EGAM5PhotonTPThinningTool)

        # Tracks associated with Jets
        if (TrackThinningKeepJetTracks):
            EGAM5JetTPThinningTool = \
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name = 'EGAM5JetTPThinningTool',
                    StreamName = streamName,
                    JetKey = 'AntiKt4EMPFlowJets',
                    InDetTrackParticlesKey = 'InDetTrackParticles')
            acc.addPublicTool(EGAM5JetTPThinningTool)
            thinningTools.append(EGAM5JetTPThinningTool)

        # Tracks associated with Muons
        if (TrackThinningKeepMuonTracks):
            EGAM5MuonTPThinningTool = \
                acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM5MuonTPThinningTool',
                    StreamName = streamName,
                    MuonKey = 'Muons',
                    InDetTrackParticlesKey = 'InDetTrackParticles') )
            thinningTools.append(EGAM5MuonTPThinningTool)

        # Tracks associated with Taus
        if (TrackThinningKeepTauTracks):
            EGAM5TauTPThinningTool = \
                acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM5TauTPThinningTool',
                    StreamName = streamName,
                    TauKey = 'TauJets',
                    ConeSize = 0.6,
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    DoTauTracksThinning    = True,
                    TauTracksKey           = 'TauTracks') )
            thinningTools.append(EGAM5TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = ' && '.join([
            '(InDetTrackParticles.DFCommonTightPrimary)',
            '(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm)', 
            '(InDetTrackParticles.pt > 10*GeV)'])
        if (TrackThinningKeepPVTracks):
            EGAM5TPThinningTool = \
                acc.getPrimaryAndMerge(TrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM5TPThinningTool',
                    StreamName = streamName,
                    SelectionString = thinning_expression,
                    InDetTrackParticlesKey = 'InDetTrackParticles') )
            thinningTools.append(EGAM5TPThinningTool)


    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM5SkimmingToolCfg(ConfigFlags))


    # setup the kernel
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,
                                      SkimmingTools = [skimmingTool],
                                      AugmentationTools = augmentationTools,
                                      ThinningTools = thinningTools) )

    return acc



def EGAM5Cfg(ConfigFlags):

    acc = ComponentAccumulator()


    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # DODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    EGAM5TriggerListsHelper = TriggerListsHelper(ConfigFlags)
    #EGAM5TriggerListsHelper.Run3TriggerNames = EGAM5TriggerListsHelper.Run3TriggerNamesNoTau
    #EGAM5TriggerListsHelper.Run3TriggerNamesTau = []
    #EGAM5TriggerListsHelper.Run2TriggerNamesTau = []

    # configure skimming/thinning/augmentation tools
    acc.merge(EGAM5KernelCfg(ConfigFlags,
                             name = 'EGAM5Kernel',
                             StreamName = 'StreamDAOD_EGAM5',
                             TriggerListsHelper = EGAM5TriggerListsHelper))
    

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    EGAM5SlimmingHelper = SlimmingHelper(
        'EGAM5SlimmingHelper',
        NamesAndTypes = ConfigFlags.Input.TypedCollections,
        ConfigFlags = ConfigFlags )


    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM5SlimmingHelper.AllVariables =[
        'Electrons',
        'GSFTrackParticles',
        'egammaClusters' ]

    # for trigger studies we also add:
    MenuType = None
    if ConfigFlags.Trigger.EDMVersion == 2:
        MenuType = 'Run2'
    elif ConfigFlags.Trigger.EDMVersion == 3:
        MenuType = 'Run3'
    else:
        MenuType = ''
    EGAM5SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
    EGAM5SlimmingHelper.AllVariables += ExtraContainersElectronTrigger[MenuType]
    if not ConfigFlags.Input.isMC:
        EGAM5SlimmingHelper.AllVariables += ExtraContainersTriggerDataOnly[MenuType]
        
    # and on MC we also add:
    if ConfigFlags.Input.isMC:
        EGAM5SlimmingHelper.AllVariables += [
            'TruthEvents',
            'TruthParticles',
            'TruthVertices',
            'egammaTruthParticles'
        ]



    # -------------------------------------------
    # containers that we slim
    # -------------------------------------------

    # first add variables from smart-slimming
    # adding only also those for which we add all variables since
    # the XXXCPContent.py files also bring in some extra variables 
    # for other collections
    EGAM5SlimmingHelper.SmartCollections = ['Electrons',
                                            'Photons',
                                            'Muons',
                                            'TauJets',
                                            'InDetTrackParticles',
                                            'PrimaryVertices',
                                            'AntiKt4EMPFlowJets',
                                            'MET_Baseline_AntiKt4EMPFlow',
                                            'BTagging_AntiKt4EMPFlow'
                                            ]
    # muons, tau, MET, b-tagging could be switched off if not needed 
    # and use too much space
    if ConfigFlags.Input.isMC:
        EGAM5SlimmingHelper.SmartCollections += ['AntiKt4TruthJets',
                                                 'AntiKt4TruthDressedWZJets']

    # then add extra variables:

    # muons
    EGAM5SlimmingHelper.ExtraVariables += [
        'Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40' ]

    # conversion vertices
    EGAM5SlimmingHelper.ExtraVariables += [
        'GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo', 
        'GSFConversionVertices.trackParticleLinks' ]

    # primary vertices
    EGAM5SlimmingHelper.ExtraVariables += [
        'PrimaryVertices.x.y.sumPt2' ]

    # photons: detailed shower shape variables
    EGAM5SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons and electrons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations, getClusterEnergyPerLayerDecorations )
    gainDecorations = getGainDecorations(acc, 'EGAM5Kernel')
    print('EGAM5 gain decorations: ', gainDecorations)
    EGAM5SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(
        acc, 'EGAM5Kernel' )
    print('EGAM5 cluster energy decorations: ', clusterEnergyDecorations)
    EGAM5SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM5SlimmingHelper.ExtraVariables += [ 
        'TopoClusterIsoCentralEventShape.Density',
        'TopoClusterIsoForwardEventShape.Density',
        'NeutralParticleFlowIsoCentralEventShape.Density',
        'NeutralParticleFlowIsoForwardEventShape.Density']

    # truth
    if ConfigFlags.Input.isMC:
        EGAM5SlimmingHelper.ExtraVariables += [
            'MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType' ]

        EGAM5SlimmingHelper.ExtraVariables += [
            'Photons.truthOrigin.truthType.truthParticleLink' ]

    # Add event info
    if ConfigFlags.Derivation.Egamma.doEventInfoSlimming:
        EGAM5SlimmingHelper.SmartCollections.append('EventInfo')
    else:
        EGAM5SlimmingHelper.AllVariables += ['EventInfo']    
    
    # Add egamma trigger objects
    EGAM5SlimmingHelper.IncludeEGammaTriggerContent = True

    # Add trigger matching info
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            AddRun2TriggerMatchingToSlimmingHelper )
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper = EGAM5SlimmingHelper, 
            OutputContainerPrefix = 'TrigMatch_',
            TriggerList = EGAM5TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import (
            AddRun3TrigNavSlimmingCollectionsToSlimmingHelper )
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM5SlimmingHelper)

    
    EGAM5ItemList = EGAM5SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags,
                              'DAOD_EGAM5',
                              ItemList = EGAM5ItemList,
                              AcceptAlgs = ['EGAM5Kernel']))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, 'DAOD_EGAM5',
                                AcceptAlgs=['EGAM5Kernel'],
                                createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
    
