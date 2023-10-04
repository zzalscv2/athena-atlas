#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: main/AnalysisBase
# art-output: hist-data22_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYS -t data22 -m 20000'"
TestSUSYToolsAlg.py -f PHYS -t data22 -m 20000
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-data22_DAOD_PHYS.root

echo "Running DCube post-processing"

tName="data22"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-data22_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22_Run3.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-data22_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
