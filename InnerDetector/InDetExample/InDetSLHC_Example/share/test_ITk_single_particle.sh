#!/bin/bash
# art-description: InclinedDuals sim+reco
# art-type: grid
# art-include: 21.9/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*

echo "Input Parameters"

particle=$1
energy=$2
geometry=$3
script=$4
clustering=$5

echo 'Particle type: '${particle}', energy: '${energy}', geometry: '${geometry}', script name: '${script}', clustering: '${clustering}

if [ ${particle} == "muons" ]; then
  particle1="mu"
elif [ ${particle} == "electrons" ]; then
  particle1="e"
fi
if [ ${energy} == "1GeV" ]; then
  energy1="Pt1"
elif [ ${energy} == "10GeV" ]; then
  energy1="Pt10"
elif [ ${energy} == "100GeV" ]; then
  energy1="Pt100"
fi

# Set analogue clustering by default
if [ ${clustering} != 'digital' ]; then
  clustering="analogue"
fi

if [ ${clustering} == 'digital' ]; then
  clustering_type='InDetSLHC_Example/postInclude.DigitalClustering.py'
elif [ ${clustering} == 'analogue' ]; then
  clustering_type='InDetSLHC_Example/postInclude.AnalogueClustering.py'
fi

echo "Clustering: "${clustering_type}

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

# Following specify which steps to run.
dosim=1
dorec=1
dophy=1    # If dorec=0, set dophy=1 to run InDetPhysValMonitoring over old ESD
dofast=1    # Run fast reco
dophyfast=1 # Run IDPVM over fast reco outputs

# Disable ART tests for fast tracking with 1 GeV config, since reco has 2 GeV pt cut
if [ ${energy} == "1GeV" ]; then
  dofast=0
  dophyfast=0
fi

# Following specify DCube output directories. Set empty to disable.
dcube_sim_fixref="dcube_sim_${particle}_${energy}"
dcube_sim_lastref="dcube_sim_${particle}_${energy}_last"
dcube_digi_pixel_fixref="dcube_digi_pixel_${particle}_${energy}"
dcube_digi_pixel_lastref="dcube_digi_pixel_${particle}_${energy}_last"
dcube_digi_strip_fixref="dcube_digi_strip_${particle}_${energy}"
dcube_digi_strip_lastref="dcube_digi_strip_${particle}_${energy}_last"
dcube_rec_fixref="dcube_${particle}_${energy}"
dcube_rec_lastref="dcube_${particle}_${energy}_last"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
#artdata=/eos/atlas/atlascerngroupdisk/data-art/grid-input

name="InclinedAlternative"


# Set the input file
if [ $particle == "muons" -a $energy == "1GeV" ]; then
  evnt_particle_energy=$artdata/InDetSLHC_Example/inputs/EVNT.09244569.*.pool.root.1
elif [ $particle == "muons" -a $energy == "10GeV" ]; then
  evnt_particle_energy=$artdata/InDetSLHC_Example/inputs/EVNT.09244573.*.pool.root.1
elif [ $particle == "muons" -a $energy == "100GeV" ]; then
  evnt_particle_energy=$artdata/InDetSLHC_Example/inputs/EVNT.09244578.*.pool.root.1
elif [ $particle == "electrons" -a $energy == "100GeV" ]; then
  evnt_particle_energy=$artdata/InDetSLHC_Example/inputs/electrons_100GeV_EVNT.09244567._000004.pool.root.1
fi



if [ $dosim -ne 0 ]; then
  hits_particle_energy=physval_${particle}_${energy}.HITS.root
else
  echo "Sim job not configured to run... no HITS input available for reco step, exiting test!"
  exit
fi
if [ $dorec -ne 0 ]; then
  esd_particle_energy=physval_${particle}_${energy}.ESD.root
  daod_particle_energy=physval_${particle}_${energy}.DAOD_IDTRKVALID.root
else
  echo "Sim job not configured to run... no HITS input available for reco step, exiting test!"
  exit
fi

dcubemon_sim=SiHitValid_${particle}_${energy}.root
dcubemon_digi_pixel=PixelRDOAnalysis_${particle}_${energy}.root
dcubemon_digi_strip=SCT_RDOAnalysis_${particle}_${energy}.root
dcubemon_rec=physval_${particle}_${energy}.root


