#!/bin/bash
#
# art-description: Run 4 digitization of a ttbar sample without pile-up
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: RUN4_ttbar.RDO.pool.root
# art-output: RDOAnalysis.root

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=25
HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-01-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4038/HITS.32253544._000100.pool.root.1"
DigiOutFileName="RUN4_ttbar.RDO.pool.root"

Digi_tf.py \
--CA \
--conditionsTag default:OFLCOND-MC15c-SDR-14-05 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--geometryVersion default:ATLAS-P2-RUN4-01-01-00 \
--inputHITSFile ${HSHitsFile} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileName} \
--preInclude 'HITtoRDO:Campaigns.PhaseIINoPileUp' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

rc2=-9999
if [ $rc -eq 0 ]; then
  RunRDOAnalysis.py -i "$DigiOutFileName"
  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 analysis"

if command -v art.py >/dev/null 2>&1; then
  rc3=-9999
  if [ $rc -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --file="$DigiOutFileName"
    rc3=$?
    status=$rc3
  fi
  echo "art-result: $rc3 regression"
fi

exit $status
