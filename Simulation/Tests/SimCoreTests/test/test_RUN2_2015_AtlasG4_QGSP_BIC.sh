#!/bin/sh
#
# art-description: Run simulation outside ISF, with alternative QGSP_BIC physics list, reading in SU3 SUSY events, using 2015 geometry and conditions
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
--CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/susy_SU3-herwigpp-7000.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '5' \
    --skipEvents '0' \
    --randomSeed '42' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'QGSP_BIC' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationNoIoV' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/susy_SU3-herwigpp-7000.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '5' \
    --skipEvents '0' \
    --randomSeed '42' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'QGSP_BIC' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationNoIoV.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/susy_SU3-herwigpp-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '5' \
    --skipEvents '0' \
    --randomSeed '42' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'QGSP_BIC' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationNoIoV.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
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
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 5 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
