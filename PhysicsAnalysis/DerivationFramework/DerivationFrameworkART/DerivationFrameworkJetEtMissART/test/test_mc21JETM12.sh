#!/bin/sh

# art-include: main/Athena
# art-include: 23.0/Athena
# art-description: DAOD building JETM12 mc21
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--CA True \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/AOD/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8453_s3873_r13829/1000events.AOD.29787656._000153.pool.root.1 \
--outputDAODFile art.pool.root \
--formats JETM12 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_JETM12.art.pool.root > checkFile_JETM12.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM12.art.pool.root > checkxAOD_JETM12.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM12.art.pool.root > checkIndexRefs_JETM12.txt 2>&1

echo "art-result: $?  checkIndexRefs"
