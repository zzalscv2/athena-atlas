#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: master/AnalysisBase
# art-output: hist-data22_DAOD_PHYSLITE.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYSLITE -t data22 -p p5514 --inputFile DAOD_PHYSLITE.data22_13p6TeV.00440543_p5514.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/'"
TestSUSYToolsAlg.py -f PHYSLITE -t data22 -p p5514 --inputFile DAOD_PHYSLITE.data22_13p6TeV.00440543_p5514.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-data22_DAOD_PHYSLITE.root

echo "Running DCube post-processing"

tName="data22"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-data22_DAOD_PHYSLITE-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22_Run3.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-data22_DAOD_PHYSLITE.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
