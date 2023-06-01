# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# default configuration of the PhotonIsEMSelectorCutDefs



def PhotonIsEMLooseSelectorConfig(theTool):
    '''
    These are the photon isEM definitions Loose
    '''

    #
    # PHOTON Loose cuts
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/PhotonIsEMLooseSelectorCutDefs.conf"


def PhotonIsEMMediumSelectorConfig(theTool):
    '''
    These are the photon isEM definitions Medium
    '''

    # 
    #  PHOTON Medium cuts 
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/PhotonIsEMMediumSelectorCutDefs.conf"


def PhotonIsEMTightSelectorConfig(theTool):
    '''
    These are the photon isEM definitions for Tight menu
    '''

    #
    #  PHOTON Tight cuts
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/PhotonIsEMTightSelectorCutDefs.conf"

