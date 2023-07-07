#!/bin/sh

# art-description: MC+MC Overlay without reco for MC20e, ttbar
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 22.0-mc20/Athena
# art-include: 23.0/Athena
# art-include: main/Athena

# art-output: *.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

events=20
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"

Overlay_tf.py \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile MC_plus_MC.RDO.pool.root \
--maxEvents ${events} \
--skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag OFLCOND-MC16-SDR-RUN2-09  \
--geometryVersion ATLAS-R2-2016-01-00-01 \
--preInclude 'all:Campaigns/MC20e.py' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlay"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --file MC_plus_MC.RDO.pool.root --diff-root
    rc2=$?
    status=$rc2
fi
echo "art-result: $rc2 regression"

exit $status
