#!/bin/sh
#
# art-description: Reco_tf runs on 2020 IDCosmic stream, without trigger.
# art-athena-mt: 4
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena

Reco_tf.py  --AMI=x636 --athenaopts="--threads=8" --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_cos/data22_cos.00412343.physics_IDCosmic.merge.RAW/data22_cos.00412343.physics_IDCosmic.merge.RAW._lb0763._SFO-ALL._0001.1 --maxEvents=300 --outputESDFile=myESD.pool.root --outputAODFile=myAOD.pool.root --conditionsTag=CONDBR2-ES1PA-2022-06
RES=$?
echo "art-result: $RES Reco"
