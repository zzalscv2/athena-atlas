#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: main/AnalysisBase
# art-output: hist-data18_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYS -t data18 -p p5511 --inputFile DAOD_PHYS.data18_13TeV.00356250_p5511.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/'"
TestSUSYToolsAlg.py -f PHYS -t data18 -p p5511 --inputFile DAOD_PHYS.data18_13TeV.00356250_p5511.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-data18_DAOD_PHYS.root

echo "Running DCube post-processing"

tName="data18"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-data18_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-data18_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
