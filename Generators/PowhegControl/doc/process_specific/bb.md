# Process-specific information for $`b\bar{b}`$ and $`c\bar{c}`$ production

This is largely a stub, collecting some experience from @jkretz from test productions performed:
* https://its.cern.ch/jira/browse/ATLMCPROD-9635
* https://its.cern.ch/jira/browse/ATLHI-420

At this stage, we have yet to see a completely validated physics setup.

This uses the `hvq` module otherwise widely used for ttbar
production. Obviously all top-quark decay options are disabled as
nonsensical: b- or c-quarks will be hadronised by the shower program
and decayed then.

A minimal bbbar example is in the usual place: https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/PowhegControl/share/example/processes/bb/mc.PhPy8EG_A14NNPDF23_bb_example.py

The ccbar production is (unintuitively) enabled by
```
PowhegConfig.mass_b = 1.55
```

One issue is that the only parameter available to steer the production is
```
PowhegConfig.bornsuppfact = XX
```
that will suppress (down-weight) the cross section below a Born pT configuration of `XX` GeV.
There is no option to cut away lower pT configurations completely.

A more extensive sliced setup with tuned parameters can be found in these jO: 
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950455/mc.PhPy8EG_A14_bb_JZ0_valid.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950456/mc.PhPy8EG_A14_bb_JZ1_valid.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950457/mc.PhPy8EG_A14_bb_JZ2_valid.py  
...

Integrations files needed multicore running for hours or days due to high folding parameters.

Negative weight fractions are quite significant, ccbar production has more than bbbar.

In practice, distributions were found to be very 'spiky' towards
higher pT, so it may well be that this configuration is simply not
suitable to produce b/c-jets with pT far above the respective quark masses.


Two more recent test productions filtering the events for bb/cc->di-muon decays here

https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/950xxx/950624/mc.PhPy8EG_A14_bb_2mu3p5_valid.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/950xxx/950625/mc.PhPy8EG_A14_cc_2mu3p5_valid.py

Validation of these test productions is pending.