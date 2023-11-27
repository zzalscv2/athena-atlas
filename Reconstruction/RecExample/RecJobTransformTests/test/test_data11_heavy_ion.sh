#!/bin/sh
#
# art-description: heavy ion reconstruction test from Andrzej Olszewski and Iwona Grabowska-Bold
# art-athena-mt: 4
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena

Reco_tf.py --CA \
--inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data11_hi.00193321.physics_HardProbes.merge.RAW._lb0050._SFO-9._0002.1 \
--athenaopts="--threads=1" \
--maxEvents 10 \
--conditionsTag "COMCOND-BLKPA-RUN1-07" \
--geometryVersion "ATLAS-R1-2011-02-00-00" \
--outputESDFile myESD.pool.root \
--outputAODFile myAOD.pool.root \
--preInclude "all:HIRecConfig.HIModeFlags.HImode" \
--preExec "all:flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False" \

RES=$?
echo "art-result: $RES Reco"
