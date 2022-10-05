#!/bin/sh
#
# art-description: Reco_tf runs on 13TeV collision data 2018 in an isOnline format
# art-athena-mt: 8
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena

#The input file has to be specified twice because Reco_tf requires such an argument, whilst The RecExOnline job options ignore that and instead uses its own fileName to configure
#the input file - without that specified the code crashes out complaining that fileName was not specified.
inputFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data"

preIncludeString="RecExOnline/RecExOnline_globalconfig.py,RecExOnline/RecExOnline_recoflags.py,RecExOnline/RecExOnline_monitoring.py"
preExecStringOne="isOnline=True;isOnlineStateless=True;isGlobalMonitoring=False;useEmon=False;useAtlantisEmon=False;evtMax=300;from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.Trigger.triggerConfig=\"DB\";fileName=\"${inputFile}\""

Reco_tf.py --inputBSFile="${inputFile}" --preInclude="${preIncludeString}" --preExec="${preExecStringOne}" --postInclude="RecExOnline/RecExOnline_postconfig.py" \
        --AMI=f1263 --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root --conditionsTag="CONDBR2-HLTP-2022-02"

#Remember retval of transform as art result
RES=$?
echo "art-result: $RES Reco"

