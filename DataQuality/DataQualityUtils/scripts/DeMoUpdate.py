#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (LPSC - Grenoble) - 2017 - 2022
# Python 3 migration by Miaoran Lu         
# Udpates the year stats
##################################################################

import os, sys, socket, pathlib, errno

import time

from ROOT import TFile
from ROOT import TH1F,TProfile
from ROOT import TCanvas,TPaveText
from ROOT import kBlack,kOrange,kGreen
from ROOT import gStyle
from ROOT import gROOT
from ROOT import TLatex
gROOT.SetBatch(False)

import xmlrpc.client

sys.path.append("/afs/cern.ch/user/l/larmon/public/prod/Misc")
from LArMonCoolLib import GetLBTimeStamps,GetOnlineLumiFromCOOL,GetOfflineLumiFromCOOL,GetLBDuration,GetReadyFlag,GetNumberOfCollidingBunches

from DeMoLib import strLumi,plotStack,initialize,MakeTH1,MakeTProfile,SetXLabel 
import DQDefects

sys.path.append("/afs/cern.ch/user/l/larcalib/LArDBTools/python/")
import showEventVeto,showEventVetoNoLumi

from DQUtils import fetch_iovs

global startrun
global endrun

global debug
debug = False
#debug = True

passfile = open("/afs/cern.ch/user/l/larmon/public/atlasdqmpass.txt")
passwd = passfile.read().strip(); passfile.close()
dqmapi = xmlrpc.client.ServerProxy('https://%s@atlasdqm.cern.ch'%(passwd))

scriptdir = str(pathlib.Path(__file__).parent.resolve())
runListDir = scriptdir+"/RunList"


################################################################################################################################################
#### Ancillary functions
def chmkDir( path ):
  # Safely create a directory
  try:
      os.makedirs(path)
  except OSError as exception:
      if exception.errno != errno.EEXIST:
          raise

def printProp(varname):
  print("**",varname,"**")
  if hasattr(sys.modules[__name__],varname):
    print(getattr(sys.modules[__name__],varname))


################################################################################################################################################
def listify(l):
  if len(l)==0: return ''
  elif len(l)==1: return str(l[0]) 
  l.sort()
  interval=False
  a = ''
  for i in range(len(l)-1):
    if interval: 
      if l[i+1]-l[i]==1: pass
      else: a += str(l[i])+' '; interval=False
    else:
      a += str(l[i])
      if l[i+1]-l[i]==1: a += '-'; interval=True
      else: a += ' '
  a += str(l[-1])
  return a

################################################################################################################################################
def findLB(lbts,startOfVetoPeriod):
  # Find the lumiblock where a veto period starts
  for i in list(lbts.keys()):
    if (startOfVetoPeriod>lbts[i][0] and startOfVetoPeriod<lbts[i][1]):
      return i
  return i

################################################################################################################################################
#### Functions dedicated to output printing
def printBoth(string0,boolean,f):
  # Print and, if the boolean is true, also write to the provided file
  print(string0)
  if boolean:# Also write on txt file
      f.write(string0+'\n')
  return

def singleRunReport(runNumber,dict1,dict2,directory,defects,veto,exactVetoComput):
  # Print single run report. Only printing, no computation  
  import string

  runDir = directory+"/Run"
  chmkDir(runDir)

  if dict1['signoff'] == "DONE" or dict1['signoff'] == "FINAL OK":
    repOnDisk = True
    f = open(runDir+'/%s.txt' % (runNumber), 'w')
  else:
    repOnDisk = False
    f = open(runDir+'/dummy.txt', 'w')
    
  printBoth('Run start : %s'%dict1['Start'],repOnDisk,f)
  printBoth('Run stop  : %s'%dict1['Stop'],repOnDisk,f)
  printBoth('LB with ATLAS ready                      = %9s'%(listify(dict1["readyLB"])),repOnDisk,f)
  printBoth('LB with ATLAS ready and no global defect = %9s'%(listify(dict1["readyLB_globalFilter"])),repOnDisk,f)
  printBoth('Nb of bunches                            = %d'%(dict1['nBunches']),repOnDisk,f)
  printBoth('Peak lumi                                = %.1e'%(dict1['peakLumi']*1e30),repOnDisk,f)
  printBoth('Integrated luminosity used for normalis. = %s'%(strLumi(dict1['Lumi'],"ub",False,True)),repOnDisk,f)
  printBoth('%s GRL inefficiency                      : %.2f%% / %s'%(options["system"],dict1["ineffDefect_allIntol"],strLumi(dict1['Lumi']*dict1["ineffDefect_allIntol"]/100.,"ub",False,True)),repOnDisk,f)
  for idef in (defects["globIntol"]):
    if (len(dict2[idef])>0):
      printBoth('Nb of LBs with %24s: %i -> %.2f%% (%s)'%(defectVeto["description"][idef].ljust(24),len(dict2[idef]),dict1["ineffDefect_%s"%(idef)],str(dict2[idef])),repOnDisk,f)
  for idef in (defects["partIntol"]):
    if (len(dict2[idef]["AllPartitions"])>0):
      printBoth('Nb of LBs with %24s: %i -> %.2f%% (%s)'%(defectVeto["description"][idef].ljust(24),len(dict2[idef]["AllPartitions"]),dict1["ineffDefect_%s"%(idef)],str(dict2[idef]["AllPartitions"])),repOnDisk,f)
      #print "-----> LBs affected : ",dict2[idef]["AllPartitions"]
  if len(veto["all"])>0:
    if exactVetoComput:
      printBoth('LAr veto inefficiency                  : %.3f%%'%(dict1["ineffVeto_allVeto"]),repOnDisk,f)
    else:
      printBoth('LAr veto inefficiency (rough computat.): %.3f%%'%(dict1["ineffVeto_allVeto"]),repOnDisk,f)
    for iVeto in veto["all"]:
      printBoth('%s veto inefficiency  : %.3f%%'%(defectVeto["description"][iVeto],dict1["ineffVeto_%s"%(iVeto)]),repOnDisk,f)
  f.close()

  return

################################################################################################################################################
#### Functions dedicated to defect retrieval and basic string manipulations
def extractNamePartition(foundDefect):
    # Parse the defect name, return name, partition where available
    defectName = ""
    defectPart = ""
    if (foundDefect.startswith("PIXEL")): # [PIXEL]_[PARTITION]_[NAME]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectPart=defectSplitted[1]
        defectName=defectSplitted[2]      
    elif (foundDefect.startswith("SCT")): # SCT_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]      
    elif (foundDefect.startswith("TRT")): # TRT_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]      
    elif (foundDefect.startswith("LAR")): # [LAR]_[PART]_[Name] or [LAR]_[Name] - No "_" in any [NAME]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) == 2: # LAR_[NAME]
        defectName=defectSplitted[1]
      elif len(defectSplitted) == 3: # LAR_[PART]_[NAME]
        defectPart=defectSplitted[1]
        defectName=defectSplitted[2]
    elif (foundDefect.startswith("CALO_ONLINEDB")):# CALO_ONLINEDB_[NAME] (only for CALO_ONLINEDB_LOWMU) - Accounted to LAr
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectName=defectSplitted[2]
    elif (foundDefect.startswith("TILE")): # [TILE]_[PART]_[Name]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectPart=defectSplitted[1]
        defectName=defectSplitted[2]
    elif (foundDefect.startswith("MS")): # MS_[SUBDETECTOR]_[PARTITION]_[NAME]
      defectSplitted = foundDefect.split("_",3)
      if len(defectSplitted) > 3:
        systemAffected=defectSplitted[1]
        defectPart=defectSplitted[2]
        defectName=defectSplitted[3]
    elif (foundDefect.startswith("MCP")): # MCP_[NAME]
      defectSplitted = foundDefect.split("_",1) 
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]
    elif (foundDefect.startswith("ID")):# ID_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]
    elif (foundDefect.startswith("JET")):# JET_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]
    elif (foundDefect.startswith("MET")):# MET_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]
    elif (foundDefect.startswith("EGAMMA")):# EGAMMA_[NAME]_[PART] or EGAMMA_[NAME]
      if ("BARREL" in foundDefect or "ENDCAP" in foundDefect or "FORWARD" in foundDefect): #EGAMMA_[NAME]_[PART]
        defectSplitted = foundDefect.split("_",2)
        if len(defectSplitted) > 2:
          defectName=defectSplitted[1] 
          defectPart=defectSplitted[2]
      else:
        defectSplitted = foundDefect.split("_",1)
        if len(defectSplitted) > 1:
          defectName=defectSplitted[1] 
    elif (foundDefect.startswith("TAU")): # TAU_[PART]_[NAME]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectPart=defectSplitted[1]
        defectName=defectSplitted[2]
    elif (foundDefect.startswith("CALO")): # CALO_[PART]_[NAME]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectPart=defectSplitted[1]
        defectName=defectSplitted[2]
    elif (foundDefect.startswith("BTAG")):# BTAG_[NAME]
      defectSplitted = foundDefect.split("_",1)
      if len(defectSplitted) > 1:
        defectName=defectSplitted[1]
    elif (foundDefect.startswith("TRIG")): # TRIG_L1_[NAME] TRIG_HLT_[NAME]
      defectSplitted = foundDefect.split("_",2)
      if len(defectSplitted) > 2:
        defectName=defectSplitted[2]

    return defectName, defectPart
    
