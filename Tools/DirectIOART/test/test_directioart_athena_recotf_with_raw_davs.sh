#!/bin/bash

# art-description: DirectIOART Athena Reco_tf.py inputFile:RAW protocol=DAVS
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --AMI q442 --inputBSFile davs://lcg-lrz-http.grid.lrz.de:443/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/data15_13TeV/71/42/data15_13TeV.00284285.physics_Main.daq.RAW._lb0856._SFO-1._0003.data --outputESDFile art.pool.root --ignoreErrors True

echo "art-result: $? DirectIOART_Athena_RecoTF_inputBS_protocol_DAVS"
