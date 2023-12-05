#!/bin/bash
#
# art-description: Reco_tf.py data23 RAWtoALL w/ AOD in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="540"

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="True" \
  --maxEvents="${NEVENTS}" \
  --inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data" \
  --outputAODFile="myAOD.pool.root" \
  --multithreaded="True" \
  --autoConfiguration="everything" \
  --conditionsTag="all:CONDBR2-BLKPA-2023-01" \
  --geometryVersion="all:ATLAS-R3S-2021-03-02-00" \
  --steering="doRAWtoALL" \
  --outputFileValidation="False" \
  --checkEventCount="False" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? reconstruction";

files=( myAOD.pool.root )
for i in "${files[@]}"
do
    python -m "PyJobTransforms.trfValidateRootFile" "${i}" "event" "false" "on" > log."${i}".validation 2>&1;
    grep -zq "Checking ntuple of key RNT:CollectionTree.*Checking ${NEVENTS} entries.*NTuple of key RNT:CollectionTree looks ok" log."${i}".validation;
    echo "art-result: $? ${i} validation";
done