def retrieveDefectsFromDB(run, defectTag, grlDef):
    # Get the list of defects for this run, parse them
    defectDatabase = DQDefects.DefectsDB(tag=defectTag)
    system_defects = []
    for iPrefix in grlDef["prefix"]:
        system_defects += [d for d in (defectDatabase.defect_names | defectDatabase.virtual_defect_names) if (d.startswith(iPrefix))]
    retrievedDefects = defectDatabase.retrieve((run, 1), (run+1, 0), system_defects)
    parsed_defects = {}
    for rd in retrievedDefects:
        defectName, defectPart = extractNamePartition(rd.channel)
        if not( defectName is None and defectName is None):
            parsed_defects[rd] = [defectName, defectPart]
    return parsed_defects

################################################################################################################################################
#### Retrieval of run characteristics or new runs
def updateRunList(year=time.localtime().tm_year, runListDir=runListDir):
    # Update list of runs. If there is not an up-to-date file, get the latest info from the atlas dqm APIs'''
    print("Checking run list for year",year)

    chmkDir(runListDir)
    latestRun=dqmapi.get_latest_run()
    recentRuns = dqmapi.get_run_beamluminfo({'low_run':str(latestRun-200),'high_run':str(latestRun)})
    def writeRuns(outfile, fileRuns = []):
        for r in sorted(recentRuns.keys(), key=int):
            if (recentRuns[r][2]): # ATLAS ready
                if r not in fileRuns:
                    fileRuns.append(r)
                    print("Adding the ATLAS ready run: %s"%r)
                    outfile.write("%s\n"%r)
    allRunListDat = runListDir+"/all-"+str(year)+".dat"
    if os.path.isfile(allRunListDat):
        fRunList = open(allRunListDat,'r+')
        fileRuns = [l.strip() for l in fRunList.readlines() ]
        if len(fileRuns)>0:
          latestFileRun = max([int(r) for r in fileRuns])
        else: latestFileRun = 0
        if latestFileRun < latestRun:
          print("Run file",allRunListDat,"is not up-to-date")
          print("Latest run is ",latestRun, " while latest run in file is",latestFileRun)
          print("Will check ATLAS ready filters & update if needed")
          writeRuns(fRunList, fileRuns)
        fRunList.close()
        sys.exit()
    else:
      print("The run list for year ",year," is not available... please create one by hand")
      sys.exit()

def getRunInfo(runlist, defectTag="HEAD"):
    # Retrieve run characteristics (period,  time...)
    possDefects = sum(list(signOff.values()),[])
    run_defects = dqmapi.get_defects_lb({'run_list':runlist},possDefects,defectTag,True,True)

    infokeys = ["Run type", "Project tag", "Partition name", "Number of events passing Event Filter", "Run start", "Run end", "Number of luminosity blocks", "Data source", "Detector mask", "Recording enabled", "Number of events in physics streams" ]

    defectvals = [ "startlb", "endlb", "status", "time"]

    beamkeys = ["Max beam energy during run", "Stable beam flag enabled during run", "ATLAS ready flag enabled during run", "Total integrated luminosity", "ATLAS ready luminosity (/nb)"]

    for rd in run_defects.keys():
        for defect in run_defects[rd].keys():
            run_defects[rd][defect] = { ik:li for ik,li in zip(defectvals,run_defects[rd][defect][0]) }
            
    run_spec = {'run_list':runlist, 'stream': 'physics_CosmicCalo', 'source': 'tier0'}
    run_info = dqmapi.get_run_information(run_spec)
    for ri in run_info.keys():
        run_info[ri] = { ik:li for ik,li in zip(infokeys,run_info[ri]) }

    beam_info = dqmapi.get_run_beamluminfo(run_spec)
    for bi in beam_info.keys():
        beam_info[bi] = { ik:li for ik,li in zip(beamkeys,beam_info[bi]) }

    run_periods = dqmapi.get_data_periods({'run_list':runlist})
    allinfo = {}
    def addDetails(inputDict, col=None):
        for r in inputDict.keys():
            run = int(r)
            if run not in allinfo.keys():
                allinfo[run] = {}
            if col is not None:
                allinfo[run][col] = inputDict[r]
            else:
                allinfo[run].update(inputDict[r])
    addDetails(run_info)
    addDetails(beam_info)
    addDetails(run_periods, "periods")
    addDetails(run_defects, "defects")

    # only take first period??
    for r in allinfo.keys():
        try:
            allinfo[r]["period"] = allinfo[r]["periods"][0]
        except IndexError:
            print("Weird list of periods for",r,":",allinfo[r]["periods"])
            allinfo[r]["period"]="?"


    return allinfo


################################################################################################################################################
################################################################################################################################################
# Main script
from argparse import RawTextHelpFormatter,ArgumentParser

parser = ArgumentParser(description='',formatter_class=RawTextHelpFormatter)
parser.add_argument('-r','--run',type=int,dest='run',default=[],help='Run or run range',nargs='*',action='store')
parser.add_argument('-y','--year',dest='year',default = str(time.localtime().tm_year),help='Year [Default: '+str(time.localtime().tm_year)+']. May also include special conditions such as 5TeV, hi... Check the RunList files',action='store')
parser.add_argument('-t','--tag',dest='tag',default = "Tier0_"+str(time.localtime().tm_year),help='Defect tag [Default: "Tier0_'+str(time.localtime().tm_year)+'"]',action='store')
parser.add_argument('-b','--batch',dest='batchMode',help='Batch mode',action='store_true')
parser.add_argument('-s','--system',dest='system',default="LAr",help='System: LAr, CaloCP [Default: "LAr"]',action='store')
parser.add_argument('--runListUpdate',dest='runListUpdate',help='Run list update. No other action allowed. Exit when done',action='store_true')
parser.add_argument('--weekly',dest='weekly',help='Weekly report. No run range to specify',action='store_true')
parser.add_argument('--allRuns',dest='allRuns',help='All year runs. No run range to specify',action='store_true')
parser.add_argument('--grlUpdate',dest='grlUpdate',help='GRL update. No run range to specify',action='store_true')
parser.add_argument('--noPlot',dest='plotResults',help='Do not plot the results',action='store_false')
parser.add_argument('--resetYS',dest='resetYearStats',help='Reset year stats',action='store_true')
parser.add_argument('--noUpdateYS',dest='updateYearStats',help='Do not update year stats with new runs',action='store_false')
parser.add_argument('--skipUnsignedOff',dest='skipUnsignedOff',help='Ignore completely runs that are not signed off',action='store_true')
parser.add_argument('--skipAlreadyUpdated',dest='skipAlreadyUpdated',help='Ignore completely runs that are already in year stats',action='store_true')
parser.add_argument('--noVeto',dest='noVeto',help='Do not consider time-veto information',action='store_true')
parser.add_argument('--vetoLumiEvol',dest='vetoLumiEvolution',help='Plot the evolution of veto as a function of lumi',action='store_true')
parser.add_argument('--saveHistos',dest='saveHistos',help='Save all histograms (NOT needed for year stats)',action='store_true')
parser.add_argument('--savePlots',dest='savePlots',help='Save all plots in Weekly dir',action='store_true')
parser.add_argument('--roughVetoComput',dest='exactVetoComput',help='Rough veto computation (no lumi weighting,no correlation with intol defect.)',action='store_false')
parser.add_argument('--noGlobalFilter',dest='noGlobalFilter',help='',action='store_true')
parser.add_argument('--deliveredLumiNorm',dest='deliveredLumiNorm',help='Normalize by the delivered lumi (used until 2018)',action='store_true')
parser.add_argument('--onlineLumiNorm',dest='onlineLumiNorm',help='Normalize by the online lumi (used until 2018)',action='store_true')
parser.add_argument('--noGrlFilter',dest='noGrlFilter',help='By default, --grlUpdate update the year stats for the runs stored in the RunList/grl-*.dat file (created/updated by DQ coordinators). With this option, all runs are considered for update',action='store_true')

args = parser.parse_args()
# parser.print_help()

if args.batchMode:
  gROOT.SetBatch(True)

print("Current time: %s"%(time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())))
print("Currently running from",os.getcwd(),"on",socket.gethostname())
options = {}
yearTagProperties = {}
partitions = {}
grlDef = {}
defectVeto = {}
veto = {}
signOff = {}

if args.runListUpdate:
  updateRunList(year=args.year)

  if args.updateYearStats:
      if 'tokenName' in dir(): os.system("rm -f %s"%tokenName) 
      print("I am exiting due to updateYearStats option...")
      sys.exit()


initialize(args.system,yearTagProperties,partitions,grlDef,defectVeto,veto,signOff,args.year,args.tag)

if debug:
  printProp("yearTagProperties")
  printProp("partitions")
  printProp("grlDef")
  printProp("defectVeto")
  printProp("veto")
  printProp("signOff")
  
