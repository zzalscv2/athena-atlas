# Multi-scale improved NLO (MiNLO) and MiNNLOPS

Multi-scale improved NLO ([MiNLO](http://arxiv.org/abs/1206.3572)) is a
method for simultaneously achieving NLO-accurate predictions for processes two
different jet multiplicities. Example: a MiNLO `Zj` sample is NLO accurate for `Z+0j` and `Z+1j` at the same time
(and LO accurate for `Z+2j`).

It is enabled by default for all supported processes (and in fact we enable the so-called MiNLO' by default).
The list of processes for which MiNLO is enabled is: `Hj`, `Hjj`, `HWj`,
`HZj`, `jjj`, `Wbbj`, `Wj`, `Wjj`, `Zj`, `Zjj`

As extension to MiNLO, recently processes have appeared that make the lower multiplicity NNLO accurate.
Example: a MiNNLO `Zj` sample is NNLO accurate for `Z+0j`, NLO accurate for `Z+1j`, and LO accurate for `Z+2j`.
The list of all existing processes can be found on the [Powheg Homepage, section MiNNLOPS](https://powhegbox.mib.infn.it/#MiNNLOps).
We are currently testing `Hj_MiNNLO`, `Zj_MiNNLO`, `Wj_MiNNLO`, `ttj_MiNNLO`.

It appears the newly available MiNNLO diboson processes require a very large installation, which has not been tackled yet.

## NNLO reweighting (V2 processes only)


Some processes (currently only `Hj`, `Wj` and `Zj`) come with external
reweighting programs which can take the LHE events and provide
event-by-event weights which approximate the full NNLO distribution. The
syntax is slightly different for NNLOPS for `Hj` and DYNNLO for `Wj` and
`Zj`

*Warning: the only existing large-scale NNLOPS production has been the Run 2 ggH sample.
Since the move away from the /afs installation, this has been broken and apparently not fixed.
So what follows is a mostly historical description.
We are currently commissioning the MiNNLOPS version, which should supersede NNLOPS by a single-step generation.*

**NNLOPS for Hj** needs two input commands. The first,
`NNLO_reweighting_inputs` needs arguments which map `name` to
`file_name` where `file_name` is pre-generated reweighting file and
`name` is a user-specified name for the weight corresponding to this.
Similarly `NNLO_output_weights` takes arguments which map `name` to
`operation` where `operation` is a command in the NNLOPS mini-language
and `name` is a user-specified name for the output weight resulting from
this operation.

```py
#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = 'POWHEG H+jet production with NNLOPS'
evgenConfig.keywords = ['Higgs', '1jet']
evgenConfig.contact = ['<james.robinson@cern.ch>']

#--------------------------------------------------------------
# Powheg Hj setup starting from ATLAS defaults
#--------------------------------------------------------------
include('PowhegControl/PowhegControl_Hj_Common.py')
PowhegConfig.NNLO_reweighting_inputs["nn-mtinf"] = "H1250_CM13_CT10_APX0_11.top"
PowhegConfig.NNLO_reweighting_inputs["nn-mtmb"] = "H1250_CM13_CT10_APX2_22.top"
PowhegConfig.NNLO_output_weights["nnlops-mtmb"] = "combine 'nn-mtmb' and 'mtmb'"
PowhegConfig.NNLO_output_weights["nnlops-mtinf" = "combine 'nn-mtinf' and 'mtinf'"
PowhegConfig.NNLO_output_weights["'nnlops-mtmb-bminlo"] = "combine 'nn-mtmb' and 'mtmb-bminlo'"
PowhegConfig.generate()
```

**DYNNLO for Wj and Zj** has simpler syntax: `NNLO_output_weights` is
not needed because DYNNLO always reweights the nominal weight.

```py
#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = 'POWHEG W+jet production with DYNNLO'
evgenConfig.keywords = ['SM', 'W', '1jet']
evgenConfig.contact = ['<james.robinson@cern.ch>']

#--------------------------------------------------------------
# Powheg Wj setup starting from ATLAS defaults
#--------------------------------------------------------------
include('PowhegControl/PowhegControl_Wj_Common.py')
PowhegConfig.NNLO_reweighting_inputs["DYNNLO"] = "Wp_CM8_MMHT14NNLO_11.top'"
PowhegConfig.generate()
```

**Available reweighting files** have already been generated for the `Hj`
and `Wj` processes. where the naming convention is `${process description}_CM${energy}_${PDF}_${further details}_${renormalisation/factorisation scales}`.
We **will not** make
these files ourselves, but if you provide them, we are happy to include
them in future releases. You can see a full list of available
reweighting files available in the release that you're using by running
the command:

```bash
${POWHEGPATH}/AuxFiles/*/
```

- Hj NNLO reweighting files
  - H1250_CM13_CT10_APX0_11.top
  - H1250_CM13_CT10_APX2_11.top
  - H1250_CM13_CT10_APX2_22.top
  - H1250_CM13_CT10_APX2_HH.top
  - H1250-CM13-NNPDF3-APX0-HH.top
  - H1250-CM13-NNPDF3-APX1-HH.top
  - H1250-CM13-NNPDF3-APX2-11.top
  - H1250-CM13-NNPDF3-APX2-HH.top
  - H1250-CM13-NNPDF3-APX2-QQ.top
  - H1250_CM13_PDF4LHC30-APX0-11.top
  - H1250-CM13-PDF4LHC30-APX0-HH.top
  - H1250-CM13-PDF4LHC30-APX1-HH.top
  - H1250_CM13_PDF4LHC30-APX2-11.top
  - H1250_CM13_PDF4LHC30-APX2-22.top
  - H1250_CM13_PDF4LHC30-APX2-HH.top
  - H1250_CM13_PDF4LHC30-APX2-QQ.top
  - H1250_CM8_CT10_APX0_11.top
  - H1250_CM8_CT10_APX2_11.top
  - H1250_CM8_CT10_APX2_22.top
  - H1250_CM8_CT10_APX2_HH.top
- Wj NNLO reweighting files
  - Wp_CM8_MMHT14NNLO_11.top


Please bear in mind that it is not possible for PowhegControl to check
the syntax of the commands provided to the reweighters:
**this is the responsibility of the
user.** Common errors are:

  - specifying reweighting input files that are not in our repository
    and have not been provided by the user
  - trying to combine weights that have not been specified.

Either of these will cause the reweighter to crash in hard-to-debug
ways.

# Particle spin and decays with MadSpin

For some processes involving top quarks, it may be interesting to run MadSpin
over the (undecayed) tops from Powheg to correctly model their spins.
PowhegControl provides a convenient interface for doing this, which must
be enabled by setting the tops as undecayed (for example
=PowhegConfig.decay_mode = "t t\~ \> undecayed"= in the case of the
`tt` process). The following options can then be set in the usual way

|                 Option                  |               Default | Meaning                                                              |
| :-------------------------------------: | --------------------: | :------------------------------------------------------------------- |
|      `PowhegConfig.MadSpin_decays`      | leptonic and hadronic | decays allowed by MadSpin                                            |
|     `PowhegConfig.MadSpin_enabled`      |                  True | set this to `False` if you want undecayed tops without using MadSpin |
|      `PowhegConfig.MadSpin_model`       |          loop\_sm-ckm | which model to import in MadSpin                                     |
|       `PowhegConfig.MadSpin_mode`       |                  full | which spin mode to use in MadSpin                                    |
|    `PowhegConfig.MadSpin_nFlavours`     |                     4 | which flavour scheme to use                                          |
|     `PowhegConfig.MadSpin_process`      |     process-dependent | process that MadSpin is operating on                                 |
| `PowhegConfig.MadSpin_taus_are_leptons` |                  True | whether lepton definitions should include taus                       |
|     `PowhegConfig.MadSpin_paramcard`    |       an empty string | if not empty, dump the param card from the string into the lhe file  |

**You** are responsible for ensuring that the options passed to MadSpin
are correct. A common error is requesting `MadSpin_decays` or
`MadSpin_process` that are not possible with the input Powheg LHE events
(for example, asking MadSpin to decay both tops and antitops in a sample
which contains only tops).
