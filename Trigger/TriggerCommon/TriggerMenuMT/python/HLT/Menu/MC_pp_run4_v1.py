# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run4_v1.py menu for Phase-II developments (to be kept empty for now)
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],


#from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp ###temporarily commented out only
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 as physics_menu 
###temporarily commented out only
#from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import ( 
#    SingleElectronGroup, SinglePhotonGroup, BphysicsGroup, EOFBPhysL1MuGroup,
#    EOFL1MuGroup, EgammaJetGroup, EgammaMETGroup, MultiJetGroup, PrimaryLegGroup,
#    PrimaryPhIGroup, PrimaryL1MuGroup, SupportGroup, SupportPhIGroup,
#    SupportLegGroup, SingleBjetGroup, MultiBjetGroup, SingleJetGroup,
#    SingleMuonGroup, MultiMuonGroup, BphysElectronGroup,
#    Topo2Group, Topo3Group, LegacyTopoGroup,
#)


def addMCSignatures(chains):
    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')

    chainsMC = ChainStore()

    chainsMC['Muon'] = []

    chainsMC['Jet'] = []

    chainsMC['Bjet'] = []

    chainsMC['Egamma'] = []

    chainsMC['Bphysics'] = []

    chainsMC['Streaming'] += []

    chainsMC['Combined'] += []

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
