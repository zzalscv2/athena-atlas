# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

# User options, which can be set from command line after a "-" character
# athena FullCPAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaCommon.AthArgumentParser import AthArgumentParser
athArgsParser = AthArgumentParser()
athArgsParser.add_argument("--force-input", action = "store", dest = "force_input",
                           default = None,
                           help = "Force the given input file")
athArgsParser.add_argument("--force-output", action = "store", dest = "force_output",
                           default = None,
                           help = "Force the given output file")
athArgsParser.add_argument("--data-type", action = "store", dest = "data_type",
                           default = "data",
                           help = "Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgsParser.add_argument( '--block-config', dest='block_config',
                            action = 'store_true', default = False,
                            help = 'Configure the job with block configuration' )
athArgsParser.add_argument( '--for-compare', dest='for_compare',
                            action = 'store_true', default = False,
                            help = 'Configure the job for comparison of sequences vs blocks' )
athArgsParser.add_argument( '--no-systematics', dest='no_systematics',
                            action = 'store_true', default = False,
                            help = 'Configure the job to with no systematics' )
athArgsParser.add_argument( '--physlite', dest='physlite',
                            action = 'store_true', default = False,
                            help = 'Configure the job for physlite' )
athArgsParser.add_argument( '--no-physlite-broken', dest='no_physlite_broken',
                            action = 'store_true', default = False,
                            help = 'Configure the job to skip algorithms that fail on physlite test file' )
athArgs = athArgsParser.parse_args()

dataType = athArgs.data_type
blockConfig = athArgs.block_config
forCompare = athArgs.for_compare
isPhyslite = athArgs.physlite
noPhysliteBroken = athArgs.no_physlite_broken
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

if isPhyslite :
    inputfile = {"data": 'ASG_TEST_FILE_LITE_DATA',
                 "mc":   'ASG_TEST_FILE_LITE_MC',
                 "afii": 'ASG_TEST_FILE_LITE_MC_AFII'}
else :
    inputfile = {"data": 'ASG_TEST_FILE_DATA',
                 "mc":   'ASG_TEST_FILE_MC',
                 "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
import AthenaPoolCnvSvc.ReadAthenaPool
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
if athArgs.force_input :
    testFile = athArgs.force_input
svcMgr.EventSelector.InputCollections = [testFile]

from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, blockConfig, forCompare=forCompare,
                       noSystematics = athArgs.no_systematics,
                       isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken)
print (algSeq) # For debugging

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
if not blockConfig :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTest." + dataType + ".hist.root' OPT='RECREATE'"
else :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest." + dataType + ".hist.root' OPT='RECREATE'"
if athArgs.force_output :
    outputFile = athArgs.force_output
ServiceMgr.THistSvc.Output += [ outputFile ]

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )
