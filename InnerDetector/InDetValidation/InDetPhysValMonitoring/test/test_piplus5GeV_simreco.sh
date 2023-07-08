#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single piplus 5GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422048.ParticleGun_single_piplus_Pt5.merge.EVNT.e7967_e5984_tid20255154_00
# art-input-nfiles: 1
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: SiHitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml 
# art-output: dcube*

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

# Following specify DCube output directories. Set empty to disable.
dcube_sim_fixref="dcube_sim"
dcube_sim_lastref="dcube_sim_last"
dcube_pixel_fixref="dcube_pixel"
dcube_pixel_lastref="dcube_pixel_last"
dcube_sct_fixref="dcube_sct"
dcube_sct_lastref="dcube_sct_last"
dcube_rec_fixref="dcube_shifter"
dcube_rec_lastref="dcube_shifter_last"
dcube_rec_expert_fixref="dcube_expert"
dcube_rec_expert_lastref="dcube_expert_last"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
name="run2"
relname="r24"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  relname="r23"
fi
script="`basename \"$0\"`"
hits=physval.HITS.root
dcubemon_sim=SiHitValid.root
dcubemon_rec=physval.ntuple.root
dcubemon_pixel=PixelRDOAnalysis.root
dcubemon_sct=SCT_RDOAnalysis.root
dcubecfg_sim=$artdata/InDetPhysValMonitoring/dcube/config/run2_SiHitValid.xml
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_piplus_5GeV_simreco_${relname}.root
dcubecfg_pixel=$artdata/InDetPhysValMonitoring/dcube/config/run2_PixelRDOAnalysis.xml
dcuberef_pixel=$artdata/InDetPhysValMonitoring/ReferenceHistograms/PixelRDOAnalysis_piplus_5GeV_simreco_${relname}.root
dcubecfg_sct=$artdata/InDetPhysValMonitoring/dcube/config/run2_SCTRDOAnalysis.xml
dcuberef_sct=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SCT_RDOAnalysis_piplus_5GeV_simreco_${relname}.root
dcubecfgshifter_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_baseline.xml
dcubecfgexpert_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_expert.xml
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/physval_piplus_5GeV_simreco_${relname}.root
art_dcube=$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py

lastref_dir=last_results


# Sim step (based on PanDA job 3777178576 which made HITS.12860054._032508.pool.root.1, which is the input for AMI config q221):
 run Sim_tf.py \
    --AMITag          s3505 \
    --inputEVNTFile   ${ArtInFile} \
    --outputHITSFile  "$hits" \
    --skipEvents      0 \
    --maxEvents       10000 \
    --randomSeed      24304 \
    --physicsList     FTFP_BERT_ATL_VALIDATION \
    --simulator       FullG4 \
    --runNumber       410470 \
    --firstEvent      24303001 \
    --DataRunNumber   284500 \
    --truthStrategy   MC15aPlus \
    --conditionsTag   'OFLCOND-MC16-SDR-RUN2-08' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01_VALIDATION' \
    --preExec         EVNTtoHITS:'simFlags.SimBarcodeOffset.set_Value_and_Lock(200000)' \
                      EVNTtoHITS:'simFlags.TRTRangeCut=30.0; simFlags.TightMuonStepping=True' \
    --preInclude      EVNTtoHITS:'SimulationJobOptions/preInclude.FrozenShowersFCalOnly.py' \
    --postInclude     'default:PyJobTransforms/UseFrontier.py,InDetPhysValMonitoring/postInclude.SiHitAnalysis.py'
sim_tf_exit_code=$?
echo "art-result: $sim_tf_exit_code sim"

