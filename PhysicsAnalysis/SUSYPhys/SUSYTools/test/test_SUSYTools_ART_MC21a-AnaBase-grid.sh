#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: master/AnalysisBase
# art-output: hist-MC21a_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYS -t mc21a -m 2000 -p p5631 --inputFile mc21_13p6TeV.601229.FS_mc21a_p5631.PHYS.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/ --dosyst'"
TestSUSYToolsAlg.py -f PHYS -t mc21a -m 2000 -p p5631 --inputFile mc21_13p6TeV.601229.FS_mc21a_p5631.PHYS.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/ --dosyst 
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-mc21a_DAOD_PHYS.root

echo "Running DCube post-processing"

tName="mc21a"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc21a_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22_Run3.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-mc21a_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
