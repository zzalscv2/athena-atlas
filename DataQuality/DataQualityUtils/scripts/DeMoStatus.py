#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble) - 2017 - 2022
# Python 3 migration by Miaoran Lu (University of Iowa)- 2022
#
# Display the year stats
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

import os,sys  

import time

from ROOT import TFile
from ROOT import TProfile,TH1F
from ROOT import TCanvas
from ROOT import gStyle,gPad,gROOT
from ROOT import kBlack,kOrange,kGreen
gROOT.SetBatch(False)

sys.path.append("/afs/cern.ch/user/l/larmon/public/prod/Misc")

from DeMoLib import strLumi, plotStack, initialize, MakeTH1,SetXLabel,MakeTProfile

global debug
debug = False
#debug = True

########################################################################
def mergeSubPeriod(tprof_subperiod,tprof_letter):
  index_letter = {}
  nb_letter = tprof_letter.GetNbinsX()
  nb_subperiod = tprof_subperiod.GetNbinsX()
  for iBin in range(1,nb_letter): # Loop on all subperiods of letter TProfile
    index_letter[tprof_letter.GetXaxis().GetBinLabel(iBin)] = iBin
  
  for iBin in range(1,nb_subperiod): # Loop on all subperiods
    letter = tprof_subperiod.GetXaxis().GetBinLabel(iBin)[0]
    if (tprof_letter.IsA().InheritsFrom("TProfile")):
      tprof_letter.Fill(index_letter[letter]-1,tprof_subperiod.GetBinContent(iBin),tprof_subperiod.GetBinEntries(iBin))
      tprof_letter.Fill(nb_letter-1,tprof_subperiod.GetBinContent(iBin),tprof_subperiod.GetBinEntries(iBin))
    elif (tprof_letter.IsA().InheritsFrom("TH1")):
      tprof_letter.Fill(index_letter[letter]-1,tprof_subperiod.GetBinContent(iBin))
      tprof_letter.Fill(nb_letter-1,tprof_subperiod.GetBinContent(iBin))
  
  return

########################################################################
# ATLASLabel copied from atlastyle package, as import does not work for unknown reasons
def ATLASLabel(x,y,text=""):
  from ROOT import TLatex
  from ROOT import gPad
  l = TLatex()
  l.SetNDC();   
  l.SetTextFont(72);
  l.DrawLatex(x,y,"ATLAS")
  delx = 0.115*696*gPad.GetWh()/(472*gPad.GetWw())
  if (text != ""):
    p = TLatex()
    p.SetNDC()
    p.SetTextFont(42)
    p.DrawLatex(x+delx,y,text);

  return


########################################################################
########################################################################
# Main script
from argparse import RawTextHelpFormatter,ArgumentParser
from ROOT import gROOT

parser = ArgumentParser(description='',formatter_class=RawTextHelpFormatter)
parser.add_argument('-y','--year',dest='parser_year',default = ["2018"],nargs='*',help='Year [Default: 2018]',action='store')
parser.add_argument('-t','--tag',dest='parser_tag',default = ["Tier0_2018"],nargs='*',help='Defect tag [Default: "Tier0_2018"]',action='store')
parser.add_argument('-s','--system',dest='parser_system',default="LAr",help='System: LAr, CaloCP [Default: "LAr"]',action='store')
parser.add_argument('-d','--directory',dest='parser_directory',default="./",help='Directory to display',action='store')
parser.add_argument('-b','--batch',dest='parser_batchMode',help='Batch mode',action='store_true')
parser.add_argument('--yearStats',dest='parser_plotYS',help='Plot the year stats per period',action='store_true')
parser.add_argument('--yearStatsLarge',dest='parser_letterYS',help='Plot the year stats as a function of the letter (large) periods',action='store_true')
parser.add_argument('--lumiNotPercent',dest='parser_lumiNotPercent',help='Display results in term of lumi and not percent',action='store_true')
parser.add_argument('--noRecovPlot',dest='parser_noRecovPlot',help='Do not plot the recoverable histograms',action='store_false')
parser.add_argument('--savePlots',dest='parser_savePlots',help='Save yearStats results in ~larmon/public/prod/LADIeS/DeMoPlots',action='store_true')
parser.add_argument('--approvedPlots',dest='parser_approvedPlots',help='Cosmetics to get the approved plots',action='store_true')
parser.add_argument('--noSingleTagPlot',dest='parser_noSingleTagPlot',help='When > 1 tag, display only comparison plots',action='store_true')

