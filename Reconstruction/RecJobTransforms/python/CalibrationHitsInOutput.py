# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Useage: place in postInclude eg. RAWtoALL:RecJobTransforms.CalibrationHitsInOutput.CalibrationHitsInESD

def CalibrationHitsInESD(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD
    toESD=[f'{"CaloCalibrationHitContainer#*"}',]
    return addToESD(flags, toESD)

def CalibrationHitsInAOD(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    toAOD=[f'{"CaloCalibrationHitContainer#*"}',]
    return addToAOD(flags, toAOD)

