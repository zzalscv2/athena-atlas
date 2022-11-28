# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import ProductionStep
from Campaigns.Utils import Campaign


def MC23a(flags):
    """MC23a flags for MC to match 2022 Run 3 data"""
    flags.Input.MCCampaign = Campaign.MC23a

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    flags.Digitization.DoPixelPlanarRadiationDamage = True

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 84.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 84.335
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.165
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC21a_MultiBeamspot'

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1


def MC23aSingleBeamspot(flags):
    """MC23a flags for MC to match 2022 Run 3 data (single beamspot version)"""
    MC23a(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC21a_SingleBeamspot'


def MC23LowMu(flags):
    """MC23 flags for MC to match Run 3 data with low pile-up"""
    flags.Beam.NumberOfCollisions = 60.
    flags.Input.ConditionsRunNumber = 410000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    flags.Digitization.DoPixelPlanarRadiationDamage = True

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 0.05
    # to simulate low pile-up. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 0.0499
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.0001
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'


def MC23NoPileUp(flags):
    """MC23 flags for MC without pile-up"""
    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 410000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    flags.Digitization.DoPixelPlanarRadiationDamage = True


def BeamspotSplitMC23a():
    """MC23a beamspot splitting configuration"""
    substeps = 4
    event_fractions = [0.14, 0.14, 0.14, 0.58]

    return substeps, event_fractions


def MC23SimulationNoIoV(flags):
    """MC23 base flags for simulation without specifying conditions IoVs"""
    from SimulationConfig.SimEnums import SimulationFlavour, TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TRTRangeCut = 30.0
    flags.Sim.TightMuonStepping = True

    from SimuJobTransforms.SimulationHelpers import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    if flags.Sim.ISF.Simulator in [SimulationFlavour.FullG4MT, SimulationFlavour.FullG4MT_QS]:
        enableFrozenShowersFCalOnly(flags)

    from SimuJobTransforms.G4Optimizations import enableG4Optimizations
    enableG4Optimizations(flags)


def MC23SimulationSingleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)

    flags.Input.RunNumber = [410000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumber = [1] # dummy value


def MC23SimulationMultipleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)

    flags.Input.OverrideRunNumber = True

    from RunDependentSimComps.PileUpUtils import generateRunAndLumiProfile
    generateRunAndLumiProfile(flags,
                              profile= 'RunDependentSimData.PileUpProfile_run410000_MC21a_MultiBeamspot')


def MC23SimulationSingleIoVCalibrationHits(flags):
    """MC23 flags for simulation with CalibrationHits"""
    MC23SimulationSingleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)


def MC23SimulationMultipleIoVCalibrationHits(flags):
    """MC23 flags for simulation with CalibrationHits"""
    MC23SimulationMultipleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)