args = parser.parse_args()

parser.print_help()

if args.parser_batchMode:
  gROOT.SetBatch(True)

yearTagProperties = {}
partitions = {}
defects = {}
defectVeto = {}
veto = {}
signOff = {}
initialize(args.parser_system,yearTagProperties,partitions,defects,defectVeto,veto,signOff,args.parser_year[0],args.parser_tag[0])

yearTagList = []
yearTagDir = {}
for iYear in args.parser_year:
  for iTag in args.parser_tag:
    directory = "%s/YearStats-%s/%s/%s"%(args.parser_directory,args.parser_system,iYear,iTag)
    if os.path.exists(directory):
      yearTag = "%s%s"%(iYear,yearTagProperties["description"][iTag])
      yearTagList.append(yearTag)
      yearTagDir[yearTag] = directory

yearTagList.sort()

if len(yearTagList) == 0:
  print("No year / tag matching - Please check YearStats directory")
  sys.exit()

options = {}
options['plotYearStats'] = args.parser_plotYS
options['plotYearStatsLarge'] = args.parser_letterYS # Plot results as a function of letter period (A,B,C...) and not only as a function of subperiod (A1,A2,B1,B2,B3...)
options['lumiNotPercent'] = args.parser_lumiNotPercent
options['recovPlot'] = args.parser_noRecovPlot
options['savePlots'] = args.parser_savePlots  
options['approvedPlots'] = args.parser_approvedPlots
options['noSingleTagPlot'] = args.parser_noSingleTagPlot

if options['savePlots']:
  options['plotYearStats'] = True
  options['plotYearStatsLarge'] = False
  options['lumiNotPercent'] = False

if options['approvedPlots']:
  options['plotYearStats'] = False
  options['plotYearStatsLarge'] = True
  options['lumiNotPercent'] = False  
  options['recovPlot'] = False

if not (options['plotYearStats'] or options['plotYearStatsLarge'] or options['savePlots']):
  options['plotYearStats'] = True

yearTagNb = len(yearTagList)
yearTagLabels = yearTagNb*[""]
for iYT in range(yearTagNb):
  if options['approvedPlots']: yearTagLabels[iYT] = yearTagList[iYT].split("/")[0]
  else: yearTagLabels[iYT] = yearTagList[iYT]

gStyle.SetOptStat(0)

canvasResults = {}
legendResults = {}
stackResults = {}
file = {}

hProfPeriod_IntolDefect = {}
hProfPeriod_Veto = {}
hProfPeriodLett_IntolDefect = {}
hProfPeriodLett_Veto = {}
h1Period_IntLuminosity = {}
h1PeriodLett_IntLuminosity = {}
subperiodNb = {}
runsCharact = {}

