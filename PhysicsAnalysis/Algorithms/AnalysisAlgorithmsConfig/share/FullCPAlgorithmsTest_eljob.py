#!/usr/bin/env python
#
# Copyright (C) 2002-2022  CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# Read the submission directory as a command line argument. You can
# extend the list of arguments with your private ones later on.
import optparse
parser = optparse.OptionParser()
parser.add_option( '-d', '--data-type', dest = 'data_type',
                   action = 'store', type = 'string', default = 'data',
                   help = 'Type of data to run over. Valid options are data, mc, afii' )
parser.add_option( '-s', '--submission-dir', dest = 'submission_dir',
                   action = 'store', type = 'string', default = 'submitDir',
                   help = 'Submission directory for EventLoop' )
parser.add_option( '-u', '--unit-test', dest='unit_test',
                   action = 'store_true', default = False,
                   help = 'Run the job in "unit test mode"' )
parser.add_option( '--direct-driver', dest='direct_driver',
                   action = 'store_true', default = False,
                   help = 'Run the job with the direct driver' )
parser.add_option( '--algorithm-timer', dest='algorithm_timer',
                   action = 'store_true', default = False,
                   help = 'Run the job with a timer for each algorithm' )
parser.add_option( '--block-config', dest='block_config',
                   action = 'store_true', default = False,
                   help = 'Run the job in "unit test mode"' )
parser.add_option( '--for-compare', dest='for_compare',
                   action = 'store_true', default = False,
                   help = 'Configure the job for comparison of sequences vs blocks' )
( options, args ) = parser.parse_args()

# Set up (Py)ROOT.
import ROOT
ROOT.xAOD.Init().ignore()

# Force-load some xAOD dictionaries. To avoid issues from ROOT-10940.
ROOT.xAOD.TauJetContainer()

# ideally we'd run over all of them, but we don't have a mechanism to
# configure per-sample right now

dataType = options.data_type
blockConfig = options.block_config
forCompare = options.for_compare

if dataType not in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

# Set up the sample handler object. See comments from the C++ macro
# for the details about these lines.
import os
sh = ROOT.SH.SampleHandler()
sh.setMetaString( 'nc_tree', 'CollectionTree' )
sample = ROOT.SH.SampleLocal (dataType)
if dataType == "data" :
    sample.add (os.getenv ('ASG_TEST_FILE_DATA'))
    pass
if dataType == "mc" :
    sample.add (os.getenv ('ASG_TEST_FILE_MC'))
    pass
if dataType == "afii" :
    sample.add (os.getenv ('ASG_TEST_FILE_MC_AFII'))
    pass
sh.add (sample)
sh.printContent()

# Create an EventLoop job.
job = ROOT.EL.Job()
job.sampleHandler( sh )
job.options().setDouble( ROOT.EL.Job.optMaxEvents, 500 )
if options.algorithm_timer :
    job.options().setBool( ROOT.EL.Job.optAlgorithmTimer, True )


from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, blockConfig, forCompare=forCompare)
print( algSeq ) # For debugging
algSeq.addSelfToJob( job )

# Make sure that both the ntuple and the xAOD dumper have a stream to write to.
job.outputAdd( ROOT.EL.OutputStream( 'ANALYSIS' ) )

# Find the right output directory:
submitDir = options.submission_dir
if options.unit_test:
    import os
    import tempfile
    submitDir = tempfile.mkdtemp( prefix = 'fullCPTest_'+dataType+'_', dir = os.getcwd() )
    os.rmdir( submitDir )
    pass


# Run the job using the local driver.  This is intentionally the local
# driver, unlike most other tests that use the direct driver.  That
# way it tests whether the code works correctly with that driver,
# which is a lot more similar to the way the batch/grid drivers work.
driver = ROOT.EL.LocalDriver()

if options.direct_driver :
    # this is for testing purposes, as only the direct driver respects
    # the limit on the number of events.
    driver = ROOT.EL.DirectDriver()

driver.submit( job, submitDir )
