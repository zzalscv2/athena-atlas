# Process-specific information for VBF $`W \to \ell\nu`$ and $`Z \to \ell\ell`$ production

This is a rough collection of experience from @jkretz from productions performed:
* https://its.cern.ch/jira/browse/ATLMCPROD-9440
* Slides discussiong the setups were presented in the [PMG Weak boson meeting](https://indico.cern.ch/event/1074613/#39-new-vbf-vjj-powhegpythia8-s)

https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600931/mc.PhPy8EG_A14NNPDF23_VBF_Wminusenu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600932/mc.PhPy8EG_A14NNPDF23_VBF_Wminusmunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600933/mc.PhPy8EG_A14NNPDF23_VBF_Wminustaunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600934/mc.PhPy8EG_A14NNPDF23_VBF_Wplusenu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600935/mc.PhPy8EG_A14NNPDF23_VBF_Wplusmunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600936/mc.PhPy8EG_A14NNPDF23_VBF_Wplustaunu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600937/mc.PhPy8EG_A14NNPDF23_VBF_Zee.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600938/mc.PhPy8EG_A14NNPDF23_VBF_Zmumu.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600939/mc.PhPy8EG_A14NNPDF23_VBF_Ztautau.py

The process is overall relatively easy to integrate: expect run time
of **0.5-1 days** with 8 CPU cores.  Production speed is reasonably
fast.

Pythia8 shower options should include
```
genSeq.Pythia8.Commands += ['Powheg:NFinal = 3']
genSeq.Pythia8.Commands += ['SpaceShower:dipoleRecoil = on']
```
to set the right number of final state particles and get the special
colour-flow approximately right.

Few caveats (of the bundled VBFNLO module):
* 'VBF approximation', i.e. not the complete set of diagrams for
  electroweak $`Vjj`$ production
* only electron and muon decays of the $`W`$ and $`Z`$ -- we now
  simply 'hack' the LHE files on the fly to allow also $`\tau`$
  decays. We also rely on Pythia8 to reshuffle momenta to restore the
  $`\tau`$ mass. This all appears to work well.
* no 'invisible' $`Z \to \nu\nu`$