for iYT in yearTagList:
  print(("I am treating the following year/tag:%s"%iYT))

  canvasResults[iYT] = {}
  legendResults[iYT] = {}
  stackResults[iYT] = {}

  yearStatsArchiveFilename = '%s/TProfiles.root'%(yearTagDir[iYT])
  file[iYT] = TFile(yearStatsArchiveFilename)
  h1Period_IntLuminosity[iYT] = file[iYT].Get("h1Period_IntLuminosity_archive")
  subperiodNb[iYT] = h1Period_IntLuminosity[iYT].GetNbinsX()

  # Retrieve the nb of runs updated by reading dat files
  runsCharact[iYT] = {'Low':1e10,'High':0,'Number':0}
  
  fTemp = open("%s/runs-ALL.dat"%yearTagDir[iYT])
  for iRun in fTemp.readlines():
    if int(iRun)<runsCharact[iYT]['Low']:runsCharact[iYT]['Low']=int(iRun)
    if int(iRun)>runsCharact[iYT]['High']:runsCharact[iYT]['High']=int(iRun)
    runsCharact[iYT]['Number'] += 1

  runsCharact[iYT]['Range']="%d->%d"%(runsCharact[iYT]['Low'],runsCharact[iYT]['High'])
  print(("I found %d runs in this year/tag (%s)"%(runsCharact[iYT]['Number'],runsCharact[iYT]['Range'])))

  if (options['plotYearStats'] or options['plotYearStatsLarge']):
    if options['approvedPlots']:
      xAxisTitle = "Period"
      legendHead = "%s - %s"%(args.parser_system,iYT.split("/")[0])
    else:
      xAxisTitle = "Period (%s)"%runsCharact[iYT]['Range']
      legendHead = "%s - %s"%(args.parser_system,iYT)

    # Retrieve the defect, veto histograms from the YearStats root file
    hProfPeriod_IntolDefect[iYT] = {}
    allIntolDef = ["allIntol","allIntol_recov"] # Summed inefficiencies without double counting
    for iDef in defects["intol"]+defects["intol_recov"]+allIntolDef:
      iDefName = iDef.rsplit("__",1)[0] # Trick needed to get the defect name for the recoverable defect histogram
      hProfPeriod_IntolDefect[iYT][iDef] = file[iYT].Get("hProfPeriod_IntolDefect_%s_archive"%(iDef))
      hProfPeriod_IntolDefect[iYT][iDef].SetFillColor(defectVeto["color"][iDefName])

    hProfPeriod_Veto[iYT] = {}
    for iVeto in veto["all"]:
      hProfPeriod_Veto[iYT][iVeto] = file[iYT].Get("hProfPeriod_Veto_%s_archive"%(iVeto))
      hProfPeriod_Veto[iYT][iVeto].SetFillColor(defectVeto["color"][iVeto])
    if len(veto["all"]): # Summed inefficiencies
      hProfPeriod_Veto[iYT]["allVeto"] = file[iYT].Get("hProfPeriod_Veto_allVeto_archive")
      hProfPeriod_Veto[iYT]["allVeto"].SetFillColor(kBlack)
      

    # Plot the stack for subperiod
    if (options['plotYearStats'] and not options['noSingleTagPlot']):
      totalIneffDef = plotStack("defects--%s--%s - %s"%(xAxisTitle,args.parser_system,iYT),hProfPeriod_IntolDefect[iYT],defects["intol"],defectVeto["description"],h1Period_IntLuminosity[iYT],options['lumiNotPercent'],stackResults[iYT],canvasResults[iYT],legendResults[iYT],options['recovPlot'],False,options['approvedPlots'])
      if options['approvedPlots']:
        ATLASLabel(0.4,0.8,"Internal")
#      ATLASLabel(0.1,0.81,"Preliminary")
      if len(veto["all"]): 
        totalIneffVeto = plotStack("veto--%s--%s"%(xAxisTitle,iYT),hProfPeriod_Veto[iYT],veto["all"],defectVeto["description"],h1Period_IntLuminosity[iYT],options['lumiNotPercent'],stackResults[iYT],canvasResults[iYT],legendResults[iYT],False,False,options['approvedPlots'])
        if options['approvedPlots']:
          ATLASLabel(0.1,0.81,"Internal")
