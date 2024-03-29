theApp.setup( MONTECARLO )

include( "PartPropSvc/PartPropSvc.py" )

#--------------------------------------------------------------
# Private Application Configuration options
#--------------------------------------------------------------
theApp.Dlls  += [ "CavernBkgGenerator", "TruthExamples" ]
theApp.TopAlg = ["GenerateCavernBkg", "DumpMC"]
#theApp.TopAlg = ["GenerateCavernBkg"]
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
MessageSvc = Service( "MessageSvc" )
MessageSvc.OutputLevel               = 3
#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
# Number of events to be processed (default is 10)
theApp.EvtMax = 10
#--------------------------------------------------------------
# Algorithms Private Options
#--------------------------------------------------------------

#bkg = Algorithm( "GenerateCavernBkg" )
#bkg.InputFile = "fvpn22n.dat"

DumpMC = Algorithm ("DumpMC")
DumpMC.McEventKey = "CAVERN_BKG"
