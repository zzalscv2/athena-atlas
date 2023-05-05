#!/bin/sh
#
# art-description: Reco_tf runs on 13TeV collision data 2017, early data, A3
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
#The setup for run2 data is described in test_data15_13TeV.sh     
Reco_tf.py  --CA --multithreaded  --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data17_13TeV/data17_13TeV.00336782.physics_Main.daq.RAW._lb0875._SFO-3._0001.data --maxEvents 300 --conditionsTag="CONDBR2-BLKPA-RUN2-11" --geometryVersion="ATLAS-R2-2016-01-00-01" --preExec="flags.DQ.Steering.doHLTMon=False" --ignoreErrors 'False' --ignorePatterns 'LArRawDataReadingAlg.+ERROR.+Found.+unsupported.+Rod.+block.+type.+0|LArRawDataReadingAlg.+\|.+ERROR.+\|.|ERROR.+message.+limit.+LArRawDataReadingAlg.' --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root
#Remember retval of transform as art result
RES=$?

xAODDigest.py myAOD.pool.root digest.txt

echo "art-result: $RES Reco"

