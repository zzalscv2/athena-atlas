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
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_cos.00421739.express_express.merge.RAW._lb0534._SFO-12._0001.1" \
--preExec='all:from RecExConfig.RecFlags import rec; rec.doExpressProcessing.set_Value_and_Lock(True); rec.doZdc.set_Value_and_Lock(False); from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Common.doExpressProcessing=True; ConfigFlags.Trigger.triggerConfig="DB"; ConfigFlags.DQ.Steering.HLT.doBjet=True; ConfigFlags.DQ.Steering.HLT.doInDet=True; ConfigFlags.DQ.Steering.HLT.doBphys=True; ConfigFlags.DQ.Steering.HLT.doCalo=True; ConfigFlags.DQ.Steering.HLT.doEgamma=False; ConfigFlags.DQ.Steering.HLT.doMET=True; ConfigFlags.DQ.Steering.HLT.doJet=True; ConfigFlags.DQ.Steering.HLT.doMinBias=True; ConfigFlags.DQ.Steering.HLT.doMuon=True; ConfigFlags.DQ.Steering.HLT.doTau=True; from AthenaCommon.JobProperties import jobproperties; jobproperties.InDetJobProperties.useBeamConstraint=True; ConfigFlags.Tile.doTimingHistogramsForGain=0;' \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
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
