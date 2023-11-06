#!/bin/sh

# art-description: OverlayChain+Reco test for data16.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: main/Athena

# art-output: *.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

OverlayChain_tf.py \
--inputZeroBiasBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1 \
--DataRunNumber 2015 \
--inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.merge.EVNT.e3601_e5984/EVNT.12228944._002158.pool.root.1 \
--outputRDOFile testRTT.RDO.pool.root \
--outputHITSFile testRTT.HITS.pool.root \
--triggerBit L1_ZB \
--skipEvents 0 --maxEvents 10 --randomSeed 123456789 \
--geometryVersion ATLAS-R2-2015-03-01-00 \
--conditionsTag CONDBR2-BLKPA-RUN2-10 \
--digiSeedOffset1=211 --digiSeedOffset2=122 \
--samplingFractionDbTag FTFP_BERT_BIRK \
--preInclude 'sim:EventOverlayJobTransforms/custom.py,EventOverlayJobTransforms/magfield.py' 'overlayBS:EventOverlayJobTransforms/custom.py' \
--preExec 'from LArROD.LArRODFlags import larRODFlags;larRODFlags.nSamples.set_Value_and_Lock(4);from LArConditionsCommon.LArCondFlags import larCondFlags; larCondFlags.OFCShapeFolder.set_Value_and_Lock("4samples1phase")' \
--postInclude 'sim:EventOverlayJobTransforms/Rt_override_CONDBR2-BLKPA-2015-12.py,EventOverlayJobTransforms/muAlign.py,EventOverlayJobTransforms/g4runnumber.py' 'overlayBS:EventOverlayJobTransforms/Rt_override_CONDBR2-BLKPA-2015-12.py' \
--postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' \
--ignorePatterns "L1TopoMenuLoader.+ERROR." \
--imf False

rc=$?
status=$rc
echo "art-result: $rc dataoverlay"

rc2=-9999
if [ $rc -eq 0 ]
then
    Reco_tf.py \
    --inputRDOFile testRTT.RDO.pool.root \
    --outputESDFile testRTT.ESD.pool.root \
    --outputAODFile testRTT.AOD.pool.root \
    --preInclude 'EventOverlayJobTransforms/custom.py,EventOverlayJobTransforms/recotrfpre.py' \
    --postInclude 'all:EventOverlayJobTransforms/Rt_override_CONDBR2-BLKPA-2015-12.py' 'r2e:EventOverlayJobTransforms/muAlign_reco.py' \
    --preExec 'from LArConditionsCommon.LArCondFlags import larCondFlags;larCondFlags.OFCShapeFolder.set_Value_and_Lock("4samples1phase");rec.doTrigger=False;rec.runUnsupportedLegacyReco=True;from CaloRec.CaloCellFlags import jobproperties;jobproperties.CaloCellFlags.doLArThinnedDigits.set_Value_and_Lock(False)' \
    --ignorePatterns "L1TopoMenuLoader.+ERROR." \
    --imf False
    rc2=$?
    status=$rc2
fi
echo "art-result: $rc2 reco"

if command -v art.py >/dev/null 2>&1; then
    rc3=-9999
    if [ $rc -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --file testRTT.RDO.pool.root --diff-root
        rc3=$?
        status=$rc3
    fi
    echo "art-result: $rc3 regression"
fi

exit $status
