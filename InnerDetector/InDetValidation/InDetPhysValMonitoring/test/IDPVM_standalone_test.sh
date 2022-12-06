#!/bin/bash

##################################################
# This is a simple test for running the 
# standalone job options of the InDetPhysValMonitoring 
# package. Will not attempt to compare the outputs, 
# but only test for a successful run of athena. 
################################################### 

# Use a random AOD on CVMFS as input - we do not really care about the results, 
# the intention of the test is to ensure that the athena configuration
# for our setup is not broken 
TestFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/RDO/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/100events.RDO.pool.root"

# Run the standalone valdiation options, with additional plots enabled to ensure we also test the command line arg mechanism
athena --filesInput ${TestFile} InDetPhysValMonitoring/InDetPhysValMonitoring_topOptions.py - --doExpertPlots

exit $?