dcubecfg_sim=${artdata}/InDetSLHC_Example/dcube/config/ITk_SiHitAnalysis.xml
dcubecfg_digi_pixel=${artdata}/InDetSLHC_Example/dcube/config/ITk_PixelRDOAnalysis.xml
dcubecfg_digi_strip=${artdata}/InDetSLHC_Example/dcube/config/ITk_SCT_RDOAnalysis.xml
dcubecfg_rec=${artdata}/InDetSLHC_Example/dcube/config/ITk_IDPVM.xml


if [ ${clustering} == 'digital' ]; then
  dcuberef_rec=${artdata}/InDetSLHC_Example/ReferenceHistograms/physval.ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}_digi.root
elif [ ${clustering} == 'analogue' ]; then
  dcuberef_rec=${artdata}/InDetSLHC_Example/ReferenceHistograms/physval.ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}_ana.root
fi

dcuberef_sim=${artdata}/InDetSLHC_Example/ReferenceHistograms/SiHit_ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}.root
dcuberef_digi_pixel=${artdata}/InDetSLHC_Example/ReferenceHistograms/PixelRDOAnalysis.ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}.root
dcuberef_digi_strip=${artdata}/InDetSLHC_Example/ReferenceHistograms/SCT_RDOAnalysis.ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}.root


art_dcube=/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube
lastref_dir=last_results_${particle}_${energy}



dcube() {
  # Run DCube and print art-result (if $2 is not empty)
  step="$1" statname="$2" dcubemon="$3" dcubecfg="$4" dcuberef="$5" dcubedir="$6"
  test -n "$dcubedir" || return
  run ls -lLU "$art_dcube" "$dcubemon" "$dcubecfg" "$dcuberef"
  if [ ! -s "$dcubemon" ]; then
    echo "$script: $step output '$dcubemon' not created. Don't create $dcubedir output." 2>&1
    test -n "$statname" && echo "art-result: 20 $statname"
    return
  fi
  if [ ! -s "$dcuberef" ]; then
    echo "$script: $step DCube reference '$dcuberef' not available. Don't create $dcubedir output." 2>&1
    test -n "$statname" && echo "art-result: 21 $statname"
    return
  fi
  echo "$script: DCube comparison of $step output '$dcubemon' with '$dcuberef' reference"
  keep=""
  test "$dcubedir" != "dcube" -a -d "dcube" && keep="dcube_keep_`uuidgen`"
  test -n "$keep" && run mv -f dcube "$keep"
  run "$art_dcube" "$name" "$dcubemon" "$dcubecfg" "$dcuberef"
  dcube_stat=$?
  test -n "$statname" && echo "art-result: $dcube_stat $statname"
  test "$dcubedir" != "dcube" && run mv -f dcube "$dcubedir"
  test -n "$keep" && run mv -f "$keep" dcube
}



if [ $dosim -ne 0 ]; then

  ## this runs single particle simulation
  run ls -lL "$evnt_${particle}_${energy}"

  # Sim step: based on RTT SimInclinedDuals_GMX job:
  run Sim_tf.py \
    --inputEVNTFile   $evnt_particle_energy \
    --outputHITSFile  $hits_particle_energy \
    --skipEvents      0 \
    --maxEvents       10000 \
    --randomSeed      873254 \
    --geometryVersion ${geometry}_VALIDATION \
    --conditionsTag   OFLCOND-MC15c-SDR-14-03 \
    --truthStrategy   MC15aPlus \
    --DataRunNumber   242000 \
    --preInclude  all:'InDetSLHC_Example/preInclude.SLHC.py,InDetSLHC_Example/preInclude.SiliconOnly.py,InDetSLHC_Example/preInclude.SLHC_Setup.py,InDetSLHC_Example/preInclude.SLHC_Setup_Strip_GMX.py' \
    --preExec     all:'from InDetSLHC_Example.SLHC_JobProperties import SLHC_Flags; SLHC_Flags.doGMX.set_Value_and_Lock(True);' \
    --postInclude all:'PyJobTransforms/UseFrontier.py,InDetSLHC_Example/postInclude.SLHC_Setup_ITK.py,InDetSLHC_Example/postInclude.SLHC_Setup.py,InDetSLHC_Example/postInclude.SiHitAnalysis.py' \
    --postExec    EVNTtoHITS:'ServiceMgr.DetDescrCnvSvc.DoInitNeighbours = False; from AthenaCommon import CfgGetter;CfgGetter.getService("ISF_MC15aPlusTruthService").BeamPipeTruthStrategies+=["ISF_MCTruthStrategyGroupIDHadInt_MC15"];' all:'ServiceMgr.PixelLorentzAngleSvc.ITkL03D = True'



  echo "art-result: $? sim"
 

  
 
  mv ./SiHitValid.root ./$dcubemon_sim


  if [ \( $dosim -ne 0 -a -n "$dcube_sim_lastref" \) -o \( $dophy -ne 0 -a -n "$dcube_rec_lastref" \) ]; then
    run art.py download --user=artprod --dst="$lastref_dir" InDetSLHC_Example "$script"
    run ls -la "$lastref_dir"
  fi


  # DCube Sim hit plots
  dcube Sim_tf sim-plot "$dcubemon_sim" "$dcubecfg_sim" "$lastref_dir/$dcubemon_sim" "$dcube_sim_lastref"
  dcube Sim_tf ""       "$dcubemon_sim" "$dcubecfg_sim"                         "$dcuberef_sim" "$dcube_sim_fixref"
  
