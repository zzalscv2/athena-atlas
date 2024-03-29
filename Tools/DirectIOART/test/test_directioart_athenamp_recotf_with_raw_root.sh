#!/bin/bash

# art-description: DirectIOART AthenaMP Reco_tf.py inputFile:RAW protocol=ROOT
# art-type: grid
# art-output: *.pool.root
# art-athena-mt: 2

set -e

export ATHENA_CORE_NUMBER=2
Reco_tf.py --AMI q442 --multiprocess --inputBSFile root://lcg-lrz-rootd.grid.lrz.de:1094/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/data15_13TeV/71/42/data15_13TeV.00284285.physics_Main.daq.RAW._lb0856._SFO-1._0003.data --outputESDFile art.pool.root --ignoreErrors True

echo "art-result: $? DirectIOART_AthenaMP_RecoTF_inputBS_protocol_ROOT"
