#!/bin/sh
#
# art-description: RecoTrf no BS constraint on 2015 PbPb express stream (Run287281 LB124)
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: master/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-output: log.*

Reco_tf.py --outputESDFile=myESD_0.pool.root --outputAODFile=myAOD_0.AOD.pool.root --outputTAGFile=myTAG_0.root --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data15_hi.00287281.express_express.merge.RAW._lb0124._SFO-ALL._0001.1 --AMI=q314 --maxEvents=25 --preExec "'r2e:from InDetRecExample.InDetJobProperties import InDetFlags;InDetFlags.useBeamConstraint.set_Value_and_Lock(False)'" --imf False
echo "art-result: $? Reco"

ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_21.0_references/$2
art.py compare ref --entries 10 --file=*AOD*.pool.root . $ArtRef
echo "art-result: $? Diff"