#      ATLASLabel(0.1,0.81,"Preliminary")

    # Plot the stack for letter period  
    if (options['plotYearStatsLarge']):
      # Scan all subperiods and extract letter periods - 
      letperiodList = []
      for iBin in range(1,subperiodNb[iYT]):
        subperiod = h1Period_IntLuminosity[iYT].GetXaxis().GetBinLabel(iBin)
        if (subperiod[0] not in letperiodList):
          letperiodList.append(subperiod[0])
      letperiodNb = len(letperiodList)

      # Merge the subperiod histograms into letter period histograms
      hProfPeriodLett_IntolDefect[iYT] = {}
      hProfPeriodLett_Veto[iYT] = {}
      for iDef in defects["intol"]+defects["intol_recov"]+allIntolDef: # Loop on defect (including recoverable histos and summed inefficiencies)
        iDefName = iDef.rsplit("__",1)[0] # Trick needed to get the defect name for the recoverable defect histogram
        hProfPeriodLett_IntolDefect[iYT][iDef] = MakeTProfile("hProfPeriodLett_IntolDefect_%s_%s"%(iYT,iDef),"%s"%(defectVeto["description"][iDefName]),"Lost luminosity [%]", -0.5,+0.5+letperiodNb,letperiodNb+1,defectVeto["color"][iDefName])
        SetXLabel(hProfPeriodLett_IntolDefect[iYT][iDef],letperiodList)
        hProfPeriodLett_IntolDefect[iYT][iDef].GetXaxis().SetBinLabel(letperiodNb+1,"All") # In all bins, all runs
        mergeSubPeriod(hProfPeriod_IntolDefect[iYT][iDef],hProfPeriodLett_IntolDefect[iYT][iDef])

      for iVeto in veto["all"]: # Loop on veto
        hProfPeriodLett_Veto[iYT][iVeto] = MakeTProfile("hProfPeriodLett_Veto_%s_%s"%(iYT,iVeto),"%s"%(defectVeto["description"][iVeto]),"Lost luminosity [%]", -0.5,+0.5+letperiodNb,letperiodNb+1,defectVeto["color"][iVeto])
        SetXLabel(hProfPeriodLett_Veto[iYT][iVeto],letperiodList)
        hProfPeriodLett_Veto[iYT][iVeto].GetXaxis().SetBinLabel(letperiodNb+1,"All") # In all bins, all runs
        mergeSubPeriod(hProfPeriod_Veto[iYT][iVeto],hProfPeriodLett_Veto[iYT][iVeto])
      if len(veto["all"]): # Summed inefficiencies
        hProfPeriodLett_Veto[iYT]["allVeto"] = MakeTProfile("hProfPeriodLett_Veto_%s_allVeto"%iYT,"allVeto","Lost luminosity [%]", -0.5,+0.5+letperiodNb,letperiodNb+1,kBlack)
        SetXLabel(hProfPeriodLett_Veto[iYT]["allVeto"],letperiodList)
        hProfPeriodLett_Veto[iYT]["allVeto"].GetXaxis().SetBinLabel(letperiodNb+1,"All") # In all bins, all runs
        mergeSubPeriod(hProfPeriod_Veto[iYT]["allVeto"],hProfPeriodLett_Veto[iYT]["allVeto"])

      # Merge the subperiod luminosity histograms
      h1PeriodLett_IntLuminosity[iYT] = MakeTH1("hProfPeriodLett_IntLuminosity_%s"%(iYT),"Period","Luminosity[pb^{-1}]", -0.5,+0.5+letperiodNb,letperiodNb+1,kBlack)
      SetXLabel(h1PeriodLett_IntLuminosity[iYT],letperiodList)
      h1PeriodLett_IntLuminosity[iYT].GetXaxis().SetBinLabel(letperiodNb+1,"All") # In all bins, all runs
      h1PeriodLett_IntLuminosity[iYT].SetTitle("")
      mergeSubPeriod(h1Period_IntLuminosity[iYT],h1PeriodLett_IntLuminosity[iYT])

      if (not options['noSingleTagPlot']):
        # Plot the stack for letter periods
        totalIneffDef = plotStack("defects--%s--%s"%(xAxisTitle,legendHead),hProfPeriodLett_IntolDefect[iYT],defects["intol"],defectVeto["description"],h1PeriodLett_IntLuminosity[iYT],options['lumiNotPercent'],stackResults[iYT],canvasResults[iYT],legendResults[iYT],options['recovPlot'],False,options['approvedPlots'])
        if options['approvedPlots']:
          ATLASLabel(0.4,0.8,"Internal")
        if len(veto["all"]): 
          totalIneffVeto = plotStack("veto--%s--%s"%(xAxisTitle,legendHead),hProfPeriodLett_Veto[iYT],veto["all"],defectVeto["description"],h1PeriodLett_IntLuminosity[iYT],options['lumiNotPercent'],stackResults[iYT],canvasResults[iYT],legendResults[iYT],False,False,options['approvedPlots'])
          if options['approvedPlots']:
            ATLASLabel(0.4,0.85,"Internal")

    # And finally the integrated luminosities
    canvasResults[iYT]['intLumi']= TCanvas( "c_intLumi_%s"%(iYT),"Integrated luminosity per period", 200, 10, 1150, 500)
    # Same margin as for the stacked results
    canvasResults[iYT]['intLumi'].SetLeftMargin(0.08)
    canvasResults[iYT]['intLumi'].SetRightMargin(0.35)

    if (options['plotYearStatsLarge']):
      h1PeriodLett_IntLuminosity[iYT].Draw("P HIST")
      for iBin in range(1,h1PeriodLett_IntLuminosity[iYT].GetNbinsX()+1):
        print(("Period %s: %.3f pb-1"%(h1PeriodLett_IntLuminosity[iYT].GetXaxis().GetBinLabel(iBin),h1PeriodLett_IntLuminosity[iYT].GetBinContent(iBin))))
    else:
      h1Period_IntLuminosity[iYT].Draw("P HIST")
      for iBin in range(1,h1Period_IntLuminosity[iYT].GetNbinsX()+1):
        print(("Period %s: %.3f pb-1"%(h1Period_IntLuminosity[iYT].GetXaxis().GetBinLabel(iBin),h1Period_IntLuminosity[iYT].GetBinContent(iBin))))

    canvasResults[iYT]['intLumi'].SetGridy(1)

    # On request, save the plots 
    if options["savePlots"]:
      for iCanvas in list(canvasResults[iYT].keys()):
        canvasResults[iYT][iCanvas].Update()
        if ("defects" in iCanvas):
          canvasResults[iYT][iCanvas].Print("%s/grl-defects.png"%yearTagDir[iYT])
        if ("veto" in iCanvas):
          canvasResults[iYT][iCanvas].Print("%s/grl-veto.png"%yearTagDir[iYT])
        if ("intLumi" in iCanvas):
          canvasResults[iYT][iCanvas].Print("%s/grl-lumi.png"%yearTagDir[iYT])

