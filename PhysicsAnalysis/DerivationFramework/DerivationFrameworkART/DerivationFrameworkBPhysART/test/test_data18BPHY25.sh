#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building BPHY25 data18
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_13TeV.00364292.physics_Main.merge.AOD.f1002_m2037._lb0163._0006.1 --outputDAODFile art.pool.root --reductionConf BPHY25 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-49"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( "InDetTrackParticlesFixer", Containers = [ "InDetTrackParticlesAux." ] )' 

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_BPHY25File DAOD_BPHY25.art.pool.root --outputDAOD_BPHY25_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_BPHY25.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY25.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"
