#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building EXOT13 SUSY5 SUSY7 EXOT8 EXOT4 HIGG4D2 data18
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_13TeV.00364292.physics_Main.merge.AOD.f1002_m2037._lb0163._0006.1 --outputDAODFile art.pool.root --reductionConf EXOT13 SUSY5 SUSY7 EXOT8 EXOT4 HIGG4D2 --maxEvents 500  --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-49"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( "InDetTrackParticlesFixer", Containers = [ "InDetTrackParticlesAux." ] )'  --passThrough True 
