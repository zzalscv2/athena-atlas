#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TRUTH1 mc21
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--CA True \
--inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1 \
--outputDAODFile art.pool.root \
--formats TRUTH1 \
--maxEvents 1000

echo "art-result: $? reco"

checkFile.py DAOD_TRUTH1.art.pool.root > checkFile_TRUTH1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRUTH1.art.pool.root > checkxAOD_TRUTH1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRUTH1.art.pool.root > checkIndexRefs_TRUTH1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
