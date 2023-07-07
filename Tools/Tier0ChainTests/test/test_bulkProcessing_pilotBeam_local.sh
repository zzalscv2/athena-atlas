#!/bin/sh
#
# art-description: Test for bulk processing at Tier0
# art-type: local
# art-include: main/Athena
# art-include: 23.0/Athena

# There was a database connection problem reported in ATR-24782. Rodney Walker's solution is to use the following export to fix the problem:
export TNS_ADMIN=/cvmfs/atlas.cern.ch/repo/sw/database/DBRelease/current/oracle-admin

Reco_tf.py  \
--conditionsTag=CONDBR2-BLKPA-2022-08 \
--geometryVersion=ATLAS-R3S-2021-03-01-00 \
--autoConfiguration=everything \
--maxEvents=200 \
--preExec="all:from RecExConfig.RecFlags import rec; from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Trigger.triggerConfig= 'DB'; ConfigFlags.Trigger.enableL1MuonPhase1=True; ConfigFlags.Trigger.enableL1CaloPhase1=False; DQMonFlags.useTrigger=False; DQMonFlags.doHLTMon=False;" \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data21_comm.00404400.express_express.merge.RAW._lb2497._SFO-ALL._0001.1 \
--athenaopts="--threads=8" \
--outputAODFile=AOD.pool.root \
--outputESDFile=ESD.pool.root \
--outputHISTFile=HIST.root \
--imf=False

rc1=$?
echo "art-result: $rc1 Reco"