fi

if [ $dorec -ne 0 ]; then

  ## Starting reconstruction for single particles
  if [ $dosim -ne 0 ] && [ ! -s "$hits_particle_energy" ] ; then
    echo "$script: Sim_tf output '$hits_particle_energy' not created. Not running Reco_tf and stopping" 2>&1
    exit
  fi

  run ls -lL "$hits_particle_energy"



  # Reco step: based on RecoInclinedDuals_GMX RTT job
  # some preExecs for E2D step are workarounds - should be revisited. 
  run Reco_tf.py \
      --inputHITSFile    "$hits_particle_energy" \
      --outputRDOFile    "physval_${particle}_${energy}.RDO.root" \
      --outputESDFile    "$esd_particle_energy" \
      --outputAODFile    "physval_${particle}_${energy}.AOD.root" \
      --outputDAOD_IDTRKVALIDFile "$daod_particle_energy" \
      --maxEvents        -1 \
      --digiSteeringConf 'StandardInTimeOnlyTruth' \
      --geometryVersion "all:${geometry}" \
      --conditionsTag    OFLCOND-MC15c-SDR-14-03 \
      --DataRunNumber    242000 \
      --steering doRAWtoALL \
      --postInclude 'all:InDetSLHC_Example/postInclude.SLHC_Setup_ITK.py,InDetSLHC_Example/postInclude.SLHC_Setup.py' 'HITtoRDO:InDetSLHC_Example/postInclude.SLHC_Digitization_lowthresh.py' 'RAWtoALL:'${clustering_type}',InDetSLHC_Example/postInclude.RDOAnalysis.py'\
 --preExec 'all:from AthenaCommon.GlobalFlags import globalflags; globalflags.DataSource.set_Value_and_Lock("geant4"); from InDetSLHC_Example.SLHC_JobProperties import SLHC_Flags; SLHC_Flags.doGMX.set_Value_and_Lock(True)' 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags; digitizationFlags.doInDetNoise.set_Value_and_Lock(False); digitizationFlags.overrideMetadata+=["SimLayout","PhysicsList"]'\
 'RAWtoALL:from InDetRecExample.InDetJobProperties import InDetFlags;from PixelConditionsServices.PixelConditionsServicesConf import PixelCalibSvc;ServiceMgr +=PixelCalibSvc();InDetFlags.useDCS.set_Value_and_Lock(True);ServiceMgr.PixelCalibSvc.DisableDB=True; from InDetPrepRawDataToxAOD.InDetDxAODJobProperties import InDetDxAODFlags; InDetDxAODFlags.DumpLArCollisionTime.set_Value_and_Lock(False);InDetDxAODFlags.DumpSctInfo.set_Value_and_Lock(True); InDetDxAODFlags.ThinHitsOnTrack.set_Value_and_Lock(False)'\
 --preInclude  'all:InDetSLHC_Example/preInclude.SLHC_Setup.py,InDetSLHC_Example/preInclude.SLHC_Setup_Strip_GMX.py' 'HITtoRDO:InDetSLHC_Example/preInclude.SLHC.py,InDetSLHC_Example/preInclude.SiliconOnly.py' 'default:InDetSLHC_Example/preInclude.SLHC.SiliconOnly.Reco.py,InDetSLHC_Example/SLHC_Setup_Reco_TrackingGeometry_GMX.py'\
 --postExec 'HITtoRDO:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' 'RAWtoALL:ToolSvc.InDetSCT_ClusteringTool.useRowInformation=True; from AthenaCommon.AppMgr import ToolSvc; ToolSvc.InDetTrackSummaryTool.OutputLevel=INFO;from InDetPhysValMonitoring.InDetPhysValMonitoringConf import InDetPhysHitDecoratorTool;hitDecoratorTool = InDetPhysHitDecoratorTool(UseNewITkLayerNumbering = False);ToolSvc += hitDecoratorTool;from InDetPhysValMonitoring.InDetPhysValMonitoringConf import InDetPhysValDecoratorAlg;decorators = InDetPhysValDecoratorAlg();topSequence += decorators;' 'all:ServiceMgr.PixelLorentzAngleSvc.ITkL03D = True'



  reco_stat=$?
  echo "art-result: $reco_stat reco"
  if [ "$reco_stat" -ne 0 ]; then
      echo "$script: Reco_tf.py isn't working yet. Remove jobReport.json to prevent pilot declaring a failed job."
      run rm -f jobReport.json
  fi

  mv runargs.RAWtoALL.py runargs.default.RAWtoALL.py
  mv log.RAWtoALL log.default.RAWtoALL
  
  mv ./PixelRDOAnalysis.root ./$dcubemon_digi_pixel
  mv ./SCT_RDOAnalysis.root ./$dcubemon_digi_strip
  
  # DCube digi plots
  dcube Reco_tf digi-plot "$dcubemon_digi_pixel" "$dcubecfg_digi_pixel" "$lastref_dir/$dcubemon_digi_pixel" "$dcube_digi_pixel_lastref"
  dcube Reco_tf  ""       "$dcubemon_digi_pixel" "$dcubecfg_digi_pixel"                           "$dcuberef_digi_pixel" "$dcube_digi_pixel_fixref"
  
  dcube Reco_tf digi-plot "$dcubemon_digi_strip" "$dcubecfg_digi_strip" "$lastref_dir/$dcubemon_digi_strip" "$dcube_digi_strip_lastref"
  dcube Reco_tf  ""       "$dcubemon_digi_strip" "$dcubecfg_digi_strip"                           "$dcuberef_digi_strip" "$dcube_digi_strip_fixref"
    
