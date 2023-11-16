#!/bin/bash
#
# art-description: Reco_tf.py data16 RAWtoALL in MT mode
# art-type: grid
# art-include: master/Athena
# art-include: 22.0-mc20/Athena
# art-athena-mt: 8
# art-output: log.*

# Updated on 7.12.2021 based on the tag r13100 which is currently used for Run2 Reprocessing

timeout 64800 Reco_tf.py \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data16_13TeV.00304008.physics_Main.daq.RAW._lb0838._SFO-6._0001.data \
  --outputAODFile=myAOD.pool.root \
  --outputHISTFile=myHIST.root \
  --outputDESDM_MCPFile=myDESDM_MCP.pool.root \
  --outputDRAW_ZMUMUFile=myDRAW_ZMUMU.data \
  --outputDAOD_IDTIDEFile=myIDTIDE.pool.root \
  --multithreaded='True' \
  --preExec 'all:from AthenaCommon.AthenaCommonFlags import athenaCommonFlags; from InDetRecExample.InDetJobProperties import InDetFlags;  from RecExConfig.RecFlags import rec; InDetFlags.useMuForTRTErrorScaling.set_Value_and_Lock(True if not rec.doRDOTrigger else False); from AthenaMonitoring.DQMonFlags import DQMonFlags; DQMonFlags.doHLTMon=False;' \
  --autoConfiguration='everything' \
  --conditionsTag 'all:CONDBR2-BLKPA-RUN2-09' --geometryVersion='default:ATLAS-R2-2016-01-00-01' \
  --runNumber='357750' --steering='doRAWtoALL' --maxEvents=200 --imf False

rc1=$?
echo "art-result: $rc1 Reco_tf_data16_mt"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
    ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_22.0-mc20_references/$2
    art.py compare ref . $ArtRef --entries 30 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool --ignore-leave '(.*)HLTResult_p1_HLTResult_HLT(.*)' --ignore-leave 'TrigNavigationAux(.*)'
    rc2=$?
fi
echo  "art-result: ${rc2} Diff"
