#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building BPHY16 mc20
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--CA True \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1 \
--outputDAODFile art.pool.root \
--formats BPHY16 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_BPHY16.art.pool.root > checkFile_BPHY16.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY16.art.pool.root > checkxAOD_BPHY16.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_BPHY16.art.pool.root > checkIndexRefs_BPHY16.txt 2>&1

echo "art-result: $?  checkIndexRefs"
