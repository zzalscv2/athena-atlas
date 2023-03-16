#!/bin/sh
#
# art-description: Reading in single particle gen events, writing out full CaloCalibrationHit information, using RUN3 geometry and conditions
# art-include: 23.0/Athena
# art-include: master/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoVCalibrationHits' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
AtlasG4_tf.py \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationSingleIoVCalibrationHits.py' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationSingleIoVCalibrationHits.py' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False

rc2=$?
mv log.AtlasG4Tf log.AtlasG4Tf_OLD
echo  "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
