#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena

Reco_tf.py \
--CA 'all:True' 'RDOtoRDOTrigger:False' \
--AMI=q445 \
--preExec "r2a:flags.DQ.Steering.HLT.doInDet=False; flags.Exec.FPE=500;" \
--athenaopts "RDOtoRDOTrigger:--threads=1" \
--maxEvents=100 \
--outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ $rc1 -eq 0 ]
then
  art.py compare grid --entries 50 "$1" "$2" --mode=semi-detailed --order-trees --ignore-exit-code diff-pool \
  --ignore-leave 'Token' --ignore-leave 'index_ref' --ignore-leave '(.*)_timings\.(.*)' --ignore-leave '(.*)_mems\.(.*)' --ignore-leave '(.*)TrigCostContainer(.*)' --ignore-leave '(.*)HLTNav_Summary_OnlineSlimmed(.*)' --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTF(.*)' --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_FTF(.*)' --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTF(.*)' --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_FTF(.*)' --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTF(.*)'  # Ignoring based on ATR-24119 and ATR-24888
  rc2=$?
fi
echo "art-result: $rc2 Diff"
