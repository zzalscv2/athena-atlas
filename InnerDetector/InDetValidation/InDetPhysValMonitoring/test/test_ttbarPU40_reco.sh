#!/bin/bash
# art-description: Standard test for Run2 with ttbar input, PU=40
# art-input: user.keli:user.keli.mc16_13TeV.410471.PhPy8EG_A14_ttbar_hdamp258p75_allhad.e6337_e5984_Rel22073
# art-input-nfiles: 10
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_idtide_last

#RDO is made at rel 22.0.73
#reference plots are made at rel 22.0.73

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }


lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_idtide.xml"
dcubeXml_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_lrt.xml"
dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_idtide_r24.root"
dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r24.root"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_idtide_r23.root"
  dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r23.root"
fi

# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
run Reco_tf.py \
  --inputRDOFile     ${ArtInFile} \
  --outputAODFile   physval.AOD.root \
  --outputDAOD_IDTIDEFile DAOD_TIDE.pool.root \
  --conditionsTag   'OFLCOND-MC16-SDR-RUN2-08' \
  --steering        doRAWtoALL \
  --checkEventCount False \
  --ignoreErrors    True \
  --maxEvents       100 \
  --postExec 'condSeq.TileSamplingFractionCondAlg.G4Version = -1' \
  --preExec 'from InDetRecExample.InDetJobProperties import InDetFlags; \
  InDetFlags.doSlimming.set_Value_and_Lock(False); rec.doTrigger.set_Value_and_Lock(False); \
  rec.doDumpProperties=True; rec.doCalo=True; rec.doEgamma=True; \
  rec.doForwardDet=False; rec.doInDet=True; rec.doJetMissingETTag=True; \
  rec.doLArg=True; rec.doLucid=True; rec.doMuon=True; rec.doMuonCombined=True; \
  rec.doSemiDetailedPerfMon=True; rec.doTau=True; rec.doTile=True; \
  from ParticleBuilderOptions.AODFlags import AODFlags; \
  AODFlags.ThinGeantTruth.set_Value_and_Lock(False);  \
  AODFlags.ThinNegativeEnergyCaloClusters.set_Value_and_Lock(False); \
  AODFlags.ThinNegativeEnergyNeutralPFOs.set_Value_and_Lock(False);\
  AODFlags.ThinInDetForwardTrackParticles.set_Value_and_Lock(False) ' "all:import InDetPhysValMonitoring.InDetPhysValDecoration; InDetPhysValMonitoring.InDetPhysValDecoration.getMetaData = lambda : ''"
rec_tf_exit_code=$?
echo "art-result: $rec_tf_exit_code reco"

if [ $rec_tf_exit_code -eq 0 ]  ;then
  #run IDPVM for IDTIDE derivation
  run runIDPVM.py --doIDTIDE --doTracksInJets --doTracksInBJets --filesInput DAOD_TIDE.pool.root --outputFile physval_idtide.ntuple.root
  #for LRT
  run runIDPVM.py --doLargeD0Tracks --filesInput physval.AOD.root --outputFile physval_lrt.ntuple.root

  echo "download latest result"
  run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
  run ls -la "$lastref_dir"

  echo "compare with R23.0.23 or 24.0.1"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide \
    -c ${dcubeXml_idtide} \
    -r ${dcubeRef_idtide} \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide_last \
    -c ${dcubeXml_idtide} \
    -r ${lastref_dir}/physval_idtide.ntuple.root \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide_last"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt \
    -c ${dcubeXml_lrt} \
    -r ${dcubeRef_lrt} \
    physval_lrt.ntuple.root
  echo "art-result: $? shifter_plots_lrt"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt_last \
    -c ${dcubeXml_lrt} \
    -r ${lastref_dir}/physval_lrt.ntuple.root \
    physval_lrt.ntuple.root
  echo "art-result: $? shifter_plots_lrt_last"
fi

