#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-output: log.*

Reco_tf.py --AMI=q222 --athenaopts='--nprocs=2' --maxEvents=100 --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1

