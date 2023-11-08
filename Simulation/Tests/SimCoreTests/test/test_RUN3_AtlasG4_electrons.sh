#!/bin/sh
#
# art-description: Tests detector response to single electrons, generated on-the-fly, using Run3 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl


AtlasG4_tf.py \
    --CA \
    --preExec 'flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_SingleElectronCfg"' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '1000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --runNumber '999999' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False
rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '1000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --preInclude 'AtlasG4Tf:SimulationJobOptions/preInclude.SingleElectronGenerator.py,Campaigns/MC23SimulationSingleIoV.py' \
    --runNumber '999999' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '1000' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --preInclude 'AtlasG4Tf:SimulationJobOptions/preInclude.SingleElectronGenerator.py,Campaigns/MC23SimulationSingleIoV.py' \
    --runNumber '999999' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
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
