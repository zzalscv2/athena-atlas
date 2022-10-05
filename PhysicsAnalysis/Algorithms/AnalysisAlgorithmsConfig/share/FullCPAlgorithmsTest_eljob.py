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
parser.add_option("--force-input", action = "store", dest = "force_input",
                  default = None,
                  help = "Force the given input file")
parser.add_option( '-u', '--unit-test', dest='unit_test',
                   action = 'store_true', default = False,
                   help = 'Run the job in "unit test mode"' )
parser.add_option( '--direct-driver', dest='direct_driver',
                   action = 'store_true', default = False,
                   help = 'Run the job with the direct driver' )
parser.add_option( '--max-events', dest = 'max_events',
                   action = 'store', type = 'int', default = 500,
                   help = 'Number of events to run' )
parser.add_option( '--algorithm-timer', dest='algorithm_timer',
                   action = 'store_true', default = False,
                   help = 'Run the job with a timer for each algorithm' )
parser.add_option( '--no-systematics', dest='no_systematics',
                   action = 'store_true', default = False,
                   help = 'Configure the job to with no systematics' )
parser.add_option( '--hard-cuts', dest='hard_cuts',
                   action = 'store_true', default = False,
                   help = 'Configure the job with harder cuts' )
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
inputfile = {"data": 'ASG_TEST_FILE_DATA',
             "mc":   'ASG_TEST_FILE_MC',
             "afii": 'ASG_TEST_FILE_MC_AFII'}
if options.force_input :
    sample.add (options.force_input)
else :
    sample.add (os.getenv (inputfile[dataType]))
sh.add (sample)
sh.printContent()

# Create an EventLoop job.
job = ROOT.EL.Job()
job.sampleHandler( sh )
if options.max_events > 0:
    job.options().setDouble( ROOT.EL.Job.optMaxEvents, options.max_events )
if options.algorithm_timer :
    job.options().setBool( ROOT.EL.Job.optAlgorithmTimer, True )


from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, blockConfig, forCompare=forCompare, noSystematics = options.no_systematics, hardCuts = options.hard_cuts)
print( algSeq ) # For debugging
algSeq.addSelfToJob( job )

# Make sure that both the ntuple and the xAOD dumper have a stream to write to.
job.outputAdd( ROOT.EL.OutputStream( 'ANALYSIS' ) )

# Find the right output directory:
submitDir = options.submission_dir
if options.unit_test:
    job.options().setString (ROOT.EL.Job.optSubmitDirMode, 'unique')
else :
    job.options().setString (ROOT.EL.Job.optSubmitDirMode, 'unique-link')


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
