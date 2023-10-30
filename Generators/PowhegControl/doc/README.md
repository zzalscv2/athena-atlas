# Powheg for ATLAS

This directory contains the documentation and user instructions of the ATLAS Powheg interface. It is called [PowhegControl](https://gitlab.cern.ch/atlas/athena/tree/21.6/Generators/PowhegControl) and is a part of [Athena](https://gitlab.cern.ch/atlas/athena). PowhegControl provides an interface to the [Powheg Box](http://powhegbox.mib.infn.it) software installed on CVMFS.

This file contains the most relevant documentation for users. Further documentation of various advanced aspects is organised into additional files:

* [Systematic variations (PDF, QCD scales, and beyond)](Generators/PowhegControl/doc/variations.md)
* [Phase space integration parameters and event generation times](Generators/PowhegControl/doc/integration.md)
* [Advanced physics features: MiNLO, MiNNLO, NNLOPS, MadSpin](Generators/PowhegControl/doc/beyond_vanilla.md)
* [Non-standard ways of using PowhegControl](Generators/PowhegControl/doc/experimental.md)
* [Legacy documentation for old ATLAS software releases](Generators/PowhegControl/doc/legacy.md)

Contents of this file:

[[_TOC_]]

# ATLAS Powheg experts and maintainers

The current Powheg experts in ATLAS and maintainers of Powheg installations and the PowhegControl interface are:

* **Dan Hayden** (@dhayden) — mostly handles Powheg source code and installation
* **Stefan Richter** (@strichte) — mostly maintains PowhegControl interface, i.e. the Athena user interface to Powheg
* **Timothée Theveneaux-Pelzer** (@tpelzer) — mostly maintains PowhegControl interface
* **Jan Kretzschmar** (@jkretz) — Powheg 'power user' happy to share his knowledge or help out with simple code changes
* All of us provide help with software usage and physics questions to our best availability and ability

For any questions about Powheg or PowhegControl, please **contact** the experts at [**atlas-generators-powhegcontrol-experts@cern.ch**](mailto:atlas-generators-powhegcontrol-experts@cern.ch)!

If you're interested in getting news about Powheg(Control) in general, you can subscribe to the **e-group** [**atlas-generators-powhegcontrol@cern.ch**](mailto:atlas-generators-powhegcontrol@cern.ch).



# Supported processes

The currently installed processes are listed. If the version is of the form `PowhegControl-XX-XX-XX`, the process has been available since long and will be available for any future releases. In case the form is `ATLASOTF-XX-XX-YY`, you may cross-check with [these tables](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgMcSoftware#Versions_release_21_6_AthGenerat) - generally you want the latest release if starting fresh.

We try to make the naming of processes consistent in Athena. Therefore some processes are **named differently** than they are in Powheg Box wherever shown in the table.

| PowhegControl name | Powheg Box name           | Process description           | Available since                                                          | Citation(s)                                          | Remarks              |
| :----------------- | :------------------------ | :---------------------------- | :----------------------------------------------------------------------- | :--------------------------------------------------- | :------------------- |
| `bb`               | `hvq`                     | bbbar                         | `PowhegControl-00-00-08`                                                 | [0707.3088](https://arxiv.org/abs/0707.3088)   | Can also be used for ccbar production, [Process-specific documentation](process_specific/bb.md) |
| `bbH`              | -                         | bbbar+Higgs                   | `PowhegControl-00-03-10`                                                 | [1509.05843](https://arxiv.org/abs/1509.05843) |                      |
| `bblvlv`           | `b_bbar_4l`               | ttbar / Wt interference       | `PowhegControl-00-03-00`                                                 | [1607.04538](https://arxiv.org/abs/1607.04538) | [Process-specific documentation](process_specific/bblvlv.md)             |
| `chi0chi0`         | `weakinos/neuIneuJ`       | neutralino pair               | `PowhegControl-00-03-00`                                                 | [1605.06509](https://arxiv.org/abs/1605.06509) |                      |
| `chi0chi1`         | `weakinos/neuIchaJ`       | neutralino + chargino         | `PowhegControl-00-03-00`                                                 | [1605.06509](https://arxiv.org/abs/1605.06509) |                      |
| `chi1chi1`         | `weakinos/chaIchaJ`       | chargino pair                 | `PowhegControl-00-03-00`                                                 | [1605.06509](https://arxiv.org/abs/1605.06509) |                      |
| `DMGG`             | -                         | dark matter                   | `PowhegControl-00-02-08`                                                 | [1310.4491](https://arxiv.org/abs/1310.4491)   |                      |
| `DMS_tloop`        | -                         | dark matter                   | `PowhegControl-00-02-08`                                                 | [1503.00691](https://arxiv.org/abs/1503.00691) |                      |
| `DMV`              | -                         | dark matter                   | `PowhegControl-00-02-08`                                                 | [1310.4491](https://arxiv.org/abs/1310.4491)   |                      |
| `ggF_H`            | `ggH_quark-mass-effects`  | gg → H                        | `PowhegControl-00-02-00`                                                 | [1111.2854](https://arxiv.org/abs/1111.2854)   | in principle superseded by `Hj_MiNNLO`  |
| `ggF_HH`           | `ggHH`                    | gg → HH                       | `PowhegControl-00-03-12`                                                 | [1703.09252](https://arxiv.org/abs/1703.09252) |                      |
| `ggF_HZ`           | `ggHZ`                    | gg → H+Z                      | `PowhegControl-00-02-00`                                                 | no citation                                          |                      |
| `Hj`               | `HJ`                      | Higgs+1 jet                   | `PowhegControl-00-02-05`                                                 | [1202.5475](https://arxiv.org/abs/1202.5475)   | can (could?) also do NNLOPS                     |
| `Hj_MiNNLO`        | `HJMiNNLO`                | Higgs at NNLO                 | `ATLASOTF-05-01`                                                                     | [2006.04133](https://arxiv.org/abs/2006.04133) |                      |
| `Hjj`              | `HJJ`                     | Higgs+2 jets                  | `PowhegControl-00-02-05`                                                 | [1202.5475](https://arxiv.org/abs/1202.5475)   |                      |
| `HWj`              | `HWJ`                     | Higgs+W+1 jet                 | `PowhegControl-00-02-00`                                                 | [1306.2542](https://arxiv.org/abs/1306.2542)   |                      |
| `HWj_EW`           | `HWJ_EW`                  | Higgs+W+1 jet with EW effects | `PowhegControl-00-03-10`                                                 | [1706.03522](https://arxiv.org/abs/1706.03522) |                      |
| `HZj`              | `HZJ`                     | Higgs+Z+1 jet with EW effects | `PowhegControl-00-02-00`                                                 | [1306.2542](https://arxiv.org/abs/1306.2542)   |                      |
| `HZj_EW`           | `HZJ_EW`                  | Higgs+Z+1 jet                 | `PowhegControl-00-03-10`                                                 | [1706.03522](https://arxiv.org/abs/1706.03522) |                      |
| `jj`               | `dijet`                   | dijet                         | `PowhegControl-00-00-08`                                                 | [1012.3380](https://arxiv.org/abs/1012.3380)   | [Process-specific documentation](process_specific/jj.md)      |
| `jjj`              | `trijet`                  | trijet                        | `PowhegControl-00-00-12`                                                 | [1402.4001](https://arxiv.org/abs/1402.4001)   | [Process-specific documentation](process_specific/jjj.md)  |
| `ssWWjj`           | `Wp_Wp_J_J`               | same-sign WW+2 jets           | `PowhegControl-00-02-14`                                                 | [1102.4846](https://arxiv.org/abs/1102.4846)   |                      |
| `t_sch`            | `ST_sch`                  | single t (s-channel)          | `PowhegControl-00-02-09`                                                 | [0907.4076](https://arxiv.org/abs/0907.4076)   |                      |
| `t_tch_4FS`        | `ST_tch_4f`               | single t (s-channel) 4FS      | `PowhegControl-00-03-00`                                                 | [1207.5391](https://arxiv.org/abs/1207.5391)   |                      |
| `tj`               | `STJ`                     | single t + jet                | TODO                                                                     | [1805.09855](https://arxiv.org/abs/1805.09855) |                      |
| `tt`               | `hvq`                     | ttbar                         | `PowhegControl-00-00-10`                                                 | [0707.3088](https://arxiv.org/abs/0707.3088)   |                      |
| `tt_NLOdecays`     | `ttb_NLO_dec`             | ttbar with NLO decays         | `PowhegControl-00-03-00`                                                 | [1412.1828](https://arxiv.org/abs/1412.1828)   |                      |
| `ttbb`             | -                         | $`\mathrm{t}\bar{\mathrm{t}}\mathrm{b}\bar{\mathrm{b}}`$  | `ATLASOTF-05-01`                                         | [1802.00426](https://arxiv.org/abs/1802.00426) | [Process-specific documentation](process_specific/ttbb.md) |
| `ttH`              | -                         | ttbar+Higgs                   | `PowhegControl-00-02-09`                                                 | [1501.04498](https://arxiv.org/abs/1501.04498) |                      |
| `ttj`              | `ttJ`                     | ttbar+1 jet                   | `PowhegControl-00-02-14`                                                 | [1110.5251](https://arxiv.org/abs/1110.5251)   |                      |
| `ttj_MiNNLO`       | `ttJ_MiNNLO`              | ttbar at NNLO                 | `ATLASOTF-05-05?`                                                 | [2012.14267](https://arxiv.org/abs/2012.14267), [2112.12135](https://arxiv.org/abs/2112.12135)   | being commissioned                     |
| `ttWm_EW`          | `Wtt_dec/pp_ttWm_EW`      | $`\mathrm{t}\bar{\mathrm{t}}\mathrm{W}^{-}`$, NLO electroweak | `ATLASOTF-04-05-02`                      | [2101.11808](https://arxiv.org/abs/2101.11808) | [Process-specific documentation](process_specific/ttW.md). :hot_pepper: : **Physics validation ongoing.** |
| `ttWm_QCD`         | `Wtt_dec/pp_ttWm_QCD`     | $`\mathrm{t}\bar{\mathrm{t}}\mathrm{W}^{-}`$, NLO QCD | `ATLASOTF-04-05-02`                              | [2101.11808](https://arxiv.org/abs/2101.11808) | [Process-specific documentation](process_specific/ttW.md). :hot_pepper: : **Physics validation ongoing.** |
| `ttWp_EW`          | `Wtt_dec/pp_ttWp_EW`      | $`\mathrm{t}\bar{\mathrm{t}}\mathrm{W}^{+}`$, NLO electroweak | `ATLASOTF-04-05-02`                      | [2101.11808](https://arxiv.org/abs/2101.11808) | [Process-specific documentation](process_specific/ttW.md). :hot_pepper: : **Physics validation ongoing.** |
| `ttWp_QCD`         | `Wtt_dec/pp_ttWp_QCD`     | $`\mathrm{t}\bar{\mathrm{t}}\mathrm{W}^{+}`$, NLO QCD | `ATLASOTF-04-05-02`                              | [2101.11808](https://arxiv.org/abs/2101.11808) | [Process-specific documentation](process_specific/ttW.md). :hot_pepper: : **Physics validation ongoing.** |
| `VBF_H`            | -                         | VBF Higgs                     | `PowhegControl-00-02-00`                                                 | [0911.5299](https://arxiv.org/abs/0911.5299)   |                      |
| `VBF_osWW`         | `VBF_Wp_Wm`               | VBF $`\mathrm{W}^+\mathrm{W}^-`$ | In testing phase                                                      | [1301.1695](https://arxiv.org/abs/1301.1695)   | [Process-specific documentation](process_specific/VBF_osWW.md). PDF/scale reweighting broken, being investigated |
| `VBF_ssWW`         | `Wp_Wp_J_J`               | VBF $`\mathrm{W}^{\pm}\mathrm{W}^{\pm}`$ | `PowhegControl-00-02-14`                                      | [1108.0864](https://arxiv.org/abs/1108.0864)   |                      |
| `VBF_W`            | `VBF_W-Z`                 | VBF W                         | `PowhegControl-00-02-17`                                                 | [1302.2884](https://arxiv.org/abs/1302.2884)   | [Process-specific documentation](process_specific/VBF_W-Z.md) |
| `VBF_Z`            | `VBF_W-Z`                 | VBF Z                         | `PowhegControl-00-02-17`                                                 | [1302.2884](https://arxiv.org/abs/1302.2884)   | [Process-specific documentation](process_specific/VBF_W-Z.md) |
| `W`                | -                         | W                             | `PowhegControl-00-00-09`                                                 | [0805.4802](https://arxiv.org/abs/0805.4802)   | in principle superseded by `W_EW`  |
| `W_EW`             | `W_ew-BMNNP`              | W with/without NLO EW effects             | `PowhegControl-00-02-18`                                                 | [1202.0465](https://arxiv.org/abs/1202.0465)   | [Process-specific documentation](process_specific/W-Z_EW.md) |
| `W_SMEFT`          | `W_smeft`                 | W in Standard Model Effective Field Theory | TODO                                                        | [1804.07407](https://arxiv.org/abs/1804.07407), see also [1703.04751](https://arxiv.org/abs/1703.04751) |                      |
| `Wbb`              | `Wbb_dec`                 | W ( → l nu) + bbbar           | `PowhegControl-00-03-00`                                                 | [1502.01213](https://arxiv.org/abs/1502.01213) |                      |
| `Wbbj`             | `Wbbj`                    | W ( → l nu) + bbbar + jet     | `PowhegControl-00-03-00`                                                 | [1502.01213](https://arxiv.org/abs/1502.01213) |                      |
| `Wj`               | -                         | W+1 jet                       | `PowhegControl-00-00-09`                                                 | [1009.5594](https://arxiv.org/abs/1009.5594)   |                      |
| `Wj_MiNNLO`        | `WjMiNNLO`                | W at NNLO                     | `ATLASOTF-05-01`                                                                     | [2006.04133](https://arxiv.org/abs/2006.04133) |                      |
| `Wjj`              | -                         | W+2 jets                      | `PowhegControl-00-02-17`                                                 | [1303.5447](https://arxiv.org/abs/1303.5447)   |                      |
| `Wt_DR`            | `ST_wtch_DR`              | W+t (diagram removal)         | `PowhegControl-00-02-01`                                                 | [1009.2450](https://arxiv.org/abs/1009.2450)   |                      |
| `Wt_DS`            | `ST_wtch_DS`              | W+t (diagram subtraction)     | `PowhegControl-00-02-09`                                                 | [1009.2450](https://arxiv.org/abs/1009.2450)   |                      |
| `WW`               | -                         | W+W-                          | `PowhegControl-00-00-08`                                                 | [1311.1365](https://arxiv.org/abs/1311.1365)   |                      |
| `Wy`               | `Wgamma`                  | W+gamma                       | `PowhegControl-00-03-10`                                                 | [1410.3802](https://arxiv.org/abs/1410.3802)   |                      |
| `WZ`               | -                         | WZ                            | `PowhegControl-00-00-08`                                                 | [1311.1365](https://arxiv.org/abs/1311.1365)   | [Process-specific documentation](process_specific/WZ.md) |
| `yj`               | `directphoton`            | γ + jet                       | In testing phase (crashing)                                              | [1610.02275](https://arxiv.org/abs/1610.02275), see also [1709.04154](https://arxiv.org/abs/1709.04154) |                      |
| `Z`                | -                         | Z                             | `PowhegControl-00-00-09`                                                 | [0805.4802](https://arxiv.org/abs/0805.4802)   | in principle superseded by `Z_EW`    |
| `Z_EW`             | `Z_ew-BMNNPV`             | Z with/without NLO EW effects             | `PowhegControl-00-02-18`                                                 | [1302.4606](https://arxiv.org/abs/1302.4606)   | [Process-specific documentation](process_specific/W-Z_EW.md)   |
| `Z_SMEFT`          | `Z_smeft`                 | Z in Standard Model Effective Field Theory | TODO                                                        | [1804.07407](https://arxiv.org/abs/1804.07407) |                      |
| `Zj`               | -                         | Z+1 jet                       | `PowhegControl-00-00-09`                                                 | [1009.5594](https://arxiv.org/abs/1009.5594)   |                      |
| `Zj_MiNNLO`        | `ZjMiNNLO`                | Z/gamma* at NNLO                 | `ATLASOTF-05-01`                                                      | [2006.04133](https://arxiv.org/abs/2006.04133) |                      |
| `Zjj`              | -                         | Z+2 jets                      | `PowhegControl-00-02-17`                                                 | [1303.5447](https://arxiv.org/abs/1303.5447)   |                      |
| `ZZ`               | -                         | ZZ                            | `PowhegControl-00-00-08`                                                 | [1311.1365](https://arxiv.org/abs/1311.1365)   |


## Requesting new processes

If you would like a new process installed that is not on the list above, please let us know by creating a new [JIRA issue here](https://its.cern.ch/jira/browse/AGENE-968).
If there are problems with the install/special instructions then the user is asked to debug and provide instructions for the proper installation.

The standard steps needed are:

| Step number | Person responsible | Description                                                                                                                         | Approximate time |
| :---------- | :----------------- | :---------------------------------------------------------------------------------------------------------------------------------- | :--------------- |
| 1           | You                | Identify a release of POWHEG in which this process exists and compiles correctly                                                    | 1 day            |
| 2           | You                | Ask the Powheg on-the-fly authors to add this process                                                                               | \<1 day          |
| 3           | Us                 | Download, compile and debug the code at CERN                                                                                        | 1 days           |
| 4           | Us                 | Write a new interface class to ensure that PowhegControl knows about the process                                                    | 1 day            |
| 5           | Us                 | Optimise the integration parameters for this process (if you know some which have been used previously, that would be a good start) | \>1 week         |
| 6           | Us                 | Generate a small test sample of events                                                                                              | \~1 week         |
| 7           | You/Us             | Test your generation setup works as expected                                                                                        | \~1 week         |
| 8           | You                | Perform whatever validation is requested by the MC generators group                                                                 | \>1 week         |
| 9           | You                | Make a JIRA request for whatever number of events you need                                                                          | \>1 week         |

**New available processes are only added to Athena by request**, so please get in touch early if you're interested.


# Usage instructions

All of the following instructions assume that you are working in ATLAS generator software release 21.6. In the rare event that you need instructions for earlier (or later) releases, please [contact the Powheg experts](mailto:atlas-generators-powhegcontrol-experts@cern.ch).
Most instructions will directly apply as well to the release 22.6 series.

## Setting up

Powheg for ATLAS is available in the **release series 21.6 and 22.6** (`AthGeneration` releases). The recommendation is to use [the most recent release](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgMcSoftware#Versions_release_21_6_AthGenerat), unless
specifically noted otherwise. To set up, do the following on any machine with access to the ATLAS software (CERN LXPlus, your institute's cluster, …):

```bash
setupATLAS # = source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
asetup AthGeneration 21.6.99 # or whichever release number you want
```

#### Side note about releases and versioning

While PowhegControl is part of Athena and therefore automatically versioned with the release, the Powheg Box installations are versioned and installed on CVMFS separately from Athena. The install path is contained in the environment variable `$POWHEGPATH` - this may be handy also for very advanced users to have a look at the Powheg documentation or source code of your process.

To ensure compatibility, each Athena release points to a specific ATLAS Powheg installation version. Normally you don't need to know their version, but if you ever want to check it, you can find it in the PowhegControl source at `https://gitlab.cern.ch/atlas/athena/-/blob/<release of interest>/Generators/PowhegControl/cmake/PowhegEnvironmentConfig.cmake.in`
Just replace `<release of interest>` by whatever release you're interested in. E.g. for release 21.6.99 you would substitute in `release/21.6.99` to get [this URL](https://gitlab.cern.ch/atlas/athena/-/blob/release/21.6.99/Generators/PowhegControl/cmake/PowhegEnvironmentConfig.cmake.in) and then you can see that the Powheg installation version for that release is `ATLASOTF-00-05-05`.


## Running event generation

After setting up, you can generate events using the `Gen_tf.py` executable. Since this is the same for all event generators used via Athena, full instructions are maintained by the Physics Modelling Group; see [here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware) and [here](https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/SoftwareTutorialMonteCarlo) (search for "Gen_tf.py"). We also provide an example below.

If you have a job option with MC ID number `999999` in its name inside directory `foo/`, you can run it by using:

```bash
Gen_tf.py --jobConfig foo --ecmEnergy 13000 --runNumber 999999 --firstEvent 1 --randomSeed 42 --outputEVNTFile Powheg.EVNT.root --maxEvents 10
```

[You can find example job option directories for all Powheg processes supported in ATLAS here](https://gitlab.cern.ch/atlas/athena/-/tree/21.6/Generators/PowhegControl/share/example/processes).


**Powheg generates only the hard scattering process** (typically $`2 \to 1`$ or $`2 \to 2`$ or $`2 \to 3`$ at Born level) **plus NLO QCD corrections** (and sometimes other higher-order corrections). This includes the **virtual corrections** as well as the **real-emission correction** that involves the emission of an additional coloured parton (quark or gluon). These hard events are stored in the text-based [Les Houches Event format](https://arxiv.org/abs/hep-ph/0609017) (LHE). These Les Houches event may then be read by a suitably configured general-purpose MC generator, most commonly Pythia 8 or Herwig 7, which then generate

* additional parton emissions in the [parton shower formalism](http://www.scholarpedia.org/article/Parton_shower_Monte_Carlo_event_generators),
* (additional electroweak radiation,)
* the underlying event (beam remnants, soft multiple parton interactions),
* resonance decays (e.g. $`H \to b\bar{b}`$; if these were not included in the Powheg matrix element or performed by Powheg in the narrow-width approximation),
* hadronisation (formation of physically observable, colour-neutral composite states from unobservable partons),
* decays of "long-lived" particles such as τ-leptons and hadrons.

For some processes, PowhegControl implements a few special features that go beyond this simple picture, e.g. performing more sophisticated resonance decays using MadSpin before handing the events over to Pythia or Herwig. For more information, see the [process-specific instructions](#supported-processes).

Since Powheg Box only provides *executables*, event generation with Powheg is done quite differently in Athena than for most other generators: PowhegControl simply writes out a config file that Powheg understands and then calls the right Powheg process executable with that input file. So the entire Powheg run is finished and the LHE events written out before any other MC generator comes into play.

#### Setting the number of events

**Use the `Gen_tf.py` argument `--maxEvents` to request the number of events you want.** If you are generating showered events, i.e. not just [LHE-level events](#generating-lhe-only-events), PowhegControl applies a **default safety factor of 1.1 to your requested number of events**. This is to prevent the parton shower MC from "running out" of events in case some events are discarded. **In cases where a generator filter is used** that rejects a significant fraction of your events, you will have to test and set this factor around $`1.1 / \mathrm{filter\,efficiency} `$, where again an 10% safety buffer is included here. E.g. in the case your filter only keeps 10% of the events, you will produce 11 times more LHE events than will eventually be available in the EVNT (and passed to simulation).

If your parton shower MC does run out of events (you should see a line similar to: `Abort from Pythia::next: reached end of Les Houches Events File...` in the `log.generate` file), you need to understand why and potentially increase this factor in your job option like this:

```py
PowhegConfig.nEvents *= 5 # or whatever factor
```

This gets applied on top of the 1.1 default factor, so the overall safety factor in this example would be $`1.1 \times 5`$.

**If `--maxEvents` is not provided, Powheg defaults to 10000 events**. Also in this case a default safety factor of 1.1 is applied automatically if showering is done, i.e. you actually get 11000 events from Powheg.



### Generating LHE-only events

By default, only the output EVNT files will be saved at the end of a
production run, but if the `--outputTXTFile <filename>` option is given to `Gen_tf.py`,
then the LHE files will also be saved into whatever container
name is specified by this option. **See the PMG's full instructions [here](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/SpecialConfigurations#Producing_only_LHE_output).**
(A trick is currently necessary to make this work in Athena. It consists of actually producing one Pythia event (that is ignored) but only keeping the LHE output.)

:warning: It has been reported to us that crashes related to Athena not finding files it expects can occur if the file name provided with `--outputTXTFile <filename>` does not have the extension `.tar.gz`, so make sure you use that extension!

## Finding and writing job options

### Existing official job options

Official MC16/MC20/MC21 campaign job options can be found [here](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions). Information about them can be found [in this PMG Twiki](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware). The job options containing [MC IDs in the range 600000–699999 correspond to Powheg](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#DSID_blocks).

Even when you want to make your own job options, say to study varying some parameter, it's often easiest to find an official job option that is close to what you want to do and then modify it.

### Common parameters, their meaning, and useful/common values - :construction: this section is still under construction :construction:

Most Powheg parameters exist for multiple processes. Important ones are listed here. For process-specific parameters, see TODO.

| Parameter name | Meaning | Allowed values | Notes and examples |
| :------------: | :-----: | :------------: |:----------------: |
| `PDF`          | Choice of nominal and (optionally) variation PDF sets | A single LHAPDF ID (`int`) or a `list` of LHAPDF IDs. You can find a table of all available LHAPDF sets and their IDs [here](https://lhapdf.hepforge.org/pdfsets). By default PDFs are taken from the central repository `/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current` that will contain basically all sets although the LHAPDF team may need a bit of time to get the latest sets installed. Ask experts for help if necessary.




### Writing/changing job options

The Powheg Box executables need to receive a runcard which specifies
various parameters and options. Default parameters are already set for
all the parameters associated with each process, and these are
automatically set by including the relevant
`PowhegControl_MyProcess_Common.py` file. Each process can all be
accessed by changing the `MyProcess` in the jobOption include to the
_Athena_ name of the desired process --- see the table of supported processes above. Example job options are available [here](https://gitlab.cern.ch/atlas/athena/-/tree/21.6/Generators/PowhegControl/share/example/processes).

Here is an example for the production of $`\mathrm{t}\bar{\mathrm{t}}`$ events through the PowhegControl interface.

```py
evgenConfig.description = "POWHEG + Pythia8 top pair production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "top"]
evgenConfig.contact = ["luke.skywalker@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg tt process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_tt_Common.py")

# --------------------------------------------------------------
# YOU CAN CHANGE ANY PROCESS PARAMETERS YOU WANT HERE, EXAMPLES BELOW!
# --------------------------------------------------------------

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
```

Powheg Box processes have many parameters which can be changed via the
job options. In order to standardise the interface across processes, the
syntax for these commands has been changed from what can be
found in the relevant Powheg Box manual. **A list of user-configurable
parameters for the process in question, along with a description of their meaning, is printed at the top of the `log.generate` file produced when
running PowhegControl jobOptions.**

For example, to require hadronic top and antitop decays in the above job option, you would add

```py
PowhegConfig.decay_mode = 't t~ > b j j b~ j j'
```

***before***

```py
PowhegConfig.generate()
```

giving an overall set of jobOptions like this (comments and empty lines removed for brevity):


```py
evgenConfig.description = "POWHEG + Pythia8 top pair production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "top"]
evgenConfig.contact = ["luke.skywalker@cern.ch"]
include("PowhegControl/PowhegControl_tt_Common.py")
PowhegConfig.decay_mode = 't t~ > b j j b~ j j'
PowhegConfig.generate()
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
```

As the native runcard for Powheg Box is generated when the job options are run,
parameter changes like this **must be placed before the call to
`PowhegConfig.generate()`**, or they will be ignored in favour of the
default settings.

The last two includes would run Pythia8 after the production of the LHE events by Powheg.
The line
```py
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
```
makes use of the A14 parton shower tune of Pythia8, and the line
```py
include("Pythia8_i/Pythia8_Powheg_Main31.py")
```
instructs Pythia8 to use the events in the produced LHE file assuming they
are events produced by Powheg, using the main31 routine which is the 
appropriate 'matching routine' for the majority of Powheg processes.
Other tunes and/or other routines may be preferable or needed for some cases; in doubt, ask your MC expert.




# PowhegControl interface: more details

## External/Powheg

Processes are installed on cvmfs, so they should be available to anyone setting
up Athena in the standard way. It is often useful to know the specific
version of a PowhegControl process that was used to generate a particular
dataset (for example, for correct citation in a paper). Currently a
mixture of Powheg Box V1, Powheg Box V2 and Powheg Box-RES processes are
used, although V1 processes are by now deprecated and generally replaced by V2,
which also allows features such as PDF reweighting.
Contact the experts in case your desired process is not available in V2, a request might need to be made to the process authors.

**Powheg Box SVN revisions for the processes used in each
`External/Powheg` tag are listed [HERE](https://docs.google.com/spreadsheets/d/16XvI5k2I2On4TkkIWJC9MXr0fMaRU2SvOkgBucRPzs0)
in a Google doc.**







## Advanced event generation

There are several options in Powheg Box generation which can have
complicated effects - sometimes even rendering the output events
theoretically invalid. The defaults set in the `PowhegControl` package
should be correct for the majority of users, but it is possible for
advanced users to change them.

### Powheg Box integration parameters

Powheg Box performs an integration over a
multi-dimensional parameter space before generating events. The
parameters used for this integration must be tuned to get the best
integration speed while providing adequately numerically accurate results. The values used in
`PowhegControl` have been chosen in line with the following guidelines
recommended by the Powheg Box authors.

| Test description                    | Requirement                  | How to reduce if this is too high                   |
| :---------------------------------- | :--------------------------- | :-------------------------------------------------- |
| Cross section uncertainty           | \< 1% of total cross section | increase `ncall1`, `itmx1`, `ncall2` and/or `itmx2` |
| Negative weight events              | \< 1% of events              | increase `foldx`, `foldy` and/or `foldphi`          |
| Upper bound failures: cross-section | \< 1% of events              | increase `ncall2`                                   |
| Upper bound failures: radiation     | \< 1% of events              | increase `nubound`, `xupbound`, `icsimax`, `iymax`  |

The output of these tests is reported in the `log.generate` file
produced when running the jobOptions. You will see output like

```terminal
Py:PowhegControl INFO Integration test :: cross-section uncertainty : 0.26%
00:52:07 Py:PowhegControl INFO Integration test :: negative weight fraction : 0.77%
00:52:07 Py:PowhegControl INFO Integration test :: upper bound violations : 0.90%
```

If any of the integration tests have failed, the INFO messages will be
WARNING messages, and you should increase the appropriate integration
parameters. Few considerations:
* The integration speed scales typically with `ncall1*itmx1` and `ncall2*itmx2`, if
  integrations become too slow to do, consider reusing integration grids and going multicore.
* Both integration and unweighting (event generation) speed scale typically `foldx*foldy*foldphi`, where values of 1,2,5,10 are permissible for each of the three parameters.
  E.g. the speed difference between `1*1*1` and `10*10*10` will be roughly a factor of 1000, i.e. massive.
  It may not be feasible to push the negative weight fraction below 1%, in that case it is advisable to
  find a compromise between generation speed and negative weight fraction, see also [TWiki PmgNegativeWeights](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgNegativeWeights).

In case you change any of the parameters, you will need to reintegrate 
the process and re-check the `PowhegControl` validation:
`bornktmin`, `bornsuppfact`, `foldx`, `foldy`, `foldphi`, `itmx1`,
`itmx2`, `ncall1`, `ncall2`, `nubound`, `xupbound`. Changes
to the phase space under consideration (for example changing pT cuts or
masses of particles) will generally also change the performance.

If you make any changes from the defaults for a particular process,
**YOU** are responsible for ensuring that the output passes these tests.

### Automatic optimisation of integration parameters - :warning: section contains outdated information :warning:

***Please contact the Powheg experts if you need this feature. The tool has been updated and we still need to add it to the documentation.***

A more detailed automated testing suite for trying out and testing
different parameters is here:
<https://svnweb.cern.ch/cern/wsvn/atlas-jrobinso/PowhegAutomator/trunk/>;
please get in touch with Stefan Richter (<stefan.richter@cern.ch>) to
find out how to use this if you want to test several parameter sets.

An example where this has been done and documented is here:
<https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DMProductionRun2#MC15_pilot_request>

### Generating integration grids

The best way to generate and test grids for long-running jobs is to
run on a batch system. A convenience script
for doing this is available
[powhegintegrator](https://gitlab.cern.ch/atlas-physics/pmg/tools/powhegintegrator) and here is the [QT report](https://cds.cern.ch/record/2718541)
describing the development. Instructions are contained in the git package.

*Note*: in case you have no access to a batch system or cannot get the above package to work,
there is also the option to simply run the desired process/setup from the command line and
collect the file `integration_grids.tar.gz` that will be produced at the end.
Make sure you have measures in place to allow the job to take the required CPU cores and time, e.g.
when running for a prolonged time on lxplus your jobs may just get killed.



### Event weights: Born-level suppression and negative weights

Despite the name, POWHEG does generate negative weight events, although
generally much less than NLO setups based on MC@NLO matching. These
are accepted by default in all processes, this would otherwise result in biases.
Negative weights can, in some cases, lead to unphysical negative weight integrals (=events)
in poorly populated areas of phase space and in general they may incur a significant penalty in statistical accuracy,
see also [TWiki PmgNegativeWeights](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgNegativeWeights).

The `bornsuppfact` parameter can be used to suppress regions of the phase space (typically
low pT regions) and bias the event generation towards the region of interest.
`Powheg` will 'compensate' for the bias by generating weighted events.
The average weight is equal to the cross-section of the process in question.

If the resulting spread of weights is large and events with very different weights end up in the same
phase space, the events with large weights may disturb distributions
as well as cross section (and filter efficiency) calculations. A typical case are
low pT events that pick up hard jets from the parton shower. (Although by definition this
should generally not happen, as `Powheg` is supposed to generate the hardest emission.)

| Born-level suppression by default     | Negative weights by default | No weighting by default |
| :------------------------------------ | :-------------------------- | :---------------------- |
| `jjj`, `ssWWjj`, `ttj`, `Wbbj`, `Wbb` | All processes               | None                    |


If either Born-level suppression or negative weights are used (i.e.
for all processes by default), the cross-section reported by the
Powheg Box executable will be incorrect. Instead, PowhegControl will
recompute the cross section as the average event weight (over the
generated LHE events) and print this to the log file.

### Using a particular version of PowhegControl

Besides using a version of PowhegControl that is already in a release (i.e.
something you set up with `asetup`), you can also check out any particular
version you like. The most common use case of this is testing a version that is
still being developed and hasn't been added to a release yet, e.g. when the
interface of a new process is being added. To get any version from the Powheg
experts' fork of Athena, use the following in a clean directory:

```bash
setupATLAS
lsetup git
git clone ssh://git@gitlab.cern.ch:7999/atlas-physics/pmg/mcexperts/powheg-experts/athena.git
# It's probably a good idea to use the Powheg expert's fork of Athena, which
# you get with the above command. This is where the development of PowhegControl
# is done; any important changes are then regularly merged into central Athena.
# If you want to use central Athena, just replace the line above with:
# git clone ssh://git@gitlab.cern.ch:7999/atlas/athena.git
cd athena
git checkout 21.6 # or whichever branch/commit you wish to use!
# Branch 21.6 is the "master" for things related to MC16/20 event generation
# Branch 22.6/master is used for MC21+ event generation
cd ..
echo "+ Generators/PowhegControl" > package_filters.txt
echo "- .*" >> package_filters.txt
mkdir build
cd build
asetup AthGeneration,21.6.99 # or whichever release you want to use
cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt ../athena/Projects/WorkDir
cmake --build ./ # or just use the command "make" instead
source */setup.sh
cd ..
mkdir run
cd run
# and run your event generation here
```

If you want to be sure that indeed your local build of PowhegControl is being
used instead of the one from the release, you can open a Python prompt
(`python`), run

```python
# In Python prompt
import PowhegControl
print PowhegControl.__file__
```

and check that the path that gets printed points to a location inside your local
build directory (rather than to some CVMFS location). If this is not the case,
something went wrong: either the build failed (there should be error messages
from that!), or you forgot to `source */setup.sh` inside your build directory.

Remember that if you want to use a very recent Powheg Box or Powheg process
installation, **you need to also manually update the shell environment variable
`POWHEGPATH` _after the release setup (`asetup ...`)_** to point to the desired
location, e.g.:

```bash
export POWHEGPATH=/cvmfs/atlas.cern.ch/repo/sw/Generators/powheg/ATLASOTF-05-05
```

Hint: use `ls $POWHEGPATH/..` to list the available ATLAS Powheg installations.



### Re-using integration files

See also the above section `Generating integration grids`.

If the integration step (before events are generated) is lengthy, the
integration step can be performed once beforehand and then 
skipped during event generation by re-using the Powheg integration
grids. **You** will have to ensure that appropriate grids are used, i.e. those matching
the setup.

Please note that there are usually four steps in PowhegControl
event generation:

- Powheg integration
- Powheg event generation (and generation of weights)
- Athena-based event hadronisation (eg. with Pythia or Herwig)
- (optional) event filtering.

By re-using the Powheg integration grids only the first of these steps
can be skipped. The following files (produced during a local run) are
needed:

| Generation stage |               V1 jobs                |               V2 jobs                |               RES jobs               |
| :--------------: | :----------------------------------: | :----------------------------------: | :----------------------------------: |
|        1         |  `pwgxgrid.dat`, `pwggridinfo*.dat`  |  `pwg*xg*.dat`, `pwggridinfo*.dat`   |           `pwg*xgrid*.dat`           |
|        2         |    `pwg*upb*.dat`, `pwggrid*.dat`    |    `pwg*upb*.dat`, `pwggrid*.dat`    |    `pwg*upb*.dat`, `pwggrid*.dat`    |
|        3         | `pwgfullgrid*.dat`, `pwgubound*.dat` | `pwgfullgrid*.dat`, `pwgubound*.dat` | `pwgfullgrid*.dat`, `pwgubound*.dat` |


If you want to re-use integration grids later, simply put the file
renamed to mc_[SQRTS]TeV.[physicsShort].GRID.tar.gz into the job
option directory, where `SQRTS` is similar to what the MC production system
and rucio use for dataset scopes, e.g. `5` (5.02 TeV), `7`, `8`, `13`, `13p6` (13.6 TeV), `14`.
For production, the integration grids will be registered together with the job options, see
[McSampleRequestProcedure](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/McSampleRequestProcedure)
 and look for "If you are using input tarballs".

Please check the Powheg log file that the use of integration grids give a significant speed
up. Simple processes like (NLO inclusive) ttbar or single W,Z boson processes 
may run the integrations on the fly on a single CPU core in minutes and thus do not 
necessarily need pre-made integration grids. On the other hand, many more complex processes may need
days on multiple CPU cores to perform the integrations and will not be feasible to run
in central production without.

Please note that integration grids generated in either single-core or
multi-core mode can be re-used.

*Old and possibly wrong:* however, if integration files are provided as input to a multi-core
generation job, they will not be used - only single core Powheg
makes the appropriate check for existing integration grids.


### Running in multicore mode

It is possible to run Powheg Box in multicore mode which can speed up
event generation by parallelising the event generation. In order to
enable this, simple set the following environment variable `ATHENA_PROC_NUMBER`
to the desired number of cores to use, e.g.:

```bash
export ATHENA_PROC_NUMBER=8
```



<!--
---

# Frequently Asked Questions

**How do I generate events with Powheg in Athena?**

  - Use `Gen_tf` as explained
    [here](powheg_for_atlas#Generating_events_with_Powheg_OT)


**How do I run with additional event weights?**

  - See [here](powheg_for_atlas#Running_with_multiple_scale_PDF) for
    PDF/scale weights and
    [here](powheg_for_atlas#Running_with_multiple_non_scale) for other
    weights.

**How do I request support for a new process?**

  - Make a JIRA request as explained
    [here](powheg_for_atlas#Requesting_new_processes)

**Where are the user-configurable options listed?**

  - If you run jobOptions that include the Powheg process that you're
    interested in, you'll see these at the top of the log file
    (`log.generate`). See
    [here](powheg_for_atlas#Changing_parameters_in_the_jobOp) for more
    details.

**How long will generation typically take?**

  - Typical times for interactive lxplus runs are shown
    [here](powheg_for_atlas#Approximate_generation_time)

**How can I speed up long-running processes?**

The first thing to do here is to check what's taking the time. Look for
lines like this in your log file

```terminal
22:11:20 Py:PowhegControl INFO Running nominal Powheg took 0h 06m 06s for 1100 events => 3.0024 Hz ...
03:37:45 Py:PowhegControl INFO Running Powheg afterburners took 5h 26m 25s
```

This will tell you how much time is being spent on generating events and
how much time is taken by calculating additional weights (if
applicable). If most of the time is taken in event generation, then
re-use of integration grids is explained [here](powheg_for_atlas#Re_using_integration_files)

**How can I generate integration grids for a long-running process?**

  - Check
    [here](powheg_for_atlas#Generating_integration_grids)

**How can I tell whether the integration grids are being loaded?**

  - Check `log.generate` for the appropriate lines listed in the table
    [here](powheg_for_atlas#Re_using_integration_files)

**Which SVN release of the Powheg Box process am I using?**

  TODO update!

  - Setup the release as usual with `asetup $my_athena_version`
  - Find the version of `External/Powheg` by running `cmt show versions
    External/Powheg`
  - Look up the SVN release of the Powheg Box process in the table
    linked from [here](powheg_for_atlas#External_Powheg)

**How do I enable generator-level multiple event weights?**

  - For renormalisation/factorisation scale changes or PDF reweighting,
    see [this section](powheg_for_atlas#Running_with_multiple_scale_PDF)
  - For other weights such as NNLOPS/DYNNLO, `hdamp` or other parameter
    changes, see [this
    section](powheg_for_atlas#Running_with_multiple_non_scale)

**Which processes have MiNLO enabled?**

  - See the list [here](powheg_for_atlas#Multi_scale_improved_NLO)

 -->
