#!/bin/sh
#
# art-description: Reco_tf runs on 2022 CosmicCalo stream
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
#See data22_13p6TeV test for details of choices.
Reco_tf.py --CA --multithreaded  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_calib/data22_calib.00412340.physics_CosmicCalo.merge.RAW/data22_calib.00412340.physics_CosmicCalo.merge.RAW._lb0016._SFO-16._0001.1 --maxEvents=300 --conditionsTag="CONDBR2-BLKPA-2022-09" --geometryVersion="ATLAS-R3S-2021-03-01-00" --outputESDFile=myESD.pool.root --outputAODFile=myAOD.pool.root 

RES=$?
echo "art-result: $RES Reco"