options = args.__dict__
options['recordedLumiNorm'] = not args.deliveredLumiNorm
# tag to access defect database and COOL database (for veto windows) - Defined in DeMoLib.py
options['defectTag']=yearTagProperties["defect"][args.tag]
if len(yearTagProperties["veto"])>0: # The veto tag is not defined for all systems
  options['vetoTag']=yearTagProperties["veto"][args.tag]
else:
  options['vetoTag']=""
options['yearStatsDir'] = "YearStats-%s/%s/%s"%(options['system'],options['year'],args.tag)
# Token to avoid having multiple yearStats update in the same time
if options['updateYearStats']:
  tokenName = "DeMo-%s-%s-%s.token"%(options['system'],options['year'],args.tag)
  if os.path.exists(tokenName):
    print("A different DeMoUpdate is apparently running (or it was not properly ended recently). This may cause trouble when updating yearStats")
    os.system("ls -ltr %s"%tokenName)
    print("If you are sure that it is not the case, you can remove the %s..."%tokenName)
    print("If you are not the owner, contact the DQ coordinator")
    sys.exit()
  else:
    os.system("touch %s"%tokenName)


runlist = {'all':[],'grl':[],'%s/roughVeto'%(options['system']):[],'range':[]}
for iRunList in list(runlist.keys()):
    iRunListDat = runListDir+"/%s-%s.dat"%(iRunList,options['year'])
    # If the grl filter is deactivated, replace the grl run list by the whole run list
    if (options['noGrlFilter']) and iRunList == "grl":
        iRunListDat = runListDir+"/all-%s.dat"%(options['year'])
    if os.path.exists(iRunListDat):
        fRunList = open(iRunListDat,'r')
        for iRun in fRunList.readlines():
            runlist[iRunList].append(int(iRun))
        fRunList.close()
if len(veto["all"]) == 0:
    options['noVeto'] = True
    print("No veto information provided in DeMoLib.py")
else:
    if options['noVeto']:
        print("WARNING: I do not consider time veto information...")

chmkDir(options['yearStatsDir'])
if options['updateYearStats']:
    yearStatsArchiveFilename = '%s/TProfiles.root'%options['yearStatsDir']
    if not os.path.exists(yearStatsArchiveFilename):
        print("No archive file found in %s"%options['yearStatsDir'])
        print("I am forcing the year stats reset...")
        options['resetYearStats'] = True
    elif os.path.getsize("%s/runs-ALL.dat"%options['yearStatsDir']) == 0.:
        # runs-ALL.data and runs-[period].dat updated only for runs in GRL
        # Test here relevant at the beginning of the year when some runs have been reviewed at EXPR/BULK level (but not FINAL hence no year stats)
        # In such a case a TProfiles.root file may exist even if no update was made
        # April 18: I am not sure that this situation is still relevant... 
        print("No run found in %s"%options['yearStatsDir'])
        print("I am forcing the year stats reset...")
        options['resetYearStats'] = True

if options['updateYearStats'] and options['resetYearStats']:
    print("WARNING: I am going to reset the %s stats..."%options['yearStatsDir'])
    if (options['batchMode']): # In batch mode, no confirmation requested
        confirm = "y"
    else:
        confirm = input("Please confirm by typing y: ")

    if ("y" in confirm):
        print("I reset the %s stats"%options['yearStatsDir'])
        # Delete the dat files that contains the runs updated and the associated lumi
        os.system("rm -f %s/lumi*.dat"%options['yearStatsDir'])
        os.system("rm -f %s/runs*.dat"%options['yearStatsDir'])
        os.system("rm -f %s/errors.log"%options['yearStatsDir'])
        if (args.resetYearStats): # The loss files are deleted only if explicit request by the user. Relevant when no GRL run found yet (hence reset forced) but defect already set
            os.system("rm -f %s/loss*.dat"%options['yearStatsDir'])
            os.system("rm -f %s/Run/*.txt"%options['yearStatsDir'])
        else:
            print("However, I did NOT delete the loss files to preserve defects set in non-GRL runs")

    # Delete the root file that contains the TProfiles
    os.system("rm -f %s"%(yearStatsArchiveFilename))
else:
    print("I did NOT reset the %s stats"%options['yearStatsDir'])
    options['resetYearStats'] = False


errorLogFile = open("%s/errors.log"%options['yearStatsDir'],'a')
notYetSignedOffRuns = open('%s/runs-notYetSignedOff.dat'%options['yearStatsDir'],'w')

runinfo = getRunInfo(runlist['all'],options['defectTag'])

for run in runlist['all']:
    if runinfo[run]["Run end"] == 0:
        print("Run",run,"is still ongoing")
        runinfo.pop(run)
        runlist['all'].pop(runlist['all'].index(run))
    if (time.time() - runinfo[run]["Run end"])/3600 < 2:
        print("Run",run,"ended very recently. It is best to wait for the defects DBs to properly populate")
        runinfo.pop(run)
        runlist['all'].pop(runlist['all'].index(run))
    

# Fill the list of runs to be considered 
if args.allRuns: # all year runs
    runlist['toprocess'] = runlist['all']
elif args.weekly: # Weekly report - Look for the last 7-days runs + unsigned off
    print("I am looking for all runs signed off in the past week and the older ones not yet signed off...")
    options['savePlots'] = True
    runlist['toprocess'] = []
    oneWeek = 7*24*3600 # Nb of seconds in one week
    runSignedOffBeyondOneWeek = []
    for run in sorted(runlist['all'], key=int, reverse=True):
        if len(runSignedOffBeyondOneWeek) >= 5: # Do not look too far in the past for unsigned off runs...
            continue
        if "defects" not in runinfo[run].keys():
            print("Run",run, "has no defects... perhaps it is still ongoing?")
            continue
        defects = runinfo[run]["defects"].keys()
        statuses = [ runinfo[run]["defects"][d]["status"] for d in defects ]

        fullySignedOff = True
        if any('Red' in s for s in statuses):
            fullySignedOff = False

        signOffTime = 0
        if any('Green' in s for s in statuses):
            signOffTime = max([ runinfo[run]["defects"][d]["time"] for d in defects if runinfo[run]["defects"][d]["status"] == "Green" ] )
        if (fullySignedOff and time.time()-signOffTime > oneWeek):
            print("Run",run,"was fully signed off > 1 week ago")
            runSignedOffBeyondOneWeek.append(run)
        else:
            if debug:
                print("Will process",run,"with signoff status:",[ runinfo[run]["defects"][d]["status"] for d in runinfo[run]["defects"].keys() ])
            runlist['toprocess'].append(run)

    runlist['toprocess'].reverse()
    print("I will process these runs :",runlist['toprocess'])
elif args.grlUpdate: # Reprocess all grl runs skipping the ones already updated
    runlist['toprocess'] = runlist['grl']
    options['skipAlreadyUpdated'] = True
    print("I am here",runlist)
elif len(args.run) == 1: # Single run 
    runNb = args.run[0]
    if (runNb not in (runlist['all'])):
        print("------------>Please first add the run in the run list")
        if options['updateYearStats']:
            os.system("rm -f %s"%tokenName)
        sys.exit()
    runlist['toprocess'] = [runNb]
    if (not options['vetoLumiEvolution']):
        options['plotResults']=False;
    options['saveHistos']=False #Do not produce plots, nor save histos! 
elif len(args.run) == 2: # Run range
    startrun = min(args.run)
    endrun   = max(args.run)
    runlist['toprocess'] = []
    for runNb in (runlist['all']): # Add all runs of runList.py in the run range
        if (runNb>=startrun and runNb<=endrun):
            runlist['toprocess'].append(runNb)
else:
    print("Please specify run number or run range with -r option")
    if options['updateYearStats']:
        os.system("rm -f %s"%tokenName)
    sys.exit()

if len(runlist['toprocess']) == 0 and len(args.run)>0:
    print("No run found in this run range...")
    print("Please double check or update the runlist file...")
    if options['updateYearStats']:
        os.system("rm -f %s"%tokenName)
    sys.exit()

periodListCurrent = {} # Dictionary of all runs referenced by period name
newPeriodInYearStats = [] # List of periods not yet considered
bool_newRunsInYearStats = False

allperiods_full = list(set([ p for irun in runlist['toprocess'] for p in runinfo[irun]["periods"] ]))  # should we just take the first period listed??
allperiods = list(set([runinfo[irun]["period"] for irun in runlist['toprocess'] if runinfo[irun]["period"] != "?"]))  # should we just take the first period listed??
print("Periods (full)", allperiods_full)
print("Periods",allperiods)
for p in allperiods:
    for run in runlist['toprocess']:
        if run in runinfo.keys() and p in runinfo[run]["periods"]:
            if p not in periodListCurrent: 
                periodListCurrent[p] = [run]
            else:
                periodListCurrent[p].append(run)

for irun in runinfo.keys():
    runinfo[irun]['newInYearStats'] = False

