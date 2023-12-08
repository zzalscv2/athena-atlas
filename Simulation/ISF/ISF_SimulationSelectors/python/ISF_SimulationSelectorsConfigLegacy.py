# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""
ISF_SimulationSelectors configurations for ISF
Elmar Ritsch, 04/02/2013
"""

from AthenaCommon import CfgMgr
from ISF_SimulationSelectors import SimulationFlavor
### DefaultSimSelector configurations

def usesSimKernelMT():
    from ISF_Config.ISF_jobProperties import ISF_Flags
    return (ISF_Flags.Simulator.get_Value() in ['FullG4MT', 'FullG4MT_QS', 'PassBackG4MT', 'ATLFASTIIMT', 'ATLFAST3MT', 'ATLFAST3MT_QS', 'FullG4MT_LongLived', 'ATLFASTIIF_ACTS', 'ATLFAST3F_ACTSMT'])

def getDefaultSimSelector(name="ISF_DefaultSimSelector", **kwargs):
    return CfgMgr.ISF__DefaultSimSelector(name, **kwargs )

def getDefaultParticleKillerSelector(name="ISF_DefaultParticleKillerSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_ParticleKillerSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.ParticleKiller)
    return getDefaultSimSelector(name, **kwargs )

def getDefaultGeant4Selector(name="ISF_DefaultGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_Geant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )

def getDefaultAFIIGeant4Selector(name="ISF_DefaultAFIIGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_AFIIGeant4SimSvc')
    return getDefaultGeant4Selector(name, **kwargs )

def getDefaultLongLivedGeant4Selector(name="ISF_DefaultLongLivedGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_LongLivedGeant4SimSvc')
    return getDefaultGeant4Selector(name, **kwargs )

def getDefaultAFII_QS_Geant4Selector(name="ISF_DefaultAFII_QS_Geant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_AFII_QS_Geant4SimSvc')
    return getDefaultGeant4Selector(name, **kwargs )

def getFullGeant4Selector(name="ISF_FullGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FullGeant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )

def getPassBackGeant4Selector(name="ISF_PassBackGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_PassBackGeant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )

def getDefaultFastCaloSimV2Selector(name="ISF_DefaultFastCaloSimV2Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FastCaloSimSvcV2')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.FastCaloSimV2)
    return getDefaultSimSelector(name, **kwargs )

def getDefaultDNNCaloSimSelector(name="ISF_DefaultDNNCaloSimSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_DNNCaloSimSvc')
    return getDefaultSimSelector(name, **kwargs )

def getDefaultFatrasSelector(name="ISF_DefaultFatrasSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FatrasSimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return getDefaultSimSelector(name, **kwargs )

def getDefaultActsSelector(name="ISF_DefaultActsSelector", **kwargs):
    if not usesSimKernelMT():
        raise RuntimeError("SimulationSelector '%s' does not support running with SimKernel." % name)
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return getDefaultSimSelector(name, **kwargs)

### KinematicSimSelector Configurations

# BASE METHODS
def getKinematicGeant4Selector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_Geant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)

def getKinematicAFIIGeant4Selector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_AFIIGeant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)

def getKinematicAFII_QS_Geant4Selector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_AFII_QS_Geant4SimSvc')
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getKinematicFatrasSelector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_FatrasSimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)

def getKinematicParticleKillerSimSelector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_ParticleKillerSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.ParticleKiller)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)

# Electrons
def getElectronGeant4Selector(name="ISF_ElectronGeant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 11)
    return getKinematicGeant4Selector(name, **kwargs)

# Protons
def getProtonAFIIGeant4Selector(name="ISF_ProtonAFIIGeant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 2212)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getProtonATLFAST3Geant4Selector(name="ISF_ProtonATLFAST3Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2212)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getProtonATLFAST3_QS_Geant4Selector(name="ISF_ProtonATLFAST3_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2212)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

# Pions
def getPionAFIIGeant4Selector(name="ISF_PionAFIIGeant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 200)
    kwargs.setdefault('ParticlePDG'     , 211)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getPionATLFAST3Geant4Selector(name="ISF_PionATLFAST3Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'          , 200)
    kwargs.setdefault('ParticlePDG'     , 211)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getPionATLFAST3_QS_Geant4Selector(name="ISF_PionATLFAST3_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'          , 200)
    kwargs.setdefault('ParticlePDG'     , 211)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

# Neutrons
def getNeutronATLFAST3Geant4Selector(name="ISF_NeutronATLFAST3Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2112)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getNeutronATLFAST3_QS_Geant4Selector(name="ISF_NeutronATLFAST3_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2112)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

# Charged Kaons
def getChargedKaonAFIIGeant4Selector(name="ISF_ChargedKaonAFIIGeant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 321)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getChargedKaonATLFAST3Geant4Selector(name="ISF_ChargedKaonATLFAST3Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 321)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getChargedKaonATLFAST3_QS_Geant4Selector(name="ISF_ChargedKaonATLFAST3_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 321)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

# KLongs
def getKLongAFIIGeant4Selector(name="ISF_KLongAFIIGeant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 130)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getKLongATLFAST3Geant4Selector(name="ISF_KLongATLFAST3Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 130)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getKLongATLFAST3_QS_Geant4Selector(name="ISF_KLongATLFAST3_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 130)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

# Muons
def getMuonGeant4Selector(name="ISF_MuonGeant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicGeant4Selector(name, **kwargs)

def getMuonAFIIGeant4Selector(name="ISF_MuonAFIIGeant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicAFIIGeant4Selector(name, **kwargs)

def getMuonAFII_QS_Geant4Selector(name="ISF_MuonAFII_QS_Geant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicAFII_QS_Geant4Selector(name, **kwargs)

def getMuonFatrasSelector(name="ISF_MuonFatrasSelector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicFatrasSelector(name, **kwargs)

# General Eta-based selectors
def getEtaGreater5ParticleKillerSimSelector(name="ISF_EtaGreater5ParticleKillerSimSelector", **kwargs):
    kwargs.setdefault('MinPosEta'       , -5.0 )
    kwargs.setdefault('MaxPosEta'       ,  5.0 )
    kwargs.setdefault('InvertCuts'      , True )
    return getKinematicParticleKillerSimSelector(name, **kwargs)
