#!/usr/bin/env bash
# art-description: test ttFC_fastSim_fastDigi + ttFC_reco_noSplit_noPseudoT_fastSim_fastDigi
# art-type: grid

FastChain_tf.py --simulator ATLFASTIIF_PileUp --digiSteeringConf "SplitNoMergeFF" --useISF True --randomSeed 123 --enableLooperKiller True --inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/ttbar_muplusjets-pythia6-7000.evgen.pool.root --outputRDOFile RDO_pileup_fastsim_fastdigi.pool.root --maxEvents 100 --skipEvents 0 --geometryVersion ATLAS-R2-2015-03-01-00 --conditionsTag OFLCOND-RUN12-SDR-31 --preSimExec 'from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags;TrkDetFlags.TRT_BuildStrawLayers=True' --preSimInclude FastChainPileup/FastPileup.py --postInclude='PyJobTransforms/UseFrontier.py,G4AtlasTests/postInclude.DCubeTest_FCpileup.py,DigitizationTests/postInclude.RDO_Plots.py' --postExec 'RAWtoESD:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' --DataRunNumber '284500' --postSimExec='genSeq.Pythia8.NCollPerEvent=10;' --preDigiInclude="FastTRT_Digitization/preInclude.FastTRT_Digi.Validation.py"

echo "art-result: $? RDO step"


FastChain_tf.py --maxEvents 500 --skipEvents 0 --geometryVersion ATLAS-R2-2015-03-01-00 --conditionsTag OFLCOND-RUN12-SDR-31  --inputRDOFile 'RDO_pileup_fastsim_fastdigi.pool.root' --outputAODFile AOD_noSplit_noPseudoT_fastSim_fastDigi.pool.root --preExec "RAWtoESD:rec.doTrigger.set_Value_and_Lock(False);recAlgs.doTrigger.set_Value_and_Lock(False);InDetFlags.pixelClusterSplittingType.set_Value_and_Lock(\"AnalogClus\");InDetFlags.doTIDE_Ambi.set_Value_and_Lock(False);InDetFlags.doStandardPlots.set_Value_and_Lock(True)"

echo "art-result: $? AOD step"

ArtPackage=$1
ArtJobName=$2
art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} 
echo  "art-result: $? regression"
#add an additional payload from the job (corollary file).                                                           
# art-output: InDetStandardPlots.root  
/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube TEST_ttFC_reco_noSplit_noPseudoT_fastSim_fastDigi InDetStandardPlots.root /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/dcube_configs/config/dcube_indetplots_no_pseudotracks.xml /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/InDetStandardPlots_TEST.root




# InDetStandardPlots.root -l dcube.log -p -r   -x dcube.xml -s /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/

# art-output: dcube.xml
# art-output: dcube.log
# art-output: dcubelog.xml
# art-output: dcube.xml.php
echo  "art-result: $? histcomp"
