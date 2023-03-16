#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building BPHY24 mc16
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc16_13TeV.801756.P8BEG_A4_CTEQ6L1_Bd_K0_mu4mu4.recon.AOD.e8392_a875_r10724/AOD.29384833._000009.pool.root.1 --outputDAODFile art.pool.root --reductionConf BPHY24 --maxEvents 5000 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-49" ' 

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_BPHY24File DAOD_BPHY24.art.pool.root --outputDAOD_BPHY24_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_BPHY24.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY24.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"
