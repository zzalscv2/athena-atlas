# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python

## FPGATrackSim Simulation Transform
# @version $Id: FPGATrackSimConstantGen_tf.py 718878 2016-01-20 17:27:51Z jwebster $
import argparse
import sys
import time
import traceback

import logging

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.trfExitCodes import trfExit
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace

import PyJobTransforms.trfExceptions as trfExceptions
import PyJobTransforms.trfArgClasses as trfArgClasses
from FPGATrackSimConfig.parseRunArgs import addFPGATrackSimMapsArgs

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s' % sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)


## Get the base transform with all arguments added
def getTransform():
    trf = transform(executor = athenaExecutor(name = 'FPGATrackSimConstantGen',
                                              skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimConstantGen.py'))
    addFPGATrackSimPattGenArgs(trf.parser)
    addFPGATrackSimMapsArgs(trf.parser)
    return trf


def addFPGATrackSimPattGenArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimConstantGen', 'Fast tracker simulation options')

    parser.add_argument('--NBanks', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Number of pattern banks', group='FPGATrackSimConstantGen')

    parser.add_argument('--bankregion', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Bank region number', group='FPGATrackSimConstantGen')

    parser.add_argument('--genconst', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='Generate the sectors and constants', group='FPGATrackSimConstantGen')

    parser.add_argument('--skip_sectors', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help='Sectors to skip', group='FPGATrackSimConstantGen')

    parser.add_argument('--inputFPGATrackSimMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True),
                       help="input matrix file", group="FPGATrackSimConstantGen" )

    parser.add_argument('--outputFPGATrackSimGoodMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output'), help="output good matrix file", group="FPGATrackSimConstantGen")


    parser.add_argument('--outputFPGATrackSimGoodMatrixReducedCheckFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output',type='htt_matrix_good_reduced_check'), help="output reduced good matrix file, check whether 2nd stage is good or not", group="FPGATrackSimConstantGen")


if __name__ == '__main__':
    main()
