#!/bin/sh
#
# art-description: heavy ion reconstruction test from Andrzej Olszewski and Iwona Grabowska-Bold
# art-athena-mt: 4
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena

Reco_tf.py  --athenaopts="--threads=8"  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data15_hi.00286711.physics_MinBiasOverlay.daq.RAW._lb0217._SFO-2._0001.data --outputESDFile=ESD.root --outputAODFile=AOD.root --maxEvents=25 --conditionsTag 'default:CONDBR2-BLKPA-RUN2-11' --geometryVersion 'default:ATLAS-R2-2015-03-01-00' --autoConfiguration 'everything' --preExec 'all:rec.doHeavyIon.set_Value_and_Lock(True);rec.doZdc.set_Value_and_Lock(False);rec.doTrigger.set_Value_and_Lock(False);'

RES=$?
echo "art-result: $RES Reco"
