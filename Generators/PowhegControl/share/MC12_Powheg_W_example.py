#--------------------------------------------------------------
# Powheg_W setup with sensible defaults
#--------------------------------------------------------------
include('PowhegControl/PowhegControl_W_Common.py')
PowhegConfig.idvecbos = 24 #24 for W^{+}, -24 for W^{-}
PowhegConfig.vdecaymode = 1 #1 = e, 2 = mu, 3 = tau
PowhegConfig.generateRunCard()
PowhegConfig.generateEvents()

#--------------------------------------------------------------
# Pythia8 showering with new, main31-style shower
#--------------------------------------------------------------
include('MC12JobOptions/Pythia8_AU2_CT10_Common.py')
include('MC12JobOptions/Pythia8_LHEF.py')

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = 'POWHEG+Pythia8 W^{+} production with AU2 CT10 tune'
evgenConfig.keywords    = [ 'SM', 'Wp' ]
evgenConfig.contact     = [ 'stephen.paul.bieniek@cern.ch' ]
evgenConfig.generators += [ 'Pythia8' ]

