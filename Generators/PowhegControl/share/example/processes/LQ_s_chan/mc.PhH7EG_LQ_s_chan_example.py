# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Herwig LQ s channel production"
evgenConfig.keywords 	= ["BSM", "leptoquark"]
evgenConfig.contact 	= ["andrej.saibel@cern.ch"]
evgenConfig.tune       	= "H7.2-Default"
evgenConfig.generators 	= ["Powheg", "Herwig7", "EvtGen"]
# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg LQ-s-chan process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_LQ_s_chan_Common.py")

PowhegConfig.mLQ 		= 1000.
PowhegConfig.BWgen		= 1
PowhegConfig.LQmasslow		= 200
PowhegConfig.LQmasshigh		= 5000
PowhegConfig.widthLQ		= 39.8
PowhegConfig.runningscale	= 1

# --------------------------------------------------------------
# Integration settings
# --------------------------------------------------------------
PowhegConfig.ncall1 		= 10000
PowhegConfig.ncall2 		= 10000
PowhegConfig.nubound 		= 10000

# Depending on the coupling defined below. 
# The LUX pdfs might be necessary if only y_Xe are set.	
PowhegConfig.PDF	= list(range(82400, 82500))


# Settings for the couplings. 
#  / y_1e y_1m y_1t \    u/d
#  | y_2e y_2m y_2t |    c/s
#  \ y_3e y_3m y_3t /    t/b

PowhegConfig.y_1e = 1
PowhegConfig.y_2e = 0
PowhegConfig.y_3e = 0
PowhegConfig.y_1m = 0
PowhegConfig.y_2m = 0
PowhegConfig.y_3m = 0
PowhegConfig.y_1t = 0
PowhegConfig.y_2t = 0
PowhegConfig.y_3t = 0

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()


pdf_order = "NLO"
me_order = "NLO"

include("Herwig7_i/Herwig72_LHEF.py")
# configure Herwig7
Herwig7Config.add_commands("""
# read LQ model
read Leptoquark.model

# OR, define LQ yourself (use right el. charge!):
# create /ThePEG/ParticleData S0bar
# setup S0bar 9911561 S0bar 1000.0 0.0 0.0 0.0 -1 3 1 0
# create /ThePEG/ParticleData S0
# setup S0 -9911561 S0 1000.0 0.0 0.0 0.0 1 -3 1 0
# makeanti S0bar S0

set /Herwig/Generators/EventGenerator:UseStdout No
set /Herwig/Generators/EventGenerator:PrintEvent 1
set /Herwig/Generators/EventGenerator:MaxErrors 100000
set /Herwig/Partons/RemnantDecayer:AllowTop Yes
set /Herwig/Partons/RemnantDecayer:AllowLeptons Yes

set /Herwig/Particles/e-:PDF /Herwig/Partons/NoPDF
set /Herwig/Particles/e+:PDF /Herwig/Partons/NoPDF

# Optional arguments for the Remnant extraction problem, but used in the Herwig.in by Luca et al.
set /Herwig/Generators/EventGenerator:EventHandler:CascadeHandler:MPIHandler NULL 
set /Herwig/Shower/ShowerHandler:HardEmission 0
set /Herwig/Shower/ShowerHandler:Interactions QCDandQED #options: QCD, QED and QCDandQED
set /Herwig/Generators/EventGenerator:EventHandler:StatLevel Full
""")


Herwig7Config.tune_commands()
Herwig7Config.lhef_powhegbox_commands(lhe_filename=runArgs.inputGeneratorFile, me_pdf_order=me_order, usespin=True, usepwhglhereader=True)
Herwig7Config.me_pdf_commands(order=pdf_order, name="LUXlep-NNPDF31_nlo_as_0118_luxqed")
Herwig7Config.add_commands("""
# These commands are needed to drastically reduce the ratio of the 
# "Remnant extraction failed in ShowerHandler::cascade() from primary interaction"
# error.
set /Herwig/Shower/ShowerHandler:PDFARemnant /Herwig/Partons/Hard{0}PDF
set /Herwig/Shower/ShowerHandler:PDFBRemnant /Herwig/Partons/Hard{0}PDF
set /Herwig/EventHandlers/LHEReader:PDFA /Herwig/Partons/Hard{0}PDF
set /Herwig/EventHandlers/LHEReader:PDFB /Herwig/Partons/Hard{0}PDF
set /Herwig/EventHandlers/LHEReader:WeightWarnings false
""".format(pdf_order))

# add EvtGen
include("Herwig7_i/Herwig71_EvtGen.py")

Herwig7Config.run()


