# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
from AthenaCommon.SystemOfUnits import GeV,ns # noqa: F401
# Example specialConfiguration {'GMSBSlepton': '100.0*GeV', 'GMSBGravitino': '1e-07*GeV', 'GMSBSleptonTime': '0.01*ns'}

"""
Defining default settings for slepton/staus. Possible options are:
G4ParticleMass (default 0.0*GeV)
G4ParticleWidth (default 0.0*GeV)
G4ParticleCharge (default +/-1.*eplus)
G4ParticlePDGCode (default sparticle pdgid)
G4ParticleStable (default True)
G4ParticleLifetime (default -1)
G4ParticleShortlived (default False)
where Particle = [STau1Minus, STau1Plus, STau2Minus, STau2Plus, SElectronRMinus, SElectronRLinus, SElectronRPlus, SElectronLPlus, SMuonRMinus, SMuonLMinus, SMuonRPlus, SMuonLPlus]
"""
# The mass of the a1(1260) meson.  The a1(1260) meson is the heaviest of the decay products when the tau is off shell.
Mass_a1Meson = 1260. # MeV

def getSleptonsPhysicsTool(name="SleptonsPhysicsTool", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if simFlags.specialConfiguration.get_Value().has_key("GMSBStau") or simFlags.specialConfiguration.get_Value().has_key("coannihilationStau"):
        StauMass = None
        if simFlags.specialConfiguration.get_Value().has_key("GMSBStau"):
            StauMass = eval(simFlags.specialConfiguration.get_Value().get("GMSBStau", "None"))
            kwargs.setdefault("G4STau1MinusMass",             StauMass)
            kwargs.setdefault("G4STau1PlusMass",              StauMass)
            # TODO Check whether G4STau2(Plus/Minus)Mass should also be set here
        elif simFlags.specialConfiguration.get_Value().has_key("coannihilationStau"):
            StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
            kwargs.setdefault("G4STau1MinusMass",             StauMass)
            kwargs.setdefault("G4STau1PlusMass",              StauMass)
            kwargs.setdefault("G4STau2MinusMass",             StauMass)
            kwargs.setdefault("G4STau2PlusMass",              StauMass)

    if simFlags.specialConfiguration.get_Value().has_key("GMSBSlepton"):
        GMSBSlepton = eval(simFlags.specialConfiguration.get_Value().get("GMSBSlepton", "None"))

        kwargs.setdefault("G4SElectronRMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronRPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SMuonRMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonRPlusMass",             GMSBSlepton)

    return CfgMgr.SleptonsPhysicsTool(name, **kwargs)


def getAllSleptonsPhysicsTool(name="AllSleptonsPhysicsTool", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if simFlags.specialConfiguration.get_Value().has_key("GMSBStau") or simFlags.specialConfiguration.get_Value().has_key("coannihilationStau"):
        StauMass = None
        StauLifetime = None
        if simFlags.specialConfiguration.get_Value().has_key("GMSBStau"): # Check for GMSBStau key word in job options. If found set Stau values.
            StauMass = eval(simFlags.specialConfiguration.get_Value().get("GMSBStau", "None"))
            StauLifetime = eval(simFlags.specialConfiguration.get_Value().get("GMSBStauTime", "None"))
        elif simFlags.specialConfiguration.get_Value().has_key("coannihilationStau"): # Check for coannihilationStau key word in evgen special configs. This is an option that is normally put in the event gen job options file.
            StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
            StauLifetime = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStauTime", "None"))

        kwargs.setdefault("G4STau1MinusMass",             StauMass)
        kwargs.setdefault("G4STau1MinusPDGCode",          1000015)
        kwargs.setdefault("G4STau1MinusStable",           False)
        kwargs.setdefault("G4STau1MinusLifetime",         StauLifetime)

        kwargs.setdefault("G4STau1PlusMass",              StauMass)
        kwargs.setdefault("G4STau1PlusPDGCode",           -1000015)
        kwargs.setdefault("G4STau1PlusStable",            False)
        kwargs.setdefault("G4STau1PlusLifetime",          StauLifetime)

        kwargs.setdefault("G4STau2MinusMass",             StauMass)
        kwargs.setdefault("G4STau2MinusPDGCode",          2000015)
        kwargs.setdefault("G4STau2MinusStable",           False)
        kwargs.setdefault("G4STau2MinusLifetime",         StauLifetime)

        kwargs.setdefault("G4STau2PlusMass",              StauMass)
        kwargs.setdefault("G4STau2PlusPDGCode",           -2000015)
        kwargs.setdefault("G4STau2PlusStable",            False)
        kwargs.setdefault("G4STau2PlusLifetime",          StauLifetime)

    if simFlags.specialConfiguration.get_Value().has_key("GMSBSlepton"):
        GMSBSlepton = eval(simFlags.specialConfiguration.get_Value().get("GMSBSlepton", "None"))
        GMSBSleptonTime = eval(simFlags.specialConfiguration.get_Value().get("GMSBSleptonTime", "None"))

        kwargs.setdefault("G4SElectronLMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronLMinusPDGCode",     1000011)
        kwargs.setdefault("G4SElectronLMinusStable",      False)
        kwargs.setdefault("G4SElectronLMinusLifetime",    GMSBSleptonTime)

        kwargs.setdefault("G4SElectronLPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SElectronLPlusPDGCode",      -1000011)
        kwargs.setdefault("G4SElectronLPlusStable",       False)
        kwargs.setdefault("G4SElectronLPlusLifetime",     GMSBSleptonTime)

        kwargs.setdefault("G4SMuonLMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonLMinusPDGCode",         1000013)
        kwargs.setdefault("G4SMuonLMinusStable",          False)
        kwargs.setdefault("G4SMuonLMinusLifetime",        GMSBSleptonTime)

        kwargs.setdefault("G4SMuonLPlusMass",             GMSBSlepton)
        kwargs.setdefault("G4SMuonLPlusPDGCode",          -1000013)
        kwargs.setdefault("G4SMuonLPlusStable",           False)
        kwargs.setdefault("G4SMuonLPlusLifetime",         GMSBSleptonTime)

        kwargs.setdefault("G4SElectronRMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronRMinusPDGCode",     2000011)
        kwargs.setdefault("G4SElectronRMinusStable",      False)
        kwargs.setdefault("G4SElectronRMinusLifetime",    GMSBSleptonTime)


        kwargs.setdefault("G4SElectronRPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SElectronRPlusPDGCode",      -2000011)
        kwargs.setdefault("G4SElectronRPlusStable",       False)
        kwargs.setdefault("G4SElectronRPlusLifetime",     GMSBSleptonTime)

        kwargs.setdefault("G4SMuonRMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonRMinusPDGCode",         2000013)
        kwargs.setdefault("G4SMuonRMinusStable",          False)
        kwargs.setdefault("G4SMuonRMinusLifetime",        GMSBSleptonTime)

        kwargs.setdefault("G4SMuonRPlusMass",             GMSBSlepton)
        kwargs.setdefault("G4SMuonRPlusPDGCode",          -2000013)
        kwargs.setdefault("G4SMuonRPlusStable",           False)
        kwargs.setdefault("G4SMuonRPlusLifetime",         GMSBSleptonTime)


    return CfgMgr.SleptonsPhysicsTool(name, **kwargs)


## Gravitino Options
def getSElectronRPlusToElectronGravitino(name="SElectronRPlusToElectronGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronRMinusToElectronGravitino(name="SElectronRMinusToElectronGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonRPlusToMuonGravitino(name="SMuonRPlusToMuonGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonRMinusToMuonGravitino(name="SMuonRMinusToMuonGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToTauGravitino(name="STauLPlusToTauGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToTauGravitino(name="STauLMinusToTauGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronLPlusToElectronGravitino(name="SElectronLPlusToElectronGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronLMinusToElectronGravitino(name="SElectronLMinusToElectronGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonLPlusToMuonGravitino(name="SMuonLPlusToMuonGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonLMinusToMuonGravitino(name="SMuonLMinusToMuonGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToTauGravitino(name="STauRPlusToTauGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRMinusToTauGravitino(name="STauRMinusToTauGravitino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


## Neutralino-Stau
def getSTauRMinusToTauNeutralino(name="STauRMinusToTauNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToTauNeutralino(name="STauRPlusToTauNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToTauNeutralino(name="STauLMinusToTauNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToTauNeutralino(name="STauLPlusToTauNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


#########################################################################################
### Neutralino-Stau Off shell tau
## Stau-Neutralino Pion Neutrino
#########################################################################################
def getSTauRMinusToPionMinusNeutralino(name="STauRMinusToPionMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR)  # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToPionPlusNeutralino(name="STauRPlusToPionPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToPionMinusNeutralino(name="STauLMinusToPionMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToPionPlusNeutralino(name="STauLPlusToPionPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Muon Neutrino
#########################################################################################
def getSTauRMinusToRhoMinusNeutralino(name="STauRMinusToRhoMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToRhoPlusNeutralino(name="STauRPlusToRhoPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToRhoMinusNeutralino(name="STauLMinusToRhoMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToRhoPlusNeutralino(name="STauLPlusToRhoPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Electron Neutrino
#########################################################################################
def getSTauRMinusToEMinusNeutralino(name="STauRMinusToEMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-,nu_tau,anti_nu_e")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToEPlusNeutralino(name="STauRPlusToEPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+,anti_nu_tau,nu_e")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToEMinusNeutralino(name="STauLMinusToEMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-,nu_tau,anti_nu_e")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToEPlusNeutralino(name="STauLPlusToEPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .03
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+,anti_nu_tau,nu_e")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Muon Neutrino
#########################################################################################
def getSTauRMinusToMuMinusNeutralino(name="STauRMinusToMuMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-,nu_tau,anti_nu_mu")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToMuPlusNeutralino(name="STauRPlusToMuPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+,anti_nu_tau,nu_mu")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToMuMinusNeutralino(name="STauLMinusToMuMinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-,nu_tau,anti_nu_mu")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToMuPlusNeutralino(name="STauLPlusToMuPlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+,anti_nu_tau,nu_mu")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Pseuddo-Vector a1(1260) meson Neutrino
#########################################################################################
def getSTauRMinusToa1MinusNeutralino(name="STauRMinusToa1MinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15 ## Set the branching Ratio for if there is enough energy for.c
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauRPlusToa1PlusNeutralino(name="STauRPlusToa1PlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLMinusToa1MinusNeutralino(name="STauLMinusToa1MinusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)-,nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSTauLPlusToa1PlusNeutralino(name="STauLPlusToa1PlusNeutralino", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "None"))
    StauMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson >  NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)+,anti_nu_tau")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


## Neutralino Selectron
def getSElectronRPlusToElectronNeutralino(name="SElectronRPlusToElectronNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronRMinusToElectronNeutralino(name="SElectronRMinusToElectronNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronLPlusToElectronNeutralino(name="SElectronLPlusToElectronNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_e_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSElectronLMinusToElectronNeutralino(name="SElectronLMinusToElectronNeutralino", **kwargs): # TODO: getSElectronLMinusToElectronNeutralino not found later. This might be an error
    kwargs.setdefault("ParticleName","s_e_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


## Neutralino SMuon
def getSMuonLPlusToMuonNeutralino(name="SMuonLPlusToMuonNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonLMinusToMuonNeutralino(name="SMuonLMinusToMuonNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonRPlusToMuonNeutralino(name="SMuonRPlusToMuonNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)


def getSMuonRMinusToMuonNeutralino(name="SMuonRMinusToMuonNeutralino", **kwargs):
    kwargs.setdefault("ParticleName","s_mu_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-")
    return CfgMgr.AddPhysicsDecayTool(name, **kwargs)
