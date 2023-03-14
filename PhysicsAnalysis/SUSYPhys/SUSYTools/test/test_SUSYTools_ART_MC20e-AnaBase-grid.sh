#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: master/AnalysisBase
# art-output: hist-MC20e_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYS -t mc20e -m 2000 -p p5511 --inputFile DAOD_PHYS.mc20_13TeV.410470.FS_mc20e_p5511.PHYS.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/ --dosyst'"
TestSUSYToolsAlg.py -f PHYS -t mc20e -m 2000 -p p5511 --inputFile DAOD_PHYS.mc20_13TeV.410470.FS_mc20e_p5511.PHYS.pool.root --inputDir /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/ --dosyst
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-mc20e_DAOD_PHYS.root

echo "Running DCube post-processing"

tName="MC20e"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc20e_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-mc20e_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
