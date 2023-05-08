#!/bin/sh
#
# art-description: Reco_tf runs on 13TeV collision data with all streams 2015
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena

export ATHENA_CORE_NUMBER=8
#This configuration is documented here: https://twiki.cern.ch/twiki/bin/view/Atlas/2021FullRun2Reprocessing#Configuration                                                                                                                                                    
#The below is the CA equivalent
#Note that in the CA there is no useMuForTRTErrorScaling flag - this correctly auto configures
#Also note that it is believed the bunch structure value no longer needs to be changed to avoid crashes and so is not included.
Reco_tf.py  --CA --multithreaded --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data15_13TeV.00283429.physics_Main.daq.RAW._lb0154._SFO-1._0001.data --maxEvents 300 --autoConfiguration everything --conditionsTag="CONDBR2-BLKPA-RUN2-11" --geometryVersion="ATLAS-R2-2016-01-00-01" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"