for iper in allperiods:
    periodFileName = "%s/runs-%s.dat"%(options['yearStatsDir'],iper)
    if os.path.exists(periodFileName): # This period is already treated in year stats. Look for new runs
        f = open(periodFileName,'r')
        existingRuns = f.readlines()
        for irun in periodListCurrent[iper]:
            if options['updateYearStats']:
                if irun in runlist['grl']:
                    runinfo[irun]['newInYearStats'] = True
                else:
                    runinfo[irun]['newInYearStats'] = False
                    print("Run %d not in GRL run list -> Ignored for YearStats"%irun)
            else:
                runinfo[irun]['newInYearStats'] = False
            if "%d\n"%(irun) in existingRuns:
                runinfo[irun]['newInYearStats'] = False # This run was already treated. Not used in incremental year stats
        f.close()
    else: # This is a period not yet treated in year stats.
        periodToBeAdded = False
        for irun in periodListCurrent[iper]:
            if options['updateYearStats']:
                if irun in runlist['grl']:
                    runinfo[irun]['newInYearStats'] = True
                    periodToBeAdded = True
                else:
                    runinfo[irun]['newInYearStats'] = False
                    print("Run %d not in GRL run list -> Ignored for YearStats"%irun)
            else:
                runinfo[irun]['newInYearStats'] = False
        if options['updateYearStats'] and periodToBeAdded:
            print("I am going to add period %s in year stats!"%(iper))
            newPeriodInYearStats.append(iper)
for iper in list(periodListCurrent.keys()): # Loop on all periods found and print the runs to be updated
    for irun in periodListCurrent[iper]:
        if runinfo[irun]['newInYearStats']:
            print("I am going to add run %d (period %s) in %s stats (provided that it is fully signed off - Not yet known...)!"%(irun,runinfo[irun]['period'],options['year']))
            bool_newRunsInYearStats = True
        else:
            if (options['skipAlreadyUpdated']):
                runinfo.pop(irun)
                runlist['toprocess'].pop(runlist['toprocess'].index(irun))
                print("%d was already processed in yearStats - I am complety ignoring it..."%(irun))

if (not bool_newRunsInYearStats):
    options['updateYearStats'] = False # No new run -> no update needed
    if 'tokenName' in dir(): os.system("rm -f %s"%tokenName)


######################################

runinfo['AllRuns'] = {}
runinfo['AllRuns']['Start'] = "-"
runinfo['AllRuns']['Stop'] = "-"
runinfo['AllRuns']['readyLB'] = "-"
runinfo['AllRuns']['readyLB_globalFilter'] = "-"
runinfo['AllRuns']['peakLumi'] = 0.
runinfo['AllRuns']['period'] = "-"
runinfo['AllRuns']['signoff'] = "-"

################################################################
# Book Histograms for general plot with intolerable defects/veto
# Fill them with past period inefficiencies
hProfRun_IntolDefect = {}
hProfRun_Veto = {}

if (options['updateYearStats']):
  hProfPeriod_IntolDefect = {}
  hProfPeriod_Veto = {}
 
allIntolDef = ["allIntol","allIntol_recov"] # New histograms defined to avoid double counting

for idef in grlDef["intol"]+grlDef["intol_recov"]+allIntolDef: #Intolerable defects only
  idefName = idef.split("__")[0] # Trick needed to get the defect name for the recoverable defect histogram
  # Histogram of defect ineffiency per run
  hProfRun_IntolDefect[idef] = MakeTProfile("hProfRun_Defect_%s"%(idef),"%s"%(defectVeto["description"][idefName]),"Lost luminosity (%)", -0.5,+0.5+len(runlist['toprocess']),len(runlist['toprocess'])+1,defectVeto["color"][idefName])
  SetXLabel(hProfRun_IntolDefect[idef],runlist['toprocess'])
  hProfRun_IntolDefect[idef].GetXaxis().SetBinLabel(len(runlist['toprocess'])+1,"All")
  # If an update of year stats is requested, define empty histograms, retrieve the already known periods and fill the new histos
  if (options['updateYearStats']):
    periodListYear = [] # This is the period list used for the TProfile
    profPeriodName = "hProfPeriod_IntolDefect_%s"%(idef)
    if (not options['resetYearStats']): # Retrieve the already known period
      file = TFile(yearStatsArchiveFilename)
      hProfPeriod_archive = file.Get("%s_archive"%(profPeriodName))
      for iBin in range(1,hProfPeriod_archive.GetNbinsX()): # Retrieve name of past periods
        periodListYear.append(hProfPeriod_archive.GetXaxis().GetBinLabel(iBin))
      file.Close()
      
    if len(periodListYear) != 0 or len(periodListCurrent) != 0: # At least one period found in current or past runs, otherwise no way to plot year stats
      # Collect all periods (archived ones + new ones)
      periodListYear = periodListYear + newPeriodInYearStats 
      periodListYear.sort() # The list of periods is now sorted
      periodNbYear = len(periodListYear) # Number of periods      
      # Create the empty year stats TProfile histograms for the updated period list
      hProfPeriod_IntolDefect[idef] = MakeTProfile(profPeriodName,"%s"%(defectVeto["description"][idefName]),"Lost luminosity (%)", -0.5,+0.5+periodNbYear,periodNbYear+1,defectVeto["color"][idefName])
      SetXLabel(hProfPeriod_IntolDefect[idef],periodListYear)
      hProfPeriod_IntolDefect[idef].GetXaxis().SetBinLabel(periodNbYear+1,"All") # In all bins, all runs
      # Fill them with the previous period data
      if (not options['resetYearStats']): 
        file = TFile(yearStatsArchiveFilename)
        hProfPeriod_archive = file.Get("%s_archive"%(profPeriodName))
        for iBin in range(1,periodNbYear+1): # Loop on all extended periods and check if already considered in past runs - Mandatory as periodListYear was sorted
          for iBin2 in range(1,hProfPeriod_archive.GetNbinsX()):
            if (hProfPeriod_IntolDefect[idef].GetXaxis().GetBinLabel(iBin) == hProfPeriod_archive.GetXaxis().GetBinLabel(iBin2)):
              hProfPeriod_IntolDefect[idef].Fill(iBin-1,hProfPeriod_archive.GetBinContent(iBin2),hProfPeriod_archive.GetBinEntries(iBin2))
          # And the last bin for all periods
        hProfPeriod_IntolDefect[idef].Fill(len(periodListYear),hProfPeriod_archive.GetBinContent(hProfPeriod_archive.GetNbinsX()),hProfPeriod_archive.GetBinEntries(hProfPeriod_archive.GetNbinsX()))
        file.Close()

# Book histograms for time window veto
if (not options['noVeto']):
  for iVeto in veto["all"]+["allVeto"]: # New "allVeto" histograms defined similarly as for defect - Not sure how to deal with double counting in veto but not problematic at first order.
    hProfRun_Veto[iVeto] = MakeTProfile("hProfRun_Veto_%s"%(iVeto),"","Inefficiency - Time veto (%)",-0.5,+0.5+len(runlist['toprocess']),len(runlist['toprocess'])+1,defectVeto["color"][iVeto])
    hProfRun_Veto[iVeto].SetMarkerColor(defectVeto["color"][iVeto])
    SetXLabel(hProfRun_Veto[iVeto],runlist['toprocess'])
    hProfRun_Veto[iVeto].GetXaxis().SetBinLabel(len(runlist['toprocess'])+1,"All")
  
    if (options['updateYearStats']):
      profPeriodName = "hProfPeriod_Veto_%s"%(iVeto)
      # Create the period TProfile - The extended period list is the same as for the defects
      hProfPeriod_Veto[iVeto] = MakeTProfile(profPeriodName,"","Inefficiency - Time veto (%%) %s"%(defectVeto["description"][iVeto]), -0.5,+0.5+len(periodListYear),len(periodListYear)+1,defectVeto["color"][iVeto])
      hProfPeriod_Veto[iVeto].SetMarkerColor(defectVeto["color"][iVeto])
      SetXLabel(hProfPeriod_Veto[iVeto],periodListYear)
      hProfPeriod_Veto[iVeto].GetXaxis().SetBinLabel(len(periodListYear)+1,"All")
  
      if (not options['resetYearStats']): # Fill with the previous period data
        file = TFile(yearStatsArchiveFilename)
        hProfPeriod_archive = file.Get("%s_archive"%(profPeriodName))
        for iBin in range(1,periodNbYear+1): # Loop on all extended periods and check if already considered in past runs - Mandatory as periodListYear was sorted
          for iBin2 in range(1,hProfPeriod_archive.GetNbinsX()):
            if (hProfPeriod_Veto[iVeto].GetXaxis().GetBinLabel(iBin) == hProfPeriod_archive.GetXaxis().GetBinLabel(iBin2)):
              hProfPeriod_Veto[iVeto].Fill(iBin-1,hProfPeriod_archive.GetBinContent(iBin2),hProfPeriod_archive.GetBinEntries(iBin2))
        # And the last bin for all periods
        hProfPeriod_Veto[iVeto].Fill(len(periodListYear),hProfPeriod_archive.GetBinContent(hProfPeriod_archive.GetNbinsX()),hProfPeriod_archive.GetBinEntries(hProfPeriod_archive.GetNbinsX()))
        file.Close()

h1Run_IntLuminosity  = MakeTH1("h1_IntLumi","Run","Integrated luminosity", -0.5,+0.5+len(runlist['toprocess']),len(runlist['toprocess'])+1,kBlack)
SetXLabel(h1Run_IntLuminosity,runlist['toprocess'])
h1Run_IntLuminosity.GetXaxis().SetBinLabel(len(runlist['toprocess'])+1,"All")

