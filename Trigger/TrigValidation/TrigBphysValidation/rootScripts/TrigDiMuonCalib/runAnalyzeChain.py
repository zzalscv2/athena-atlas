#!/bin/python

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
##-----------------------------------------------------------
##
## runAnalyzeChain.py
##
## Configuration script for TriggerJpsiAnalysis
## Creates ROOT macro with desired configuration,
##
## author: Daniel Scheirich <scheiric@cern.ch>
## Part of TriggerJpsiAnalysis in TrigBphysValidation package
##
##-----------------------------------------------------------
import string, os

##-----------------------------------------------------------
## Basic settings
##-----------------------------------------------------------

chain        = "L2_MU4_DiMu"       # name of the chain that should be analyzed
outputLevel  = "INFO"                 # output level: "DEBUG", "INFO", "WARNING", "ERROR"
execute      = True                   # execute macro

##-----------------------------------------------------------
## More settings
##-----------------------------------------------------------
## add flags to the settings string to activate features
##    gaus+const                        fit J/psi mass with Gaussian + constant
##    gaus+pol1                         fit J/psi mass with Gaussian + linear polynomial
##    fitMin=2000 fitMax=5000           defines fit range
##    massBinLo=8 massBinHi=13          bins berween which efficiency is calculated
settings = ""
#settings    += " gaus+pol1 fitMin=2.2 fitMax=4 "
settings    += " massBinLo=8 massBinHi=13 "
settings    += ' ConstantsFile=\\"constants_final.py\\" '
settings    += ' CutConeFile=\\"cut_cone_final.py\\" '

## trigger cuts
perfCuts    = [ ]

## TrigDiMuon chains cuts
perfCuts += [ "HypoNMuHits(3)"   ]    # no. of associated muon hits requested in Hypo step
#perfCuts += [ "DoNMuHitsScan(1)" ]    # perform number-of-hits scan (only for TrigDiMuon)
#perfCuts += [ "DoChi2Scan(1)"    ]    # perform chi2 scan
perfCuts += [ "ExtraChi2(30)"    ]    # additional chi2 cut
#perfCuts   += [ "HypoMass(2500,4000)" ]      # TrigL2Bphys invariant mass cut

if "_noOS" not in chain :
  perfCuts += [ "HypoCheckOS(1)" ]    # check that track have opposite sign


## additional J/psi cuts
## NOTE that most of the cuts are alredy performed in runConvertChain.py !
#perfCuts += [ "JpsiDPhi(.75)"    ]    # Jpsi opening angle in phi
#perfCuts += [ "JpsiDEta(.75)"    ]    # Jpsi opening angle in eta

#perfCuts += [ "DetPart(BB)" ]         # select only muons from certain part of detector: BB, EE, BE

##-----------------------------------------------------------
## Input files
##-----------------------------------------------------------
## You can either specify list of files explicitly using inFileNames
## or you can load entire directory using dirPath vatiable

## list of input files created by TriggerJpsiAnalysis algorithm
inFileNames  = [ ]
inFileNames += [ chain+".02.root" ]

## directory containing root files created by TriggerJpsiAnalysis algorithm
#dirPath       = "/net/s3_datac/scheiric/data/output_TrigDiMuonCalib/2010-06-18/rootfiles"   ## muon stream, fixed TrigDecisionToolBug

##-----------------------------------------
## Main script -- DO NOT EDIT !
##-----------------------------------------
## add J/psi cuts

cutStr = ""
for cut in perfCuts:
  cutStr += cut + " "

## generate root macro

macro   = ""
macro   += "// Generated by runAnalyzeChain.py\n"
macro   += "#include <vector>\n"
macro   += "#include <string>\n"
macro   += "#include \"Log.h\"\n"
macro   += "void runAnalyzeChain() {\n"
macro   += "  gSystem->Load(\"TrigDiMuonRootCalib_cpp.so\");\n"
if('dirPath' in dir()):
  macro   += "  TrigDiMuonRootCalib analysis(" + outputLevel + ", \"" + dirPath + "\");\n"

else:
  macro   += "  std::vector<std::string> files;\n"
  for file in inFileNames :
    macro += "  files.push_back( \"" + file + "\" );\n"
  macro   += "  TrigDiMuonRootCalib analysis(" + outputLevel + ", files);\n"

macro   += "  analysis.analyzeChain(\""+ chain +"\", \""+ settings +"\", \""+ cutStr +"\");\n"
macro   += "}\n"

## create macro
f = open("runAnalyzeChain.C","w")
f.write(macro)
f.close()

## execute root macro
if(execute) :
  os.system( "root -l runAnalyzeChain.C" )

## end of script



