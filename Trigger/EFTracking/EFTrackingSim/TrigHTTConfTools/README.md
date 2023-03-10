# This package contains config of HTT simulation and various HTT utilities


# How to run HTTMapMaker jobs
See info on how to run this in rel 21: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/TrigHTT/TrigHTTAlgorithms/README.md

and the info on the format of the map files: https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-htt/tdaq-htt-offline/athena/-/blob/21.9/Trigger/TrigHTT/TrigHTTMaps/README.md

To run the HTTMapMaker in master:
```
python3 -m TrigHTTConfTools.HTTMapMakerConfig --filesInput=../singlemu_invPtFlat1_1M_wrap.root --evtMax=5 OutFileName="NN_" region=0 Exec.DebugMessageComponents='["*/HTT*"]'
```
There is more options exposed (look for the addFlags in the HTTMapMakerConfig.py)



