#!/bin/sh
#
# art-description: Generate SQL files for muon spectrometer description, just AMDB part from amdb_simrec
#
# art-type: grid
# art-include: main/Athena
#
# art-output: *.data

art.py createpoolfile

set -x

# download an amdb file to test
wget http://atlas.web.cern.ch/Atlas/GROUPS/MUON/AMDB/amdb_simrec.r.08.01
# create the AMDB tables from it
athena.py AmdcAth/AmdcAth_GeometryTasks.py -c "input_amdb_simrec='amdb_simrec.r.08.01';DoAMDBTables=True;"

echo "art-result: $?"
