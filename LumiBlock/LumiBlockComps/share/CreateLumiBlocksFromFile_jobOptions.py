#--------------------------------------------------------------
# job options fragment to bootstrap metadata
#--------------------------------------------------------------
include ("LumiBlockAthenaPool/LumiBlockAthenaPool_joboptions.py")

#To run the luminosity block from event info 'bootstrap' algo
theApp.Dlls += [ "LumiBlockComps" ]
theApp.TopAlg += [ "CreateLumiBlockCollectionFromFile" ]
CreateLumiBlockCollectionFromFile = Algorithm( "CreateLumiBlockCollectionFromFile");


