#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: master/AnalysisBase
# art-output: hist-data18_DAOD_PHYSLITE.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYSLITE -t data18 -p p5631 -m 20000 --inputFile data18_13TeV.00356250_p5631.PHYSLITE.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/'"
TestSUSYToolsAlg.py -f PHYSLITE -t data18 -p p5631 -m 20000 --inputFile data18_13TeV.00356250_p5631.PHYSLITE.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-data18_DAOD_PHYSLITE.root

echo "Running DCube post-processing"

tName="data18"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-data18_DAOD_PHYSLITE-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-data18_DAOD_PHYSLITE.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
