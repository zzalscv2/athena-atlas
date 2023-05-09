#!/usr/bin/env python

# Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration
import TopExamples.grid
import DerivationTags
import Data_rel21
import MC16_TOPQ1

config = TopExamples.grid.Config()
config.code          = 'top-xaod'
config.settingsFile  = 'custom-saver-test.txt'
config.gridUsername  = 'username'
config.suffix        = '04-05-23'
config.excludedSites = ''
config.noSubmit      = False # set to True if you just want to test the submission
config.CMake         = True # need to set to True for CMake-based releases (release 22)
config.mergeType     = 'Default' #'None', 'Default' or 'xAOD'
config.destSE        = '' #This is the default (anywhere), or try e.g. 'UKI-SOUTHGRID-BHAM-HEP_LOCALGROUPDISK'


###Data - look in Data_rel21.py #TO BE UPDATED
###MC Simulation - look in MC16_TOPQ1.py #TO BE UPDATED
###Using a few test samples produced with release 22
###Edit these lines if you don't want to run everything!
names = [
    'TOPQ1_ttbar_PowPy8',
#    'Data16_TOPQ1',
]

samples = TopExamples.grid.Samples(names)
TopExamples.grid.submit(config, samples)

