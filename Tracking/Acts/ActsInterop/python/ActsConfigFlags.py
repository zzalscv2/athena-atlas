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

def createActsConfigFlags():
    actscf = AthConfigFlags()

    # General Flags - TO BE ADDED
    
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

    # Track fitting
    actscf.addFlag('Acts.writeTrackCollection', False) # save to file (ESD, AOD) the Resolved and Refitted track collections

    return actscf
