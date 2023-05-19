#!/bin/sh

# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-description: DAOD building PHYS valid1 MP
# art-type: grid
# art-output: checkFile.txt
# art-output: checkxAOD.txt
# art-output: checkIndexRefs*.txt
# art-athena-mt: 8

set -e

Derivation_tf.py --CA --athenaopts='--nprocs=2' --athenaMPMergeTargetSize 'DAOD_*:0' --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e5112_s3227_r13145/AOD.27253804._000061.pool.root.1 --outputDAODFile art.pool.root --formats PHYS --maxEvents -1

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_PHYSFile DAOD_PHYS.art.pool.root --outputDAOD_PHYS_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_PHYS.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYS.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYS.art.pool.root > checkIndexRefs_PHYS.txt 2>&1

echo "art-result: $?  checkIndexRefs PHYS"
