# This package contains config of FPGATrackSim simulation and various FPGATrackSim utilities


# How to run FPGATrackSimMapMaker jobs
See info on how to run this in rel 21: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/FPGATrackSim/FPGATrackSimAlgorithms/README.md

and the info on the format of the map files: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/FPGATrackSim/FPGATrackSimMaps/README.md

To run the FPGATrackSimMapMaker in master:
```
python3 -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig --filesInput=../singlemu_invPtFlat1_1M_wrap.root --evtMax=5 OutFileName="NN_" region=0 Exec.DebugMessageComponents='["*/FPGATrackSim*"]'
```
There is more options exposed (look for the addFlags in the FPGATrackSimMapMakerConfig.py)



