#!/bin/sh

# art-description: DAOD building EGAM1 data17_13TeV
# art-type: grid
# art-ci: 21.2

Reco_tf.py --inputAODFile /eos/atlas/atlascerngroupdisk/data-art/DerivationFrameworkART/data17_13TeV.00327342.physics_Main.merge.AOD.f838_m1824._lb0300._0001.1 --outputDAODFile art.pool.root --reductionConf EGAM1 --maxEvents 5000
