# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python

## FPGATrackSim Simulation Transform
# @version $Id: FPGATrackSimMatrixMerge_tf.py 574395 2013-12-07 10:13:16Z gvolpi $
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

from FPGATrackSimConfig.parseRunArgs import addFPGATrackSimBanksArgs
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
    trf = transform(executor = athenaExecutor(name = 'FPGATrackSimMatrixMerge',
                                              skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimMatrixMerge.py'))
    addFPGATrackSimBanksArgs(trf.parser)
    addFPGATrackSimPattGenArgs(trf.parser)
    return trf


def addFPGATrackSimPattGenArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimMatrixMerge', 'Fast tracker simulation options')

    parser.add_argument('--NBanks', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Number of pattern banks', group='FPGATrackSimMatrixMerge')

    parser.add_argument('--bankregion', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help='Bank region number', group='FPGATrackSimMatrixMerge')

    parser.add_argument('--allregions', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='Merge all regions', group='FPGATrackSimMatrixMerge')

    parser.add_argument('--genconst', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='Generate the sectors and constants', group='FPGATrackSimMatrixMerge')

    parser.add_argument('--inputFPGATrackSimMatrixFile',
                        type=trfArgClasses.argFactory(trfArgClasses.argList, runarg=True),
                        help='List of matrix files',group='FPGATrackSimMatrixMerge',nargs='+')

    parser.add_argument('--FPGATrackSimMatrixFileRegEx', group="FPGATrackSimMatrixMerge",
                        type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help="Regular expression to select the input files from the working dir")

    parser.add_argument('--MaxInputFiles', group="FPGATrackSimMatrixMerge",
                        type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True),
                        help="Max number of input files to include, only interpretted when used in combination with FPGATrackSimMatrixFileRegEx")

    parser.add_argument('--outputMergedFPGATrackSimMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True),
                       help="Output file", group="FPGATrackSimMatrixMerge" )
if __name__ == '__main__':
    main()
