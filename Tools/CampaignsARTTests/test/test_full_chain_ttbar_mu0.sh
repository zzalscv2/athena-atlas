#!/bin/bash
# art-description: Run-4 Sim to DAOD_PHYSVAL and output plots via dcube, on non-all-had ttbar with no pile-up
# art-input: mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e8185
# art-input-nfiles: 1
# art-type: grid
# art-include: master/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last
# art-athena-mt: 4

#Option for sim/digi/reco
default_geometry="ATLAS-P2-RUN4-01-00-00"
default_condition="OFLCOND-MC15c-SDR-14-05"
number_of_events=4000

#Post-processing for ID/ITk and FTag
ftag_merge_DQA="${Athena_DIR}/src/PhysicsAnalysis/JetTagging/JetTagValidation/JetTagDQA/scripts/"
ftag_merge_script="mergePhysValFiles.py"
ftag_roc_script="Draw_PhysVal_btagROC.c"
idpvm_merge_script="postProcessIDPVMHistos.py"

#References for producing the output dcube pages
lastref_dir=last_results
# TODO: This one needs to be moved to CVMFS once it's cleaned up by the different domains (~5000 histograms right now)
dcube_xml="${Athena_DIR}/src/Tools/CampaignsARTTest/config/dcube_config_all_domains_reduced.xml"

run () {
  name="${1}"
  cmd="${@:2}"
  echo "Running transform for ${name}\n"
  time ${cmd}
  rc=$?
  echo "art-result: $rc ${name}"
  return $rc
}

checkstep () {
  if [ $? != 0 ]
  then
    exit $?
  else
    echo "${1} Succeeded"
  fi
}

run "Simulation" Sim_tf.py \
  --CA "all:True" \
  --conditionsTag "default:${default_condition}" \
  --geometryVersion "default:${default_geometry}" \
  --multithreaded "True" \
  --postInclude "default:PyJobTransforms.UseFrontier" \
  --preInclude "EVNTtoHITS:Campaigns.PhaseIISimulation" \
  --simulator "FullG4MT" \
  --inputEVNTFile ${ArtInFile} \
  --outputHITSFile "HITS.pool.root" \
  --imf False \
  --maxEvents ${number_of_events}

checkstep "Simulation"

export ATHENA_CORE_NUMBER=1
#export ATHENA_PROC_NUMBER=1

run "RAWtoALL" Reco_tf.py \
  --CA "all:True" \
  --athenaMPEventsBeforeFork "1" \
  --autoConfiguration "everything" \
  --conditionsTag "all:${default_condition}" \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --geometryVersion "all:${default_geometry}" \
  --multithreaded "True" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --preInclude "all:Campaigns.PhaseIINoPileUp" \
  --steering "doRAWtoALL" \
  --inputHitsFile "HITS.pool.root" \
  --outputAODFile "AOD.pool.root" \
  --maxEvents ${number_of_events}

checkstep "RAWtoALL"

run "AODtoDAOD_PHYSVAL" Derivation_tf.py \
  --CA "all:True" \
  --athenaMPMergeTargetSize "DAOD_*:0" \
  --formats "PHYSVAL" \
  --multiprocess "True" \
  --sharedWriter "True" \
  --inputAODFile "AOD.pool.root" \
  --outputDAODFile "OUT.root" \
  --maxEvents ${number_of_events}

checkstep "AODtoDAOD_PHYSVAL"

run "NTUP_PHYSVAL" Derivation_tf.py \
  --CA \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_PHYSVAL.root" \
  --validationFlags doInDet, doMET, doEgamma, doTau, doJet, doTopoCluster, doPFlow, doMuon \
  --format NTUP_PHYSVAL \
  --maxEvents ${number_of_events}

checkstep "NTUP_PHYSVAL"

#Run btag separately because they are ..doing things differently >:V
run "NTUP_BTAG_PHYSVAL" Derivation_tf.py \
  --CA \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_BTAG_PHYSVAL.root" \
  --validationFlags doBtag \
  --format NTUP_PHYSVAL \
  --maxEvents ${number_of_events}

checkstep "NTUP_BTAG_PHYSVAL"
 
if [ -d art_core_* ]
then
  echo "Merging histograms"
  hadd NTUP_PHYS.root art_core_*/NTUP_PHYSVAL.root
  $idpvm_merge_script NTUP_PHYSVAL.root
  python $ftag_merge_script --input art_core_*/* --pattern "*BTAG_PHYSVAL*" --output NTUP_BTAG_MERGE_PHYSVAL.root -d BTag
  root -l -b -q $ftag_merge_DQA/$ftag_roc_script\(\"ttbar\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"ROC\",\{\"IP2D\",\"IP3D\"\}\)
  hadd NTUP_MERGE_PHYSVAL.root NTUP_PHYSVAL.root NTUP_BTAG_MERGE_PHYSVAL.root ROC_NTUP_BTAG_MERGE_PHYSVAL.root
else
  python $ftag_merge_script --pattern "*BTAG_PHYSVAL*"  --output NTUP_BTAG_MERGE_PHYSVAL.root -d BTag
  root -l -b -q $ftag_merge_DQA/$ftag_roc_script\(\"ttbar\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"ROC\",\{\"IP2D\",\"IP3D\"\}\)
  hadd NTUP_MERGE_PHYSVAL.root NTUP_PHYSVAL.root NTUP_BTAG_MERGE_PHYSVAL.root ROC_NTUP_BTAG_MERGE_PHYSVAL.root
fi

checkstep "Merging and post processing"

echo "Download results from previous nightly"
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "DCUBE_LAST" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
  -p -x dcube_wrt_last_nightly \
  -c ${dcube_xml} \
  -r ${lastref_dir}/NTUP_MERGE_PHYSVAL.root \
  NTUP_MERGE_PHYSVAL.root

checkstep "DCUBECREATION"
exit "Everything done $?"
