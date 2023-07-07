#!/bin/sh
#
# art-description: Run test-beam simulation outside ISF, generating events on-the-fly, writing HITS
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test*HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

TestBeam_tf.py \
    --CA \
    --DataRunNumber '1' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --conditionsTag 'OFLCOND-MC16-SDR-RUN2-09' \
    --maxEvents '10' \
    --Eta '0.35' \
    --testBeamConfig 'tbtile' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False
rc=$?
mv log.TestBeamTf log.TestBeamTf.CA
echo  "art-result: $rc simCA"
status=$rc

TestBeam_tf.py \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --Eta '0.35' \
    --testBeamConfig 'tbtile' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

TestBeam_tf.py \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '10' \
    --Eta '0.35' \
    --testBeamConfig 'tbtile' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --imf False

rc2=$?
mv log.TestBeamTf log.TestBeamTf.CG
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
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