if [ $sim_tf_exit_code -eq 0 ]  ;then

 echo "download latest result"
 run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
 run ls -la "$lastref_dir"
 # DCube Sim hit plots
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_fixref} \
   -c ${dcubecfg_sim} \
   -r ${dcuberef_sim} \
   ${dcubemon_sim}
 
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_lastref} \
   -c ${dcubecfg_sim} \
   -r ${lastref_dir}/${dcubemon_sim} \
   ${dcubemon_sim}
 echo "art-result: $? dcube_sim_last"

 run Reco_tf.py \
   --inputHITSFile   "$hits" \
   --outputRDOFile   output.RDO.root \
   --postExec 'condSeq.TileSamplingFractionCondAlg.G4Version = -1' \
   --postInclude 'default:PyJobTransforms/UseFrontier.py,InDetPhysValMonitoring/postInclude.RDOAnalysis.py'
 echo "art-result: $? reco_rdo"
 
 # Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
 run Reco_tf.py \
   --inputRDOFile    output.RDO.root \
   --outputAODFile   physval.AOD.root \
   --outputNTUP_PHYSVALFile ${dcubemon_rec} \
   --conditionsTag   'OFLCOND-MC16-SDR-RUN2-08' \
   --steering        doRAWtoALL \
   --checkEventCount False \
   --ignoreErrors    True \
   --maxEvents       -1 \
   --valid           True \
   --validationFlags doInDet \
   --postExec 'condSeq.TileSamplingFractionCondAlg.G4Version = -1' \
   --preExec 'from InDetRecExample.InDetJobProperties import InDetFlags; \
   InDetFlags.doSlimming.set_Value_and_Lock(False); rec.doTrigger.set_Value_and_Lock(False); \
   from InDetPhysValMonitoring.InDetPhysValJobProperties import InDetPhysValFlags; \
   InDetPhysValFlags.doValidateTightPrimaryTracks.set_Value_and_Lock(True); \
   InDetPhysValFlags.doValidateTracksInJets.set_Value_and_Lock(False); \
   InDetPhysValFlags.doValidateGSFTracks.set_Value_and_Lock(False); \
   InDetPhysValFlags.doExpertOutput.set_Value_and_Lock(True); \
   InDetPhysValFlags.doHitLevelPlots.set_Value_and_Lock(True); \
   rec.doDumpProperties=True; rec.doCalo=True; rec.doEgamma=True; \
   rec.doForwardDet=False; rec.doInDet=True; rec.doJetMissingETTag=True; \
   rec.doLArg=True; rec.doLucid=True; rec.doMuon=True; rec.doMuonCombined=True; \
   rec.doSemiDetailedPerfMon=True; rec.doTau=True; rec.doTile=True; \
   from ParticleBuilderOptions.AODFlags import AODFlags; \
   AODFlags.ThinGeantTruth.set_Value_and_Lock(False);  \
   AODFlags.ThinNegativeEnergyCaloClusters.set_Value_and_Lock(False); \
   AODFlags.ThinNegativeEnergyNeutralPFOs.set_Value_and_Lock(False);\
   AODFlags.ThinInDetForwardTrackParticles.set_Value_and_Lock(False) '
 rec_tf_exit_code=$?
 echo "art-result: $rec_tf_exit_code reco"
 
 if [ $rec_tf_exit_code -eq 0 ]  ;then
 
   echo "compare with a fixed R22"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_fixref} \
     -c ${dcubecfgshifter_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_lastref} \
     -c ${dcubecfgshifter_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}
   echo "art-result: $? dcube_rec_last"

   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_fixref} \
     -c ${dcubecfgexpert_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_lastref} \
     -c ${dcubecfgexpert_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}

   echo "compare with a fixed R22 for PixelRDOAnalysis"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_pixel_fixref} \
     -c ${dcubecfg_pixel} \
     -r ${dcuberef_pixel} \
     ${dcubemon_pixel}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_pixel_lastref} \
     -c ${dcubecfg_pixel} \
     -r ${lastref_dir}/${dcubemon_pixel} \
     ${dcubemon_pixel}
   echo "art-result: $? dcube_pixel_rdo_last"

   echo "compare with a fixed R22 for SCT_RDOAnalysis"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_sct_fixref} \
     -c ${dcubecfg_sct} \
     -r ${dcuberef_sct} \
     ${dcubemon_sct}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_sct_lastref} \
     -c ${dcubecfg_sct} \
     -r ${lastref_dir}/${dcubemon_sct} \
     ${dcubemon_sct}
   echo "art-result: $? dcube_SCT_rdo_last"
 fi

fi
