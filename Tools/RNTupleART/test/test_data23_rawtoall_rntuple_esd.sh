#!/bin/bash
#
# art-description: Reco_tf.py data23 RAWtoALL w/ ESD in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="True" \
  --inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data" \
  --outputESDFile="myESD.pool.root" \
  --multithreaded="True" \
  --autoConfiguration="everything" \
  --conditionsTag="all:CONDBR2-BLKPA-2023-01" \
  --geometryVersion="all:ATLAS-R3S-2021-03-02-00" \
  --steering="doRAWtoALL" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? reco";

files=( myESD.pool.root )
for i in "${files[@]}"
do
    if [ -f "$i" ]; then
        rcFile=0
    else
        rcFile=1
    fi
    echo "art-result: ${rcFile} $i exists";
done
