## Enable MG5_aMC@NLO LHEF reading in Pythia8
include("Pythia8_i/Pythia8_LHEF.py")
evgenConfig.generators += ["aMcAtNlo"]

#aMC@NLO default Pythia8 settings from http://amcatnlo.web.cern.ch/amcatnlo/list_detailed2.htm#showersettings
#plus MEC fix see https://arxiv.org/pdf/2308.06389.pdf
genSeq.Pythia8.Commands += ["SpaceShower:pTmaxMatch = 1",
                            "SpaceShower:pTmaxFudge = 1",
                            "SpaceShower:MEcorrections = off",
                            "TimeShower:pTmaxMatch = 1",
                            "TimeShower:pTmaxFudge = 1",
                            "TimeShower:MEcorrections = on",
                            "TimeShower:MEextended    = off",
                            "TimeShower:globalRecoil = on",
                            "TimeShower:limitPTmaxGlobal = on",
                            "TimeShower:nMaxGlobalRecoil = 1",
                            "TimeShower:globalRecoilMode = 2",
                            "TimeShower:nMaxGlobalBranch = 1.",
                            "TimeShower:weightGluonToQuark = 1.",
                            "Check:epTolErr = 1e-2" ]
