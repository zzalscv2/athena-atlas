#!/bin/sh

# art-include: master/Athena
# art-description: DAOD building EGAM3 EGAM4 JETM4 mc21
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
--formats EGAM3 EGAM4 JETM4 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_EGAM3.art.pool.root > checkFile_EGAM3.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM3.art.pool.root > checkxAOD_EGAM3.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM3.art.pool.root > checkIndexRefs_EGAM3.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM4.art.pool.root > checkFile_EGAM4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM4.art.pool.root > checkxAOD_EGAM4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM4.art.pool.root > checkIndexRefs_EGAM4.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM4.art.pool.root > checkFile_JETM4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM4.art.pool.root > checkxAOD_JETM4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM4.art.pool.root > checkIndexRefs_JETM4.txt 2>&1

echo "art-result: $?  checkIndexRefs"