# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

# User options, which can be set from command line after a "-" character
# athena FullCPAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaCommon.AthArgumentParser import AthArgumentParser
athArgsParser = AthArgumentParser()
athArgsParser.add_argument("--data-type", action = "store", dest = "data_type",
                           default = "data",
                           help = "Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgsParser.add_argument( '--block-config', dest='block_config',
                            action = 'store_true', default = False,
                            help = 'Configure the job with block configuration' )
athArgsParser.add_argument( '--for-compare', dest='for_compare',
                            action = 'store_true', default = False,
                            help = 'Configure the job for comparison of sequences vs blocks' )
athArgs = athArgsParser.parse_args()

dataType = athArgs.data_type
blockConfig = athArgs.block_config
forCompare = athArgs.for_compare
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

inputfile = {"data": 'ASG_TEST_FILE_DATA',
             "mc":   'ASG_TEST_FILE_MC',
             "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
import AthenaPoolCnvSvc.ReadAthenaPool
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
svcMgr.EventSelector.InputCollections = [testFile]

from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, blockConfig, forCompare=forCompare)
print (algSeq) # For debugging

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
if not blockConfig :
    ServiceMgr.THistSvc.Output += [
        "ANALYSIS DATAFILE='FullCPAlgorithmsTest." + dataType + ".hist.root' OPT='RECREATE'"
    ]
else :
    ServiceMgr.THistSvc.Output += [
        "ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest." + dataType + ".hist.root' OPT='RECREATE'"
    ]

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )
