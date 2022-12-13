# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# This file defines a factory method that can create a configuration
# block sequence based on a passed in name.  This avoids having to
# import all the various config block sequence makers in the
# configuration code, and also would make it easier to create them
# from a text configuration file.

# This relies heavily on the blocks exposing all configurable
# parameters as options, since there is no other mechanism to
# configure them through this interface.

# The implementation itself is probably not the best possible, it
# lacks all extensibility, gathers all information in a single place,
# etc.  Still for now (08 Dec 22) this ought to be good enough.

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

def makeConfig (factoryName, groupName) :

    configSeq = ConfigSequence ()
    if factoryName == 'Muons' :
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig
        makeMuonCalibrationConfig (configSeq, groupName)
    elif factoryName == 'Muons.Selection' :
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makeMuonWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, postfix=groupSplit[1])
    elif factoryName == 'Photons' :
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import makePhotonCalibrationConfig
        makePhotonCalibrationConfig (configSeq, groupName)
    elif factoryName == 'Photons.Selection' :
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import makePhotonWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makePhotonWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, postfix=groupSplit[1])
    else :
        raise ValueError ('unknown factory: ' + factoryName)

    if groupName is not None :
        for config in configSeq :
            if config.groupName() != groupName :
                raise Exception ("inconsistent group names: " + config.groupName() + " " + groupName)

    return configSeq