# Produce a unique comparison plot between the different year tags (if more than 2)
if (yearTagNb >= 2 and (options['plotYearStats'] or options['plotYearStatsLarge'])):
  h1YearTag_IntLuminosity = MakeTH1("h1YearTag_IntLuminosity","Year/Tag","Luminosity[pb^{-1}]", -0.5,-0.5+yearTagNb,yearTagNb,kBlack)
  SetXLabel(h1YearTag_IntLuminosity,yearTagLabels)
  for iYT in range(yearTagNb):
    h1YearTag_IntLuminosity.Fill(iYT,h1Period_IntLuminosity[yearTagList[iYT]].GetBinContent(subperiodNb[yearTagList[iYT]]))

  h1YearTag_IntolDefect = {}
  for iDef in defects["intol"]+defects["intol_recov"]+allIntolDef:
    iDefName = iDef.rsplit("__",1)[0] # Trick needed to get the defect name for the recoverable defect histogram
    h1YearTag_IntolDefect[iDef] = MakeTH1("h1YearTag_IntolDefect_%s"%(iDef),"%s"%(defectVeto["description"][iDefName]),"Lost luminosity [%]", -0.5,-0.5+yearTagNb,yearTagNb,defectVeto["color"][iDefName])
    SetXLabel(h1YearTag_IntolDefect[iDef],yearTagLabels)
    for iYT in range(yearTagNb):
      h1YearTag_IntolDefect[iDef].Fill(iYT,hProfPeriod_IntolDefect[yearTagList[iYT]][iDef].GetBinContent(subperiodNb[yearTagList[iYT]]))
      
