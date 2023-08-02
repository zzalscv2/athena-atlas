**For support: [Mailing list](mailto:atlas-phys-pmg-rivet@cern.ch)**

Rivet contact persons in ATLAS: [Deepak Kar](mailto:deepak.kar@cern.ch)
and [Christian Gütschow ](mailto:chris.g@cern.ch).

For adding your awesome analysis in the official Rivet framework
[see here](https://gitlab.cern.ch/atlas-physics/pmg/rivet-routines).

Finally, if all else fails, you can contact the [Rivet developers](mailto:rivet-support@cern.ch),
but remember that this is an external address and that you should not discuss anything ATLAS Internal!

[[_TOC_]]

# How to use Rivet

Rivet has a growing [wiki](https://gitlab.com/hepcedar/rivet/-/wikis/home) of its own with many useful examples
for using standalone Rivet. This tutorial will focus on the Athena wrapper around Rivet.

# Setup

In general, the latest 22.0/22.6 releases should have the latest Rivet release supported by ATLAS.

```bash
asetup 22.6.1,AthGeneration # or later
source setupRivet
```

The last line is necessary to have the main Rivet executable
as well as useful helper scripts (e.g. `yodamerge`, `rivet-mkhtml`, etc.)
available on the command line.

Occasionally it might be useful to set up the latest nightly in order to benefit
from recent merge requests that haven't made it into a proper release yet:

```bash
asetup main,latest,AthGeneration
source setupRivet
```

That's all.

_Please note this also allow you to run all native Rivet/Yoda commands without the wrapper as well._

The Rivet3 series comes with automatic handling of multiweights as well as couple of syntax changes.
For a quick tutorial on how to make a Rivet2-style routine compatible with Rivet3,
[check this out](https://gitlab.com/hepcedar/rivet/blob/release-3-1-x/doc/tutorials/mig2to3.md).


## Older Rivet versions

We recommend you always upgrade to the latest version in order to pick up bugfixes and new routines as they are added to the Rivet
repo. If you're having trouble migrating - get in touch and we'll be happy to help!

If for some reason, you do need to revert back to an older Rivet version, feel free to pick from the following selection:

| Rivet version | Athena release | Comments |
| :----:  | :-------:| :----- |
| v3.1.8 | `23.6.13,AthGeneration` | |
| v3.1.7 | `22.6.26,AthGeneration` | |
| v3.1.6 | `22.6.20,AthGeneration` | |
| v3.1.5 | `22.6.14,AthGeneration` | |
| v3.1.2 | `22.6.1,AthGeneration` | |
| v3.1.2 | `22.6.0,AthGeneration` | `Rivet_i` disabled by accident |



# Running Rivet over a local EVNT file

Standalone Rivet cannot deal with EVNT files, but that's why we have a `Rivet_i` wrapper in Athena.
As with everything in Athena, this requires some JOs. We've added an [example](share/example/local_jO.py) to this repo.
These JOs are very simple. Take a look:

```python
theApp.EvtMax = -1

import AthenaPoolCnvSvc.ReadAthenaPool
svcMgr.EventSelector.InputCollections = [ 'EVNT.root' ]

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from xAODEventInfoCnv.xAODEventInfoCnvConf import xAODMaker__EventInfoCnvAlg
job += xAODMaker__EventInfoCnvAlg()

from Rivet_i.Rivet_iConf import Rivet_i
rivet = Rivet_i()
import os
rivet.AnalysisPath = os.environ['PWD']

rivet.Analyses += [ 'MC_JETS' ]
rivet.RunName = ''
rivet.HistoFile = 'MyOutput.yoda.gz'
rivet.CrossSection = 1.0
#rivet.IgnoreBeamCheck = True
#rivet.SkipWeights=True
job += rivet
```

One can use the analysis options as:
```python
rivet.Analyses += [ 'ATLAS_2019_I1724098:MODE=TW' ]
```

The cross-section is deliberately set to 1.0 as the `yodamerge` script makes it very straightforward
to scale the output files to some cross-section later on.
Rivet3 will produce one histogram per variation weight in the EVNT file. If you only care about the nominal
you can suppress the extra histograms in the output file by setting the `SkipWeights` flag to `True`.


Hint: In the example above, the output files will be zipped since the additional variation weights can
increase the file size significantly. You do not need to unzip the files, since all of the other
Rivet scripts (`yodamerge`, `rivet-mkhtml`, etc.) happily read in zipped yodas!

More `Rivet_i` options are defined [here](src/Rivet_i.cxx).



You can run these JobOptions like so:

```bash
athena local_jO.py
```


### Automatic cross-section extraction from local runs

In the previous example, we assumed that you downloaded an EVNT file from the grid.

If you generated the EVNT files locally, you typically have the generator cross-section stored in the `log.generate`
file in each run. In that case you can extend the JobOption as follows to feed the correct cross-section
directly into Rivet:

```python
theApp.EvtMax = -1

import re, os, glob

evntfiles=[]
xssum=0.
effsum=0.
nfiles=0
for dir in glob.glob("run_*"):  # assuming EVNT and log.generate files live in directories run_*/file.EVNT.root
    evntfile=dir+"/file.EVNT.root"
    if os.path.exists(evntfile):
        with open(dir+"/log.generate") as logfile:
            for line in logfile:
                xsline = re.findall(r'MetaData: cross-section', line)
                if xsline:
                    xsline = line.split(' ')[4]
                    xssum += float(xsline)
                    evntfiles.append(evntfile)
                    nfiles += 1
                effline = re.findall(r'MetaData: GenFiltEff', line)
                if effline:
                    effline = line.split(' ')[4]
                    effsum += float(effline)

import AthenaPoolCnvSvc.ReadAthenaPool
svcMgr.EventSelector.InputCollections = evntfiles

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from xAODEventInfoCnv.xAODEventInfoCnvConf import xAODMaker__EventInfoCnvAlg
job += xAODMaker__EventInfoCnvAlg()

from Rivet_i.Rivet_iConf import Rivet_i

rivet = Rivet_i()
rivet.Analyses += [ 'MC_JETS' ]
rivet.RunName = ''
rivet.HistoFile = 'MyOutput.yoda.gz'
print (xssum/float(nfiles) * effsum/float(nfiles))
rivet.CrossSection = xssum/nfiles * effsum/nfiles
#rivet.IgnoreBeamCheck = True
#rivet.SkipWeights=True
job += rivet
```


# Running Rivet over an EVNT container on the grid

This works pretty much the same way as above, except you don't want to specify a local file in the JOs.
We've prepared an example for grid running to illustrate the difference:

```python
theApp.EvtMax = -1

import AthenaPoolCnvSvc.ReadAthenaPool

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from xAODEventInfoCnv.xAODEventInfoCnvConf import xAODMaker__EventInfoCnvAlg
job += xAODMaker__EventInfoCnvAlg()

from Rivet_i.Rivet_iConf import Rivet_i
rivet = Rivet_i()
import os
rivet.AnalysisPath = os.environ['PWD']

rivet.Analyses += [ 'MC_JETS' ]
rivet.RunName = ''
rivet.HistoFile = 'MyOutput.yoda.gz'
rivet.CrossSection = 1.0
#rivet.IgnoreBeamCheck = True
#rivet.SkipWeights = True
job += rivet
```


This can be submitted like so:

```bash
lsetup panda
pathena --extOutFile=MyOutput.yoda.gz \
--inDS=mc15_13TeV.123456.MySampleOfInterest.evgen.EVNT.e1234 \
--outDS=user.`whoami`.RivetOnTheGrid.v1 \
grid_jO.py
```


**Note:**

_If you want to run a custom routine, don't forget to send the compiled
library to the grid as well!_ This can be achieved by passing the
`--extFile=RivetMY_ANALYSIS.so` flag.
If you are autobooking histograms from data yoda file, that file need to be
sent as well.


If the container is large and you know the jobs are fast,
it might be more efficient to tweak the number of input files per jobs
using the `--nFilesPerJob` flag.


This can also be done by the
[PMG systemtics tool](https://gitlab.cern.ch/atlas-physics/pmg/tools/systematics-tools/tree/master#installation-on-afs),
which can also combine the outputs.



# Run Rivet on-the-fly with Gen_tf

**Note that due to a bug in HepMC, this feature did not work with Rivet3 until release 21.6.46,AthGeneration or later.**

Rivet jobs can be run in the same job as a `Gen_tf` run,
without needing to modify the standard production jO fragment.
The easiest and the recommended way is to use `--rivetAnas`
option followed by the list of rivet analysis separated by commas. One example is shown below.

```bash
Gen_tf.py --ecmEnergy=13000.0 --randomSeed=1234 --jobConfig=830011 --outputEVNTFile=tmp.EVNT.root --maxEvents=10 --rivetAnas=MC_GENERIC,ATLAS_2020_I1790256.cc
```

_More details about the new generation setup is [here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#Production_transforms_and_job_op)._


The other possibility is to use a `--postInclude=local_jO.py`, where `local_jO.py`
are some after-burner-like JOs to configure Rivet, similar to the standalone JOs mentioned above,
but without the following lines (or equivalent):

```python
import AthenaPoolCnvSvc.ReadAthenaPool
svcMgr.EventSelector.InputCollections = [ 'EVNT.root' ]
```

Also note that when Rivet is run using a `--postInclude` instead of with `--rivetAnas`,
the YODA output file should be specified on the command line using the `--outputYODAFile` arument of `Gen_tf`.
In this way it is also possible to run custom routines on the fly (i.e. as part of running `Gen_tf`).

If the (possibly large) EVNT output is not needed to be saved, then the `--outputEVNTFile` argument can simply
be omitted, but the `--outputYODAFile` would still need to be kept of course, so that the YODA file
will be kept as an output.



# Running Rivet over the on-the-fly variations

Multiweight support is at the heart of the Rivet3. We recommend you use it.
It will book one histogram per available weight variation behind the scenes for you
and add them all to the same output file (with a `histogram name [weight name]` suffix).

This is the default behaviour in Rivet3. If you only care about the nominal weight, you
can disable the extra histograms by setting `rivet.SkipWeights=True` in the JOs.


# Writing a custom routine

## Where to start

Rivet has a huge library of existing routines. Chances are that there is already a routine
which is similar enough to your phase space of interest that it's worth copying it and
using it as a starting point.

If you prefer to start from scratch, the following command will give you a skeleton routine:

```bash
rivet-mkanalysis MY_ANALYSIS
```

The routine will have familiar `init()`, `analyze()` and `finalize()` methods.
Check out the [Rivet wiki](https://gitlab.com/hepcedar/rivet/-/blob/master/doc/tutorials)
for more information.

Note: To enable the Rivet commands for merging and plotting to work with your new local routine,
you will have to set an enviroment variable as e.g.:
```bash
export RIVET_ANALYSIS_PATH=$PWD
```

It's worth reading the
[physics tips and pitfalls](https://gitlab.com/hepcedar/rivet/-/blob/release-3-1-x/doc/tutorials/tips-pitfalls.md)
as well.

## How to compile

Simply run

```bash
rivet-build RivetMY_ANALYSIS.so MY_ANALYSIS.cc
```

This is a nifty little script that attaches all the right compiler flags for you.
Note that the `rivet.AnalysisPath = os.environ['PWD']` part of our example JOs
ensures that the current working directory is added to the relevant environment
variable to make sure that Rivet can find the compiled library (i.e. the `*.so` file).

**Note:**

Remember to send the compiled library to the grid as well if you want to run a custom routine
over a grid container.



# How to merge output files

Rivet provides multiple handy scripts that can be used to merge yoda files:
`rivet-merge` and `yodamerge` and `yodastack`. The `rivet-merge` script is
more sophisticated in the sense that it is Rivet-based and so has access
to the original routine (and actually re-runs the `finalize` method over
the merged result, provided the routine is reentrant safe).
The other two scripts are YODA-based and so know nothing about the routines
and just blindly merge (or stack) YODA objects with the same path.

See this [page](https://gitlab.com/hepcedar/rivet/-/blob/release-3-1-x/doc/tutorials/merging.md)
for a more extensive discussion of the differences between them, including
short tutorials.

As a rule of thumb, use `rivet-merge` if you can, otherwise fall back to the others.
Equivalent merging (e.g. multiple grid outputs from a parallelised run over the same setup):

```
rivet-merge -e -o my_merged_output.yoda.gz MY_GRID_OUTPUT/*
yodamerge -o my_merged_output.yoda.gz MY_GRID_OUTPUT/*
```

When you have to run Rivet over several processes and you want
to merge the grid output as shown above for each process,
you can stack the resulting files to yield the combined output
file like so:

```
rivet-merge -o my_stacked_outpyt.yoda.gz process1.yoda.gz:12.34 process2.yoda.gz:4.56
yodastack -o my_stacked_outpyt.yoda.gz process1.yoda.gz:12.34 process2.yoda.gz:4.56
```

where the optional multipliers at the end of the input files could be the sample
cross-sections from the PMG Central Page, thereby scaling the YODA files on the fly
to the relevant generator cross-section.

# How to plot the output

There are several options, but our favourite way is the following:

```bash
rivet-mkhtml --errs -o my_plots prediction1.yoda:"Title=MC 1" prediction2.yoda:"Title=MC 2"
```

This will create a nice little html booklet that you can stare at in your favourite browser.

Several other options are available with `rivet-mkhtml --help`.
For example, one can select the analyses' plot to run with the option `-a`.
One can also customize the cosmetic of the plots with the
help of a `.plot` file that.
Check the rivet documentation for `make-plots` for more information.


# ATLAS Routines in preparation

The implementation of ATLAS analyses as Rivet plugins is tracked at [RivetMCValidation twiki](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/RivetMCValidation).
Your help will be very appreciated to Rivet-ise your analysis!

[Routines currently in progress](https://gitlab.cern.ch/atlas-physics/pmg/rivet-routines)


# FAQ

This section will be populated from the mailing list questions/answers.

## Converting YODA files to ROOT format

To first order YODA can do what ROOT can, minus all the ROOT bugs,
but if you _really_ need the data in ROOT format, it is
straightforward to convert them using Python, e.g.


```python
from array import array
import ROOT as rt
import yoda

fName = 'myYODAfile.yoda'
yodaAOs = yoda.read(fName)
rtFile = rt.TFile(fName[:fName.find('.yoda')] + '.root', 'recreate')
for name in yodaAOs:
  yodaAO = yodaAOs[name];  rtAO = None
  if 'Histo1D' in str(yodaAO):
    rtAO = rt.TH1D(name, '', yodaAO.numBins(), array('d', yodaAO.xEdges()))
    rtAO.Sumw2(); rtErrs = rtAO.GetSumw2()
    for i in range(rtAO.GetNbinsX()):
      rtAO.SetBinContent(i + 1, yodaAO.bin(i).sumW())
      rtErrs.AddAt(yodaAO.bin(i).sumW2(), i+1)
  elif 'Scatter2D' in str(yodaAO):
    rtAO = rt.TGraphAsymmErrors(yodaAO.numPoints())
    for i in range(yodaAO.numPoints()):
      x = yodaAO.point(i).x(); y = yodaAO.point(i).y()
      xLo, xHi = yodaAO.point(i).xErrs()
      yLo, yHi = yodaAO.point(i).yErrs()
      rtAO.SetPoint(i, x, y)
      rtAO.SetPointError(i, xLo, xHi, yLo, yHi)
  else:
    continue
  rtAO.Write(name)
rtFile.Close()
```

We've added this as a script to the repo too.


## How to use cross-section in a Rivet run:

The cross-section can be set in the jOs or it can be
set to unity and then the resulting yoda file(s) can be scaled after the run. The advantage
of the latter approach is, the user does not have modify the joboption for (say) every jet slice.
If the user does not set the cross-section in the JOs at all,
then I believe in the old version of Rivet_i, then the default value is unity.
Athena expects cross-sections in pb, so if the user is scaling the histograms
using the "/femtobarn" in the routine, it assumes that the value coming out of "crossSections()" is in pb.


## How to book histograms in Rivet3

The correct syntax for user-defined histograms is:

```cpp
book(_h_myhisto,"_h_myhisto",10,0,200);
```

Omitting the `"_h_myhisto"` part will result in it being interpreted as an autobooked histogram
from a reference yoda file, with the numbers corresponding to `d`, `x` and `y` from the usual
`d01-x01-y01` names assigened by HepData.


## How to implement ATLAS truth jet collections in my routine?

### AntiKt4TruthJets

```
FinalState fs(Cuts::abseta < 4.5);
FastJets(fs, 0.4, ANTIKT, JetAlg::Muons::NONE, JetAlg::Invisibles::NONE)
```

### AntiKt4TruthWZJets

```
// Photons
FinalState photons(Cuts::abspid == PID::PHOTON);

// Muons
PromptFinalState bare_mu(Cuts::abspid == PID::MUON, true); // true = use muons from prompt tau decays
DressedLeptons all_dressed_mu(photons, bare_mu, 0.1, Cuts::abseta < 2.5, true);

// Electrons
PromptFinalState bare_el(Cuts::abspid == PID::ELECTRON, true); // true = use electrons from prompt tau decays
DressedLeptons all_dressed_el(photons, bare_el, 0.1, Cuts::abseta < 2.5, true);

//Jet forming
VetoedFinalState vfs(FinalState(Cuts::abseta < 4.5));
vfs.addVetoOnThisFinalState(all_dressed_el);
vfs.addVetoOnThisFinalState(all_dressed_mu);

FastJets jet(vfs, FastJets::ANTIKT, 0.4, JetAlg::Muons::ALL, JetAlg::Invisibles::DECAY);
```


### AntiKt10TruthJets

Same as AntiKt4TruthJets, just change the radius from 0.4 to 1.0


### AntiKt10TruthWZJets

Same as AntiKt4TruthWZJets, just change the radius from 0.4 to 1.0


### AntiKt10TruthTrimmedPtFrac5SmallR20Jets

Declare in `init`:
```
 _trimmer = fastjet::Filter(fastjet::JetDefinition(fastjet::kt_algorithm, 0.2), fastjet::SelectorPtFractionMin(0.05));
```
Then in `execute`:
```
PseudoJets tr_ljets;
for (const Jet& fjet : fjets) {
   tr_ljets += _trimmer(fjet);
}
```
where `fjets` are obtained from AntiKt10TruthJets.


### AntiKt10TruthSoftDropBeta100Zcut10Jets

Include the following header:
```
#include "fastjet/contrib/SoftDrop.hh"
```
In `execute`:
```
fastjet::contrib::SoftDrop sd(1.0, 0.1);
for (const Jet& fjet : fjets) {
   sd_ljets += sd(fjet);
}
```
where `fjets` are obtained from AntiKt10TruthJets.


### AntiKt10TruthBottomUpSoftDropBeta100Zcut5Jets

Include the following header:
```
#include "fastjet/contrib/BottomUpSoftDrop.hh"
```
As above, but:
```
fastjet::contrib::BottomUpSoftDrop busd(1.0, 0.05);
```

### AntiKt10TruthRecursiveSoftDropBeta100Zcut5NinfJets

Include the following header:
```
#include "fastjet/contrib/RecursiveSoftDrop.hh"
```
As above, but:
```
fastjet::contrib::RecursiveSoftDrop rsd(1.0, 0.05);
```


## Plotting issues:

### Where is my histogram?

histograms with a leading `_`
in their name are interpreted as auxiliary objects and
hence skipped by the plotting scripts, similar to the `_EVTCOUNT` and `_XSEC`
objects in the output file.

### How to plot with/without weights?

While `rivet-mkhtml` can plot the default weight, if the user needs to plot another weight, say
`ATLAS_2014_I1319490/d01-x01-y01[1010]`, it can be done by:
`rivet-mkhtml out.yoda:"DefaultWeight=1010"`

On the other hand, `--no-weights` flag will only plot the nominal.

### How to add the ATLAS logo?

In the `.plot` file, you can add the following:

```
# BEGIN SPECIAL /MY_ROUTINE/my_histo logo
\psclip{\psframe[linewidth=0, linestyle=none](0,0)(1,1)}
\rput[Br]{0}(0.95,0.85){\fontsize{14pt}{14pt}\selectfont {\bfseries \itshape ATLAS} \fontsize{12pt}{12pt}\selectfont Preliminary}
\rput[Br]{0}(0.95,0.78){\fontsize{10pt}{10pt}\selectfont $\sqrt{s}$ = 13 TeV, 150.0 fb$^{-1}$}
\endpsclip
# END SPECIAL
```
where the text is right-aligned and the round parenthesis is the fractional coordinate pair (in this case the top right corner).
For left-aligned text in the top left corner, something like the following should work:

```
# BEGIN SPECIAL /MY_ROUTINE/my_histo anotherLogo
\psclip{\psframe[linewidth=0, linestyle=none](0,0)(1,1)}
\rput[Bl]{0}(0.05,0.85){\fontsize{14pt}{14pt}\selectfont {\bfseries \itshape ATLAS} \fontsize{12pt}{12pt}\selectfont Generator Level}
\rput[Bl]{0}(0.05,0.78){\fontsize{10pt}{10pt}\selectfont $\sqrt{s}$ = 13 TeV}
\endpsclip
# END SPECIAL
```

## How to scale a yoda file?

````
yodascale -c '.* 10x' file.yoda
````

will create a `file-scaled.yoda`, where every histogram will be scaled by a factor of 10.


## Can I run over (D)AOD files?

Yes! AODs and TRUTH1 DAOD can be read in and passed onto Rivet (Unfortunately TRUTH3 DAOD cannot, because the full truth record is not retained for TRUTH3).  For AOD files specifically, some of the events may be missing beam protons, causing Rivet to complain about a beam mismatch. If you encounter this problem, you can ask Rivet_i to add some dummy protons to the reconstructed GenEvent by setting the following flag in the JobOptions:
```
rivet.AddMissingBeamParticles = True
```
this will inject the necessary protons in the targeted events (those missing beam protons) with the appropriate centre of mass energy.

## Analysis plugins not found?

In case your custom routine cannot be found, you can point Rivet at its location by setting
`RIVET_ANALYSIS_PATH` to the location of the compiled plugin.
Most scripts will also accept a `--pwd` flag to look for it in the current working directory,
and JobOptions can be supplemented with an equivalent
```
rivet.AnalysisPath = os.environ['PWD']
```
setting.

Note that in the cvmfs installation of v3.1.8, the build-in analysis plugins cannot be found.
This is a known bug that has been fixed for later versions.
This can be worked around by setting the environment variable:
```
export RIVET_ANALYSIS_PATH=$RIVET_PATH/lib/Rivet:$RIVET_PATH/share/Rivet
```

