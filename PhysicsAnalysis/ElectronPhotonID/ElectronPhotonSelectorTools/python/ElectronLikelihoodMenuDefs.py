# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration



def ElectronLikelihoodLooseTriggerConfig2015(theTool):
    '''
    This is for the custom implementation of the Loose, No GSF-variable (trigger-friendly), no d0significance (for now) definitions.
    This uses Online PDFs, and currently has no conversion bit either. Also note that the PDF for trackd0 comes from
    trig_EF_trackd0_physics, but the LH tool requires it to be named el_trackd0pvunbiased. (NOTE: same signal eff as offline Loosepp + 1%)
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150712/ElectronLikelihoodLooseTriggerConfig2015.conf"


def ElectronLikelihoodMediumTriggerConfig2015(theTool):
    '''
    This is for the custom implementation of the Medium, No GSF-variable (trigger-friendly), no d0significance (for now) definitions.
    This uses Online PDFs, and currently has no conversion bit either. Also note that the PDF for trackd0 comes from
    trig_EF_trackd0_physics, but the LH tool requires it to be named el_trackd0pvunbiased. (NOTE: same signal eff as Offline Mediumpp + 1%)
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150712/ElectronLikelihoodMediumTriggerConfig2015.conf"


def ElectronLikelihoodTightTriggerConfig2015(theTool):
    '''
    This is for the custom implementation of the Tight, No GSF-variable (trigger-friendly), no d0significance (for now) definitions.
    This uses Online PDFs, and currently has no conversion bit either. Also note that the PDF for trackd0 comes from
    trig_EF_trackd0_physics, but the LH tool requires it to be named el_trackd0pvunbiased. (NOTE: same signal eff as offline Tightpp + 1%)
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/mc15_20150712/ElectronLikelihoodTightTriggerConfig2015.conf"

# Standard configuration MC20
def ElectronLikelihoodVeryLooseOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the VeryLoose offline likelihood for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf"


def ElectronLikelihoodLooseOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the Loose offline likelihood for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf"

def ElectronLikelihoodLooseBLOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the Loose + b-layer offline likelihood for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodLooseOfflineConfig2017_CutBL_Smooth.conf"

def ElectronLikelihoodMediumOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the Medium offline likelihood for MC20 / Run2 / Release 22.
    (NOTE: same signal eff as offline Mediumpp + 1%)
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodMediumOfflineConfig2017_Smooth.conf"

def ElectronLikelihoodTightOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the Tight offline likelihood for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodTightOfflineConfig2017_Smooth.conf"

# LLP Configuration MC20
def ElectronLikelihoodVeryLooseLLPOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the VeryLooseLLP offline likelihood for LRT electrons for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodVeryLooseTriggerConfig_NoPix.conf"


def ElectronLikelihoodLooseLLPOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the LooseLLP offline likelihood for LRT electrons  for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodLooseTriggerConfig_NoPix.conf"

def ElectronLikelihoodMediumLLPOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the MediumLLP offline likelihood for LRT electrons  for MC20 / Run2 / Release 22.
    (NOTE: same signal eff as offline Mediumpp + 1%)
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodMediumTriggerConfig_NoPix.conf"

def ElectronLikelihoodTightLLPOfflineConfigMC20(theTool):
    '''
    This is for the custom implementation of the TightLLP offline likelihood for LRT electrons  for MC20 / Run2 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodTightTriggerConfig_NoPix.conf"

# Standard configuration MC21
def ElectronLikelihoodVeryLooseOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the VeryLoose offline likelihood for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf"

def ElectronLikelihoodLooseOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the Loose offline likelihood for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf"

def ElectronLikelihoodLooseBLOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the Loose + b-layer offline likelihood for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodLooseOfflineConfig2017_CutBL_Smooth.conf"

def ElectronLikelihoodMediumOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the Medium offline likelihood for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodMediumOfflineConfig2017_Smooth.conf"

def ElectronLikelihoodTightOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the Tight offline likelihood for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodTightOfflineConfig2017_Smooth.conf"

# LLP Configuration MC21
def ElectronLikelihoodVeryLooseLLPOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the VeryLooseLLP offline likelihood for LRT electrons  for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodVeryLooseTriggerConfig_NoPix.conf"

def ElectronLikelihoodLooseLLPOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the LooseLLP offline likelihood for LRT electrons  for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodLooseTriggerConfig_NoPix.conf"

def ElectronLikelihoodMediumLLPOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the MediumLLP offline likelihood for LRT electrons  for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodMediumTriggerConfig_NoPix.conf"

def ElectronLikelihoodTightLLPOfflineConfigMC21(theTool):
    '''
    This is for the custom implementation of the TightLLP offline likelihood for LRT electrons  for MC21 / Run3 / Release 22.
    '''
    theTool.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodTightTriggerConfig_NoPix.conf"