### TO BE MODIFIED WHEN TH1 IS SAVED IN TPROFILE.ROOT. Can be filled in a more logical way
if (options['updateYearStats'] and periodNbYear>0): # If update is required, it is now sure that some periods exist. Create a TH1 to store the integrated luminosity
  h1Per_IntLumi = MakeTH1("periodLuminosity","Period","Luminosity(pb-1)",-0.5,+0.5+periodNbYear,periodNbYear+1,1)
  h1Per_IntLumi.SetTitle("")
  SetXLabel(h1Per_IntLumi,periodListYear)
  h1Per_IntLumi.GetXaxis().SetBinLabel(periodNbYear+1,"All")

  for iBin in range(1,hProfPeriod_IntolDefect[idef].GetNbinsX()+1): # No matter what is idef, we just need the number of entries per bin to get the luminosity of past periods
    h1Per_IntLumi.Fill(iBin-1,hProfPeriod_IntolDefect[idef].GetBinEntries(iBin)/1e6)
  h1Per_IntLumi.Print("all")


#######################################################################################
# Book histograms for luminosity profile and veto rejection as a function of luminosity
if options['vetoLumiEvolution']:
  h1_vetoInstLumiEvol = {}
  h1_vetoInstLumiEvol['NoVeto'] = MakeTH1("LuminosityProfile","Instantaneous luminosity (10^{33} cm^{-2}s^{-1})","Time length (s)",4,19,60,1)
  for iVeto in veto["all"]:
    h1_vetoInstLumiEvol[iVeto] = MakeTH1("LuminosityVeto_%s"%(iVeto),"Instantaneous luminosity (10^{33} cm^{-2}s^{-1}) - Runs %d-%d"%(runlist['toprocess'][0],runlist['toprocess'][-1]),"#splitline{Inefficiency Time Veto}{(%s) [percent]}"%(defectVeto["description"][iVeto]),4,19,60,1)


#######################################################################################
# Initialize total luminosity affected per defect for all runs
runinfo['AllRuns']['Lumi'] = 0. # Total luminosity
for idef in (grlDef["tol"]+grlDef["intol"]+grlDef["intol_recov"]+allIntolDef):
  runinfo['AllRuns']['lumiDefect_%s'%(idef)] = 0.
  runinfo['AllRuns']['ineffDefect_%s'%(idef)] = 0.

for iVeto in veto["all"]+["allVeto"]:
  runinfo['AllRuns']['lumiVeto_%s'%iVeto] = 0. # Total luminosity rejected by each time veto
  runinfo['AllRuns']['ineffVeto_%s'%iVeto] = 0. # Overall inefficiency due to each time veto

if (len(list(runinfo.keys())) == 1):
  print("I did not find any run in runList.")
  print("Please check the run range/options")

print("I will use the following defect/veto tags:%s %s"%(options['defectTag'],options['vetoTag']))

