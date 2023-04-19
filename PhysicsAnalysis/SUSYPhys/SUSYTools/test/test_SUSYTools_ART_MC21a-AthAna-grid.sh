#!/bin/sh

# art-description: SUSYTools ART test - share/minimalExampleJobOptions.py
# art-type: grid
# art-include: master/AthAnalysis
# art-output: hist-Ath_mc20e_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: 'share/minimalExampleJobOptions.py -c \'MCCampaign=\"mc20e\"\' '"
athena SUSYTools/minimalExampleJobOptions.py --evtMax=2000 -c 'MCCampaign="mc20e"'
echo  "art-result: $? TEST"

echo "Running DCube post-processing"

tName="mc20e"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc21a_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName}  hist-Ath_${tName}_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