#  legendHeader = ""
  legendHeader = "#splitline{                   %s  "%(yearTagLabels[0].split("/")[0])
  for iTag in range(1,yearTagNb):
    legendHeader += "   /  %s "%(yearTagLabels[iTag].split("/")[0])
  legendHeader += "}{Luminos. : %3.1f fb^{-1} "%(h1Period_IntLuminosity[yearTagList[0]].GetBinContent(subperiodNb[yearTagList[0]])/1000.)
  for iTag in range(1,yearTagNb):
    legendHeader += "/ %3.1f fb^{-1}"%(h1Period_IntLuminosity[yearTagList[iTag]].GetBinContent(subperiodNb[yearTagList[iTag]])/1000.)
  legendHeader += "}"

  plotStack("defects--Year--%s"%legendHeader,h1YearTag_IntolDefect,defects["intol"],defectVeto["description"],h1YearTag_IntLuminosity,options['lumiNotPercent'],stackResults,canvasResults,legendResults,False,True,options['approvedPlots'])
  if options['approvedPlots']:
    ATLASLabel(0.1,0.85,"Internal")
  if options["savePlots"]:
    canvasResults["defects--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-defects-DQPaper_.png"%(args.parser_directory,args.parser_system))
    canvasResults["defects--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-defects-DQPaper_.pdf"%(args.parser_directory,args.parser_system))
    canvasResults["defects--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-defects-DQPaper_.eps"%(args.parser_directory,args.parser_system))
  
  h1YearTag_Veto = {}
  if (len(veto["all"])):
    for iVeto in (veto["all"]+["allVeto"]):
      h1YearTag_Veto[iVeto] = MakeTH1("h1YearTag_Veto_%s"%(iVeto),"%s"%(defectVeto["description"][iVeto]),"Lost luminosity [%]", -0.5,-0.5+yearTagNb,yearTagNb,defectVeto["color"][iVeto])
      SetXLabel(h1YearTag_Veto[iVeto],yearTagLabels)
      for iYT in range(yearTagNb):
        h1YearTag_Veto[iVeto].Fill(iYT,hProfPeriod_Veto[yearTagList[iYT]][iVeto].GetBinContent(subperiodNb[yearTagList[iYT]]))

  if len(veto["all"]): 
    plotStack("veto--Year--%s"%legendHeader,h1YearTag_Veto,veto["all"],defectVeto["description"],h1YearTag_IntLuminosity,options['lumiNotPercent'],stackResults,canvasResults,legendResults,False,True,options['approvedPlots'])
    if options['approvedPlots']:
      ATLASLabel(0.1,0.81,"Internal")
    if options["savePlots"]:
      canvasResults["veto--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-veto-DQPaper_.png"%(args.parser_directory,args.parser_system))
      canvasResults["veto--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-veto-DQPaper_.pdf"%(args.parser_directory,args.parser_system))
      canvasResults["veto--Year--%s"%legendHeader].Print("%s/YearStats-%s/run2-veto-DQPaper_.eps"%(args.parser_directory,args.parser_system))

if not args.parser_batchMode:
  input("I am done. Type <return> to exit...")
