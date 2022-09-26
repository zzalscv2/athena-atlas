# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack, Teng Jian Khoo

# User options, which can be set from command line after a "-" character
# athena EgammaAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaCommon.AthArgumentParser import AthArgumentParser
athArgsParser = AthArgumentParser()
athArgsParser.add_argument("--data-type", action = "store", dest = "data_type",
                           default = "data",
                           help = "Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgs = athArgsParser.parse_args()

dataType = athArgs.data_type
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

inputfile = {"data": 'ASG_TEST_FILE_DATA',
             "mc":   'ASG_TEST_FILE_MC',
             "afii": 'ASG_TEST_FILE_MC_AFII'}

jetContainer = "AntiKt4EMPFlowJets"

# Set up the reading of the input file:
import AthenaPoolCnvSvc.ReadAthenaPool
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
svcMgr.EventSelector.InputCollections = [testFile]

from JetAnalysisAlgorithms.JetAnalysisAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, jetContainer)
print (algSeq) # For debugging

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += [
    "ANALYSIS DATAFILE='JetAnalysisAlgorithmsTestPFlow." + dataType + ".hist.root' OPT='RECREATE'"
    ]

# Workaround for running the event selection algs on MC with multiple event weights
from AthenaCommon.Configurable import ConfigurableRun3Behavior
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg

with ConfigurableRun3Behavior():
    cfg = CutFlowSvcCfg(ConfigFlags)

from AthenaConfiguration.LegacySupport import conf2toConfigurable, appendCAtoAthena
appendCAtoAthena(cfg)

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )
