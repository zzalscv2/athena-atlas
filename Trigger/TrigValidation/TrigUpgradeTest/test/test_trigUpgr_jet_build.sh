#!/bin/bash
# art-description: athenaMT trigger test using simpleJetJob job options
# art-type: build
# art-include: master/Athena
# art-output: *.log
# art-output: *.new
# art-output: *.txt
# art-output: *.root

export SKIPEVENTS=5
export EVENTS=10
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/simpleJetJob.py"

# This test doesn't produce any output useful for RegTest, so do a dummy reference comparison
export REGTESTEXP="leaving with code"
export REGTESTREF=`find_data.py TrigUpgradeTest/dummy_regtest.ref`

# Skip dumping chain counts because this test doesn't produce the histogram including them
export SKIP_CHAIN_DUMP=1

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh
