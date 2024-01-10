"""Construct ConfigFlags for Digitization

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import ProductionStep
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType


def constBunchSpacingPattern(constBunchSpacing):
    """Return a valid value for Digitization.BeamIntensity, which
    matches the specified constBunchSpacing
    """
    if type(constBunchSpacing) is not int:
        raise TypeError("constBunchSpacing must be int, "
                        "not %s" % type(constBunchSpacing).__name__)
    if constBunchSpacing % 25 != 0:
        raise ValueError("constBunchSpacing must be a multiple of 25, "
                         "not %s" % constBunchSpacing)

    # special case
    if constBunchSpacing == 25:
        return [1.0]

    # general case
    pattern = [0.0, 1.0]
    nBunches = (constBunchSpacing//25) - 2
    pattern += nBunches*[0.0]
    return pattern


def createDigitizationCfgFlags():
    """Return an AthConfigFlags object with required flags"""
    flags = AthConfigFlags()
    # Digitization Steering - needed for easy comparison with the
    # old-style configuration, but can potentially drop
    def _checkDigiSteeringConf(prevFlags):
        digiSteeringConf = "StandardPileUpToolsAlg"
        if prevFlags.Input.Files:
            digiSteeringConf = GetFileMD(prevFlags.Input.Files).get("digiSteeringConf", "StandardPileUpToolsAlg")
        return digiSteeringConf

    flags.addFlag("Digitization.DigiSteeringConf", _checkDigiSteeringConf)
    # Run Inner Detector noise simulation
    flags.addFlag("Digitization.DoInnerDetectorNoise", lambda prevFlags: not prevFlags.Common.isOverlay)
    # Run pile-up digitization on one bunch crossing at a time?
    flags.addFlag("Digitization.DoXingByXingPileUp", False)
    # Run Calorimeter noise simulation
    flags.addFlag("Digitization.DoCaloNoise", lambda prevFlags: not prevFlags.Common.isOverlay)
    # Produce inputs for Calorimeter hard scatter truth reconstruction
    flags.addFlag("Digitization.EnableCaloHSTruthRecoInputs", False)
    # Use high-gain Forward Calorimeters
    flags.addFlag("Digitization.HighGainFCal", False)
    # Use high-gain ElectroMagnetic EndCap Inner Wheel
    flags.addFlag("Digitization.HighGainEMECIW", True)
    # Do global pileup digitization
    flags.addFlag("Digitization.PileUp",
                  lambda prevFlags: GetFileMD(prevFlags.Input.Files).get("pileUp", "False") != "False")
    # Temporary TGC flag
    flags.addFlag("Digitization.UseUpdatedTGCConditions", False)
    # Write out truth information
    flags.addFlag("Digitization.EnableTruth", True)
    # Write out calorimeter digits
    flags.addFlag("Digitization.AddCaloDigi", False)
    # Write out thinned calorimeter digits
    flags.addFlag("Digitization.AddCaloDigiThinned", False)
    # Integer offset to random seed initialisation
    flags.addFlag("Digitization.RandomSeedOffset", 0)
    # Digitization extra input dependencies
    flags.addFlag("Digitization.ExtraInputs", [("xAOD::EventInfo", "EventInfo")])
    # Beam spot reweighting (-1 disables it)
    flags.addFlag("Digitization.InputBeamSigmaZ", -1)

    # Set the type of the radiation damage simulation type for pixel planar sensors
    flags.addFlag("Digitization.PixelPlanarRadiationDamageSimulationType",
                  PixelRadiationDamageSimulationType.NoRadiationDamage, type=PixelRadiationDamageSimulationType)
    # Set the type of the radiation damage simulation type for 3D planar sensors
    flags.addFlag("Digitization.Pixel3DRadiationDamageSimulationType",
                  PixelRadiationDamageSimulationType.NoRadiationDamage, type=PixelRadiationDamageSimulationType)

    # for PileUp digitization
    # Bunch structure configuration
    flags.addFlag("Digitization.PU.BunchStructureConfig", "")
    # Pile-up profile configuration
    flags.addFlag("Digitization.PU.ProfileConfig", "")
    # Custom pile-up profile configuration - fully custom or for mu range
    flags.addFlag("Digitization.PU.CustomProfile", "")
    # Force sequential event numbers
    flags.addFlag("Digitization.PU.ForceSequentialEventNumbers",
                  lambda prevFlags: prevFlags.Common.ProductionStep == ProductionStep.PileUpPresampling)
    # Beam Halo input collections
    flags.addFlag("Digitization.PU.BeamHaloInputCols", [])
    # LHC Bunch Structure (list of non-negative floats)
    flags.addFlag("Digitization.PU.BeamIntensityPattern",
                  lambda prevFlags: constBunchSpacingPattern(prevFlags.Beam.BunchSpacing))
    # Beam Gas input collections
    flags.addFlag("Digitization.PU.BeamGasInputCols", [])
    # LHC bunch spacing, in ns, to use in pileup digitization. Only multiples of 25 allowed.
    # Not necessarily equal to Beam.BunchSpacing
    flags.addFlag("Digitization.PU.BunchSpacing",
                  lambda prevFlags: prevFlags.Beam.BunchSpacing)
    # PileUp branch crossing parameters
    flags.addFlag("Digitization.PU.InitialBunchCrossing", -32)
    flags.addFlag("Digitization.PU.FinalBunchCrossing", 6)
    # Add the cavern background every bunch, independent of any bunch structure?
    flags.addFlag("Digitization.PU.CavernIgnoresBeamInt", False)
    # Cavern input collections
    flags.addFlag("Digitization.PU.CavernInputCols", [])
    # Central bunch crossing location in the BeamIntensityPattern
    flags.addFlag("Digitization.PU.FixedT0BunchCrossing", 0)
    # Superimpose mixed high pt minimum bias events (pile-up) on signal events?
    # If so, set this to a list of: High Pt Mixed ND, SD, DD minimum bias input collections
    flags.addFlag("Digitization.PU.HighPtMinBiasInputCols", [])
    # Offset into the input collections of high pt min-bias events
    flags.addFlag("Digitization.PU.HighPtMinBiasInputColOffset", 0)
    # Superimpose mixed low pt minimum bias events (pile-up) on signal events?
    # If so, set this to a list of: Low Pt Mixed ND, SD, DD minimum bias input collections
    flags.addFlag("Digitization.PU.LowPtMinBiasInputCols", [])
    # Number of low pt min-bias events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfLowPtMinBias", 0.0)
    # Number of high pt min-bias events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfHighPtMinBias", 0.0)
    # Number of beam gas events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfBeamGas", 0.0)
    # Number of beam halo events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfBeamHalo", 0.0)
    # Number of mixed ND, SD, DD min-bias events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfCollisions", 0.0)
    # Number of cavern events to superimpose per signal event per beam crossing
    flags.addFlag("Digitization.PU.NumberOfCavern", 0.0)
    # Repeating pattern to determine which events to simulate when using Stepping Cache
    flags.addFlag("Digitization.PU.SignalPatternForSteppingCache", [])
    # Which sub-systems should use Fast Digitization
    flags.addFlag("Digitization.DoFastDigi", [])
    # Set the flag to True if the Common.ProductionStep is not one of the steps in the list
    flags.addFlag("Digitization.ReadParametersFromDB", lambda prevFlags : prevFlags.Common.ProductionStep in [ProductionStep.Digitization, ProductionStep.PileUpPresampling, ProductionStep.Overlay, ProductionStep.FastChain])
    return flags


def digitizationRunArgsToFlags(runArgs, flags):
    """Fill digitization configuration flags from run arguments."""
    # from SimDigi
    if hasattr(runArgs, "DataRunNumber"):
        flags.Input.ConditionsRunNumber = runArgs.DataRunNumber

    # from SimDigi
    if hasattr(runArgs, "jobNumber"):
        flags.Input.JobNumber = runArgs.jobNumber

    if hasattr(runArgs, "PileUpPresampling"):
        flags.Common.ProductionStep = ProductionStep.PileUpPresampling
    elif flags.Common.ProductionStep == ProductionStep.Default: # Do not override previous settings
        flags.Common.ProductionStep = ProductionStep.Digitization

    if hasattr(runArgs, "doAllNoise"):
        flags.Digitization.DoInnerDetectorNoise = runArgs.doAllNoise
        flags.Digitization.DoCaloNoise = runArgs.doAllNoise

    if hasattr(runArgs, "AddCaloDigi"):
        flags.Digitization.AddCaloDigi = runArgs.AddCaloDigi

    if hasattr(runArgs, "digiSeedOffset1") or hasattr(runArgs, "digiSeedOffset2"):
        flags.Digitization.RandomSeedOffset = 0
        if hasattr(runArgs, "digiSeedOffset1"):
            flags.Digitization.RandomSeedOffset += int(runArgs.digiSeedOffset1)
        if hasattr(runArgs, "digiSeedOffset2"):
            flags.Digitization.RandomSeedOffset += int(runArgs.digiSeedOffset2)
    else:
        flags.Digitization.RandomSeedOffset = 3  # for legacy compatibility

    if hasattr(runArgs, "digiSteeringConf"):
        flags.Digitization.DigiSteeringConf = runArgs.digiSteeringConf + "PileUpToolsAlg"

    # TODO: Not covered yet as no flag equivalents exist yet
    # '--digiRndmSvc'
    # '--samplingFractionDbTag'


def pileupRunArgsToFlags(runArgs, flags):
    """Fill pile-up digitization configuration flags from run arguments."""
    if hasattr(runArgs, "numberOfLowPtMinBias"):
        flags.Digitization.PU.NumberOfLowPtMinBias = runArgs.numberOfLowPtMinBias

    if hasattr(runArgs, "numberOfHighPtMinBias"):
        flags.Digitization.PU.NumberOfHighPtMinBias = runArgs.numberOfHighPtMinBias

    if hasattr(runArgs, "numberOfBeamHalo"):
        flags.Digitization.PU.NumberOfBeamHalo = runArgs.numberOfBeamHalo

    if hasattr(runArgs, "numberOfBeamGas"):
        flags.Digitization.PU.NumberOfBeamGas = runArgs.numberOfBeamGas

    if hasattr(runArgs, "numberOfCavernBkg"):
        flags.Digitization.PU.NumberOfCavern = runArgs.numberOfCavernBkg

    if hasattr(runArgs, "bunchSpacing"):
        flags.Digitization.PU.BunchSpacing = runArgs.bunchSpacing

    if hasattr(runArgs, "pileupInitialBunch"):
        flags.Digitization.PU.InitialBunchCrossing = runArgs.pileupInitialBunch

    if hasattr(runArgs, "pileupFinalBunch"):
        flags.Digitization.PU.FinalBunchCrossing = runArgs.pileupFinalBunch

    # sanity check
    if flags.Digitization.PU.InitialBunchCrossing > flags.Digitization.PU.FinalBunchCrossing:
        raise ValueError("Initial bunch crossing should not be larger than the final one")

    if hasattr(runArgs, "inputLowPtMinbiasHitsFile"):
        from RunDependentSimComps.PileUpUtils import generateBackgroundInputCollections
        flags.Digitization.PU.LowPtMinBiasInputCols = \
            generateBackgroundInputCollections(flags, runArgs.inputLowPtMinbiasHitsFile,
                                               flags.Digitization.PU.NumberOfLowPtMinBias, True)

    if hasattr(runArgs, "inputHighPtMinbiasHitsFile"):
        from RunDependentSimComps.PileUpUtils import getInputCollectionOffset, generateBackgroundInputCollections
        if flags.Digitization.PU.HighPtMinBiasInputColOffset < 0:
            # Calculate a pseudo random offset into the collection from the jobNumber
            flags.Digitization.PU.HighPtMinBiasInputColOffset = getInputCollectionOffset(flags, runArgs.inputHighPtMinbiasHitsFile)

        flags.Digitization.PU.HighPtMinBiasInputCols = \
            generateBackgroundInputCollections(flags, runArgs.inputHighPtMinbiasHitsFile,
                                               flags.Digitization.PU.NumberOfHighPtMinBias, True)

    if hasattr(runArgs, "inputCavernHitsFile"):
        from RunDependentSimComps.PileUpUtils import generateBackgroundInputCollections
        flags.Digitization.PU.CavernInputCols = \
            generateBackgroundInputCollections(flags, runArgs.inputCavernHitsFile,
                                               flags.Digitization.PU.NumberOfCavern, True)  # TODO: ignore?

    if hasattr(runArgs, "inputBeamHaloHitsFile"):
        from RunDependentSimComps.PileUpUtils import generateBackgroundInputCollections
        flags.Digitization.PU.BeamHaloInputCols = \
            generateBackgroundInputCollections(flags, runArgs.inputBeamHaloHitsFile,
                                               flags.Digitization.PU.NumberOfBeamHalo, True)

    if hasattr(runArgs, "inputBeamGasHitsFile"):
        from RunDependentSimComps.PileUpUtils import generateBackgroundInputCollections
        flags.Digitization.PU.BeamGasInputCols = \
            generateBackgroundInputCollections(flags, runArgs.inputBeamGasHitsFile,
                                               flags.Digitization.PU.NumberOfBeamGas, True)

    # TODO: Not covered yet as no flag equivalents exist yet
    # '--testPileUpConfig'


def setupDigitizationFlags(runArgs, flags):
    """Setup common digitization flags."""
    # autoconfigure pile-up if inputs are present
    if (hasattr(runArgs, "inputLowPtMinbiasHitsFile")
        or hasattr(runArgs, "inputHighPtMinbiasHitsFile")
        or hasattr(runArgs, "inputCavernHitsFile")
        or hasattr(runArgs, "inputBeamHaloHitsFile")
        or hasattr(runArgs, "inputBeamGasHitsFile")):
        flags.Digitization.PileUp = True

    if flags.Digitization.PileUp:
        flags.Input.OverrideRunNumber = True
        # Needs to be False for MT pileup
        if flags.Concurrency.NumThreads > 0:
            flags.Digitization.DoXingByXingPileUp = False
        else:
            flags.Digitization.DoXingByXingPileUp = True
    else:
        flags.Input.OverrideRunNumber = flags.Input.ConditionsRunNumber > 0
