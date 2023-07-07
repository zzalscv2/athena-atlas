#!/bin/bash
# art-description: AOD->HIST, R22 Run 2 data AOD
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: ExampleMonitorOutput.root
# art-output: log*

art.py download Tier0ChainTests test_q442.sh
AODFILE=(./ref-*/myAOD.pool.root)
Run3DQTestingDriver.py --inputFiles=${AODFILE} DQ.Environment=AOD DQ.Steering.doHLTMon=False > log.HIST_Creation 2>&1

echo "art-result: $? HIST_Creation"
rm -rf ref-*

ArtPackage=$1
ArtJobName=$2
art.py download ${ArtPackage} ${ArtJobName}
REFFILE=(./ref-*/ExampleMonitorOutput.root)
hist_diff.sh ExampleMonitorOutput.root $REFFILE -x TIME_execute -i > log.HIST_Diff 2>&1
echo "art-result: $? HIST_Diff"
