# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createActsConfigFlags():
    actscf = AthConfigFlags()

    # General Flags - TO BE ADDED
    
    # Geometry Flags
    actscf.addFlag('Acts.TrackingGeometry.MaterialSource', 'None') # Input, a path to a JSON file or None

    # Monitoring
    actscf.addFlag('Acts.doMonitoring', False)
    actscf.addFlag('Acts.doAnalysis', False)

    return actscf
