#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@file FPGATrackSimLogicalHitsToAlg_tf.py
@date July 14, 2020
@author Riley Xu - riley.xu@cern.ch
@brief This transform runs FPGATrackSimLogicalHitsProcessAlgo.

Usage:
    FPGATrackSimLogicalHitsToAlg_tf.py \
            --maxEvents 100 \
            --InFileName "hits.raw.root" \
            --OutFileName "hits.logical.root" \
'''

import argparse
import sys
import time
import traceback
import logging

from PyJobTransforms.trfLogger import msg
from PyJobTransforms.trfExitCodes import trfExit
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace

import PyJobTransforms.trfExceptions as trfExceptions
import PyJobTransforms.trfArgClasses as trfArgClasses

from FPGATrackSimConfTools.parseRunArgs import addFPGATrackSimMapsArgs, addFPGATrackSimBanksArgs, addFPGATrackSimAlgorithmsArgs, addFPGATrackSimHitFilteringArgs

@stdTrfExceptionHandler
@sigUsrStackTrace

# ___________________________________________________________________________ #
def main():

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)


# ___________________________________________________________________________ #
def getTransform():
    # Get the base transform with all arguments added
    trf = transform(executor = athenaExecutor(name = 'FPGATrackSimLogicalHitsToAlg',
                                              skeletonFile = 'FPGATrackSimAlgorithms/skeleton.FPGATrackSimLogicalHitsToAlg.py'))
    addAthenaArguments(trf.parser)
    addFPGATrackSimMapsArgs(trf.parser)
    addFPGATrackSimBanksArgs(trf.parser)
    addFPGATrackSimAlgorithmsArgs(trf.parser)
    addFPGATrackSimLogicalHitsToAlgArgs(trf.parser)
    addFPGATrackSimHitFilteringArgs(trf.parser)
    return trf



# ___________________________________________________________________________ #
def addFPGATrackSimLogicalHitsToAlgArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimLogicalHitsToAlg', 'Options for FPGATrackSimLogicalHitsToAlg')

    # Enable easy copy-paste from C++ argument initializers
    def declareProperty(argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group='FPGATrackSimLogicalHitsToAlg')

    declareProperty("InFileName", trfArgClasses.argList, "input raw hit file path");
    declareProperty("InFileName2", trfArgClasses.argString, "input raw hit file path for 2nd input tool");
    declareProperty("OutFileName", trfArgClasses.argString, "output logical hit file path");


if __name__ == '__main__':
    main()
