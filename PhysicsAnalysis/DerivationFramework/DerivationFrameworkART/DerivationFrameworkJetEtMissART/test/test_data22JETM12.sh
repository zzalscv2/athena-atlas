#!/bin/sh

# art-include: main/Athena
# art-include: 23.0/Athena
# art-description: DAOD building JETM12 data22
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--CA True \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1 \
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
