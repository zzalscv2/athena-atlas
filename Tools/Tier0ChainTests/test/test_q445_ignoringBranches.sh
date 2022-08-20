#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-athena-mt: 8

Reco_tf.py \
--AMI=q445 \
--multithreaded \
--maxEvents=80 \
--outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --outputHISTFile=myHIST.root \
--imf False
rc1=$?
echo "art-result: $rc1 Reco"



rc2=-9999
if [ $rc1 -eq 0 ]
then
# ignoring as suggested by Tim Martin (the same as in CI)
  art.py compare grid --file=*AOD*.pool.root --diff-root --entries 80 "$1" "$2" --mode=semi-detailed --order-trees \
  --ignore-leave '^HLT_(.*)' \
  --ignore-leave '^HLTNav_(.*)' \
  --ignore-leave '^L1_(.*)' \
  --ignore-leave '^LVL1(.*)' \
  --ignore-leave 'BunchConfKey(.*)' \
  --ignore-leave 'TrigConfKeys(.*)' \
  --ignore-leave 'xTrigDecisionAux(.*)'
  rc2=$?
fi
echo "art-result: $rc2 AOD diff-root comparison"


rc3=-9999
if [ $rc1 -eq 0 ]
then
 # Ignoring based on ATLASRECTS-7101, ATR-24119 and ATR-24888, and on the suggestion from Tim Martin (as done in CI)
  art.py compare grid --file=*ESD*.pool.root --diff-root --entries 80 "$1" "$2" --mode=semi-detailed --order-trees \
  --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTF(.*)' \
  --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_FTF(.*)' \
  --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTF(.*)' \
  --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_FTF(.*)' \
  --ignore-leave '(.*)TrackParticleAuxContainer_v5_HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTF(.*)' \
  --ignore-leave '(.*)_HLT_(.*)' \
  --ignore-leave '(.*)_HLTNav_(.*)' \
  --ignore-leave '(.*)_L1_(.*)' \
  --ignore-leave '(.*)_LVL1(.*)' \
  --ignore-leave '(.*)_BunchConfKey(.*)' \
  --ignore-leave '(.*)_RoIBResult(.*)' \
  --ignore-leave '(.*)_TrigConfKeys(.*)' \
  --ignore-leave '(.*)_xTrigDecisionAux(.*)' \
  --ignore-leave 'Muon::MuonPRD_Container_p2<Muon::MMPrepData_p1>_MM_Measurements.m_prds.m_stripNumbers' \
  --ignore-leave 'Muon::MuonPRD_Container_p2<Muon::sTgcPrepData_p1>_STGC_Measurements.m_prds.m_stripNumbers' \
  --ignore-leave 'Muon::MuonPRD_Container_p2<Muon::sTgcPrepData_p2>_STGC_Measurements.m_prds.m_stripNumbers'
  rc3=$?
fi
echo "art-result: $rc3 ESD diff-root comparison"


rc4=-9999
if [ $rc1 -eq 0 ]
then
  art.py compare grid --file=*RDO*.pool.root --diff-root --entries 80 "$1" "$2" --mode=semi-detailed --order-trees
  rc4=$?
fi
echo "art-result: $rc4 RDO diff-root comparison"



rc5=-9999
if [ $rc1 -eq 0 ]
then
  # ignoring as suggested by Tim Martin (the same as in CI)
  ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3-22.0_references_for_comparison/test_q445_ignoringBranches
  cat $ArtRef/version.txt
  art.py compare ref --file=*AOD*.pool.root --diff-root --entries 80 . $ArtRef --mode=semi-detailed --order-trees \
  --ignore-leave '^HLT_(.*)' \
  --ignore-leave '^HLTNav_(.*)' \
  --ignore-leave '^L1_(.*)' \
  --ignore-leave '^LVL1(.*)' \
  --ignore-leave 'BunchConfKey(.*)' \
  --ignore-leave 'TrigConfKeys(.*)' \
  --ignore-leave 'xTrigDecisionAux(.*)'
  rc5=$?
fi
echo "art-result: $rc5 AOD diff-root (fixed reference)"
