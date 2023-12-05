# TrigTauMonitoring:

Tau offline monitoring package.

To add or remove chains from the monitoring, add/remove the `tauMon:t0`, `tauMon:shifter`, or `tauMon:val` monitoring groups in the [Trigger Menu definition files](https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TriggerCommon/TriggerMenuMT/python/HLT/Menu/) as required. If the Monitoring framework is executed standalone on AOD files that don't contain Monitoring information for the used Menu in the Metadata, the manual chain list defined in [ManualChains.py](python/ManualChains.py) will be loaded.

## How to Run standalone:

Execute the tau monitoring locally excluding all other signatures monitoring.

### Run:
```
Run3DQTestingDriver.py --dqOffByDefault Input.Files="['path_to_myxAOD.root file']" DQ.Steering.doHLTMon=True DQ.Steering.HLT.doBjet=False DQ.Steering.HLT.doBphys=False DQ.Steering.HLT.doCalo=False DQ.Steering.HLT.doEgamma=False DQ.Steering.HLT.doJet=False DQ.Steering.HLT.doMET=False DQ.Steering.HLT.doMinBias=False DQ.Steering.HLT.doMuon=False DQ.Steering.HLT.doInDet=False 
```

## How to Run on GRID:

### Setup athena:
```
setupATLAS
lsetup git
asetup Athena,main,latest
```
### Local Installation (optional):

In case you would like to run with a modified version of the monitoring, you would need first to check the TrigTauMonitoring monitoring package following the instructions from:

Clone Athena locally:  https://atlassoftwaredocs.web.cern.ch/gittutorial/git-clone/

and

Compile the TrigTauMonitoring package : https://atlassoftwaredocs.web.cern.ch/gittutorial/branch-and-change/

### Run on the GRID :

Execute the tau monitoring on the GRID excluding all other signatures monitoring.

```
pathena --trf "Run3DQTestingDriver.py --dqOffByDefault DQ.Steering.doHLTMon=True DQ.Steering.HLT.doBjet=False DQ.Steering.HLT.doBphys=False DQ.Steering.HLT.doCalo=False DQ.Steering.HLT.doEgamma=False DQ.Steering.HLT.doJet=False DQ.Steering.HLT.doMET=False DQ.Steering.HLT.doMinBias=False DQ.Steering.HLT.doMuon=False DQ.Steering.HLT.doInDet=False --inputFiles=%IN" --inDS=MyxAOD.root --extOutFile=ExampleMonitorOutput.root --outDS=user.myname.myMonPlots.root

```

## Additional options

### Calculate total efficiencies

All HLT efficiencies are estimated by the Tau monitoring with respect to the L1 accepted RoIs. The total efficiencies, with respect to all offline objects, can also be estimated for single-tau and di-tau chains, by adding the following configuration:

```
--preExec 'from TrigTauMonitoring.TrigTauMonitoringConfig import TrigTauMonAlgBuilder; TrigTauMonAlgBuilder.do_total_efficiency=True'
```
**Be careful!** You shouldn't enable these options when running over data samples acquired with the usual data-taking conditions, since comparisons between chains can be meaningless due to using different prescales! This is mainly to be used on Enhanced Bias and Monte Carlo samples.
