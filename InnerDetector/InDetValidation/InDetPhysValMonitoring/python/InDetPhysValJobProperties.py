# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#
# purpose Python module to hold common flags to configure the InDetPhysValMonitoring
##

from __future__ import print_function
from InDetRecExample.InDetJobProperties import Enabled
from AthenaCommon.JobProperties import jobproperties
from AthenaCommon.JobProperties import JobProperty, JobPropertyContainer

""" InDetPhysValJobProperties
    Python module to hold common flags to configure InDetPhysValMonitoring JobOptions.

"""
__doc__ = "InDetPhysValJobProperties"
__all__ = ["InDetPhysValJobProperties"]

# import AthenaCommon.SystemOfUnits as Units


def isMC():
    '''
    Test whether the input is monte carlo.
    @return true if this is simulation and should have truth information
    '''
    from AthenaCommon.GlobalFlags import globalflags
    return globalflags.DataSource() != 'data'


class InDetPhysValFlagsJobProperty(JobProperty):
    """ This class stores if a user changed a property once in the variable setByUser
    """
    setByUser = False

    def _do_action(self):
        self.setByUser = True

    def _undo_action(self):
        self.setByUser = True

    def get_Value(self):
        if (self.allowedTypes[0] == 'bool'):
            return self.statusOn and self.StoredValue
        else:
            return self.StoredValue


class doValidateLargeD0Tracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False

class doValidateMergedLargeD0Tracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False

class doRecoOnly(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False

class doValidateGSFTracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateMuonMatchedTracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateElectronMatchedTracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateLooseTracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateTightPrimaryTracks(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateTracksInJets(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateTracksInBJets(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doValidateTruthToRecoNtuple(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doTruthOriginPlots(InDetPhysValFlagsJobProperty):
    statusOn     = True
    allowedTypes = ['bool']
    StoredValue  = False


class doPerAuthorPlots(InDetPhysValFlagsJobProperty):
    statusOn     = True
    allowedTypes = ['bool']
    StoredValue  = False

class doHitLevelPlots(InDetPhysValFlagsJobProperty):
    statusOn     = True
    allowedTypes = ['bool']
    StoredValue  = False

class validateExtraTrackCollections(InDetPhysValFlagsJobProperty):
    """List of extra track collection names to be validated in addition to Tracks."""
    statusOn = True
    allowedTypes = ['list']
    StoredValue = []


class doPhysValOutput(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class doExpertOutput(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


class setTruthStrategy(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['string']
    StoredValue = 'HardScatter'

class ancestorIDs(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['list']
    StoredValue = []

class requiredSiHits(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['int']
    StoredValue = 0

class maxProdVertRadius(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['float']
    StoredValue = 300

class hardScatterStrategy(InDetPhysValFlagsJobProperty):
    """The hard-scatter vertex selection strategy to use when running hard-scatter efficiency / performance plots in IDPVM. 0 corresponds to sumPt^2, 1 corresponds to sumPt"""
    statusOn = True
    allowedTypes = ['int']
    StoredValue = 0 # default to sum(pt²)

class doIDTIDE(InDetPhysValFlagsJobProperty):
    statusOn = True
    allowedTypes = ['bool']
    StoredValue = False


# -----------------------------------------------------------------------------
# 2nd step
# Definition of the InDet flag container


class InDetPhysValJobProperties(JobPropertyContainer):
    """Container for the InDetPhysValMonitoring flags
    """

    def checkThenSet(self, jp, value):
        # checks if a variable has been changed by the user before
        if not jp.setByUser:
            jp.set_Value(value)

    def setupDefaults(self):
        pass

    def init(self):
        # Method to do the final setup of the flags according to user input before.
        # This method MUST ONLY BE CALLED once in InDetRecExample/InDetRec_jobOptions.py!!
        if not self.Enabled:
            print('InDetPhysValFlags.init(): ID PhysValFlags are disabled. Locking container and not doing anything else.')
        else:
            self.setup()

        # do this also if Enabled == False
        print("Initialization of InDetFlags finished - locking container!")
        self.lock_JobProperties()

    def setup(self):
        print('Initializing InDetJobPhysValProperties with InDetFlags.')
        # THIS METHOD MUST BE THE FIRST TO BE CALLED. DO NOT MOVE IT OR ADD THINGS IN FRONT
        self.setupDefaults()

        print(self)

    def printInfo(self):
        pass


jobproperties.add_Container(InDetPhysValJobProperties)

# -----------------------------------------------------------------------------
# 4th step
# adding ID flags to the InDetJobProperties container
_list_InDetPhysValJobProperties = [
    Enabled,
    doValidateGSFTracks,
    doValidateLooseTracks,
    doValidateTightPrimaryTracks,
    doValidateTracksInJets,
    doValidateTracksInBJets,
    doValidateTruthToRecoNtuple,
    validateExtraTrackCollections,
    doValidateMuonMatchedTracks,
    doValidateElectronMatchedTracks,
    doPhysValOutput,
    doExpertOutput,
    setTruthStrategy,
    doValidateLargeD0Tracks,
    doValidateMergedLargeD0Tracks,
    doRecoOnly,
    doTruthOriginPlots,
    doPerAuthorPlots,
    doHitLevelPlots,
    ancestorIDs,
    requiredSiHits,
    hardScatterStrategy,
    doIDTIDE
]

for j in _list_InDetPhysValJobProperties:
    jobproperties.InDetPhysValJobProperties.add_JobProperty(j)

InDetPhysValFlags = jobproperties.InDetPhysValJobProperties
print('DEBUG InDetPhysValJobProberties')
print(InDetPhysValFlags)
