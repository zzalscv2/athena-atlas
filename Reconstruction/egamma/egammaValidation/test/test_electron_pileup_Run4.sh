#!/bin/sh
#
# art-description: ART Monitoring Tool for electron Validation
#
# art-type: grid
# art-input: mc21_14TeV.900494.PG_single_epm_Pt10_etaFlatnp0_43.recon.RDO.e8481_s4038_r14365
# art-input-nfiles: 45
# art-cores: 4
# art-include: master/Athena
# art-output: *.hist.root
# art-output: *.txt
# art-output: *.png
# art-output: log.*
# art-output: dcube

echo "ArtProcess: $ArtProcess"

case $ArtProcess in
    
    "start")
	echo "Starting"
	echo "List of files = " ${ArtInFile}
	;;

    "end")
	echo "Ending"
	
	echo "Merging AODs"
        echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC}"
        unset  ATHENA_NUM_PROC

	AODMerge_tf.py --CA --inputAODFile=art_core_*/Nightly_AOD.pool.root --outputAOD_MRGFile=Nightly_AOD.pool.root

	echo  "art-result: $? AODMerge"

	set +e

	checkFile.py Nightly_AOD.pool.root > checkFile_Nightly_electron.txt

	echo  "art-result: $? checks_files"

	runegammaMonitoring.py -p 'electron'

	echo  "art-result: $? athena_job"

	EgammaARTmonitoring_plotsMaker.py /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Baseline_Files/rootFiles/Baseline-monitoring_electron.hist.root Nightly-monitoring.hist.root electron

	echo  "art-result: $? final_comparison"

	## dcube not relevant for the time being, need proper references
	#$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p -x dcube -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/DCube_Config/electron.xml -r /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/Nightly-monitoring_electron.hist.root  Nightly-monitoring.hist.root
	#echo  "art-result: $? plot"

	;;

    *)
	echo "Test $ArtProcess"

	mkdir "art_core_${ArtProcess}"
	cd "art_core_${ArtProcess}"

	IFS=',' read -r -a file <<< "${ArtInFile}"
	file=${file[${ArtProcess}]}
	x="../$file"

	echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC}"
	unset  ATHENA_NUM_PROC

	Reco_tf.py --CA --inputRDOFile=$x --outputAODFile=Nightly_AOD.pool.root --maxEvents=200 --autoConfiguration="everything" --conditionsTag="OFLCOND-MC15c-SDR-14-05" --preInclude egammaConfig.egammaOnlyFromRawFlags.egammaOnlyFromRaw --postInclude egammaValidation.egammaArtSpecialContent.egammaArtSpecialContent

	echo  "art-result: $? reconstruction"

	;;
esac
