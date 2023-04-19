#Author: Will Buttinger

#This is the configuration file for an athena job using SUSYTools to calibrate physics objects
#You would use this joboption by copying it and substituting the TestAlg for your own algorithm
#and subtituting your own input files

import AthenaPoolCnvSvc.ReadAthenaPool #read xAOD files

theApp.EvtMax = 500 #set to -1 to run on all events

# some defaults
if not 'MCCampaign' in dir():
    MCCampaign = 'mc20e'
    print(f'set default MCCampaign={MCCampaign}')
sample_format = "DAOD_PHYS"
if not 'IsPHYSLITE' in dir():
    IsPHYSLITE = False
    print(f'set default IsPHYSLITE={IsPHYSLITE}')
    sample_format = "DAOD_PHYSLITE"
if not 'pTag' in dir():
    pTag = 'p5511' if not 'data22' in MCCampaign else 'p5514'
    print(f'set default pTag={pTag}')
if not 'doSyst' in dir():
    doSyst = False
    print(f'set default doSyst={doSyst}')

# some inputs
inputDir = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools'
inputFiles = {}
inputFiles['data18'] = f'DAOD_PHYS.data18_13TeV.00356250_{pTag}.pool.root'
inputFiles['data22'] = f'DAOD_PHYS.data22_13p6TeV.00440543_{pTag}.pool.root'
inputFiles['mc20e'] = f'DAOD_PHYS.mc20_13TeV.410470.FS_mc20e_{pTag}.PHYS.pool.root'
inputFiles['mc21a'] = f'DAOD_PHYS.mc21_13p6TeV.601229.FS_mc21a_{pTag}.PHYS.pool.root'

inputPath = f'{inputDir}/{inputFiles[MCCampaign]}'
if IsPHYSLITE: 
    inputPathLITE = str(inputPath).replace("PHYS.","PHYSLITE.")
    inputPath = inputPathLITE
print(f'inputPath = {inputPath}')
AFII = True if 'AFII' in inputPath else False
print(f'set default AFII = {AFII}')

# setup
svcMgr.EventSelector.InputCollections = [ inputPath ] #specify input files here, takes a list
svcMgr.MessageSvc.OutputLevel = INFO

ToolSvc += CfgMgr.ST__SUSYObjDef_xAOD("SUSYTools")

ToolSvc.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default.conf" # Grab the default config file
if 'mc21' in MCCampaign or 'data2' in MCCampaign:
    ToolSvc.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default_Run3.conf" # or the Run 3 flavour
if IsPHYSLITE:
    STconfig_lite = str(ToolSvc.SUSYTools.ConfigFile).replace(".conf","_LITE.conf")
    ToolSvc.SUSYTools.ConfigFile = STconfig_lite
    ToolSvc.SUSYTools.IsPHYSLITE = True

ToolSvc.SUSYTools.AutoconfigurePRWTool = True
# set lumicalc info
PRWLumiCalc = {}
if 'mc' in MCCampaign:
    PRWLumiCalc['mc20a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root',
                            '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root']
    PRWLumiCalc['mc20d'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root']
    PRWLumiCalc['mc20e'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root']
    PRWLumiCalc['mc21a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data22_13p6TeV/20221219/ilumicalc_histograms_None_428648-440613_OflLumi-Run3-002.root']
    ToolSvc.SUSYTools.PRWLumiCalcFiles = PRWLumiCalc[MCCampaign]


ToolSvc.SUSYTools.DataSource = 0 if 'data' in MCCampaign else (1 if not AFII else 2) # data/FS/atlfast


algseq = CfgMgr.AthSequencer("AthAlgSeq") #The main alg sequence

algseq += CfgMgr.SUSYToolsAlg("STAlg",RootStreamName="MYSTREAM") #Substitute your alg here

algseq.STAlg.SUSYTools = ToolSvc.SUSYTools
algseq.STAlg.DoSyst = doSyst
algseq.STAlg.OutputLevel = INFO


svcMgr.MessageSvc.Format = "% F%50W%S%7W%R%T %0W%M" #Creates more space for displaying tool names
svcMgr += CfgMgr.AthenaEventLoopMgr(EventPrintoutInterval=100) #message every 100 events processed

#this is for the output of histograms
svcMgr += CfgMgr.THistSvc()
svcMgr.THistSvc.Output += [ "MYSTREAM DATAFILE='hist-Ath_%s_%s.root' OPT='RECREATE'" %(MCCampaign,sample_format) ]
