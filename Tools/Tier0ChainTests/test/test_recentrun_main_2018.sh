#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-mc16a/Athena
# art-include: 21.0-mc16d/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: master/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-output: log.*

Reco_tf.py --AMI=f921 --maxEvents=250 --outputESDFile=myESD_Main.pool.root --outputAODFile=myAOD_Main.AOD.pool.root --outputTAGFile=myTAG_Main.root --outputHISTFile=myMergedMonitoring_Main.root --ignoreErrors=False --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00348618.physics_Main.daq.RAW._lb0220._SFO-1._0001.data
rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
    ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_21.0-mc16a_references/$2
    art.py compare ref --entries 10 . $ArtRef
    rc2=$?
fi
echo  "art-result: ${rc2} Diff"
