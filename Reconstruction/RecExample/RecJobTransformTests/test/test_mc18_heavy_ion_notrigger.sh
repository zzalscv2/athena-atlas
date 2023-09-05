#!/bin/sh
#
# art-description: heavy ion reconstruction test from Sebastian Tapia
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
--CA \
--multithreaded \
--inputHITSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/mc16_5TeV.420000.Hijing_PbPb_5p02TeV_MinBias_Flow_JJFV6.merge.HITS.e4962_a890_s3136/HITS.17784755._001903.pool.root.1 \
--outputAODFile=AOD.pool.root \
--maxEvents=20 \
--conditionsTag 'OFLCOND-MC16-SDR-RUN2-08' \
--postInclude 'all:PyJobTransforms.UseFrontier' \
--preInclude='RAWtoALL:HIRecConfig.HIModeFlags.HImode' \
--preExec='flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False;flags.Beam.BunchSpacing=100;' \
--postExec='HITtoRDO:cfg.getCondAlgo("TileSamplingFractionCondAlg").G4Version=-1' \
--autoConfiguration 'everything' \
--DataRunNumber '313000' 

RES=$?
echo "art-result: $RES Reco"

