# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python

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

from FPGATrackSimConfTools.parseRunArgs import addFPGATrackSimMapsArgs, addFPGATrackSimBanksArgs, addFPGATrackSimAlgorithmsArgs

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
    trf = transform(executor = athenaExecutor(name = 'FPGATrackSimOutputMonitorAlg',
                                              skeletonFile = 'FPGATrackSimAlgorithms/skeleton.FPGATrackSimOutputMonitorAlg.py'))
    addAthenaArguments(trf.parser)
    addFPGATrackSimMapsArgs(trf.parser)
    addFPGATrackSimBanksArgs(trf.parser)
    addFPGATrackSimAlgorithmsArgs(trf.parser)
    addFPGATrackSimOutputMonitorAlgArgs(trf.parser)
    return trf



# ___________________________________________________________________________ #
def addFPGATrackSimOutputMonitorAlgArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimOutputMonitorAlg', 'Options for FPGATrackSimOutputMonitorAlg')

    # Enable easy copy-paste from C++ argument initializers
    def declareProperty(argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group='FPGATrackSimOutputMonitorAlg')

    declareProperty("InFileName", trfArgClasses.argString, "output logical hit file path");
    declareProperty("OutFileName", trfArgClasses.argString, "output monitoring histogram ROOT file path");
    declareProperty("OutputLevel", trfArgClasses.argInt, "set OutputLevel: DEBUG=2, INFO=3");


if __name__ == '__main__':
    main()
