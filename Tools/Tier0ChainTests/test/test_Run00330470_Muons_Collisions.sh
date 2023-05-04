#!/bin/sh
#
# art-description: RecoTrf with beamType=collisions
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: master/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-output: log.*

Reco_tf.py  --outputHISTFile=myMergedMonitoring_Muons_0.root --maxEvents=500 --outputESDFile=myESD_Muons_0.pool.root --outputAODFile=myAOD_Muons_0.AOD.pool.root --outputTAGFile=myTAG_Muons_0.root --ignoreErrors=False --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.merge.DRAW_ZMUMU.f843_m1831._0245.1 --preExec='rec.doTrigger=False;' --imf False --conditionsTag=CONDBR2-BLKPA-2018-03 --geometryVersion=ATLAS-R2-2016-01-00-01
echo "art-result: $? Reco"

ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_21.0_references/$2
art.py compare ref --entries 10 . $ArtRef
echo "art-result: $? Diff"