fi

if [ $dophy -ne 0 ]; then

  ## phys validation and dcube for single particles
  run ls -lLU "$esd_particle_energy"

  if [ ! -s "$esd_particle_energy" ]; then
    echo "$script: Reco_tf output '$esd_particle_energy' not created - exit" 2>&1
    echo "art-result: 21 physval"
    test -n "$dcube_rec_lastref" && echo "art-result: 22 plot"
    exit
  fi
  
  # Run InDetPhysValMonitoring on DAOD.
  # It should eventually be possible to include this in the reco step, but this needs Reco_tf to support the ITk IDPVM setup.
  ( set -x
    exec athena.py InDetPhysValMonitoring/PhysValITk_jobOptions.py -c "INFILE='$daod_particle_energy'"
  )
  echo "art-result: $? physval"
  

  mv ./MyPhysVal.root ./$dcubemon_rec
  
  # DCube InDetPhysValMonitoring performance plots
  dcube InDetPhysValMonitoring plot "$dcubemon_rec" "$dcubecfg_rec" "$lastref_dir/$dcubemon_rec" "$dcube_rec_lastref"
  dcube InDetPhysValMonitoring ""   "$dcubemon_rec" "$dcubecfg_rec"                         "$dcuberef_rec" "$dcube_rec_fixref"
  
fi