brokenruns = [] # list of any broken runs
#######################################################################################
#### Main loop over selected runs
for irun,runNb in enumerate(runlist['toprocess']):
  print("=================================================================") 
  print("=============================Run %d (%d/%d)======================"%(runNb,irun+1,len(runlist['toprocess']))) 
  
  # Init variables - List (indexed by partition) of tables of lumi blocks affected by defects
  lbAffected = {}
  for idef in grlDef["part"]+grlDef["partIntol_recov"]: # All partition defects
    lbAffected[idef] = {}
    lbAffected[idef]["AllPartitions"] = []
  for idef in grlDef["glob"]+grlDef["globIntol_recov"]: # All global defects
    lbAffected[idef] = [] # Global defect - Simple list and not dictionnary
  
  lbAffected['allIntol'] = [] # List of LBs affected with intolerable defect independant of the defect/partition
  lbAffected['allIntol_recov'] = [] # List of LBs affected with intolerable recoverable defect independant of the defect/partition
  lbAffected['allIntol_irrecov'] = [] # List of LBs affected with intolerable irrecoverable defect independant of the defect/partition
  

  boolExactVetoComput_run = (options['exactVetoComput']) and (runNb not in runlist['%s/roughVeto'%(options['system'])]) and (runNb in runlist['grl'])

  # Start retrieving the general characteristics of the run
  # Get run infos
  # Luminosity blocks UTC timestamps
  v_lbTimeSt = GetLBTimeStamps(runNb)
  runinfo[runNb]['Start'] = time.strftime("%a, %d %b %H:%M",time.localtime(v_lbTimeSt[1][0]))
  runinfo[runNb]['Stop'] = time.strftime("%a, %d %b %H:%M",time.localtime(v_lbTimeSt[len(v_lbTimeSt)][1]))
  runinfo[runNb]['nBunches'] = GetNumberOfCollidingBunches(runNb)
  # Number of luminosity blocks
  runinfo[runNb]['nLB'] = len(v_lbTimeSt)

  defectDatabase = DQDefects.DefectsDB(tag=options['defectTag'])
  # Treatement of global filter
  globalFilterLB = []
  if not options['noGlobalFilter']:
    retrievedDefects = defectDatabase.retrieve((runNb, 1), (runNb+1, 0), grlDef["globalFilterDefects"])   
    if any(rd.until.lumi > 4000000000 for rd in retrievedDefects):
        # some runs seem to have corruption in the trips DB
        print("Something seems wrong with run",runNb,"... huge LB number...")
        print("Run end time was:",time.ctime(runinfo[runNb]["Run end"]))
        brokenruns.append(runNb)
        continue
    for iRetrievedDefects in retrievedDefects:
      for lb in range(iRetrievedDefects.since.lumi,iRetrievedDefects.until.lumi):
        globalFilterLB.append(lb)

  # Atlas Ready
  atlasready=GetReadyFlag(runNb)
  runinfo[runNb]['readyLB']=[]
  runinfo[runNb]['readyLB_globalFilter']=[]
  for lb in list(atlasready.keys()):
    if (atlasready[lb]>0): runinfo[runNb]['readyLB']+=[lb]  #uncomment this for real runs!!
    if (atlasready[lb]>0) and (lb not in globalFilterLB): runinfo[runNb]['readyLB_globalFilter']+=[lb]  #uncomment this for real runs!!
  runinfo[runNb]['nLBready'] = float(len(runinfo[runNb]['readyLB_globalFilter']))    

  thisRunPerLB = dict() # Contains various per LB run characteristics retrieved from COOL
  # COOL delivered luminosity
  if options['onlineLumiNorm']:
    thisRunPerLB["deliveredLumi"] = GetOnlineLumiFromCOOL(runNb,0)
  else:
    if runNb in runlist['grl']:
      thisRunPerLB["deliveredLumi"] = GetOfflineLumiFromCOOL(runNb,0,yearTagProperties["offlineLumiTag"]["grl"][options['year']])
    else:
      thisRunPerLB["deliveredLumi"] = GetOfflineLumiFromCOOL(runNb,0,yearTagProperties["offlineLumiTag"]["preliminary"])

  # Look for peak lumi
  runinfo[runNb]['peakLumi'] = 0.
  for lb in list(thisRunPerLB['deliveredLumi'].keys()): 
    if thisRunPerLB['deliveredLumi'][lb] > runinfo[runNb]['peakLumi']: runinfo[runNb]['peakLumi']=thisRunPerLB['deliveredLumi'][lb] 
    if thisRunPerLB['deliveredLumi'][lb] > runinfo['AllRuns']['peakLumi']: runinfo['AllRuns']['peakLumi']=thisRunPerLB['deliveredLumi'][lb] 

  # Store the duration
  thisRunPerLB['duration'] = GetLBDuration(runNb)

  # Back up method. Retrieve the precise LB duration (more precise than GetLBDuration(runNb)) and liveFraction (relevant if recorded lumi normalisation).
  #lumiacct=fetch_iovs('COOLOFL_TRIGGER::/TRIGGER/OFLLUMI/LumiAccounting', tag='OflLumiAcct-001', since=v_lbTimeSt[1][0]*1000000000, until=v_lbTimeSt[len(v_lbTimeSt)][1]*1000000000) 
  #thisRunPerLB['duration'] = dict()
  #for iLumiAcct in range(len(lumiacct)):
  #  if options['recordedLumiNorm']: # The LB duration is corrected by the live fraction 
  #    thisRunPerLB['duration'][lumiacct[iLumiAcct].LumiBlock] = lumiacct[iLumiAcct].LBTime*lumiacct[iLumiAcct].LiveFraction
  #  else:
  #    thisRunPerLB['duration'][lumiacct[iLumiAcct].LumiBlock] = lumiacct[iLumiAcct].LBTime

  # Store the luminosity used for the efficiency normalisations
  for lb in range(1,runinfo[runNb]['nLB']+2): # Loop on all LB - Protection to set a zero luminosity if not available
    if lb in runinfo[runNb]['readyLB_globalFilter']:
      if lb not in list(thisRunPerLB["deliveredLumi"].keys()):
        thisRunPerLB["deliveredLumi"][lb] = 0.
        errorMsg = "Missing lumi for Run %d - LB %d\n"%(runNb,lb)
        print(errorMsg)  #uncomment this for real runs!!
        errorLogFile.write(errorMsg)
      if lb not in list(thisRunPerLB["duration"].keys()):
        thisRunPerLB["duration"][lb] = 0.
        errorMsg = "Missing duration/LiveFraction for Run %d - LB %d\n"%(runNb,lb)
        print(errorMsg)  #uncomment this for real runs!!
        errorLogFile.write(errorMsg)
    else:
      if lb not in list(thisRunPerLB["deliveredLumi"].keys()):
        thisRunPerLB["deliveredLumi"][lb] = 0.
      if lb not in list(thisRunPerLB["deliveredLumi"].keys()):
        thisRunPerLB["duration"][lb] = 0.

  if options['vetoLumiEvolution']:
    for lb in runinfo[runNb]['readyLB_globalFilter']: # Fill the luminosity profile
      if thisRunPerLB["deliveredLumi"][lb] != 0.:
        h1_vetoInstLumiEvol['NoVeto'].Fill(thisRunPerLB["deliveredLumi"][lb]/1e3,v_lbTimeSt[lb][1]-v_lbTimeSt[lb][0])
  
  # Finished retrieving the general characteristics of the run
  
  # Get defects

  parsedDefects =  retrieveDefectsFromDB(runNb, options['defectTag'], grlDef)
  retrievedDefects = list(parsedDefects.keys())
    
  runinfo[runNb]['exprSignedOff'] = True
  runinfo[runNb]['bulkSignedOff'] = True
  runinfo[runNb]['signoff'] = 'FINAL OK'
  # Loop over all defects and store in list (1 per partition and type) the affected LB
  # Consider only LB in runinfo[runNb]["readyLB_globalFilter"]
  for iRetrievedDefects in retrievedDefects:
    # keep track of runs with missing sign-off - Store the earliest stage of the sign off procedure
    for iSignOff in signOff["EXPR."]:
      if iRetrievedDefects.channel == iSignOff:
        runinfo[runNb]['signoff'] = "EXPR."
    if "EXPR." not in runinfo[runNb]['signoff']:
      for iSignOff in signOff["BULK"]:
        if iRetrievedDefects.channel == iSignOff:
          runinfo[runNb]['signoff'] = "BULK"
      if "BULK" not in runinfo[runNb]['signoff']:
        for iSignOff in signOff["FINAL"]:
          if iRetrievedDefects.channel == iSignOff:
            runinfo[runNb]['signoff'] = "DONE"        

    if runinfo[runNb]['signoff'] != 'FINAL OK': # Update year stats only after final signoff
      runinfo[runNb]['newInYearStats'] = False

    # Checks if the defect corresponds to a defect in the system list
    defectFound, partAffected = parsedDefects[iRetrievedDefects]

    # Now stored the affected 
    if defectFound in grlDef["part"]:
      for lb in range(iRetrievedDefects.since.lumi,iRetrievedDefects.until.lumi):
        if(lb in runinfo[runNb]['readyLB_globalFilter']):# The LB is with ATLAS ready

          if partAffected not in lbAffected[defectFound]: # Store the affected partitions - A priori no longer used
            lbAffected[defectFound][partAffected]=[]
          lbAffected[defectFound][partAffected].append(lb)

          lbAffected[defectFound]["AllPartitions"].append(lb)
          if (defectFound in grlDef["partIntol"]):# Update the LBs affected by an intolerable defect whatever is the partition
            if (lb not in lbAffected['allIntol']): 
              lbAffected['allIntol'].append(lb)
            if (not iRetrievedDefects.recoverable and lb not in lbAffected['allIntol_irrecov']): # Update the LBs affected by an intolerable irrecoverable defect
              lbAffected['allIntol_irrecov'].append(lb)

    if defectFound in grlDef["glob"]:
      # The min is a protection for a single 2017 run with Trig_HLT (326468) that lead to a crash...
      for lb in range(iRetrievedDefects.since.lumi,min(iRetrievedDefects.until.lumi,runinfo[runNb]['nLB']+1)):
        if(lb in runinfo[runNb]["readyLB_globalFilter"]):
          lbAffected[defectFound].append(lb)
          if (defectFound in grlDef["globIntol"]):# Update the LBs affected by an intolerable irrecoverable defect
            if (lb not in lbAffected['allIntol']):
              lbAffected['allIntol'].append(lb)
            if (not iRetrievedDefects.recoverable and lb not in lbAffected['allIntol_irrecov']): 
              lbAffected['allIntol_irrecov'].append(lb)

  # By default, an unsignedoff runs is kept in the final table/plots such that the LADIeS can use the plot. On 
  # request, they can be also ignored.
  # NB: in any way, a non signed off run is never considered in year stats
  if options['skipUnsignedOff'] and runinfo[runNb]['signoff'] != 'FINAL OK':
    print("Run %d is not yet signed off. Skipping it..."%runNb)
    runinfo.pop(runNb)
    continue

  # Now treat recoverability - Storing all irrecoverable lb per defect (not partition wise as useless)
  for idef in grlDef["partIntol"]:
    lbAffected['%s__recov'%idef]["AllPartitions"]=[]
    for ilb in lbAffected[idef]["AllPartitions"]:
      if ilb not in lbAffected['allIntol_irrecov']:
        lbAffected['%s__recov'%idef]["AllPartitions"].append(ilb)
  for idef in grlDef["globIntol"]:
    for ilb in lbAffected[idef]:
      if ilb not in lbAffected['allIntol_irrecov']:
        lbAffected['%s__recov'%idef].append(ilb)
        lbAffected['allIntol_recov'].append(ilb)
        
  # Prepare the computation of inefficiencies - Initialization
  for idef in (grlDef["tol"]+grlDef["intol"]+grlDef["intol_recov"]+allIntolDef):
    runinfo[runNb]["lumiDefect_%s"%(idef)] = 0.
    runinfo[runNb]["ineffDefect_%s"%(idef)] = 0.
  runinfo[runNb]['Lumi'] = 0

  # Loop on all LB of the run
  for lb in runinfo[runNb]["readyLB_globalFilter"]:
    # Compute integrated luminosities
    runinfo[runNb]['Lumi'] = runinfo[runNb]['Lumi'] +thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
    runinfo['AllRuns']['Lumi'] = runinfo['AllRuns']['Lumi'] +thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
    # Loop on defects already stored - Partition defect (Tolerable + intolerable) 
    for idef in grlDef["part"]+grlDef["partIntol_recov"]:
      if lb in lbAffected[idef]["AllPartitions"]:
        runinfo[runNb]["lumiDefect_%s"%(idef)] = runinfo[runNb]["lumiDefect_%s"%(idef)] + thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
        runinfo["AllRuns"]["lumiDefect_%s"%(idef)] = runinfo["AllRuns"]["lumiDefect_%s"%(idef)] + thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
      if (runinfo[runNb]["Lumi"] != 0.):
        runinfo[runNb]["ineffDefect_%s"%(idef)] = runinfo[runNb]["lumiDefect_%s"%(idef)]/runinfo[runNb]["Lumi"]*100.

    # Loop on defects already stored - Global defect (Tolerable + intolerable) + all intolerable (used to avoid double counting) 
    for idef in grlDef["glob"]+grlDef["globIntol_recov"]+allIntolDef: 
      if lb in lbAffected[idef]:
        runinfo[runNb]["lumiDefect_%s"%(idef)] = runinfo[runNb]["lumiDefect_%s"%(idef)] + thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
        runinfo["AllRuns"]["lumiDefect_%s"%(idef)] = runinfo["AllRuns"]["lumiDefect_%s"%(idef)] + thisRunPerLB["deliveredLumi"][lb]*thisRunPerLB['duration'][lb]
      if (runinfo[runNb]["Lumi"] != 0.):
        runinfo[runNb]["ineffDefect_%s"%(idef)] = runinfo[runNb]["lumiDefect_%s"%(idef)]/runinfo[runNb]["Lumi"]*100.
     
  if runinfo[runNb]['Lumi']==0: runinfo[runNb]['Lumi']=1e-50

  for idef in grlDef["intol"]+grlDef["intol_recov"]+allIntolDef:
    hProfRun_IntolDefect[idef].Fill(irun,runinfo[runNb]["ineffDefect_%s"%(idef)])
    hProfRun_IntolDefect[idef].Fill(len(runlist['toprocess']),runinfo[runNb]["ineffDefect_%s"%(idef)],runinfo[runNb]['Lumi']) # Fill last bins (all runs) - Reminder : this is a profile !
    if (options['updateYearStats'] and runinfo[runNb]['newInYearStats']):
      hProfPeriod_IntolDefect[idef].Fill(periodListYear.index(runinfo[runNb]["period"]),runinfo[runNb]["ineffDefect_%s"%(idef)],runinfo[runNb]['Lumi'])
      hProfPeriod_IntolDefect[idef].Fill(len(periodListYear),runinfo[runNb]["ineffDefect_%s"%(idef)],runinfo[runNb]['Lumi'])

  h1Run_IntLuminosity.Fill(irun,runinfo[runNb]['Lumi']/1e6)
  h1Run_IntLuminosity.Fill(len(runlist['toprocess']),runinfo[runNb]['Lumi']/1e6)

  if (options['updateYearStats'] and runinfo[runNb]['newInYearStats']):
    h1Per_IntLumi.Fill(periodListYear.index(runinfo[runNb]["period"]),runinfo[runNb]['Lumi']/1e6)
    h1Per_IntLumi.Fill(h1Per_IntLumi.GetNbinsX()-1,runinfo[runNb]['Lumi']/1e6) # Cumulated for the year    

  # Now starts veto inefficiency 
  # Retrieve the length of time period vetoed
  db2="COOLOFL_LAR/CONDBR2"
  folderName="/LAR/BadChannelsOfl/EventVeto"
  if (not options['noVeto']):
    if (boolExactVetoComput_run):
      totalVeto = showEventVeto.showEventVetoFolder(db2,folderName,options['vetoTag'],runNb,runNb,0) 
    else:
      print("WARNING: you use the rough event veto loss. To be used only if default is too slow...")
      totalVeto = showEventVetoNoLumi.showEventVetoFolder(db2,folderName,options['vetoTag'],runNb,runNb,0) 
  else:
    totalVeto = None

  for iVeto in veto["all"]+["allVeto"]:
    runinfo[runNb]["lumiVeto_%s"%(iVeto)] = 0.
    runinfo[runNb]["ineffVeto_%s"%(iVeto)] = 0.

  if (totalVeto is not None):    
    if (boolExactVetoComput_run):# Computation of veto rejection weighting by inst. lumi and ignoring LB already in intolerable defect list
      for iVeto in veto["all"]:
        for iVetoedLB in range(len(totalVeto[veto["COOL"][iVeto]])): # Loop on all veto periods
          lb0 = findLB(v_lbTimeSt,totalVeto[veto["COOL"][iVeto]][iVetoedLB][0]/1e9) # Start of veto period
          lb1 = findLB(v_lbTimeSt,totalVeto[veto["COOL"][iVeto]][iVetoedLB][0]/1e9) # End of veto period
          if options['vetoLumiEvolution']:
            h1_vetoInstLumiEvol[iVeto].Fill(thisRunPerLB["deliveredLumi"][lb0]/1e3,(totalVeto[veto["COOL"][iVeto]][iVetoedLB][1]-totalVeto[veto["COOL"][iVeto]][iVetoedLB][0])/1e9)
          if (lb0 not in lbAffected['allIntol'] and lb1 not in lbAffected['allIntol'] and (lb0 in runinfo[runNb]["readyLB_globalFilter"] or lb1 in runinfo[runNb]["readyLB_globalFilter"])): # If end/start not in lb with intol defect, add rejection period - Incorrect if > 1 LBs
            runinfo[runNb]["lumiVeto_%s"%iVeto] = runinfo[runNb]["lumiVeto_%s"%iVeto] + thisRunPerLB["deliveredLumi"][lb0]*((totalVeto[veto["COOL"][iVeto]][iVetoedLB][1]-totalVeto[veto["COOL"][iVeto]][iVetoedLB][0])/1e9)
        # Assumption that there is no overlap between the different tipes of time veto. To be fixed ideally...
        runinfo[runNb]["lumiVeto_allVeto"] = runinfo[runNb]["lumiVeto_allVeto"] + runinfo[runNb]["lumiVeto_%s"%iVeto]
    else:
      for iVeto in veto["all"]:
        if len(runinfo[runNb]["readyLB_globalFilter"]) != 0.:
          runinfo[runNb]["lumiVeto_%s"%(iVeto)] = totalVeto[iVeto][1] # WARNING : the veto inefficiency is computed from time (and not lumi). To be normalized by time
        # Assumption that there is no overlap between the different tipes of time veto. To be fixed ideally...
        runinfo[runNb]["lumiVeto_allVeto"] = runinfo[runNb]["lumiVeto_allVeto"] + runinfo[runNb]["lumiVeto_%s"%iVeto]
          
  if (not options['noVeto']):
    for iVeto in veto["all"]+["allVeto"]:
      if boolExactVetoComput_run:
        runinfo[runNb]["ineffVeto_%s"%(iVeto)] = runinfo[runNb]["lumiVeto_%s"%(iVeto)]/runinfo[runNb]['Lumi']*100.
      else:# The veto inefficiency is computed from time (and not lumi). The ineff is normalized by time
        if len(runinfo[runNb]["readyLB_globalFilter"]) != 0.:
          runinfo[runNb]["ineffVeto_%s"%(iVeto)] = runinfo[runNb]["lumiVeto_%s"%(iVeto)]/(len(runinfo[runNb]["readyLB_globalFilter"])*60*1e9)*100.
        else:
          runinfo[runNb]["ineffVeto_%s"%(iVeto)] = 0.
  
      hProfRun_Veto[iVeto].Fill(irun,runinfo[runNb]["ineffVeto_%s"%(iVeto)])
      hProfRun_Veto[iVeto].Fill(len(runlist['toprocess']),runinfo[runNb]["ineffVeto_%s"%(iVeto)],runinfo[runNb]['Lumi']) # Fill last bins (all runs) - Reminder : this is a profile !
  
      if (options['updateYearStats'] and runinfo[runNb]['newInYearStats']):
        hProfPeriod_Veto[iVeto].Fill(periodListYear.index(runinfo[runNb]["period"]),runinfo[runNb]["ineffVeto_%s"%(iVeto)],runinfo[runNb]['Lumi'])
        hProfPeriod_Veto[iVeto].Fill(len(periodListYear),runinfo[runNb]["ineffVeto_%s"%(iVeto)],runinfo[runNb]['Lumi']) # Fill last bins (all periods)
  
      runinfo['AllRuns']['lumiVeto_%s'%iVeto] = runinfo['AllRuns']['lumiVeto_%s'%iVeto] + runinfo[runNb]["lumiVeto_%s"%iVeto]

  singleRunReport(runNb,runinfo[runNb],lbAffected,options['yearStatsDir'],grlDef,veto,boolExactVetoComput_run)


