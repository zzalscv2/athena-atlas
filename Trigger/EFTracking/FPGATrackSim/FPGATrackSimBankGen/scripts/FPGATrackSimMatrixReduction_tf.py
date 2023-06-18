# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python

## FPGATrackSim Simulation Transform
# @version $Id: FPGATrackSimMatrixReduction_tf.py 686533 2015-07-30 15:33:01Z tomoya $
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
    trf = transform(executor = athenaExecutor(name = 'FPGATrackSimMatrixReduction',
                                              skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimMatrixReduction.py'))
    addFPGATrackSimPattGenArgs(trf.parser)
    return trf


def addFPGATrackSimPattGenArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimMatrixReduction', 'Fast tracker simulation options')

    parser.add_argument('--NBanks', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Number of pattern banks', group='FPGATrackSimMatrixReduction')

    parser.add_argument('--bankregion', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Bank region number', group='FPGATrackSimMatrixReduction')

    parser.add_argument('--allregions', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='Merge all regions', group='FPGATrackSimMatrixReduction')

    parser.add_argument('--inputFPGATrackSimMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True),
                       help="input matrix file", group="FPGATrackSimMatrixReduction" )

if __name__ == '__main__':
    main()
