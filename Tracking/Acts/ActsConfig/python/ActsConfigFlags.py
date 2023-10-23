# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class SeedingStrategy(FlagEnum):
    Default = "Default"
    Orthogonal = "Orthogonal"

# This is temporary during the integration of ACTS.
class SpacePointStrategy(FlagEnum):
    ActsCore = "ActsCore" # ACTS-based SP formation
    ActsTrk = "ActsTrk" #SP formation without ACTS

class TrackFitterType(FlagEnum):
    KalmanFitter = 'KalmanFitter' # default ACTS fitter to choose
    GaussianSumFitter = 'GaussianSumFitter' # new experimental implementation

def createActsConfigFlags():
    actscf = AthConfigFlags()

    # General Flags
    actscf.addFlag('Acts.EDM.PersistifyClusters', False)
    actscf.addFlag('Acts.EDM.PersistifySpacePoints', False)
    
    # Geometry Flags

    # MaterialSource can be:
    # a path to a local JSON file
    # 'Default' : material map source is evaluated from the geometry tag
    # 'None'    : no material map is provided
    actscf.addFlag('Acts.TrackingGeometry.MaterialSource', 'Default')
    actscf.addFlag('Acts.TrackingGeometry.MaterialCalibrationFolder', 'ACTS/MaterialMaps/ITk')

    # Monitoring
    actscf.addFlag('Acts.doMonitoring', False)
    actscf.addFlag('Acts.doAnalysis', False)

    # SpacePoint
    actscf.addFlag("Acts.SpacePointStrategy", SpacePointStrategy.ActsTrk, enum=SpacePointStrategy)  # Define SpacePoint Strategy

    # Seeding
    actscf.addFlag("Acts.SeedingStrategy", SeedingStrategy.Default, enum=SeedingStrategy)  # Define Seeding Strategy

    # Track finding
    actscf.addFlag('Acts.doRotCorrection', True)
    actscf.addFlag('Acts.doPrintTrackStates', False)
    actscf.addFlag('Acts.skipDuplicateSeeds', True)
    actscf.addFlag('Acts.trackFindingChi2CutOff', 15.0)
    actscf.addFlag('Acts.doTrackFindingTrackSelector', 1) # 0=no selection, 1=default track selection, 2=no eta-dependant cuts

    actscf.addFlag('Acts.doAmbiguityResolution', True)

    # Track fitting
    actscf.addFlag('Acts.writeTrackCollection', False) # save to file (ESD, AOD) the Resolved and Refitted track collections
    actscf.addFlag('Acts.fitFromPRD', False) # Acts.writeTrackCollection needs to be True for either cases. If Acts.fitFromPRD is False, fit from ROT; else, fit from PRD
    actscf.addFlag('Acts.trackFitterType', TrackFitterType.KalmanFitter, enum=TrackFitterType) # Define Tracking algorithm for refitting

    return actscf
