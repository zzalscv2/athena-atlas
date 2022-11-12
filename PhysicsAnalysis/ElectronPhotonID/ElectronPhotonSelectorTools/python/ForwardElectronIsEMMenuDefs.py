# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# default configuration of the Forward Electron IsEM Selectors

# Define GeV
GeV = 1000.0


def ForwardElectronIsEMLooseSelectorConfigMC15(theTool):
    '''
    These are the forward electron isEM definitions Loose
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20170711/ForwardElectronIsEMLooseSelectorCutDefs.conf"


def ForwardElectronIsEMMediumSelectorConfigMC15(theTool):
    '''
    These are the forward electron isEM definitions  Medium
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20170711/ForwardElectronIsEMMediumSelectorCutDefs.conf"


def ForwardElectronIsEMTightSelectorConfigMC15(theTool):
    '''
    These are the forward electron isEM definitions  Tight
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20170711/ForwardElectronIsEMTightSelectorCutDefs.conf"
