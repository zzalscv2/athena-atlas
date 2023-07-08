#!/bin/sh

# art-description: MC+MC Overlay with MT support, running with 8 threads, new config
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-include: main/Athena

# art-output: mcOverlayRDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*
# art-output: *.pkl
# art-output: *Config.txt

export ATHENA_CORE_NUMBER=8

HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"


Overlay_tf.py \
--CA \
--multithreaded \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile mcOverlayRDO.pool.root \
--maxEvents 50 --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag OFLCOND-MC16-SDR-RUN2-09 \
--geometryVersion ATLAS-R2-2016-01-00-01 \
--preInclude 'all:Campaigns.MC20e' \
--postInclude 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
--postExec 'with open("ConfigOverlay.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlay"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees --diff-root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
