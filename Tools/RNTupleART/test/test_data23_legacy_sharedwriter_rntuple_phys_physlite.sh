#!/bin/bash
#
# art-description: Derivation_tf.py data23 w/ PHYS and PHYSLITE in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="2000"

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --CA="True" \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1" \
  --outputDAODFile="pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --outputFileValidation="False" \
  --checkEventCount="False" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

echo "art-result: $? derivation";

files=( DAOD_PHYS.pool.root DAOD_PHYSLITE.pool.root )
for i in "${files[@]}"
do
    python -m "PyJobTransforms.trfValidateRootFile" "${i}" "event" "false" "on" > log."${i}".validation 2>&1;
    grep -zq "Checking ntuple of key RNT:CollectionTree.*Checking ${NEVENTS} entries.*NTuple of key RNT:CollectionTree looks ok" log."${i}".validation;
    echo "art-result: $? ${i} validation";
done
