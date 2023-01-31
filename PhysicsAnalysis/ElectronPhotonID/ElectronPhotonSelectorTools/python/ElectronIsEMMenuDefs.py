# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Cnfiguration of the Electron IsEM Selector

# Define GeV
GeV = 1000.0


def ElectronIsEMLooseSelectorConfigDC14(theTool):
    '''
    These are the cut base isEM definitions: Loose
    '''
    
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20150329/ElectronIsEMLooseSelectorCutDefs.conf"


def ElectronIsEMMediumSelectorConfigDC14(theTool):
    '''
    These are the cut base isEM definitions: Medium
    '''
    
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20150329/ElectronIsEMMediumSelectorCutDefs.conf"


def ElectronIsEMTightSelectorConfigDC14(theTool):
    '''
    These are the cut base isEM definitions: Tight
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20150329/ElectronIsEMTightSelectorCutDefs.conf"


def TrigElectronIsEMLooseSelectorConfigDC14(theTool):
    '''
    This is for the Loose isEM definitions for the Trigger.
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150329/ElectronIsEMLooseSelectorCutDefs.conf"


def TrigElectronIsEMMediumSelectorConfigDC14(theTool):
    '''
    This is for the Medium++ isEM definitions for the LATEST Trigger.
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150329/ElectronIsEMMediumSelectorCutDefs.conf"


def TrigElectronIsEMTightSelectorConfigDC14(theTool):
    '''
    This is for the Tight MC15 LATEST isEM definitions for the Trigger.
    '''

    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150329/ElectronIsEMTightSelectorCutDefs.conf"
