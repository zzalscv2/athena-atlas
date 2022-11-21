#!/bin/sh

# art-include: master/Athena
# art-description: DAOD building EGAM8 EGAM9 JETM1 LLP1 mc21
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
--formats EGAM8 EGAM9 JETM1 LLP1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_EGAM8.art.pool.root > checkFile_EGAM8.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM8.art.pool.root > checkxAOD_EGAM8.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM8.art.pool.root > checkIndexRefs_EGAM8.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM9.art.pool.root > checkFile_EGAM9.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM9.art.pool.root > checkxAOD_EGAM9.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM9.art.pool.root > checkIndexRefs_EGAM9.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM1.art.pool.root > checkFile_JETM1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM1.art.pool.root > checkxAOD_JETM1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM1.art.pool.root > checkIndexRefs_JETM1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_LLP1.art.pool.root > checkFile_LLP1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_LLP1.art.pool.root > checkxAOD_LLP1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_LLP1.art.pool.root > checkIndexRefs_LLP1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
