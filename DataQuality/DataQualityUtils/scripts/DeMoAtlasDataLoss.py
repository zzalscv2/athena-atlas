#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble) - 2023
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

import os,sys  
from math import fabs
from re import match
from time import strftime,localtime

from ROOT import TFile
from ROOT import TH1F,TProfile
from ROOT import TCanvas
from ROOT import kTeal
from ROOT import gStyle,gROOT,gPad
from ROOT import kYellow,kOrange,kRed,kBlue,kPink,kMagenta,kGreen,kSpring,kViolet,kAzure,kCyan,kTeal,kBlack,kWhite
gROOT.SetBatch(False)

sys.path.append("/afs/cern.ch/user/l/larmon/public/prod/Misc")
from LArMonCoolLib import GetLBTimeStamps,GetOfflineLumiFromCOOL,GetReadyFlag
from DeMoLib import retrieveYearTagProperties,returnPeriod,plotStack,MakeTH1,MakeLegend,strLumi

from DQUtils import fetch_iovs

global debug
debug = False
#debug = True

########################################################################
########################################################################
# Main script
import os,sys  

from argparse import RawTextHelpFormatter,ArgumentParser

from DQDefects import DefectsDB

parser = ArgumentParser(description='',formatter_class=RawTextHelpFormatter)
parser.add_argument('-y','--year',dest='parser_year',default = "2022",help='Year [Default: 2022]',action='store')
parser.add_argument('-t','--tag',dest='parser_tag',default = "AtlasReady",help='Defect tag [Default: "AtlasReady"]',action='store')
parser.add_argument('-b','--batch',dest='parser_batchMode',help='Batch mode',action='store_true')

args = parser.parse_args()
year = args.parser_year
tag = args.parser_tag

parser.print_help()

if args.parser_batchMode:
  gROOT.SetBatch(True)

affectedLBs = {}

systemList = ["LAr","Pixel","SCT","TRT","Tile","MDT","TGC","RPC","Trig_L1","Trig_HLT","Lumi","Global","ALFA","LUCID","ZDC","IDGlobal","BTag","CaloCP","MuonCP"]
systemList2 = systemList + ["All"]
    
for iSyst in systemList2:
  affectedLBs[iSyst] = {}

DeMoConfig = {}
for iSyst in systemList:
  recapDefectsFile = open("YearStats-%s/%s/%s/recapDefects.txt"%(iSyst,year,tag))

  runNumber = ""
  for iline in recapDefectsFile:
    ilineSplit = iline.split("|")
    lbRange = ""
    if (len(ilineSplit) == 8) and ("Run" not in ilineSplit[0]): # Found a new run
      runNumber = ilineSplit[0]
      lbRange = ilineSplit[5]
    if (len(ilineSplit) == 4): # Found a new run
      lbRange = ilineSplit[1]

    if (lbRange != ""):

      if (runNumber not in affectedLBs[iSyst].keys()):
        affectedLBs[iSyst][runNumber] = []

      if (runNumber not in affectedLBs["All"].keys()):
        affectedLBs["All"][runNumber] = []

      lbRangeSplit = lbRange.split("->")
      if len(lbRangeSplit) == 1: # Single LB
        lbAffInt = int(lbRange)
        if (lbAffInt not in affectedLBs[iSyst][runNumber]):
          affectedLBs[iSyst][runNumber].append(lbAffInt)
        if (lbAffInt not in affectedLBs["All"][runNumber]):
          affectedLBs["All"][runNumber].append(lbAffInt)          

      if len(lbRangeSplit) == 2: # Single LB
        for iLB in range(int(lbRangeSplit[0]),int(lbRangeSplit[1])+1):
          if (iLB not in affectedLBs[iSyst][runNumber]):
            affectedLBs[iSyst][runNumber].append(iLB)
          if (iLB not in affectedLBs["All"][runNumber]):
            affectedLBs["All"][runNumber].append(iLB)          
          
    if ("Tolerable defect" in iline): # Ignore the tolerable defects
      break

DeMoConfig = retrieveYearTagProperties(year,tag)

# Retrieve the luminosity for each affected LB from the COOL dbs
lbLumi = {} 
for iRun in affectedLBs["All"].keys():
  instOfflLumi = GetOfflineLumiFromCOOL(int(iRun),0,DeMoConfig["OflLumi tag"])
  atlasready=GetReadyFlag(int(iRun))

  lbLumi[iRun] = {}
  v_lbTimeSt = GetLBTimeStamps(int(iRun))

  lumiacct=fetch_iovs('COOLOFL_TRIGGER::/TRIGGER/OFLLUMI/LumiAccounting', tag=DeMoConfig["OflLumiAcct tag"], since=v_lbTimeSt[1][0]*1000000000, until=v_lbTimeSt[len(v_lbTimeSt)][1]*1000000000) 

  for iLumiAcct in range(len(lumiacct)):
    if (lumiacct[iLumiAcct].LumiBlock in affectedLBs["All"][iRun]):
      if (atlasready[lumiacct[iLumiAcct].LumiBlock]):
        lbLumi[iRun][lumiacct[iLumiAcct].LumiBlock] = lumiacct[iLumiAcct].LBTime*lumiacct[iLumiAcct].LiveFraction*lumiacct[iLumiAcct].InstLumiAll
        lbLumi[iRun][lumiacct[iLumiAcct].LumiBlock] = lumiacct[iLumiAcct].LBTime*lumiacct[iLumiAcct].LiveFraction*instOfflLumi[lumiacct[iLumiAcct].LumiBlock]
      else: # Special treatment needed when there are several IOVs with ATLAS ready
        affectedLBs["All"][iRun].remove(lumiacct[iLumiAcct].LumiBlock)
        for iSyst in systemList:
          if (iRun in affectedLBs[iSyst].keys() and lumiacct[iLumiAcct].LumiBlock in affectedLBs[iSyst][iRun]):
              affectedLBs[iSyst][iRun].remove(lumiacct[iLumiAcct].LumiBlock)

