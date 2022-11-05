# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run4_v1.py menu for Phase-II developments 
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
# ['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],


from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp 
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import ( 
    SingleMuonGroup, MultiMuonGroup, 
    MultiElectronGroup,MultiPhotonGroup,
    SingleElectronGroup, SinglePhotonGroup, 
    PrimaryLegGroup, PrimaryPhIGroup, PrimaryL1MuGroup, 
)

def addMCSignatures(chains):
    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')

    chainsMC = ChainStore()

    chainsMC['Muon'] = [
        # Single Muon Run-3 primaries
        ChainProp(name='HLT_mu24_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu50_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu60_0eta105_msonly_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu60_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_msonly_3layersEC_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),

        # Multi muon Run-3 primaries
        ChainProp(name='HLT_2mu14_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_2mu10_l2mt_L1MU10BOM', groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu22_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_2mu4noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_3mu6_L13MU5VF',  l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_3mu6_msonly_L13MU5VF',  l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_4mu4_L14MU3V',  l1SeedThresholds=['MU3V'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        
        # Multi muon with mass cut Run-3 primaries
        ChainProp(name='HLT_mu10_ivarmedium_mu10_10invmAB70_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_mu4noL1_10invmAB70_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Muon LRT chain
        ChainProp(name='HLT_mu20_LRT_d0loose_L1MU14FCH',  groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online']),

        #From TDR studies 
        ChainProp(name='HLT_mu18_ivarmedium_L1MU5VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU5VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu40_ivarmedium_L1MU8VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),

        ChainProp(name='HLT_mu20_mu8noL1_L1MU3VF',  l1SeedThresholds=['MU3VF','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_2mu10_L12MU3VF', groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
    ]

    chainsMC['Egamma'] = [

        # single electron Run-3 primaries
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_lhmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_lhloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0']),
        # electron LRT
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),
        # electron ringer
        ChainProp(name='HLT_e140_lhloose_noringer_L1eEM26M', groups=SingleElectronGroup,monGroups=['egammaMon:shifter_tp']),

        # multi electron
        ChainProp(name='HLT_2e17_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        # single photon
        ChainProp(name='HLT_g140_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),

        # multi photon
        ChainProp(name='HLT_2g20_tight_icaloloose_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_loose_L12eEM24L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        # From TDR studies
        ChainProp(name='HLT_e20_lhmedium_ivarloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e22_lhmedium_ivarloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e22_lhtight_ivarloose_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e45_lhmedium_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_lhloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        # ringer
        ChainProp(name='HLT_e26_lhtight_ivarloose_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e60_lhmedium_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_2e10_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e11_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e12_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e13_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e14_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e15_lhvloose_L12EM7', groups=PrimaryLegGroup+MultiElectronGroup), ##ATTENTION: Using legacy L1

        ChainProp(name='HLT_2e10_lhloose_noringer_L12EM7', groups=SingleElectronGroup),##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e12_lhloose_noringer_L12EM7', groups=SingleElectronGroup),##ATTENTION: Using legacy L1
        ChainProp(name='HLT_2e15_lhloose_noringer_L12EM7', groups=SingleElectronGroup),##ATTENTION: Using legacy L1

        ChainProp(name='HLT_g100_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g100_medium_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g120_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g120_medium_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g180_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),

        ChainProp(name='HLT_g25_loose_g20_loose_L12eEM18L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g25_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),

        ChainProp(name='HLT_2g20_loose_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g20_medium_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        ChainProp(name='HLT_2g25_loose_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_medium_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_tight_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
    ]

    chainsMC['Tau'] = []

    chainsMC['Jet'] = []

    chainsMC['Bjet'] = []

    chainsMC['MET'] = []

    chainsMC['Bphysics'] = []

    chainsMC['Combined'] = []

    chainsMC['UnconventionalTracking'] = []

    chainsMC['Combined'] = []

    chainsMC['Streaming'] += []

    chains['Beamspot'] += []

    # check for chains that have the 'PS:Online' group, so that they are not simulated
    # -- does not make sense in MC menu
    for sig in chainsMC:
        for chain in chainsMC[sig]:
            if 'PS:Online' in chain.groups:
                log.error("chain %s in MC menu has the group 'PS:Online', will not be simulated!", chain.name)
                raise RuntimeError("Remove the group 'PS:Online' from the chain %s",chain.name)

    for sig in chainsMC:
        chains[sig] += chainsMC[sig]

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')
    
    chains = physics_menu.setupMenu()

    addMCSignatures(chains)

    return chains
