#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-athena-mt: 8

# There was a database connection problem reported in ATR-24782. Rodney Walker's solution is to use the following export to fix the problem:
export TNS_ADMIN=/cvmfs/atlas.cern.ch/repo/sw/database/DBRelease/current/oracle-admin

Reco_tf.py  \
--conditionsTag=CONDBR2-BLKPA-2022-08 \
--geometryVersion=ATLAS-R3S-2021-03-01-00 \
--autoConfiguration=everything \
--maxEvents=-1 \
--preExec="all:from RecExConfig.RecFlags import rec; from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Trigger.triggerConfig= 'DB'; ConfigFlags.Trigger.enableL1MuonPhase1=True; ConfigFlags.Trigger.enableL1CaloPhase1=False; DQMonFlags.useTrigger=False; DQMonFlags.doHLTMon=False;" \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data21_900GeV.00405514.express_express.merge.RAW._lb0123._SFO-ALL._0001.1 \
--athenaopts="--threads=8" \
--outputAODFile=AOD.pool.root \
--outputESDFile=ESD.pool.root \
--outputHISTFile=HIST.root \
--imf=False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 30 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} (against previous nightly)"
