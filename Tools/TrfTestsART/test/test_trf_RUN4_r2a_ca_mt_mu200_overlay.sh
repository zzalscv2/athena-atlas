#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=200, Overlay,RAWtoALL in MT mode
# art-type: grid
# art-include: master/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-00-00/mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e8185_s3856/HITS.29179779._000100.pool.root.1"
RDOFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/RDO_BKG/ATLAS-P2-RUN4-01-00-00/50events.RDO.pool.root"

Reco_tf.py \
  --AMIConfig r14001 \
  --inputHITSFile "${HSHitsFile}" \
  --inputRDO_BKGFile "$RDOFile" \
  --outputAODFile RUN4.AOD.pool.root \
  --imf="False" \
  --maxEvents 50

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
