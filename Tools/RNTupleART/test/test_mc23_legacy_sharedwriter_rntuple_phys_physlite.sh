#!/bin/bash
#
# art-description: Derivation_tf.py mc23 w/ PHYS and PHYSLITE in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="1000"

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --CA="True" \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1" \
  --outputDAODFile="pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --outputFileValidation="False" \
  --checkEventCount="False" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"PHYSLITE\":100};";

echo "art-result: $? derivation";

files=( DAOD_PHYS.pool.root DAOD_PHYSLITE.pool.root )
for i in "${files[@]}"
do
    python -m "PyJobTransforms.trfValidateRootFile" "${i}" "event" "false" "on" > log."${i}".validation 2>&1;
    grep -zq "Checking ntuple of key RNT:CollectionTree.*Checking ${NEVENTS} entries.*NTuple of key RNT:CollectionTree looks ok" log."${i}".validation;
    echo "art-result: $? ${i} validation";
done
