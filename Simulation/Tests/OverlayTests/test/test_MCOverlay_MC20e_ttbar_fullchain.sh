#!/bin/sh

# art-description: MC+MC Overlay chain for MC20e, ttbar, full reco chain
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

events=3
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"

Reco_tf.py \
--CA "all:True" "RDOtoRDOTrigger:False" \
--autoConfiguration everything \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02 \
--geometryVersion default:ATLAS-R2-2016-01-00-01 \
--maxEvents ${events} --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--preInclude "all:Campaigns.MC20e" \
--postInclude "default:PyJobTransforms.UseFrontier" \
--runNumber 410470 \
--steering "doOverlay" "doRDO_TRIG" \
--triggerConfig "RDOtoRDOTrigger=MCRECO:DBF:TRIGGERDBMC:2233,87,314" --asetup "RDOtoRDOTrigger:Athena,21.0,latest" \
--outputRDOFile MC_plus_MC.RDO.pool.root \
--outputAODFile MC_plus_MC.AOD.pool.root \
--imf False

rc=$?
status=$rc
echo "art-result: $rc reco"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries ${events} "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --file MC_plus_MC.RDO.pool.root --diff-root
    rc2=$?
    status=$rc2
fi
echo "art-result: $rc2 regression"

exit $status
