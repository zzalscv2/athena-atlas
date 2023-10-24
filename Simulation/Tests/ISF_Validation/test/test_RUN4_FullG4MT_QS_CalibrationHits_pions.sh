#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading single pion events, writing HITS including full CaloCalibrationHit information, using RUN4 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# RUN4 setup
# ATLAS-P2-RUN4-03-00-00 and OFLCOND-MC15c-SDR-14-05
Sim_tf.py \
--CA \
--simulator 'FullG4MT_QS'  \
--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pi_E50_eta0-60.evgen.pool.root' \
--outputHITSFile 'test.HITS.pool.root' \
--maxEvents '10' \
--skipEvents '0' \
--randomSeed '10' \
--geometryVersion 'default:ATLAS-P2-RUN4-03-00-00' \
--conditionsTag 'default:OFLCOND-MC15c-SDR-14-05' \
--preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation,SimuJobTransforms.CalHits,SimuJobTransforms.ParticleID' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
status=$rc
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
