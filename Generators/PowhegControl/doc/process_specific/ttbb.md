# Process-specific information for $`t\bar{t}b\bar{b}`$ production

## Productions and Documentation

This process was rather deeply studied as part of the QT of Lars
Ferencz https://its.cern.ch/jira/browse/AGENE-1934 and has lead to
[ATL-PHYS-PUB-2022-006](http://cds.cern.ch/record/2802806) which
details the options used.

Relevant prodiction tickets:
* https://its.cern.ch/jira/browse/ATLMCPROD-9694 (main Powheg+Pythia8 samples)
* https://its.cern.ch/jira/browse/ATLMCPROD-9746 (Powheg+Herwig7 samples)
* and links therein

## Physics configuration

### Decay modes (`decay_mode`)

In PowhegControl, we have harmonised the way that decay modes are given by introducing the parameter `PowhegConfig.decay_mode` which accepts a MadGraph-inspired decay syntax. For this process, the possibilities are:

```
t t~ > all
t t~ > b j j b~ j j
t t~ > b l+ vl b~ l- vl~
t t~ > b emu+ vemu b~ emu- vemu~
t t~ > semileptonic
t t~ > undecayed
t t~ > all [MadSpin]
```

The native Powheg decays already **include spin correlations**, as described at the bottom of page 21 of [1802.00426](https://arxiv.org/abs/1802.00426). In addition, we have implemented an interface to MadSpin that also allows you to decay the top quarks (via any channel, let us know if you need more choices).


## Integration parameters

The parameters below were determined by Lars Ferencz (@lferencz). They use `runningscale 2` except where specified otherwise.

| `foldcsi` | `foldy` | `foldphi` | negative weight fraction | production time increase | Note |
|:-------:|:-----:|:-------:|:-------------:|:-------------------:|:---:|
|    5    |   5   |    1    |     9.7%      |         \-          |     |
|    5    |   5   |    1    |     8.1%      |         \-          | Using `runningscale 1` |
|    5    |   5   |    2    |     7.5%      |         10%         |     |
|    5    |   5   |    5    |     6.5%      |         55%         |     |
|   10    |   5   |    5    |     6.3%      |        140%         |     |
|   25    |  10   |   10    |     5.8%      |        322%         |     |


## Production settings, integrations, showering

Integrations for this process are required and take roughly **2 days
on 16 CPU cores** for the setup in  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/601xxx/601226/mc.PhPy8_A14_NNPDF31_ttbb_4FS_bzd5_dilep.py  
Note that there is no need to reintegrate for different decay
options.

Powheg options were chosen as
```
PowhegConfig.runningscales = 3
PowhegConfig.bornzerodampcut = 5
PowhegConfig.for_reweighting = 1
```
See [ATL-PHYS-PUB-2022-006](http://cds.cern.ch/record/2802806) for a discussion.

LHE file production is relatively slow, about 500 events/job on single
core. Pythia8 and Herwig7 can take the same input LHE events.

Herwig7 produces about 2% events with significant momentum-balance
violations. Removal of these events was found to not give any specific
biases, so production was performed without this issue fixed.
