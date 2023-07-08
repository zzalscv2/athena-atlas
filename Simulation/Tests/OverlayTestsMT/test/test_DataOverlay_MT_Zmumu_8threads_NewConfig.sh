#!/bin/sh

# art-description: MC+data Overlay with MT support, running with 8 threads, new config
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena

# art-output: dataOverlayRDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*
# art-output: *.pkl
# art-output: *Config.txt

export ATHENA_CORE_NUMBER=8

Overlay_tf.py \
--CA \
--multithreaded \
--inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/DataOverlaySimulation/22.0/v1/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.HITS.pool.root \
--inputBS_SKIMFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1 \
--outputRDOFile dataOverlayRDO.pool.root \
--maxEvents 100 \
--conditionsTag CONDBR2-BLKPA-RUN2-10 \
--preInclude 'Campaigns.DataOverlayPPTest' \
--postInclude 'OverlayConfiguration.DataOverlayConditions.PPTestCfg' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
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
