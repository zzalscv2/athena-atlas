## Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
## Configure Pythia 8 to shower PoWHEG input using Main31 shower veto
## this JO is only for tests, see AGENE-2176
include("Pythia8_i/Pythia8_Powheg.py")
genSeq.Pythia8.Commands += [ 'SpaceShower:pTmaxMatch = 2',
                             'TimeShower:pTmaxMatch = 2'  ]

if "UserHooks" in genSeq.Pythia8.__slots__.keys():
  genSeq.Pythia8.UserHooks += ['PowhegMain31tW']
  genSeq.Pythia8.Commands += ['Powheg:veto = 1']
else:
  raise RuntimeError("This is a test JO targeting only releases with > 8.3 Pythia8 versions");


include("Pythia8_i/Pythia8_ShowerWeights.py")
