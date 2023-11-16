#!/bin/sh
#
# art-description: Tests ATLAS + AFP simulation in ST and MT, generating events on-the-fly

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.MT.CA.HITS.pool.root
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --preInclude 'ForwardTransportSvc.ForwardTransportSvcConfig.ForwardTransportBeta90mPreInclude' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/EVNT.ALFA.pool.root' \
    --outputHITSFile 'test.MT.CA.HITS.pool.root' \
    --maxEvents '100' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'FTFP_BERT' \
    --AFPOn 'True' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False
rc=$?
echo  "art-result: $rc MTsim_CA"
status=$rc


AtlasG4_tf.py \
    --multithreaded \
    --preInclude 'ForwardTransportSvc/preInclude.ForwardTransportFlags_3.5TeV_0090.00m_nominal_v02.py,ForwardTransportSvc/ForwardTransportSvcConfig.ALFA.py' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/EVNT.ALFA.pool.root' \
    --outputHITSFile 'test.MT.HITS.pool.root' \
    --maxEvents '100' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'FTFP_BERT' \
    --AFPOn 'True' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False

rc2=$?
echo  "art-result: $rc2 MTsim"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [[ $rc -eq 0 ]] && [[ $rc2 -eq 0 ]]
then
    acmd.py diff-root test.MT.HITS.pool.root test.MT.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    if [ $status -eq 0 ]
    then
        status=$rc3
    fi
fi
echo  "art-result: $rc3 OLDvsCA"

unset ATHENA_CORE_NUMBER
AtlasG4_tf.py \
    --preInclude 'ForwardTransportSvc/preInclude.ForwardTransportFlags_3.5TeV_0090.00m_nominal_v02.py,ForwardTransportSvc/ForwardTransportSvcConfig.ALFA.py' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/EVNT.ALFA.pool.root' \
    --outputHITSFile 'test.ST.HITS.pool.root' \
    --maxEvents '100' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'FTFP_BERT' \
    --AFPOn 'True' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --imf False
rc4=$?
if [ $status -eq 0 ]
then
    status=$rc4
fi
echo  "art-result: $rc4 STsim"

rc5=-9999
if [[ $rc2 -eq 0 ]] && [[ $rc4 -eq 0 ]]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc5=$?
    if [ $status -eq 0 ]
    then
        status=$rc5
    fi
fi
echo  "art-result: $rc5 comparision"

exit $status