# End of loop on runs



if options['vetoLumiEvolution']:
  for iVeto in veto["all"]:
    h1_vetoInstLumiEvol[iVeto].Divide(h1_vetoInstLumiEvol[iVeto],h1_vetoInstLumiEvol['NoVeto'],100.,1.)
    

# I am not sure about this - BT october 2022
for br in brokenruns:
    runinfo.pop(br)
    runlist['all'].pop(runlist['all'].index(br))
    if br in runlist['toprocess']: runlist['toprocess'].pop(runlist['toprocess'].index(br))
    if br in runlist['grl']: runlist['grl'].pop(runlist['grl'].index(br))

######################### Treatment when a run range was considered (weekly report)
if (len(list(runinfo.keys()))>2 and runinfo['AllRuns']['Lumi']!=0):
  # Compute inefficiencies for the whole period
  
  # Defect inefficencies first
  for iDef in grlDef["intol"]+grlDef["intol_recov"]+allIntolDef:
    runinfo['AllRuns']['ineffDefect_%s'%iDef] = hProfRun_IntolDefect[iDef].GetBinContent(hProfRun_IntolDefect[iDef].GetNbinsX())

  if (not options['noVeto']):
    for iVeto in veto["all"]+["allVeto"]:
      runinfo['AllRuns']['ineffVeto_%s'%iVeto] = hProfRun_Veto[iVeto].GetBinContent(hProfRun_IntolDefect[iDef].GetNbinsX())
  
  # Prepare the summary tables
  lineNb = {}
  column = {}
  lineNb = {}
  c1 = {}
  
  canvasIndex = 0
  newCanvas = True
  for runNb in runlist['toprocess']+["AllRuns"]:
    if runNb not in list(runinfo.keys()):
      continue # Protection in case of the runs was not yet signed off and removed (with Unsignedoff option) from the list
          
    if newCanvas:
      # NewCanvas facility almost removed (50 runs cut) Size of the last TCanvas not properly computed
      c1[canvasIndex] = TCanvas("runSummary_%s"%canvasIndex,"Run collection - %s"%canvasIndex,10,10,1000,(len(runlist['toprocess'])+1)*22)
      column[canvasIndex] = []
      lineNb[canvasIndex] = 0
      labels_col = ["Run","Run start / stop","LB ready","Peak lumi","Int. lumi","GRL ineff.","Veto ineff.","Period","Status"]
      labels_xlow = [0.01,0.13,0.44,0.51,0.59,0.65,0.72,0.855,0.925,0.99]
      labels_xlow = [0.01,0.08,0.41,0.49,0.575,0.655,0.74,0.835,0.9,0.99]
      
      for i in range(len(labels_col)):
        column[canvasIndex].append(TPaveText(labels_xlow[i],0.01,labels_xlow[i+1],0.99))
        column[canvasIndex][i].AddText(labels_col[i])
        if (i%2 == 0):
          column[canvasIndex][i].SetFillColor(kOrange-3)
        else:
          column[canvasIndex][i].SetFillColor(kGreen+2)
      newCanvas = False
    if runNb == "AllRuns":
      column[canvasIndex][0].AddText("ALL")
    else:
      column[canvasIndex][0].AddText("%d"%(runNb))
    column[canvasIndex][1].AddText("%s / %s"%(runinfo[runNb]['Start'],runinfo[runNb]['Stop']))
    column[canvasIndex][2].AddText("%s"%(listify(runinfo[runNb]["readyLB"])))
    column[canvasIndex][3].AddText("%.1e"%(runinfo[runNb]['peakLumi']*1e30))
    column[canvasIndex][4].AddText("%s"%(strLumi(runinfo[runNb]['Lumi'])))
    column[canvasIndex][5].AddText("%.2f %%"%(runinfo[runNb]['ineffDefect_allIntol']))
    column[canvasIndex][6].AddText("%.2f %%"%(runinfo[runNb]['ineffVeto_allVeto']))
    column[canvasIndex][7].AddText("%s"%(runinfo[runNb]["period"]))
    column[canvasIndex][8].AddText("%10s"%(runinfo[runNb]["signoff"]))
    lineNb[canvasIndex] += 1
    if (lineNb[canvasIndex]==50 or runNb == "AllRuns"):
      for i in range(len(column[canvasIndex])):
        if i == 1:
          column[canvasIndex][i].AddText("Completed at %s"%(time.strftime("%H:%M (%d %b)", time.localtime())))
        else:
          column[canvasIndex][i].AddText("")
        column[canvasIndex][i].Draw()
      c1[canvasIndex].SetWindowSize(1000,lineNb[canvasIndex]*40)
      c1[canvasIndex].Update()

      newCanvas = True
      canvasIndex += 1
      
    if runinfo[runNb]["signoff"] != "FINAL OK" and runNb != "AllRuns":
      print("Run %d not fully signed off -> no year stats update. Current status: %s"%(runNb,runinfo[runNb]["signoff"]))
      notYetSignedOffRuns.write("%d (period %s) -> Current status : %s \n"%(runNb,runinfo[runNb]['period'],runinfo[runNb]["signoff"]))

  chmkDir(options['yearStatsDir']+'/Weekly')  
  if options['savePlots']:
    for iCanvas in range(len(c1)):
      c1[iCanvas].SaveAs("%s/Weekly/summary-%d.png"%(options['yearStatsDir'],iCanvas))