# Retrieve the list of periods and the total luminosity from the TProfile.root file (Global system but should be the same for all)
periodFile = TFile("YearStats-Global/%s/%s/TProfiles.root"%(year,tag))

h1_lumi = periodFile.Get("h1Period_IntLuminosity_archive")
periodListHash = h1_lumi.GetXaxis().GetLabels()
periodName = []
periodLumi = {}

dataLossPerPeriod = {}
for iSyst in systemList2:
  dataLossPerPeriod[iSyst]={}

for i in range(len(periodListHash)):
  periodName.append(str(periodListHash[i]))
  periodLumi[periodName[i]] = h1_lumi.GetBinContent(i+1)
  for iSyst in systemList2:
    dataLossPerPeriod[iSyst][periodName[i]] = 0.

hprof_dataLossPerSystem = {}
for iSyst in systemList2:
  hprof_dataLossPerSystem[iSyst] = MakeTH1("dataLoss_%s"%iSyst,"Period","dataLoss_%s"%iSyst,-0.5,-0.5+len(periodName),len(periodName),kBlack)
  for i in range(len(periodName)):
    hprof_dataLossPerSystem[iSyst].GetXaxis().SetBinLabel(i+1,periodName[i])

# Extract data loss per period for each system
colorPalette = {"Pixel":kViolet-4,
                "SCT":kSpring-9,
                "TRT":kPink,
                "LAr":kRed+1,
                "Tile":kYellow+1,
                "MDT":kBlue+3,
                "TGC":kBlue-4,
                "RPC":kGreen-2,
                "Trig_L1":kOrange,
                "Trig_HLT":kAzure+6,
                "Lumi":kBlue-9,
                "Global":kCyan,
                "IDGlobal":kMagenta+2,
                "CaloCP":kGreen+3,
                "All":kBlack}

for iSyst in systemList2:
  for iAffRun in affectedLBs[iSyst].keys():
    period = returnPeriod(int(iAffRun),"Global",year,tag)
    for iAffLB in affectedLBs[iSyst][iAffRun]:
      dataLossPerPeriod[iSyst][period] = dataLossPerPeriod[iSyst][period] + lbLumi[iAffRun][iAffLB]
      dataLossPerPeriod[iSyst]["All"] = dataLossPerPeriod[iSyst]["All"] + lbLumi[iAffRun][iAffLB]
    
  print("==== %s"%iSyst)
  if (len(affectedLBs[iSyst])):
    for i in range(len(periodName)):
      if (periodLumi[periodName[i]] != 0.):
        percentLoss = dataLossPerPeriod[iSyst][periodName[i]]/periodLumi[periodName[i]]/10000
      else:
        percentLoss = 0.
      if percentLoss != 0.:
        print("Period %s -> %.4f percent"%(periodName[i],percentLoss))
        hprof_dataLossPerSystem[iSyst].Fill(i,percentLoss)
        hprof_dataLossPerSystem[iSyst].SetBinError(i+1,0.)
    hprof_dataLossPerSystem[iSyst].SetFillColor(colorPalette[iSyst])

canvas = {}
stacks = {}
legends = {}
systemDict = {}
for iSyst in systemList:
  systemDict[iSyst] = iSyst

plotStack("Defects--Periods--%s"%DeMoConfig["Description"],hprof_dataLossPerSystem,systemList,systemDict,h1_lumi,False,stacks,canvas,legends,False)
hprof_dataLossPerSystem["All"].SetMarkerColor(kOrange+7)
hprof_dataLossPerSystem["All"].SetMarkerStyle(20)
hprof_dataLossPerSystem["All"].Draw("PSAME")

ATLASLegend = MakeLegend(0.35,0.79,0.65,0.89)
ATLASLegend.AddEntry(hprof_dataLossPerSystem["All"], "#splitline{ATLAS inefficiency}{All periods: %.2f%% / %s (%s OK)}"%(dataLossPerPeriod["All"]["All"]/periodLumi["All"]/10000,strLumi(dataLossPerPeriod["All"]["All"]),strLumi(periodLumi["All"]-dataLossPerPeriod["All"]["All"]/1000000,"pb",True,True)), "P")
ATLASLegend.SetFillColor(kWhite)
ATLASLegend.Draw()

canvas["Defects--Periods--%s"%DeMoConfig["Description"]].SaveAs("YearStats-Global/%s/%s/DataLoss.png"%(year,tag))
