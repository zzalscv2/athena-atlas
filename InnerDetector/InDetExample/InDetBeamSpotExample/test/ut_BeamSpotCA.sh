#!/bin/sh

export ATHENA_CORE_NUMBER=4
Reco_tf.py \
--CA \
--multithreaded \
--inputBSFile "/eos/atlas/atlastier0/rucio/data23_13p6TeV/calibration_BeamSpot/00460348/data23_13p6TeV.00460348.calibration_BeamSpot.merge.RAW/data23_13p6TeV.00460348.calibration_BeamSpot.merge.RAW._lb0500._SFO-ALL._0001.1" \
--maxEvents 50 \
--conditionsTag "CONDBR2-ES1PA-2023-02" \
--geometryVersion "ATLAS-R3S-2021-03-02-00" \
--autoConfiguration 'everything' \
--outputAODFile myAOD.pool.root \
--preInclude "all:InDetBeamSpotExample.BeamSpotRecoConfig.beamSpotRecoPre" \
--postInclude "all:InDetBeamSpotExample.BeamSpotRecoConfig.beamSpotRecoPost" \

