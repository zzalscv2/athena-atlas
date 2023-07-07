#!/bin/sh
#
# art-description: Tests detector response to single muons, generated on-the-fly, using 2015 geometry and conditions
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl


AtlasG4_tf.py \
    --CA \
    --preExec 'flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_SingleMuonCfg"' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '2000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --runNumber '999999' \
    --physicsList 'FTFP_BERT' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False
rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --preInclude 'SimulationJobOptions/preInclude.SingleMuonGenerator.py' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '2000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --runNumber '999999' \
    --physicsList 'FTFP_BERT' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --preInclude 'SimulationJobOptions/preInclude.SingleMuonGenerator.py' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '2000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --runNumber '999999' \
    --physicsList 'FTFP_BERT' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False

rc2=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CG
echo  "art-result: $rc simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --entries 10
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
