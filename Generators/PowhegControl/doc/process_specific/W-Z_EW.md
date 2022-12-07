# Process-specific information for $`W \to \ell\nu`$ and $`Z \to \ell\ell`$ production with and without NLO EW effects

The `W_EW` and `Z_EW` modules are effectively a Powheg-Box V2
replacement of the venerable `W` and `Z` modules. As such they offer
the usual PDF and scale weight functionality and they have been made
the 'default' for late Run-2 / early Run-3 production:
* https://its.cern.ch/jira/browse/ATLMCPROD-9551
* https://its.cern.ch/jira/browse/ATLMCPROD-9932
* See also these [google slides](https://docs.google.com/presentation/d/1HDozj5CyhdeGDYTVarDzSiCSjWzc9VYtKvhKgWE5s6M)

These can also be used in principle to simulate full NLO EW effects.

We comment on these two aspects and some observed features below.

## 'LO EW' production

The following can be considered a 'default set' of jO:

https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601183/mc.PhPy8EG_AZNLO_Wplusenu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601184/mc.PhPy8EG_AZNLO_Wplusmunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601185/mc.PhPy8EG_AZNLO_Wplustaunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601186/mc.PhPy8EG_AZNLO_Wminusenu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601187/mc.PhPy8EG_AZNLO_Wminusmunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601188/mc.PhPy8EG_AZNLO_Wminustaunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601189/mc.PhPy8EG_AZNLO_Zee.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601190/mc.PhPy8EG_AZNLO_Zmumu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/601xxx/601191/mc.PhPy8EG_AZNLO_Ztautau.py  

Some noteable features on the Powheg configuration:
```
PowhegConfig.no_ew=1
PowhegConfig.PHOTOS_enabled = False
PowhegConfig.mass_low=60
# EW parametes *only* for Z
PowhegConfig.scheme=0
PowhegConfig.alphaem=0.00781653
PowhegConfig.mass_W=79.958059
```
* 'no_ew=1' disables the NLO corrections
* `PHOTOS_enabled = False` refers to a special mode where PHOTOS is
  applied to the LHE files, we still apply PHOTOS after showering by Pythia8
* `mass_low=60` sets the usual historic mass cut of mll > 60 GeV
* The electroweak parameter handling has changed from the V1 `W` and
  `Z` modules. Notably, the code will take the W and Z mass and width
  input values in PDG/running width form and translate them to 'fixed
  width' scheme internally. The parameter `runningwidth` is no longer
  required. Also, to get the 'right' effective sin2thetaW /
  vector--axial-vector couplings we set only in the Z jO the W boson
  mass to a value that fulfills the tree-level relation
  cosThetaW=mW/mZ.

Some noteable features on the Powheg / Pythia8 shower configuration:
```
PowhegConfig.ptsqmin=4
genSeq.Pythia8.Commands += ["BeamRemnants:primordialKThard = 1.4"]
```
* To reproduce the AZNLO tune of
  [1406.3660](https://arxiv.org/abs/1406.3660) we require `ptsqmin = 4` GeV.
* Moreover, the behaviour of primordialKT changed in Pythia8.245 and
  any Pythia8.3 thus that the original AZNLO tune is 'broken' for
  lowest pTV. Fixing the value to 1.4 GeV approximately restores the
  previous behaviour

### Integrations

Integrations are in principle not required: one can drop the `ncall`
and `itmx` value by factor ~2 and integrate on the fly. Anyway, now
production was performed with integration grids produced with the abov
settings, that require about 1 CPU hour to run single core.

### Off-peak / excess weights

the mass range can in principe be steered by
```
PowhegConfig.mass_low = ...
PowhegConfig.mass_high = ...
```

However, in contrast to the V1 code version, going off-peak appears
somewhat less stable, possibly a transformation was removed?

Speficically, Z/gamma* low-mass sample test productions (e.g. mll=
10-60 GeV) were seen to randomly lead to large statistical errors of
many % despite hour-long itengrations runs. 

Moreover, generated events were seen to have a lot of 'excess weights`
leading to spiky distributions and poor performance. This is even
occasionally seen for on-peak productions. It may be advisable to
investigate this feature further and/or disable the 'excess weights'
with
```
PowhegConfig.ubexcess_correct = 0
```

### Known limitations

This setup has the advantage to be fast and relatively good for
'inclusive' V produciton, but one should not forget that it is
effectively >10 years old and well behind the state-of-the-art in
several respects:
* PDF and 'scale' weights only apply to the 'Born configuration' and
  appear incomplete in variables that test explicitely the higher
  order terms such as jet distributions, pTV etc
* While well tuned to few % at lower pTV < 50 GeV, the setup
  undershoots data at pTV > 100 GeV or in nJet >=2 selections
  by a factor of 1.5 or more.
* There's a known 'A0 bug/feature', see [ATLAS Z Ai
  paper](https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PAPERS/STDM-2014-10/fig_19a.png)


## WIP: 'NLO EW' production

while it would appear trivial to switch the code into NLO EW mode by
setting `no_ew=0`, the topic is non-trivial. Part of the issue is,
that while in Z one can in principle separate contributions from
ISR/FSR/interference, the Powheg code by default produces also the
leading real photon emission (and for W theorists insit these
contributions cannot be separated in gauge-iunvariant way). And when
merging with downstream tools, one is stuck making sure that there is
no double-counting of QED emissions, while one still wishes to retain
the 'extra' softer QED emissions.

There's generally two ways:
* Pythia8 user hooks to do custom QCD/QED vetoes - this I never fully
  understood and appears a bit black magic
* the Powheg authors provided a tool to run Photos directly on the LHE
  files with the veto done there, which is what is acivated by
  `PowhegConfig.PHOTOS_enabled = True`. The tool is a bit of a hack
  and was found in the past to be e.g. intolerant against adding
  comment lines etc in the file, so one has to check carefully


Simone Amoroso put some jO here:
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/600xxx/600003/mc.PhPy8_Zmumu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/600xxx/600004/mc.PhPy8_Zmumu_EW.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/600xxx/600005/mc.PhPy8_Zmumu_EWho.py  

It is not clear if the produced samples are 'valid' and the setup is
correct.

Especially on the Z quite some work in the LHC EW WG with the Powheg
authors (Fulvio Piccicini and Alessandro Vicini) and Elzbieta
Richter-Was. In that process the Powheg code was fixed and extended
significantly.
