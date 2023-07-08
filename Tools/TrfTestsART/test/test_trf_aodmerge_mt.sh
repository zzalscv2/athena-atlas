#!/bin/bash
#
# art-description: AODMerge_tf.py mt
# art-type: grid
# art-include: main/Athena

# art-include: 23.0/Athena
# art-athena-mt: 8

AODMerge_tf.py \
  --multithreaded="True" \
  --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3126_r12305/AOD.23662571._000001.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3126_r12305/AOD.23662571._000001.pool.root.1 \
  --maxEvents 1000 \
  --postExec 'FPEAuditor.NStacktracesOnFPE=10' \
  --outputAOD_MRGFile aod.pool.root

rc1=$?
echo "art-result: ${rc1} AODMerge_tf_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
