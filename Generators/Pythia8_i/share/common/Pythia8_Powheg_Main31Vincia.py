## Configure Pythia 8 to shower PoWHEG input with VINCIA shower using Main31 veto
include("Pythia8_i/Pythia8_Powheg.py")

genSeq.Pythia8.Commands += [ 'PartonShowers:model = 2' ]
genSeq.Pythia8.Commands += [ 'Vincia:pTmaxMatch = 2'] # recommended in https://arxiv.org/pdf/2106.10987.pdf
genSeq.Pythia8.Commands += ['Vincia:ewMode = 0']
genSeq.Pythia8.Commands += ['Vincia:interleaveResDec= off']
genSeq.Pythia8.Commands += ['Vincia:helicityShower = off']                             

if "UserHooks" in genSeq.Pythia8.__slots__.keys():
  genSeq.Pythia8.UserHooks += ['PowhegMain31Vincia']
  genSeq.Pythia8.Commands += ['Powheg:veto = 1']
else:
  genSeq.Pythia8.UserHook = 'Main31Vincia'


include("Pythia8_i/Pythia8_ShowerWeights.py")
