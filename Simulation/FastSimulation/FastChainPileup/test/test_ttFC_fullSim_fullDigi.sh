#!/usr/bin/env bash
# art-type: grid
# art-description: test job ttFC_fullSim_fullDigi

# specify branches of athena that are being targeted:                                                               
# art-include: 21.0/Athena                                                                                          
# art-include: 21.3/Athena                                                                                          
# Also include temporary branch 21.3-hmpl                                                                           
# art-include: 21.3-hmpl/Athena       

FastChain_tf.py --simulator ATLFASTII --digiSteeringConf "SplitNoMerge" --useISF True --randomSeed 123 --enableLooperKiller True --inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/ttbar_muplusjets-pythia6-7000.evgen.pool.root --outputRDOFile RDO_pileup_fullsim_fulldigi.pool.root --maxEvents 50 --skipEvents 0 --geometryVersion ATLAS-R2-2015-03-01-00 --conditionsTag OFLCOND-RUN12-SDR-31 --preSimExec 'from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags;TrkDetFlags.TRT_BuildStrawLayers=True' --postInclude='PyJobTransforms/UseFrontier.py,G4AtlasTests/postInclude.DCubeTest.py,DigitizationTests/postInclude.RDO_Plots.py' --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' --DataRunNumber '284500'

echo "art-result: $? evgen step"

ArtPackage=$1
ArtJobName=$2
art.py compare grid --entries 10 --imf=False ${ArtPackage} ${ArtJobName}
echo  "art-result: $? regression"
#add an additional payload from the job (corollary file).                                                           
# art-output: InDetStandardPlots.root  
/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube TEST_ttFC_fullSim_fullDigi InDetStandardPlots.root /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/dcube_configs/config/dcube_indetplots_no_pseudotracks.xml /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/InDetStandardPlots_TEST.root

# art-output: dcube/dcube.xml
# art-output: dcube/dcube.log
# art-output: dcube/dcubelog.xml
# art-output: dcube/dcube.xml.php
echo  "art-result: $? histcomp"
