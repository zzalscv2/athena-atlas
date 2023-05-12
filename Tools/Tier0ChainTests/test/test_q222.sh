#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-output: log.*

Reco_tf.py --AMI=q222 --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --outputHISTFile=myHIST.root --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1 --imf False
echo "art-result: $?"

rc2=-9999
if [ ${rc1} -eq 0 ]
    ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_22.0-mc20_references/$2
    art.py compare ref . $ArtRef --entries 10
    rc2=$?
fi
echo  "art-result: ${rc2} Diff"
