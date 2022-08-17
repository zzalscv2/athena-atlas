#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building HION13 mc15
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc15_5TeV.800830.P8B_A14_CTEQ6L1_SPS_2Jpsi_2mu3p5mu2p5mu1p5.recon.AOD.e8470_s3537_r11894/AOD.29653794._000001.pool.root.1 --outputDAODFile art.pool.root --reductionConf HION13 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True); rec.doHeavyIon.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags; BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-49"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( "BTaggingFixer", Containers=["BTagging_AntiKt4HIAux."] )'

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_HION13File DAOD_HION13.art.pool.root --outputDAOD_HION13_MRGFile art_merged.pool.root --preExec 'rec.doHeavyIon.set_Value_and_Lock(False)' 

echo "art-result: $? merge"

checkFile.py DAOD_HION13.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION13.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"
