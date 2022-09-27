#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=200, HITStoRDO in MP mode,RAWtoALL in MT mode
# art-type: grid
# art-include: master/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-00-00/mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e8185_s3856/HITS.29179779._000100.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-00-00/mc15_14TeV.800381.Py8EG_A3NNPDF23LO_minbias_inelastic_high_keepJets.merge.HITS.e8205_s3856_s3857/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-00-00/mc15_14TeV.800380.Py8EG_A3NNPDF23LO_minbias_inelastic_low_keepJets.merge.HITS.e8205_s3856_s3857/*"

Reco_tf.py \
  --AMIConfig r13998 \
  --inputHITSFile "${HSHitsFile}" \
  --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
  --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
  --outputAODFile RUN4.AOD.pool.root \
  --jobNumber 1 \
  --maxEvents 50

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
