# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# EGAM12.py
# This defines DAOD_EGAM12, a skimmed DAOD format for Run 3.
# Keep events passing OR of electron triggers, or inclusive
#   electron selection, to retain fake electron candidates
# Adaptation of EGAM7 format for heavy ion runs (no triggers, no pflow 
# jets, extra containers)
# It requires the flag EGAM12 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent )


# some info missing to calculate extra decorations
addGainDecorations = False
addMaxCellDecorations = False


def EGAM12SkimmingToolCfg(flags):
    '''Configure the EGAM12 skimming tool'''
    acc = ComponentAccumulator()

    # off-line based selection
    expression = 'count(Electrons.pt > 4.5*GeV) >= 1'
    print('EGAM12 offline skimming expression: ', expression)
    EGAM12_OfflineSkimmingTool = \
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = 'EGAM12_OfflineSkimmingTool',
            expression = expression,
            TrigDecisionTool = "")

    acc.addPublicTool(EGAM12_OfflineSkimmingTool, primary = True)

    return acc


def EGAM12KernelCfg(ConfigFlags, name='EGAM12Kernel', **kwargs):
    '''Configure the derivation framework driving algorithm (kernel) 
       for EGAM12'''
    acc = ComponentAccumulator()

    # Schedule extra jets collections
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo, AntiKt4PV0Track, AntiKt4Truth
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.JetConfigFlags import jetInternalFlags
    jetList = [AntiKt4EMTopo, AntiKt4PV0Track, AntiKt4Truth] 
    jetInternalFlags.isRecoJob = True
    for jd in jetList: acc.merge(JetRecCfg(ConfigFlags,jd))
    JetKey = 'AntiKt4EMTopoJets'

    # Common augmentations
    # cannot use PhysCommon sequence because
    # - no triggers
    # - no TauJets
    # so we have to use a modified version here
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc.merge(InDetCommonCfg(ConfigFlags,
                             DoVertexFinding = ConfigFlags.Tracking.doVertexFinding,
                             AddPseudoTracks = ConfigFlags.Tracking.doPseudoTracking,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = ConfigFlags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = ConfigFlags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False))
    acc.merge(MuonsCommonCfg(ConfigFlags))
    acc.merge(EGammaCommonCfg(ConfigFlags))
    # jet cleaning
    # standard way in PhysCommon is
    # - calculate tau ID (needed for default jet OR)
    # - decorate jets with overlap removal
    # - do event cleaning
    # but taus are missing in HI derivations so need to do differently

    # Decorate if jet passed JVT criteria
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtEffToolCfg
    algName = "DFJet_EventCleaning_passJvtAlg"
    passJvtTool = acc.popToolsAndMerge(getJvtEffToolCfg(ConfigFlags, 'AntiKt4EMTopo'))
    passJvtTool.PassJVTKey = "AntiKt4EMTopoJets.DFCommonJets_passJvt"
    acc.addEventAlgo(CompFactory.JetDecorationAlg(algName, JetContainer='AntiKt4EMTopoJets', Decorators=[passJvtTool]))
    
    # Decorate if jet passes OR and save decoration DFCommonJets_passOR
    # Use modified OR that does not check overlaps with tauls 
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg
    outputLabel = 'DFCommonJets_passOR'
    bJetLabel = '' #default
    tauLabel = '' #workaround for missing taus
    tauKey = ''  #workaround for missing taus
    orTool = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,outputLabel=outputLabel,bJetLabel=bJetLabel,doTaus=False))
    algOR = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg',
                                                OverlapLabel=outputLabel,
                                                OverlapRemovalTool=orTool,
                                                TauKey=tauKey,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel)
    acc.addEventAlgo(algOR)

    # Do the cleaning
    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg,JetCleaningToolCfg
    workingPoints = ['Loose']
    prefix = "DFCommonJets_"

    for wp in workingPoints:
            
        cleaningLevel = wp + 'Bad'
        # LLP WPs have a slightly different name format
        if 'LLP' in wp:
            cleaningLevel = wp.replace('LLP', 'BadLLP')

        jetCleaningTool = acc.popToolsAndMerge(JetCleaningToolCfg(ConfigFlags, 'JetCleaningTool_'+cleaningLevel, 'AntiKt4EMTopoJets', cleaningLevel, False))
        acc.addPublicTool(jetCleaningTool)

        ecTool = acc.popToolsAndMerge(EventCleaningToolCfg(ConfigFlags,'EventCleaningTool_' + wp, cleaningLevel))
        ecTool.JetCleanPrefix = prefix
        ecTool.JetContainer = "AntiKt4EMTopoJets"
        ecTool.JetCleaningTool = jetCleaningTool
        acc.addPublicTool(ecTool)

        # Alg to calculate event-level and jet-level cleaning variables
        # Only store event-level flags for Loose* WPs
        eventCleanAlg = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp,
                                                         EventCleaningTool = ecTool,
                                                         JetCollectionName = "AntiKt4EMTopoJets",
                                                         EventCleanPrefix = prefix,
                                                         CleaningLevel = cleaningLevel,
                                                         doEvent = ('Loose' in wp))
        acc.addEventAlgo(eventCleanAlg)


    # EGAM12 augmentations
    augmentationTools = []

    #====================================================================
    # Max Cell energy and time
    #====================================================================
    if addMaxCellDecorations:
        from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
            MaxCellDecoratorCfg )
        MaxCellDecorator = acc.popToolsAndMerge(MaxCellDecoratorCfg(ConfigFlags))
        acc.addPublicTool(MaxCellDecorator)
        augmentationTools.append(MaxCellDecorator)  
   
    # ====================================================================
    # Gain and cluster energies per layer decoration tool
    # ====================================================================
    if addGainDecorations:
        from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
            GainDecoratorCfg, ClusterEnergyPerLayerDecoratorCfg )
        EGAM12_GainDecoratorTool = acc.popToolsAndMerge(
            GainDecoratorCfg(ConfigFlags, name = 'EGAM12_GainDecoratorTool' ))
        acc.addPublicTool(EGAM12_GainDecoratorTool)
        augmentationTools.append(EGAM12_GainDecoratorTool)

        # might need some modification when cell-level reweighting is implemented (see share/EGAM12.py)
        cluster_sizes = (3,7), (5,5), (7,11)
        for neta, nphi in cluster_sizes:
            cename = 'EGAM12_ClusterEnergyPerLayerDecorator_%sx%s' % (neta, nphi)
            EGAM12_ClusterEnergyPerLayerDecorator = acc.popToolsAndMerge(
                ClusterEnergyPerLayerDecoratorCfg(
                    ConfigFlags,
                    neta = neta,
                    nphi = nphi,
                    name = cename ))
            acc.addPublicTool(EGAM12_ClusterEnergyPerLayerDecorator)
            augmentationTools.append(EGAM12_ClusterEnergyPerLayerDecorator)


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
        TrackThinningKeepPVTracks = False

        # Tracks associated with Electrons
        if (TrackThinningKeepElectronTracks):
            EGAM12ElectronTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM12ElectronTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Electrons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    SelectionString = 'Electrons.pt > 0*GeV',
                    BestMatchOnly = True,
                    ConeSize = 0.3)
            acc.addPublicTool(EGAM12ElectronTPThinningTool)
            thinningTools.append(EGAM12ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if (TrackThinningKeepAllElectronTracks):
            EGAM12ElectronTPThinningTool2 = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                name = 'EGAM12ElectronTPThinningTool2',
                StreamName = streamName,
                SGKey = 'Electrons',
                GSFTrackParticlesKey = 'GSFTrackParticles',
                InDetTrackParticlesKey = 'InDetTrackParticles',
                SelectionString = 'Electrons.pt > 4*GeV',
                BestMatchOnly = False,
                ConeSize = 0.6)
            acc.addPublicTool(EGAM12ElectronTPThinningTool2)
            thinningTools.append(EGAM12ElectronTPThinningTool2)

        # Tracks associated with Photons
        if (TrackThinningKeepPhotonTracks):
            EGAM12PhotonTPThinningTool = \
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name = 'EGAM12PhotonTPThinningTool',
                    StreamName = streamName,
                    SGKey = 'Photons',
                    GSFTrackParticlesKey = 'GSFTrackParticles',
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    GSFConversionVerticesKey = 'GSFConversionVertices',
                    SelectionString = 'Photons.pt > 0*GeV',
                    BestMatchOnly = True,
                    ConeSize = 0.3)
            acc.addPublicTool(EGAM12PhotonTPThinningTool)
            thinningTools.append(EGAM12PhotonTPThinningTool)

        # Tracks associated with Jets
        if (TrackThinningKeepJetTracks):
            EGAM12JetTPThinningTool = \
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name = 'EGAM12JetTPThinningTool',
                    StreamName = streamName,
                    JetKey = JetKey,
                    InDetTrackParticlesKey = 'InDetTrackParticles')
            acc.addPublicTool(EGAM12JetTPThinningTool)
            thinningTools.append(EGAM12JetTPThinningTool)

        # Tracks associated with Muons
        if (TrackThinningKeepMuonTracks):
            EGAM12MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM12MuonTPThinningTool',
                    StreamName = streamName,
                    MuonKey = 'Muons',
                   InDetTrackParticlesKey = 'InDetTrackParticles') )
            thinningTools.append(EGAM12MuonTPThinningTool)

        # Tracks associated with Taus
        if (TrackThinningKeepTauTracks):
            EGAM12TauTPThinningTool = \
                acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM12TauTPThinningTool',
                    StreamName = streamName,
                    TauKey = 'TauJets',
                    ConeSize = 0.6,
                    InDetTrackParticlesKey = 'InDetTrackParticles',
                    DoTauTracksThinning    = True,
                    TauTracksKey           = 'TauTracks') )
            thinningTools.append(EGAM12TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = ' && '.join([
            '(InDetTrackParticles.DFCommonTightPrimary)',
            '(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm)', 
            '(InDetTrackParticles.pt > 10*GeV)'])
        if (TrackThinningKeepPVTracks):
            EGAM12TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    ConfigFlags,
                    name = 'EGAM12TPThinningTool',
                    StreamName = streamName,
                    SelectionString = thinning_expression,
                    InDetTrackParticlesKey = 'InDetTrackParticles') )
            thinningTools.append(EGAM12TPThinningTool)


    # truth thinning
    if ConfigFlags.Input.isMC:
        # W, Z and Higgs
        truth_cond_WZH = ' && '.join(['(abs(TruthParticles.pdgId) >= 23)',
                                      '(abs(TruthParticles.pdgId) <= 25)'])
        # Leptons
        truth_cond_lep = ' && '.join(['(abs(TruthParticles.pdgId) >= 11)',
                                      '(abs(TruthParticles.pdgId) <= 16)'])
        # Top quark
        truth_cond_top = '(abs(TruthParticles.pdgId) ==  6)'
        # Photon
        truth_cond_gam = ' && '.join(['(abs(TruthParticles.pdgId) == 22)',
                                      '(TruthParticles.pt > 1*GeV)'])
        # stable particles
        truth_cond_finalState = ' && '.join(['(TruthParticles.status == 1)',
                                             '(TruthParticles.barcode<200000)'])
        truth_expression = '( ' + truth_cond_WZH        + ' ) || ' + \
                           '( ' + truth_cond_lep        + ' ) || ' + \
                           '( ' + truth_cond_top        + ' ) || ' + \
                           '( ' + truth_cond_gam        + ' ) || ' + \
                           '( ' + truth_cond_finalState + ' )'
        print('EGAM12 truth thinning expression: ', truth_expression)

        EGAM12TruthThinningTool = \
            CompFactory.DerivationFramework.GenericTruthThinning(
                name = 'EGAM12TruthThinningTool',
                StreamName = streamName,
                ParticleSelectionString = truth_expression,
                PreserveDescendants = False,
                PreserveGeneratorDescendants = True,
                PreserveAncestors = True)
        acc.addPublicTool(EGAM12TruthThinningTool)
        thinningTools.append(EGAM12TruthThinningTool)


    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM12SkimmingToolCfg(ConfigFlags))


    # setup the kernel
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,
                                      SkimmingTools = [skimmingTool],
                                      AugmentationTools = augmentationTools,
                                      ThinningTools = thinningTools) )

    return acc



