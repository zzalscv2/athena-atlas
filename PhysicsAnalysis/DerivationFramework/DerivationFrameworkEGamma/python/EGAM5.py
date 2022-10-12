# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# EGAM5.py
# This defines DAOD_EGAM5, a skimmed DAOD format for Run 3.
# W->enu derivation
# It requires the flag EGAM5 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.SystemOfUnits import MeV, GeV

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent )


def EGAM5SkimmingToolCfg(flags):
    '''Configure the EGAM5 skimming tool'''
    acc = ComponentAccumulator()


    # 1st selection: trigger-based (WTP triggers)

    # L1Topo W T&P 
    triggers =  ['HLT_e13_etcut_trkcut' ]
    triggers +=  ['HLT_e18_etcut_trkcut' ]
    ## # Non-L1Topo W TP commissioning triggers ==> in MC, in 50 ns data
    triggers +=  ['HLT_e13_etcut_trkcut_xs15' ]
    triggers +=  ['HLT_e18_etcut_trkcut_xs20' ]
    ## W T&P triggers ==> not in MC, in 50 ns data
    triggers +=  ['HLT_e13_etcut_trkcut_xs15_mt25' ]
    triggers +=  ['HLT_e18_etcut_trkcut_xs20_mt35' ]
    ###W T&P triggers ==> not in MC, not in 50 ns data, will be in 25 ns data
    triggers +=  ['HLT_e13_etcut_trkcut_xs15_j20_perf_xe15_2dphi05' ]
    triggers +=  ['HLT_e13_etcut_trkcut_xs15_j20_perf_xe15_2dphi05_mt25' ]
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_2dphi05_mt25' ]
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_2dphi05' ]
    triggers +=  ['HLT_e13_etcut_trkcut_xs15_j20_perf_xe15_6dphi05' ]
    triggers +=  ['HLT_e13_etcut_trkcut_xs15_j20_perf_xe15_6dphi05_mt25' ]
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_6dphi05_mt25' ]
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_6dphi05' ]
    triggers +=  ['HLT_e18_etcut_trkcut_xs20_j20_perf_xe20_6dphi15' ]
    triggers +=  ['HLT_e18_etcut_trkcut_xs20_j20_perf_xe20_6dphi15_mt35' ]
    triggers +=  ['HLT_e18_etcut_trkcut_j20_perf_xe20_6dphi15_mt35' ]
    triggers +=  ['HLT_e18_etcut_trkcut_j20_perf_xe20_6dphi15' ]

    # others
    triggers +=  ['HLT_e5_etcut_L1W-05DPHI-JXE-0']
    triggers +=  ['HLT_e5_etcut_L1W-10DPHI-JXE-0']
    triggers +=  ['HLT_e5_etcut_L1W-15DPHI-JXE-0']
    triggers +=  ['HLT_e5_etcut_L1W-10DPHI-EMXE-0']
    triggers +=  ['HLT_e5_etcut_L1W-15DPHI-EMXE-0']
    triggers +=  ['HLT_e5_etcut_L1W-05DPHI-EMXE-1']
    triggers +=  ['HLT_e5_etcut_L1W-05RO-XEHT-0']
    triggers +=  ['HLT_e5_etcut_L1W-90RO2-XEHT-0']
    triggers +=  ['HLT_e5_etcut_L1W-250RO2-XEHT-0']
    triggers +=  ['HLT_e5_etcut_L1W-HT20-JJ15.ETA49']
    triggers +=  ['HLT_e13_etcut_L1W-NOMATCH']
    triggers +=  ['HLT_e13_etcut_L1W-NOMATCH_W-05RO-XEEMHT']
    triggers +=  ['HLT_e13_etcut_L1EM10_W-MT25']
    triggers +=  ['HLT_e13_etcut_L1EM10_W-MT30']
    triggers +=  ['HLT_e13_etcut_trkcut_L1EM12']
    triggers +=  ['HLT_e13_etcut_trkcut_L1EM10_W-MT25_W-15DPHI-JXE-0_W-15DPHI-EMXE']
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_6dphi15_mt25']
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_6dphi15_mt25_L1EM12_W-MT25_W-15DPHI-JXE-0_W-15DPHI-EMXE_XS20']
    triggers +=  ['HLT_e13_etcut_trkcut_j20_perf_xe15_6dphi15_mt25_L1EM12_W-MT25_W-15DPHI-JXE-0_W-15DPHI-EMXE_W-90RO2-XEHT-0']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_xe30_mt35']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_6dphi05_mt35']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_2dphi05_mt35']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_2dphi15_mt35']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_2dphi15_mt35_L1EM12_W-MT25_W-15DPHI-JXE-0_W-15DPHI-EMXE_XS20']
    triggers +=  ['HLT_e13_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35_L1EM12_W-MT25_W-15DPHI-JXE-0_W-15DPHI-EMXE_W-90RO2-XEHT-0']
    triggers +=  ['HLT_e18_etcut_L1EM15_W-MT35']
    triggers +=  ['HLT_e18_etcut_trkcut_L1EM15']
    triggers +=  ['HLT_e18_etcut_trkcut_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EMXE']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_xe30_mt35']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi05_mt35']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_2dphi05_mt35']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_2dphi15_mt35']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi05_mt35_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EM15XE_XS30'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EM15XE_XS30'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_2dphi05_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_2dphi15_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_2dphi15_mt35_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EM15XE_XS30']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi05_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_j15_perf_xe30_6dphi15_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-15DPHI-JXE-0_W-15DPHI-EM15XE']
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_xe30_mt35_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EM15XE_XS30'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_xe30_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE'] 
    triggers +=  ['HLT_e18_etcut_trkcut_xs30_xe30_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-15DPHI-JXE-0_W-15DPHI-EM15XE']
    triggers +=  ['HLT_e18_etcut_trkcut_j20_perf_xe20_6dphi15_mt35_L1EM15_W-MT35_W-05DPHI-JXE-0_W-05DPHI-EM15XE_XS30'] 
    triggers +=  ['HLT_e18_etcut_trkcut_j20_perf_xe20_6dphi15_mt35_L1EM15_W-MT35_W-250RO2-XEHT-0_W-05DPHI-JXE-0_W-05DPHI-EM15XE']

    # added for 2017
    triggers += ['HLT_e60_etcut']
    triggers += ['HLT_e60_etcut_L1EM24VHIM']
    triggers += ['HLT_e60_etcut_trkcut_L1EM24VHIM_j15_perf_xe60_6dphi15_mt35']
    triggers += ['HLT_e60_etcut_trkcut_L1EM24VHIM_xe60_mt35']
    triggers += ['HLT_e60_etcut_trkcut_L1EM24VHIM_xs30_j15_perf_xe30_6dphi15_mt35']
    triggers += ['HLT_e60_etcut_trkcut_L1EM24VHIM_xs30_xe30_mt35']
    triggers += ['HLT_e60_lhmedium_nod0']
    triggers += ['HLT_e60_lhmedium_nod0_L1EM24VHI']
    triggers += ['HLT_e60_lhmedium_nod0_L1EM24VHIM']
    triggers += ['HLT_e60_lhvloose_nod0']
    triggers += ['HLT_e60_etcut_trkcut_j15_perf_xe60_6dphi05_mt35']
    triggers += ['HLT_e60_etcut_trkcut_xs30_j15_perf_xe30_6dphi05_mt35']
    triggers += ['HLT_e70_etcut']
    triggers += ['HLT_e70_etcut_L1EM24VHIM']
    triggers += ['HLT_e70_lhloose_nod0_L1EM24VHIM_xe70noL1']
    triggers += ['HLT_e70_lhloose_nod0_xe70noL1']
    triggers += ['HLT_noalg_l1topo_L1EM15']
    triggers += ['HLT_noalg_l1topo_L1EM7']
    triggers += ['HLT_j80_xe80']
    triggers += ['HLT_xe80_tc_lcw_L1XE50']
    triggers += ['HLT_xe90_mht_L1XE50']
    triggers += ['HLT_xe90_tc_lcw_wEFMu_L1XE50']
    triggers += ['HLT_xe90_mht_wEFMu_L1XE50']
    triggers += ['HLT_xe110_mht_L1XE50']
    triggers += ['HLT_xe110_pufit_L1XE50']
    
    #added for low-mu data analysis, 2017 and 2018 data
    triggers += ['HLT_e15_lhloose_nod0_L1EM12']
    #added for low-mu data analysis, 2018 data
    triggers += ['HLT_xe35']
    triggers += ['HLT_e15_etcut_trkcut_xe30noL1']
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
        name = 'EGAM2_SkimmingTool',
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

    # Track thinning
    if ConfigFlags.Derivation.Egamma.doTrackThinning:

        from DerivationFrameworkInDet.InDetToolsConfig import (
            TrackParticleThinningCfg, MuonTrackParticleThinningCfg,
            TauTrackParticleThinningCfg )
        streamName = kwargs['StreamName']

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
    EGAM5TriggerListsHelper = TriggerListsHelper()
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
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    EGAM5SlimmingHelper = SlimmingHelper(
        'EGAM5SlimmingHelper',
        NamesAndTypes = ConfigFlags.Input.TypedCollections )


    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM5SlimmingHelper.AllVariables =[
        'Electrons',
        'GSFTrackParticles',
        'egammaClusters' ]

    # for trigger studies we also add:  
    EGAM5SlimmingHelper.AllVariables += [
        'HLT_xAOD__ElectronContainer_egamma_Electrons',
        'HLT_xAOD__ElectronContainer_egamma_ElectronsAux.',
        'HLT_xAOD__PhotonContainer_egamma_Photons',
        'HLT_xAOD__PhotonContainer_egamma_PhotonsAux.',
        'HLT_xAOD__TrigElectronContainer_L2ElectronFex',
        'HLT_xAOD__TrigElectronContainer_L2ElectronFexAux.',
        'HLT_xAOD__TrigPhotonContainer_L2PhotonFex',
        'HLT_xAOD__TrigPhotonContainer_L2PhotonFexAux.',
        'HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFex',
        'HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFexAux.',
        'HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_EFID',
        'HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_EFIDAux.',
        'LVL1EmTauRoIs',
        'LVL1EmTauRoIsAux.',
        'HLT_TrigPassBitsCollection_passbits',
        'HLT_TrigPassBitsCollection_passbitsAux.',
        'HLT_TrigPassFlagsCollection_passflags',
        'HLT_TrigPassFlagsCollection_passflagsAux.',
        'HLT_TrigRoiDescriptorCollection_initialRoI',
        'HLT_TrigRoiDescriptorCollection_initialRoIAux.'
    ]
    if not ConfigFlags.Input.isMC:
        EGAM5SlimmingHelper.AllVariables += [
            'HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgamma',
            'HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgammaAux.'
        ]
        
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
    from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import ( 
        getGainDecorations, getClusterEnergyPerLayerDecorations )
    GainDecoratorTool = None
    ClusterEnergyPerLayerDecorators = []  
    for toolStr in acc.getEventAlgo('EGAM5Kernel').AugmentationTools:
        toolStr  = f'{toolStr}'
        splitStr = toolStr.split('/')
        tool =  acc.getPublicTool(splitStr[1])
        if splitStr[0] == 'DerivationFramework::GainDecorator':
            GainDecoratorTool = tool
        elif splitStr[0] == 'DerivationFramework::ClusterEnergyPerLayerDecorator':
            ClusterEnergyPerLayerDecorators.append( tool )

    if GainDecoratorTool : 
        EGAM5SlimmingHelper.ExtraVariables.extend( 
            getGainDecorations(GainDecoratorTool) )
    for tool in ClusterEnergyPerLayerDecorators:
        EGAM5SlimmingHelper.ExtraVariables.extend( 
            getClusterEnergyPerLayerDecorations( tool ) )

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
            TriggerList = EGAM5TriggerListsHelper.Run2TriggerNamesTau)
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

    return acc
    
