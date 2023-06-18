# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python

## FPGATrackSim Simulation Transform
# @version $Id: FPGATrackSimConstReduceConstGen_tf.py 718878 2016-01-20 17:27:51Z jwebster $ 
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
    executorSet = set()
    executorSet.add(athenaExecutor(name = 'FPGATrackSimConstantGen2ndStage', 
                                   skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimConstantGen.py', 
                                   substep='FPGATrackSimC2',
                                   inData=['FPGATrackSimMatrix'], outData=['FPGATrackSimGoodMatrix']      
                                  ))
    executorSet.add(athenaExecutor(name = 'FPGATrackSimMatrixReduction', 
                                   skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimMatrixReduction.py',
                                   inData=['FPGATrackSimGoodMatrix'], outData=['FPGATrackSimMatrixReduced'],
                                   substep='FPGATrackSimM2m'
                                  ))
    executorSet.add(athenaExecutor(name = 'FPGATrackSimConstantGen1stStageIgnore2ndStage', 
                                   skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimConstantGen.py',
                                   runtimeRunargs={'CheckGood2ndStage':0},
                                   inData=['FPGATrackSimMatrixReduced'], outData=['FPGATrackSimGoodMatrixReducedIgnore'],
                                   substep='FPGATrackSimC1I'
                                  ))
    executorSet.add(athenaExecutor(name = 'FPGATrackSimConstantGen1stStageCheck2ndStage', 
                                   skeletonFile = 'FPGATrackSimBankGen/skeleton.FPGATrackSimConstantGen.py',
                                   runtimeRunargs={'CheckGood2ndStage':1},
                                   inData=['FPGATrackSimMatrixReduced'], outData=['FPGATrackSimGoodMatrixReducedCheck'],
                                   substep='FPGATrackSimC1C'
                                  ))

    trf = transform(executor=executorSet, description = "FPGATrackSim Make 2nd stage constants, reduce matrix file, make 1st stage constants")

    addFPGATrackSimConstReduceGenArgs(trf.parser)
    addFPGATrackSimMapsArgs(trf.parser)

    return trf


def addFPGATrackSimConstReduceGenArgs(parser):
    # Add a specific FPGATrackSim argument group
    parser.defineArgGroup('FPGATrackSimConstReduceConst', 'Fast tracker simulation options')

    parser.add_argument('--CheckGood2ndStage', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True), 
                        help='Check whether 2nd stage sector is good when producing things for 1st stage', group='FPGATrackSimConstReduceConst')

    parser.add_argument('--bankregion', type=trfArgClasses.argFactory(trfArgClasses.argInt, runarg=True), 
                        help='Bank region number', group='FPGATrackSimConstReduceConst')
   
    parser.add_argument('--genconst', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True), 
                        help='Generate the sectors and constants', group='FPGATrackSimConstReduceConst')

    parser.add_argument('--allregions', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True), 
                        help='Run all regions?', group='FPGATrackSimConstReduceConst')
    
    parser.add_argument('--inputFPGATrackSimMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='input',type='htt_matrix'),help="input matrix file", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--inputFPGATrackSimMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='input',type='htt_matrix'),help="input matrix file", group="FPGATrackSimConstReduceConst")


    parser.add_argument('--outputFPGATrackSimGoodMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output',type='htt_matrix'), help="output good matrix file", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--inputFPGATrackSimGoodMatrixFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='input'), 
                       help="input good matrix file", group="FPGATrackSimConstReduceConst" )

    parser.add_argument('--outputFPGATrackSimMatrixReducedFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output',type='htt_matrix_reduced'), help="output reduced matrix file", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--outputFPGATrackSimGoodMatrixReducedIgnoreFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output',type='htt_matrix_good_reduced_ignore'), help="output reduced good matrix file, ignoring whether 2nd stage is good or not", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--outputFPGATrackSimGoodMatrixReducedCheckFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='output',type='htt_matrix_good_reduced_check'), help="output reduced good matrix file, check whether 2nd stage is good or not", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--inputFPGATrackSimMatrixReducedFile', type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile,runarg=True,io='input',type='htt_matrix_reduced'), help="input reduced matrix file", group="FPGATrackSimConstReduceConst")

    parser.add_argument('--extract1stStage', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True), 
                        help='Reduce 2nd stage matrix to 1st stage matrix', group='FPGATrackSimConstReduceConst')
    

if __name__ == '__main__':
    main()