if [ $dofast -ne 0 ]; then

  esd_particle_energy_fast=physval_${particle}_${energy}.fast.ESD.root
  aod_particle_energy_fast=physval_${particle}_${energy}.fast.AOD.root
  daod_particle_energy_fast=physval_${particle}_${energy}.fast.DAOD_IDTRKVALID.root

  dcubemon_rec_fast=physval_${particle}_${energy}.fast.root
  dcuberef_rec_fast=${artdata}/InDetSLHC_Example/ReferenceHistograms/physval.ATLAS-P2-ITK-22-02-00_single_${particle1}_${energy1}_fast.root

  dcube_rec_fast_fixref="dcube_fast_${particle}_${energy}"
  dcube_rec_fast_lastref="dcube_fast_${particle}_${energy}_last"
  dcube_rec_fast_std="dcube_fast_${particle}_${energy}_std"


  # Reco step for fast tracking, starting from previous RDO
  run Reco_tf.py \
      --inputRDOFile    "physval_${particle}_${energy}.RDO.root" \
      --outputESDFile    "$esd_particle_energy_fast" \
      --outputAODFile    "$aod_particle_energy_fast" \
      --outputDAOD_IDTRKVALIDFile "$daod_particle_energy_fast" \
      --maxEvents        -1 \
      --digiSteeringConf 'StandardInTimeOnlyTruth' \
      --geometryVersion "all:${geometry}" \
      --conditionsTag    OFLCOND-MC15c-SDR-14-03 \
      --DataRunNumber    242000 \
      --steering doRAWtoALL \
      --postInclude 'all:InDetSLHC_Example/postInclude.SLHC_Setup_ITK.py,InDetSLHC_Example/postInclude.SLHC_Setup.py' 'default:'${clustering_type}''\
      --preExec 'all:from AthenaCommon.GlobalFlags import globalflags; globalflags.DataSource.set_Value_and_Lock("geant4"); from InDetSLHC_Example.SLHC_JobProperties import SLHC_Flags; SLHC_Flags.doGMX.set_Value_and_Lock(True)'\
     'default:from InDetRecExample.InDetJobProperties import InDetFlags;from PixelConditionsServices.PixelConditionsServicesConf import PixelCalibSvc;ServiceMgr +=PixelCalibSvc();InDetFlags.useDCS.set_Value_and_Lock(True);ServiceMgr.PixelCalibSvc.DisableDB=True; from InDetPrepRawDataToxAOD.InDetDxAODJobProperties import InDetDxAODFlags; InDetDxAODFlags.DumpLArCollisionTime.set_Value_and_Lock(False);InDetDxAODFlags.DumpSctInfo.set_Value_and_Lock(True); InDetDxAODFlags.ThinHitsOnTrack.set_Value_and_Lock(False); InDetFlags.doFastTracking.set_Value_and_Lock(True);'\
       --preInclude  'all:InDetSLHC_Example/preInclude.SLHC_Setup.py,InDetSLHC_Example/preInclude.SLHC_Setup_Strip_GMX.py' 'default:InDetSLHC_Example/preInclude.SLHC.SiliconOnly.Reco.py,InDetSLHC_Example/SLHC_Setup_Reco_TrackingGeometry_GMX.py'\
       --postExec 'RAWtoESD:ToolSvc.InDetSCT_ClusteringTool.useRowInformation=True; from AthenaCommon.AppMgr import ToolSvc; ToolSvc.InDetTrackSummaryTool.OutputLevel=INFO;from InDetPhysValMonitoring.InDetPhysValMonitoringConf import InDetPhysHitDecoratorTool;hitDecoratorTool = InDetPhysHitDecoratorTool(UseNewITkLayerNumbering = False);ToolSvc += hitDecoratorTool;from InDetPhysValMonitoring.InDetPhysValMonitoringConf import InDetPhysValDecoratorAlg;decorators = InDetPhysValDecoratorAlg();topSequence += decorators;' 'all:ServiceMgr.PixelLorentzAngleSvc.ITkL03D = True'

  reco_stat=$?
  echo "art-result: $reco_stat reco"
  if [ "$reco_stat" -ne 0 ]; then
      echo "$script: Reco_tf.py isn't working yet. Remove jobReport.json to prevent pilot declaring a failed job."
      run rm -f jobReport.json
  fi

  mv runargs.RAWtoALL.py runargs.fast.RAWtoALL.py
  mv log.RAWtoALL log.fast.RAWtoALL

  if [ $dophyfast -ne 0 ]; then

    ## phys validation and dcube for single particles
    run ls -lLU "$esd_particle_energy_fast"

    if [ ! -s "$esd_particle_energy_fast" ]; then
      echo "$script: Reco_tf output '$esd_particle_energy_fast' not created - exit" 2>&1
      echo "art-result: 21 physval"
      test -n "$dcube_rec_fast_lastref" && echo "art-result: 22 plot"
      exit
    fi

    # Run InDetPhysValMonitoring on DAOD.
    # It should eventually be possible to include this in the reco step, but this needs Reco_tf to support the ITk IDPVM setup.
    ( set -x
      exec athena.py InDetPhysValMonitoring/PhysValITk_jobOptions.py -c "INFILE='$daod_particle_energy_fast'"
    )
    echo "art-result: $? physval"

    mv ./MyPhysVal.root ./$dcubemon_rec_fast

    # DCube InDetPhysValMonitoring performance plots
    dcube InDetPhysValMonitoring plot "$dcubemon_rec_fast" "$dcubecfg_rec" "$lastref_dir/$dcubemon_rec_fast" "$dcube_rec_fast_lastref"
    dcube InDetPhysValMonitoring ""   "$dcubemon_rec_fast" "$dcubecfg_rec" "$dcuberef_rec_fast" "$dcube_rec_fast_fixref"
    dcube InDetPhysValMonitoring ""   "$dcubemon_rec_fast" "$dcubecfg_rec" "$dcubemon_rec" "$dcube_rec_fast_std"

  fi

fi