canvasResults = {}
legendResults = {}
stackResults = {}

### Show Plots only for considered runs
gStyle.SetOptStat(0)
if options['plotResults']:
  gStyle.SetHistMinimumZero()

  plotStack("defects--Run--%s"%(args.tag),hProfRun_IntolDefect,grlDef["intol"],defectVeto["description"],h1Run_IntLuminosity,False,stackResults,canvasResults,legendResults)
  if (len(veto["all"])):
    plotStack("veto--Run--%s"%(args.tag),hProfRun_Veto,veto["all"],defectVeto["description"],h1Run_IntLuminosity,False,stackResults,canvasResults,legendResults)

  if options['vetoLumiEvolution']:
    for iVeto in veto["all"]:
      canvasResults["%s_veto_evol"%(iVeto)] = TCanvas("%s_veto_evol"%(iVeto),"%s inefficiency (%s time veto) vs Inst.Lumi."%(options["system"],defectVeto["description"][iVeto]), 200, 10, 1000, 500)
      canvasResults["%s_veto_evol"%(iVeto)].SetGridy(1)
      h1_vetoInstLumiEvol[iVeto].Draw()

  if options['savePlots']:
    chmkDir(options['yearStatsDir']+'/Weekly')  
    for iCanvas in list(canvasResults.keys()):
      canvasResults[iCanvas].SaveAs("%s/Weekly/%s.png"%(options['yearStatsDir'],iCanvas))
    
# End of plots
# Save the histograms when requested. NB:This is different from yearStats update
if (options['saveHistos']):
  filename = 'Files/weeklyHisto-%s-%s.root'%(startrun, endrun)
  f = TFile(filename,"recreate")
  for idef in grlDef["intol"]:
    hProfRun_IntolDefect[idef].Write()
  if options['vetoLumiEvolution']:
    h1_vetoInstLumiEvol["NoVeto"].Write()
  for iVeto in veto["all"]:
    hProfRun_Veto[iVeto].Write()
    if options['vetoLumiEvolution']:
      h1_vetoInstLumiEvol[iVeto].Write()
  f.Close()
  print("Histos saved in %s"%(filename))

# yearStats update
# If new runs were added to period plots, save them
if (options['updateYearStats'] and bool_newRunsInYearStats):
  print("WARNING: I am going to update the %s stats with the following runs:"%(options['year']))
  print("NB: only runs fully signed off are considered")
  for irun in list(runinfo.keys()):
    if (irun != "AllRuns"):
      if runinfo[irun]['newInYearStats']:
        print(irun)
        
  if (options['batchMode']): # In batch mode, no confirmation requested
    confirm = "y"
  else:
    confirm = input("Are you sure ([y]/n)?: ")
    
  if ("n" not in confirm):
    f = TFile(yearStatsArchiveFilename,"recreate")   #this file summarize all year stats with proper periods assigned
    for idef in grlDef["intol"] + grlDef["intol_recov"]+allIntolDef:#Intolerable defects only
      hProfPeriod_IntolDefect[idef].SetName("%s_archive"%(hProfPeriod_IntolDefect[idef].GetName()))
      hProfPeriod_IntolDefect[idef].Write()
    if (len(veto["all"])):
      for iVeto in (veto["all"]+["allVeto"]):
        hProfPeriod_Veto[iVeto].SetName("%s_archive"%(hProfPeriod_Veto[iVeto].GetName()))
        hProfPeriod_Veto[iVeto].Write()
      
    h1Per_IntLumi.SetName("h1Period_IntLuminosity_archive")
    h1Per_IntLumi.Write()
    f.Close()
    
    # Creating empty files for new period
    for iper in newPeriodInYearStats:
      periodFileName = "%s/runs-%s.dat"%(options['yearStatsDir'],iper)
      f = open(periodFileName,'w')
      f.close()
    
    # Adding all runs not treated in year stats
    fAll = open("%s/runs-ALL.dat"%options['yearStatsDir'],'a')
    for iper in list(periodListCurrent.keys()): # Loop on all periods found
      periodFileName = "%s/runs-%s.dat"%(options['yearStatsDir'],iper)
      f = open(periodFileName,'a')
      for irun in periodListCurrent[iper]:
        if (irun in list(runinfo.keys()) and runinfo[irun]['newInYearStats']): # Runs not yet considered in yearStats
          f.write("%d\n"%(irun))
          fAll.write("%d\n"%(irun))
      f.close()
    fAll.close()
    print("I have updated year stats")

# The update of the defect dat files is now decoupled from the yearStatsUpdate to allows to also monitor runs (special runs notably)
# that are not in the GRL.  
# for irun in list(runinfo.keys()):  # should just be processed runs, or we don't have signoff
for irun in runlist['toprocess']:
  if runinfo[irun]['signoff'] == 'FINAL OK' and irun != "AllRuns":  #uncomment this for real runs!!
    # NB : GRL information no longer stored here as this information is now stored in runlist['toprocess']
    # Backwards compatibility with DeMoScan to be checked
    irun_string = "%d (%.0f ub-1)"%(irun,runinfo[irun]['Lumi'])
   
    for idef in grlDef["intol"]+grlDef["tol"]:
      if (runinfo[irun]["lumiDefect_%s"%(idef)]>0.):
        defectFileName = "%s/loss-%s.dat"%(options['yearStatsDir'],idef)
        if idef in grlDef["intol"]:
          defString = "%s -> %.6f pb-1 (recov: %.6f pb-1)\n"%(irun_string,runinfo[irun]["lumiDefect_%s"%(idef)]/1e6,runinfo[irun]["lumiDefect_%s__recov"%(idef)]/1e6)
        else:
          defString = "%s -> %.6f pb-1\n"%(irun_string,runinfo[irun]["lumiDefect_%s"%(idef)]/1e6)
        toAdd = True
        if (os.path.exists(defectFileName)):# Check that the same defect was not already stored
          f2 = open(defectFileName,'r')
          if defString in f2.readlines(): 
            toAdd = False
          f2.close()
        if toAdd:
          f2 = open(defectFileName,'a')
          f2.write(defString)
          f2.close()

    if runinfo[irun]["ineffVeto_allVeto"]>0. : # Loss due to veto. Update the veto dat files
      for iVeto in veto["all"]:
        if (runinfo[irun]["lumiVeto_%s"%(iVeto)]>0.):
          vetoFileName = "%s/loss-%sVETO.dat"%(options['yearStatsDir'],iVeto)
          vetoString = "%s -> %.6f pb-1 \n"%(irun_string,runinfo[irun]["lumiVeto_%s"%(iVeto)]/1e6)
          toAdd = True
          if (os.path.exists(vetoFileName)):# Check that the same veto was not already stored
            f2 = open(vetoFileName,'r')
            if vetoString in f2.readlines(): 
              toAdd = False
            f2.close()
          if toAdd:
            f2 = open(vetoFileName,'a')
            f2.write(vetoString)
            f2.close()

errorLogFile.close()
notYetSignedOffRuns.close()

if options['updateYearStats']:
  os.system("rm -f %s"%tokenName)
if not options['batchMode']:
  input("I am done. Type <return> to exit...")
