#!/bin/bash
#
# art-description: Reco_tf.py q442 RAWtoALL in MT and ComponentAccumulator mode
# art-type: grid
# art-include: 23.0/Athena
# art-athena-mt: 8

mkdir ca
cd ca
Reco_tf.py --CA \
  --AMI q442  \
  --multithreaded="True" \
  --outputAODFile myAOD_ca.pool.root \
  --outputESDFile myESD_ca.pool.root \
  --outputHISTFile myHIST_ca.root \
  --preExec "all:ConfigFlags.Jet.WriteToAOD=True; ConfigFlags.MET.WritetoAOD=True" \
  --imf="False" \
  --maxEvents 100

rc1=$?
echo "art-result: ${rc1} Reco_tf_q442_r2a_ca_mt" 

cd ..
mkdir def
cd def
Reco_tf.py \
  --AMI q442  \
  --multithreaded="True" \
  --outputAODFile myAOD_def.pool.root \
  --outputESDFile myESD_def.pool.root \
  --outputHISTFile myHIST_def.root \
  --preExec "all:from JetRec.JetRecFlags import jetFlags; jetFlags.writeJetsToAOD.set_Value_and_Lock(True); from METReconstruction.METRecoFlags import metFlags; metFlags.WriteMETAssocToOutput.set_Value_and_Lock(True); metFlags.WriteMETToOutput.set_Value_and_Lock(True);" \
  --imf="False" \
  --maxEvents 100

rc2=$?
echo "art-result: ${rc2} Reco_tf_q442_r2a_mt" 

cd ..

echo "============ checkxAOD myAOD_ca.pool.root"
checkxAOD ca/myAOD_ca.pool.root
rc3=$?
echo "art-result: ${rc3} checkxAOD myAOD_ca.pool.root" 

echo "============ checkxAOD myAOD_def.pool.root"
checkxAOD def/myAOD_def.pool.root
rc4=$?
echo "art-result: ${rc4} checkxAOD myAOD_def.pool.root" 

echo "============ xAODDigest.py --extravars myAOD_ca.pool.root"
xAODDigest.py --extravars ca/myAOD_ca.pool.root myAOD_ca.txt
rc5=$?
echo "art-result: ${rc5} xAODDigest.py --extravars myAOD_ca.pool.root"
echo "============ myAOD_ca.txt"
cat myAOD_ca.txt
echo "============ myAOD_ca.txt"

echo "============ xAODDigest.py --extravars myAOD_def.pool.root"
xAODDigest.py --extravars def/myAOD_def.pool.root myAOD_def.txt
rc6=$?
echo "art-result: ${rc6} xAODDigest.py --extravars myAOD_def.pool.root"
echo "============ myAOD_def.txt"
cat myAOD_def.txt
echo "============ myAOD_def.txt"

echo "============ comparexAODDigest.py myAOD_def.txt myAOD_ca.txt"
comparexAODDigest.py myAOD_def.txt myAOD_ca.txt
rc7=$?
echo "art-result: ${rc7} comparexAODDigest.py myAOD_def.txt myAOD_ca.txt"

echo "============ hist_diff.sh ca/myHIST_ca.root def/myHIST.root -i -x (TIME_|LAr/Coverage|MismatchEventNumbers|L1Calo/Overview/Errors|noEth_rndm_CSCveto)"
hist_diff.sh ca/myHIST_ca.root def/myHIST_def.root -i -x "(TIME_|LAr/Coverage|MismatchEventNumbers|L1Calo/Overview/Errors|noEth_rndm_CSCveto)"
rc8=$?
echo "art-result: ${rc8} hist_diff.sh ca/myHIST_ca.root def/myHIST.root"

echo "============ diff-root def/myAOD_def.pool.root ca/myAOD_ca.pool.root"
acmd.py diff-root def/myAOD_def.pool.root ca/myAOD_ca.pool.root --nan-equal --error-mode resilient --order-trees --entries 50 --mode semi-detailed
rc9=$?
echo "art-result: ${rc9} diff-root def/myAOD_def.pool.root ca/myAOD_ca.pool.root"

echo "============ meta-diff def/myAOD_def.pool.root ca/myAOD_ca.pool.root"
meta-diff --ordered --mode full --diff-format diff def/myAOD_def.pool.root ca/myAOD_ca.pool.root --drop file_guid file_size
rc10=$?
echo "art-result: ${rc10} meta-diff def/myAOD_def.pool.root ca/myAOD_ca.pool.root"

echo "============ diff-root def/myESD_def.pool.root ca/myESD_ca.pool.root"
acmd.py diff-root def/myESD_def.pool.root ca/myESD_ca.pool.root --nan-equal --error-mode resilient --order-trees --entries 10 --mode semi-detailed
rc11=$?
echo "art-result: ${rc11} diff-root def/myESD_def.pool.root ca/myESD_ca.pool.root"
