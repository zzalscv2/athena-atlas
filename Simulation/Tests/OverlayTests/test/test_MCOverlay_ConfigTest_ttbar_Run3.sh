#!/bin/bash

# art-description: MC+MC Overlay with MT support, config test
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 22.0/Athena
# art-include: master/Athena

# art-output: legacyMcOverlayRDO.pool.root
# art-output: mcOverlayRDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*
# art-output: *.pkl
# art-output: *Config.txt

set -o pipefail

events=2
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"


Overlay_tf.py \
--runNumber 601229 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile legacyMcOverlayRDO.pool.root \
--maxEvents $events \
--conditionsTag OFLCOND-MC21-SDR-RUN3-07  \
--geometryVersion ATLAS-R3S-2021-03-00-00 \
--preInclude 'all:Campaigns/MC21a.py' \
--imf False \
--athenaopts '"--config-only=ConfigLegacy.pkl"'

Overlay_tf.py \
--runNumber 601229 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile legacyMcOverlayRDO.pool.root \
--maxEvents $events \
--conditionsTag OFLCOND-MC21-SDR-RUN3-07  \
--geometryVersion ATLAS-R3S-2021-03-00-00 \
--preInclude 'all:Campaigns/MC21a.py' \
--postExec 'job+=CfgMgr.JobOptsDumperAlg(FileName="OverlayLegacyConfig.txt");' \
--imf False

rc=$?
status=$rc
mv log.Overlay log.OverlayLegacy
echo "art-result: $rc configLegacy"

rc2=-9999
if [ $rc -eq 0 ]
then
    Overlay_tf.py \
    --CA \
    --runNumber 601229 \
    --inputHITSFile ${HITS_File} \
    --inputRDO_BKGFile ${RDO_BKG_File} \
    --outputRDOFile mcOverlayRDO.pool.root \
    --maxEvents $events \
    --conditionsTag OFLCOND-MC21-SDR-RUN3-07  \
    --geometryVersion ATLAS-R3S-2021-03-00-00 \
    --preInclude 'all:Campaigns.MC21a' \
    --postInclude 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
    --postExec 'with open("ConfigOverlay.pkl", "wb") as f: cfg.store(f)' \
    --imf False \
    --athenaopts="--threads=1"
    rc2=$?
    status=$rc2
    mv log.Overlay log.OverlayTest
fi
echo  "art-result: $rc2 configNew"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root legacyMcOverlayRDO.pool.root mcOverlayRDO.pool.root \
        --error-mode resilient --mode=semi-detailed \
        --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings RecoTimingObj_p1_HITStoRDO_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparison"

exit $status
