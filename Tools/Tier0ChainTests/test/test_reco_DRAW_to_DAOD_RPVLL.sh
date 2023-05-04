#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: master/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-output: log.*

Reco_tf.py --AMI=r10786 --maxEvents=25 --ignoreErrors=False --inputBSFile='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00334678.physics_Main.recon.DRAW_RPVLL.r10203/DRAW_RPVLL.12882891._007592.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00334678.physics_Main.recon.DRAW_RPVLL.r10203/DRAW_RPVLL.12882891._004274.pool.root.1' --outputAODFile=myDAOD_RPVLL.pool.root --imf=False

echo "art-result: $? Reco"

ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_21.0_references/$2
art.py compare ref --entries 10 . $ArtRef
echo "art-result: $? Diff"
