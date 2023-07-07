#!/bin/sh

# art-description: MC+data Overlay with MT support, running with 8 threads
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena

# art-output: MC_plus_data.MT.RDO.pool.root
# art-output: MC_plus_data.ST.RDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

export ATHENA_CORE_NUMBER=8

Overlay_tf.py \
--multithreaded \
--inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/DataOverlaySimulation/22.0/v1/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.HITS.pool.root \
--inputBS_SKIMFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1 \
--outputRDOFile MC_plus_data.MT.RDO.pool.root \
--triggerConfig 'Overlay=NONE' \
--maxEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag CONDBR2-BLKPA-RUN2-10 \
--samplingFractionDbTag FTFP_BERT_BIRK \
--preExec 'from LArROD.LArRODFlags import larRODFlags;larRODFlags.nSamples.set_Value_and_Lock(4);from LArConditionsCommon.LArCondFlags import larCondFlags; larCondFlags.OFCShapeFolder.set_Value_and_Lock("4samples1phase")' \
--postInclude 'EventOverlayJobTransforms/Rt_override_CONDBR2-BLKPA-2015-12.py' \
--postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlayMT"
mv log.Overlay log.OverlayMT

rc2=-9999
if [ $rc -eq 0 ]
then
    Overlay_tf.py \
    --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/DataOverlaySimulation/22.0/v1/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.HITS.pool.root \
    --inputBS_SKIMFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1 \
    --outputRDOFile MC_plus_data.ST.RDO.pool.root \
    --triggerConfig 'Overlay=NONE' \
    --maxEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
    --conditionsTag CONDBR2-BLKPA-RUN2-10 \
    --samplingFractionDbTag FTFP_BERT_BIRK \
    --preExec 'from LArROD.LArRODFlags import larRODFlags;larRODFlags.nSamples.set_Value_and_Lock(4);from LArConditionsCommon.LArCondFlags import larCondFlags; larCondFlags.OFCShapeFolder.set_Value_and_Lock("4samples1phase")' \
    --postInclude 'EventOverlayJobTransforms/Rt_override_CONDBR2-BLKPA-2015-12.py' \
    --postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' \
    --imf False
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 overlayST"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root MC_plus_data.ST.RDO.pool.root MC_plus_data.MT.RDO.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings RecoTimingObj_p1_HITStoRDO_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparison"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees --diff-root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
