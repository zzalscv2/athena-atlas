#!/bin/sh
#
# art-description: Reco_tf runs on 13.6TeV collision data 2022
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena

Reco_tf.py --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data --maxEvents 300 --AMI=f1263 --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

#Remember retval of transform as art result
RES=$?
echo "art-result: $RES Reco"

