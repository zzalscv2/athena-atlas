#!/bin/sh
#
# art-description: RecoTrf
# art-type: local
# art-include: master/Athena
# art-include: 22.0/Athena
# art-output: log.*

# There was a database connection problem reported in ATR-24782. Rodney Walker's solution is to use the following export to fix the problem:
export TNS_ADMIN=/cvmfs/atlas.cern.ch/repo/sw/database/DBRelease/current/oracle-admin


Reco_tf.py  \
--conditionsTag=CONDBR2-ES1PA-2022-06 \
--geometryVersion=ATLAS-R3S-2021-02-00-00 \
--autoConfiguration=everything \
--maxEvents=200 \
--preExec="all:from RecExConfig.RecFlags import rec; rec.doExpressProcessing.set_Value_and_Lock(True); rec.doZdc.set_Value_and_Lock(False); from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Common.doExpressProcessing=True; ConfigFlags.Trigger.triggerConfig= 'DB'; ConfigFlags.Trigger.enableL1MuonPhase1=True; ConfigFlags.Trigger.enableL1CaloPhase1=False; DQMonFlags.useTrigger=False; DQMonFlags.doHLTMon=False;" \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data21_comm.00404400.express_express.merge.RAW._lb2497._SFO-ALL._0001.1 \
--steering=doRAWtoALL \
--athenaopts="--threads=8" \
--outputAODFile=AOD.pool.root \
--outputESDFile=ESD.pool.root \
--outputHISTFile=HIST.root \
--imf=False

rc1=$?
echo "art-result: $rc1 Reco"