def EGAM12Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    JetKey = 'AntiKt4EMTopoJets'
    EGAM12TriggerListsHelper = None

    # configure skimming/thinning/augmentation tools
    acc.merge(EGAM12KernelCfg(ConfigFlags,
                              name = 'EGAM12Kernel',
                              StreamName = 'StreamDAOD_EGAM12',
                              TriggerListsHelper = EGAM12TriggerListsHelper))
    

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    EGAM12SlimmingHelper = SlimmingHelper(
        'EGAM12SlimmingHelper',
        NamesAndTypes = ConfigFlags.Input.TypedCollections,
        ConfigFlags = ConfigFlags )


    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM12SlimmingHelper.AllVariables =[
        'Electrons',
        'GSFTrackParticles',
        'egammaClusters' ]
    
    # on MC we also add:
    if ConfigFlags.Input.isMC:
        EGAM12SlimmingHelper.AllVariables += [
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
    # muons, tau, MET, b-tagging could be switched off if not needed
    # and use too much space
    EGAM12SlimmingHelper.SmartCollections = ['Electrons',
                                            'Photons',
                                            'Muons',
                                            'TauJets',
                                            'InDetTrackParticles',
                                            'PrimaryVertices',
                                            JetKey]

    if ConfigFlags.Input.isMC:
        EGAM12SlimmingHelper.SmartCollections += ['AntiKt4TruthJets',
                                                  'AntiKt4TruthDressedWZJets']

    # then add extra variables:

    # muons
    EGAM12SlimmingHelper.ExtraVariables += [
        'Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40' ]

    # conversion vertices
    EGAM12SlimmingHelper.ExtraVariables += [
        'GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo',
        'GSFConversionVertices.trackParticleLinks' ]

    # primary vertices
    EGAM12SlimmingHelper.ExtraVariables += [
        'PrimaryVertices.x.y.sumPt2' ]

    # track jets
    EGAM12SlimmingHelper.ExtraVariables += [
        'AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks' ]

    # photons: detailed shower shape variables
    EGAM12SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    if addGainDecorations:
        from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
            getGainDecorations, getClusterEnergyPerLayerDecorations )
        gainDecorations = getGainDecorations(acc, 'EGAM12Kernel')
        print('EGAM12 gain decorations: ', gainDecorations)
        EGAM12SlimmingHelper.ExtraVariables.extend(gainDecorations)
        clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(
            acc, 'EGAM12Kernel' )
        print('EGAM12 cluster energy decorations: ', clusterEnergyDecorations)
        EGAM12SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM12SlimmingHelper.ExtraVariables += [ 
        'TopoClusterIsoCentralEventShape.Density',
        'TopoClusterIsoForwardEventShape.Density'
    ]

    # truth
    if ConfigFlags.Input.isMC:
        EGAM12SlimmingHelper.ExtraVariables += [
            'MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType' ]

        EGAM12SlimmingHelper.ExtraVariables += [
            'Photons.truthOrigin.truthType.truthParticleLink' ]

    # Add event info
    if ConfigFlags.Derivation.Egamma.doEventInfoSlimming:
        EGAM12SlimmingHelper.SmartCollections.append('EventInfo')
    else:
        EGAM12SlimmingHelper.AllVariables += ['EventInfo']    
    
    # Add HIEventShape and CaloSums variables for heavy ions
    EGAM12SlimmingHelper.AllVariables += ['HIEventShape']
    EGAM12SlimmingHelper.AllVariables += ['CaloSums']

    EGAM12ItemList = EGAM12SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags,
                              'DAOD_EGAM12',
                              ItemList = EGAM12ItemList,
                              AcceptAlgs = ['EGAM12Kernel']))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, 'DAOD_EGAM12',
                                        AcceptAlgs=['EGAM12Kernel'],
                                        createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
