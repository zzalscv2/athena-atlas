#!/bin/bash
#
# art-description: Reco_tf.py data18 RAWtoALL in MP mode, use 4 cores due to large memory usage of MP
# art-type: grid
# art-include: master/Athena

# art-include: 23.0/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=4
timeout 64800 Reco_tf.py \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357750.physics_Main.daq.RAW/data18_13TeV.00357750.physics_Main.daq.RAW._lb0114._SFO-5._0003.data \
  --outputAODFile=myAOD.pool.root \
  --outputHISTFile=myHIST.root \
  --outputDESDM_MCPFile=myDESDM_MCP.pool.root \
  --outputDRAW_ZMUMUFile=myDRAW_ZMUMU.data \
  --outputDAOD_IDTIDEFile=myIDTIDE.pool.root \
  --multiprocess='True' \
  --CA='True' \
  --autoConfiguration='everything' \
  --conditionsTag 'all:CONDBR2-BLKPA-RUN2-11' --geometryVersion='default:ATLAS-R2-2016-01-00-01' \
  --runNumber='357750' --maxEvents='-1'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data18_mp"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
