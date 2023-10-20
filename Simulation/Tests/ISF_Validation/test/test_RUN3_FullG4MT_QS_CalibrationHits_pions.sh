#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading single pion events, writing HITS including full CaloCalibrationHit information, using RUN3 geometry and conditions
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01
Sim_tf.py \
    --CA \
    --simulator 'FullG4MT_QS'  \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoVCalibrationHits' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --simulator 'FullG4MT_QS'  \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --preExec 'EVNTtoHITS:simFlags.ReleaseGeoModel=False;' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoVCalibrationHits.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --simulator 'FullG4MT_QS'  \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --preExec 'EVNTtoHITS:simFlags.ReleaseGeoModel=False;' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoVCalibrationHits.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS_OLD
echo  "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 HITS_OLDvsCA"

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
