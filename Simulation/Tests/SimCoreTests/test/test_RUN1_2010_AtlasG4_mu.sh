#!/bin/sh
#
# art-description: Run simulation outside ISF, using 2010 geometry and conditions, reading single muon events, writing HITS
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
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber '155697' \
    --geometryVersion 'ATLAS-R1-2010-02-00-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber '155697' \
    --geometryVersion 'ATLAS-R1-2010-02-00-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber '155697' \
    --geometryVersion 'ATLAS-R1-2010-02-00-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
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
