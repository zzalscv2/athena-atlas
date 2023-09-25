# This package contains config of FPGATrackSim simulation and various FPGATrackSim utilities


# How to run FPGATrackSimMapMaker jobs
See info on how to run this in rel 21: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/FPGATrackSim/FPGATrackSimAlgorithms/README.md

and the info on the format of the map files: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/FPGATrackSim/FPGATrackSimMaps/README.md

To run the FPGATrackSimMapMaker in `main`:
```
python3 -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig --filesInput=../singlemu_invPtFlat1_1M_wrap.root --evtMax=5 OutFileName="NN_" region=0 Exec.DebugMessageComponents='["*/FPGATrackSim*"] GeoModel.AtlasVersion="ATLAS-P2-RUN4-03-00-00"'
```
Obviously the correct path and geo version need to be used.

Or instead of the `--filesInput` one can use directly the property (note that a proper escaping is needed - it is python list of strings): 
`Trigger.FPGATrackSim.wrapperFileName="['../singlemu_invPtFlat1_1M_wrap.root',]"`

There is more options exposed (look for the addFlags in the FPGATrackSimMapMakerConfig.py)



# How to run the simulation directly from the RDO,
```
rdo_23p0=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName=None" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimConfTools.FPGATrackSimAnalysisConfig.FPGATrackSimLogicalHistProcessAlgCfg" \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.root \
    --maxEvents 10 \
    --athenaopts="Exec.DebugMessageComponents=\"['*/FPGATrack*/*']\""
```
The last line switches on all debug messages from simulation (the algorithm and sub tools).


