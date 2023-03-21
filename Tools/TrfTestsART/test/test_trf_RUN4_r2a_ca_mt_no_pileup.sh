#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=0, HITStoRDO,RAWtoALL in MT mode
# art-type: grid
# art-include: master/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-01-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4038/HITS.32253544._000100.pool.root.1"

Reco_tf.py \
  --AMIConfig r14362 \
  --inputHITSFile "${HSHitsFile}" \
  --outputAODFile RUN4.AOD.pool.root \
  --imf="False" \
  --maxEvents 100

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
