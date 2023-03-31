# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 Drell-Yan Scalar LeptoQuark production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["BSM", "leptoquark"]
evgenConfig.contact = ["tpelzer@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg Drell-Yan Scalar LeptoQuark process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_DY_SLQ_Common.py")

# --------------------------------------------------------------
# Relevant parameters for this process
# --------------------------------------------------------------
# Yukawa couplings to down-type quarks (S1t RR)
PowhegConfig.YSD1x1 = 0.
PowhegConfig.YSD1x2 = 0.
PowhegConfig.YSD1x3 = 0.
PowhegConfig.YSD2x1 = 0.
PowhegConfig.YSD2x2 = 0.
PowhegConfig.YSD2x3 = 0.
PowhegConfig.YSD3x1 = 0.
PowhegConfig.YSD3x2 = 0.
PowhegConfig.YSD3x3 = 2.5
PowhegConfig.MSD = 2e3 # mass
# Yukawa couplings to up-type quarks (S1 RR)
PowhegConfig.YSU1x1 = 0.
PowhegConfig.YSU1x2 = 0.
PowhegConfig.YSU1x3 = 0.
PowhegConfig.YSU2x1 = 0.
PowhegConfig.YSU2x2 = 0.
PowhegConfig.YSU2x3 = 0.
PowhegConfig.YSU3x1 = 0.
PowhegConfig.YSU3x2 = 0.
PowhegConfig.YSU3x3 = 0.
PowhegConfig.MSU = 2e3 # mass
# General Leptoquark (LQ) Parameters
PowhegConfig.SM = 1 # Include SM contribution
PowhegConfig.LQ = 0 # Include basic LQ contributions
PowhegConfig.LQ_EW = 0 # Include LQ corrections to photon/Z couplings
PowhegConfig.LQ_Int = 0 # Include the interference between the SM and the LQ contributions
PowhegConfig.mass_t = 172.5 # top-quark (running) mass
PowhegConfig.mass_low = 2000 # lower limit for dilepton mass
PowhegConfig.mass_high = 2100 # upper limit for dilepton mass
#PowhegConfig.runningscale = 1
#PowhegConfig.new_damp = 1
#PowhegConfig.hnew_damp = 0.5
#PowhegConfig.hdamp = 1.0

# --------------------------------------------------------------
# Integration settings
# --------------------------------------------------------------
PowhegConfig.ncall1 		= 10000
PowhegConfig.ncall2 		= 10000
PowhegConfig.nubound 		= 10000


# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
# Setting the appropriate number of final state particles for the main31 routine
genSeq.Pythia8.Commands += [ 'Powheg:NFinal = 2' ]
