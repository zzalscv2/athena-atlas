#!/bin/sh
#
# art-description: Express processing at Tier0
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8
# art-output: log.*

Reco_tf.py  \
--AMI x701  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_calib.00421734.express_express.merge.RAW._lb0021._SFO-ALL._0001.1" \
--preExec="all:from RecExConfig.RecFlags import rec; rec.doExpressProcessing.set_Value_and_Lock(True); rec.doZdc.set_Value_and_Lock(False); from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Common.doExpressProcessing=True; ConfigFlags.Trigger.triggerConfig='DB'; DQMonFlags.useTrigger=False; DQMonFlags.doHLTMon=False;" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputDAOD_L1CALO2File="L1CALO2.pool.root" \
--outputHISTFile="HIST.root" \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_22.0_references/$2
  art.py compare ref . $ArtRef --entries 30 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Comparison with the latest result"
