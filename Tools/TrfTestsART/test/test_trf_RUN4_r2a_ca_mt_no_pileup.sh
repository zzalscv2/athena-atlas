#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=0, HITStoRDO,RAWtoALL in MT mode
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"

Reco_tf.py \
  --CA \
  --conditionsTag OFLCOND-MC15c-SDR-14-05 \
  --geometryVersion ATLAS-P2-RUN4-03-00-00 \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --preInclude "all:Campaigns.PhaseIINoPileUp" \
  --postInclude "all:PyJobTransforms.UseFrontier.py" \
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
