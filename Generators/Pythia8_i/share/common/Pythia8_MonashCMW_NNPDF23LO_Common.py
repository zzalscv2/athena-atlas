## Config for Py8 Monash CMW tune
## This is not the default recommended tune, but used for common ATLAS/CMS MC samples 


include("Pythia8_i/Pythia8_Base_Fragment.py")

genSeq.Pythia8.Commands += [
       "Tune:ee = 7",
       "Tune:pp = 14",
       "PDF:pSet = LHAPDF6:NNPDF23_lo_as_0130_qed",
       "SpaceShower:alphaSvalue = 0.118",
       "SpaceShower:alphaSorder = 2",
       "SpaceShower:alphaSuseCMW = on",
       "TimeShower:alphaSvalue = 0.118",
       "TimeShower:alphaSorder = 2",
       "TimeShower:alphaSuseCMW = on"
    ]

evgenConfig.tune = "MonashCMW"
