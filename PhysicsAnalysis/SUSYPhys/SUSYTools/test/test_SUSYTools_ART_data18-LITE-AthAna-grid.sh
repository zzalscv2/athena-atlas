#!/bin/sh

# art-description: SUSYTools ART test - share/minimalExampleJobOptions.py
# art-type: grid
# art-include: main/AthAnalysis
# art-output: hist-Ath_data18_DAOD_PHYSLITE.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: 'share/athena SUSYTools/jobOptions.py --evtMax 20000 - --testCampaign data18 --testFormat PHYSLITE'"
athena SUSYTools/jobOptions.py --evtMax 20000 - --testCampaign data18 --testFormat PHYSLITE
echo  "art-result: $? TEST"

echo "Running DCube post-processing"

tName="data18"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-data18_DAOD_PHYSLITE-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-Ath_${tName}_DAOD_PHYSLITE.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
