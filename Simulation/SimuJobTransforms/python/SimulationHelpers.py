# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization


def getDetectorsFromRunArgs(flags, runArgs):
    """Generate detector list based on runtime arguments."""
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors
        detectors = getDefaultDetectors(flags.GeoModel.AtlasVersion, includeForward=False)

    # Support switching on Forward Detectors
    if hasattr(runArgs, 'LucidOn'):
        detectors = detectors+['Lucid']
    if hasattr(runArgs, 'ZDCOn'):
        detectors = detectors+['ZDC']
    if hasattr(runArgs, 'AFPOn'):
        detectors = detectors+['AFP']
    if hasattr(runArgs, 'ALFAOn'):
        detectors = detectors+['ALFA']
    if hasattr(runArgs, 'FwdRegionOn'):
        detectors = detectors+['FwdRegion']
    # TODO here support switching on Cavern geometry
    # if hasattr(runArgs, 'CavernOn'):
    #     detectors = detectors+['Cavern']

    # Fatras does not support simulating the BCM, so have to switch that off
    from SimulationConfig.SimEnums import SimulationFlavour
    if flags.Sim.ISF.Simulator in [SimulationFlavour.ATLFASTIIFMT, SimulationFlavour.ATLFASTIIF_G4MS, SimulationFlavour.ATLFAST3F_G4MS]:
        try:
            detectors.remove('BCM')
        except ValueError:
            pass

    return detectors


def enableFrozenShowersFCalOnly(flags):
    """Turns on GFlash shower parametrization for FCAL"""
    flags.Sim.LArParameterization = LArParameterization.FrozenShowersFCalOnly
    flags.Sim.CalibrationRun = CalibrationRun.Off


def enableBeamPipeKill(flags):
    flags.Sim.BeamPipeCut = 0.
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim


def enableTightMuonStepping(flags):
    flags.Sim.TightMuonStepping = True


def enableG4SignalCavern(flags):
    """Set flags to take care of Neutron BG"""
    flags.Sim.CavernBackground = CavernBackground.Signal


def enableCalHits(flags):
    """Turns on calibration hits for LAr and Tile"""
    flags.Sim.CalibrationRun = CalibrationRun.LArTile
    # deactivate incompatible optimizations
    flags.Sim.LArParameterization = LArParameterization.NoFrozenShowers
    flags.Sim.NRRThreshold = False
    flags.Sim.NRRWeight = False
    flags.Sim.PRRThreshold = False
    flags.Sim.PRRWeight = False


def enableParticleID(flags):
    """Mods to have primary particle barcode signature on for calorimeter calibration hits."""
    flags.Sim.ParticleID=True


def enableVerboseSelector(flags):
    """ """
    flags.Sim.OptionalUserActionList += ['G4DebuggingTools.G4DebuggingToolsConfig.VerboseSelectorToolCfg']
