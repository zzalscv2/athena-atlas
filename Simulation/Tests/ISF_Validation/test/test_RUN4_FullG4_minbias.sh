#!/bin/sh
#
# art-description: MC21-style simulation using FullG4 and RUN4 geometry, minimum bias
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test_minbias.HITS.pool.root
# art-output: test_minbias.HITS_FLT.pool.root

Input="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.evgen.EVNT.e8481/EVNT.30810275._000020.pool.root.1"
Output="test_minbias.HITS.pool.root"
OutputFilter="test_minbias.HITS_FLT.pool.root"

# RUN4 setup
# ATLAS-P2-RUN4-01-01-00 and OFLCOND-MC15c-SDR-14-05
Sim_tf.py \
--multithreaded \
--CA \
--conditionsTag 'default:OFLCOND-MC15c-SDR-14-05' \
--simulator 'FullG4MT' \
--postInclude 'default:PyJobTransforms.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
--geometryVersion 'default:ATLAS-P2-RUN4-01-01-00' \
--inputEVNTFile "$Input" \
--outputHITSFile "$Output" \
--maxEvents 10 \
--imf False

rc=$?
status=$rc
echo "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]; then
  FilterHit_tf.py \
  --CA \
  --TruthReductionScheme SingleGenParticle \
  --inputHITSFile "$Output" \
  --outputHITS_FILTFile "$OutputFilter"

  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 FilterHitCA"

rc3=-9999
if [ $rc -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --diff-root --mode=semi-detailed --file="$Output"
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 regression"

rc3=-9999
if [ $rc2 -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --diff-root --mode=semi-detailed --file="$OutputFilter"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 regressionFilter"

exit $status
