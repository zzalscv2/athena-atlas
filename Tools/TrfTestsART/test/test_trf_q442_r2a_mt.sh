#!/bin/bash
#
# art-description: Reco_tf.py q442 RAWtoALL in MT mode
# art-type: grid
# art-athena-mt: 8

Reco_tf.py \
  --AMI q442 \
  --multithreaded="True" \
  --postExec 'FPEAuditor.NStacktracesOnFPE=10' \
  --maxEvents -1

rc1=$?
echo "art-result: ${rc1} Reco_tf_q442_r2a_mt" 

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
