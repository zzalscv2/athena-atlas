# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 gg4l production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "gg4l", "gluonfusion", "Higgs"]
evgenConfig.contact = ["andrej.saibel@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg bblvlv process
# -----------------------------------------------------------
include("PowhegControl/PowhegControl_gg4l_Common.py")

# set 0 if massiveloops = 0
PowhegConfig.mass_b = 0 
# 'ZZ' or 'WW'
PowhegConfig.proc = "WW"		
# 'full', 'only_h', 'no_h' or 'interf_h'
PowhegConfig.contr = "full"		
# decay mode of first vector boson
PowhegConfig.vdecaymodeV1 = 11 
# decay mode of second vector boson
# has to be opposite sign for WW
PowhegConfig.vdecaymodeV2 = -13  

# test settings for integration
PowhegConfig.ncall1 = 50
PowhegConfig.ncall1btlbrn = 50
PowhegConfig.ncall2 = 50
PowhegConfig.ncall2btlbrn = 50
PowhegConfig.nubound = 50



# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg.py")
