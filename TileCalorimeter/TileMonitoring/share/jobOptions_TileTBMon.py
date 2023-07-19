#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

#*****************************************************************
#
# """topOptions file for Tile Laser Reconstruciton and Monitoring in Athena""" 
# """This topOptions is intended to test the monitoring code"""
#=================================================================

from __future__ import print_function

from os import system, popen
from AthenaCommon.Logging import logging
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaCommon.SystemOfUnits import GeV
log = logging.getLogger( 'jobOptions_TileTBMon.py' )

MaxEnergySetupFile = "/afs/cern.ch/user/t/tiledemo/public/efmon22/MaxEnergyInMonitoring.txt"

MonitorOutput = 'Tile'
TileDCS = False

if 'PublishName' in dir() and os.access( MaxEnergySetupFile, os.R_OK ):
    include( MaxEnergySetupFile )
    if 'MaximumChannelEnergy' in dir():
        MaxEnergy = MaximumChannelEnergy

    if 'MaximumTotalEnergy' in dir():
        MaxTotalEnergy = MaximumTotalEnergy

    log.info("Range in energy monitoring histograms are set up from file: %s", MaxEnergySetupFile)
else:
    log.info("No %s found => Default range will be used for energy monitoring histograms!", MaxEnergySetupFile)

if not 'MaxEnergy' in dir():
    MaxEnergy = 60.0
if not 'MaxTotalEnergy' in dir():
    MaxTotalEnergy = 100.0

if not 'CellEnergyThreshold' in dir():
    if not 'beam_energy' in dir():
        CellEnergyThreshold = 0.1
    else:
        CellEnergyThreshold = beam_energy * 0.01

log.info("MaxEnergy = %f, MaxTotalEnergy = %f", MaxEnergy, MaxTotalEnergy)

if not 'TestOnline' in dir():
    TestOnline = False

if TestOnline:
    doOnline = True;
    storeHisto = True;

if not 'TileFELIX' in dir():
    if 'PublishName' in dir():
        TileFELIX = False
    else:
        TileFELIX = False

if not 'TileTBperiod' in dir():
    TileTBperiod = 2015 if (RunNumber/100000 == 5) else 2016

    if TileFELIX:
        TileTBperiod = 2017

    if RunNumber > 800000:
        TileTBperiod += 2

    if RunNumber >= 2200000:
        TileTBperiod = 2022
    elif RunNumber >= 2110000:
        TileTBperiod = 2021

if not 'UseDemoCabling' in dir():
    UseDemoCabling = 2016

if TileFELIX:
    UseDemoCabling = 2017

if not 'InputDirectory' in dir():
    InputDirectory = "/data1/daq"    

if not 'TileBiGainRun' in dir():
    TileBiGainRun = False

if not 'TileMonoRun' in dir():
    TileMonoRun = False

if TileMonoRun:
    TileRunType = 8
    TileBiGainRun = False

if not 'TilePedRun' in dir():
    TilePedRun = False

if TilePedRun:
    TileRunType = 4
    TileBiGainRun = True

if not 'TileCisRun' in dir():
    TileCisRun = False

if TileCisRun:
    TileRunType = 8
    TileBiGainRun = True

def FindFile(path, runinput):

    run = str(runinput)

    while len(run) < 7:
        run = '0' + run
        
    files = []
    fullname = []

    if path.startswith("/castor") :
        for f in popen('nsls %(path)s | grep %(run)s' % {'path': path, 'run':run }):
            files.append(f)

    elif path.startswith("/eos") :
        for f in popen('xrdfs eosatlas ls %(path)s | grep -v "#" | sed "s|^.*/||" | grep %(run)s' % {'path': path, 'run':run }):
            files.append(f)

    else:
        for f in popen('ls  %(path)s | grep %(run)s' % {'path': path, 'run':run }):
            files.append(f)
            
    files=list(dict.fromkeys(files))
    for nn in range(len(files)):
        temp = files[nn].split('\n')
        name=path + '/' + temp[0]
        if name not in fullname:
            fullname.append(name)
        else:
            print(name,"allready present in fullname list")

    return [fullname,run]

# include Flags jobOption
include( "TileMonitoring/TileRec_FlagOptions.py" )

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
if not 'TileSummaryUpdateFrequency' in dir():
    if doStateless:
        TileSummaryUpdateFrequency = 100
    else:
        TileSummaryUpdateFrequency = 0

## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# Get a handle to the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
from AthenaCommon import CfgMgr
toolSvc = CfgMgr.ToolSvc()


# set global flags
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('commis')
globalflags.DataSource.set_Value_and_Lock('data')
globalflags.InputFormat.set_Value_and_Lock('bytestream')

from AthenaCommon.BeamFlags import jobproperties     
jobproperties.Beam.beamType.set_Value_and_Lock(beamType)

# reset everything which is not needed
from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOff()  #Switched off to avoid geometry
DetFlags.ID_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

DetFlags.detdescr.ID_setOff()
DetFlags.detdescr.Muon_setOff()
DetFlags.detdescr.LAr_setOff()
DetFlags.detdescr.Tile_setOn()
DetFlags.readRDOBS.Tile_setOn()

# Switch off DCS
DetFlags.dcs.Tile_setOff()

DetFlags.Print()

from RecExConfig.RecFlags import rec
rec.doLArg = False

# set online flag if neeed

if athenaCommonFlags.isOnline() or doOnline or doStateless:
    athenaCommonFlags.isOnline=True
    log.info( 'athenaCommonFlags.isOnline = True : Online Mode' )
    if doStateless:
        athenaCommonFlags.isOnlineStateless=True
        log.info( 'athenaCommonFlags.isOnlineStateless = True : Stateless Online Mode' )


#-----------------
# ByteSream Input 
#-----------------

if not athenaCommonFlags.isOnline() or TestOnline:

    include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )
    include( "ByteStreamCnvSvcBase/BSAddProvSvc_RDO_jobOptions.py" )

    if not 'InputDirectory' in dir():
        InputDirectory="/castor/cern.ch/grid/atlas/t0/perm/DAQ"
    if not 'RunNumber' in dir():
        RunNumber=0
    if not 'RunFromLocal' in dir():
        if InputDirectory=="." or RunNumber<10:
            RunFromLocal=True
        else:
            RunFromLocal=False
   
    if not 'FileNameVec' in dir():
        if not 'FileName' in dir():

            tmp = FindFile(InputDirectory,RunNumber)
            FileNameVec  = tmp[0]
            FormattedRunNumber = tmp[1]
            
        else:
            FileNameVec = [ InputDirectory+'/'+FileName ]
            FormattedRunNumber = RunNumber
    else:
        FormattedRunNumber = RunNumber

    svcMgr.EventSelector.SkipEvents = EvtMin
    theApp.EvtMax = EvtMax

    if  'beam_energy' not in dir():
        energy = GetFileMD(FileNameVec).get('beam_energy', 0)
        if energy > 0:
            beam_energy = energy
            log.info( "Beam energy auto configured is " + str(beam_energy) + " GeV")

    log.info( "InputDirectory is " + str(InputDirectory) )
    log.info( "RunNumber is " + str(FormattedRunNumber) )
    log.info( "FullFileName is " + str(FileNameVec) )
    log.info( "Skip Events is " + str(EvtMin) )
    log.info( "Max events is " + str(EvtMax) )

    svcMgr.EventSelector.Input = FileNameVec
    svcMgr.EventSelector.MaxBadEvents = MaxBadEvents
   
    athenaCommonFlags.FilesInput = FileNameVec

    projectName = FileNameVec[0].split('/').pop().split('.')[0]
    log.info( "Project name is " + projectName )
    rec.projectName = projectName


# init DetDescr
from AthenaCommon.GlobalFlags import jobproperties
if not 'DetDescrVersion' in dir():
    DetDescrVersion = 'ATLAS-R2-2015-04-00-00'
jobproperties.Global.DetDescrVersion = DetDescrVersion 
log.info( "DetDescrVersion = %s" % (jobproperties.Global.DetDescrVersion() ))

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
GeoModelSvc.IgnoreTagDifference = True
log.info( "GeoModelSvc.AtlasVersion = %s" % (GeoModelSvc.AtlasVersion) )

# Setup Db stuff
if TileUseCOOL:
    from IOVDbSvc.CondDB import conddb
    log.info( 'Tile COOL tag: ' + tileCOOLtag )
    conddb.setGlobalTag(tileCOOLtag)

    db = '/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db'
    from TileConditions.TileCoolMgr import tileCoolMgr
    tileCoolMgr.setGlobalDbConn(db)




# setting option to build frag->ROB mapping at the begin of run
ByteStreamCnvSvc = Service( "ByteStreamCnvSvc" )

ByteStreamCnvSvc.ROD2ROBmap = [
    # frag-to-ROB mapping, bypassing ROD ID which is not unique
    "0xff",  "0x500000",
    "0x13",  "0x500000",
    "0x1",   "0x500000",
    "0x2",   "0x500000",
    "0x3",   "0x500000",
    "0x200", "0x0",
    "0x201", "0x0",
    ]

if RunNumber < 600000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x0"]
elif RunNumber >= 600000 and RunNumber < 611300:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x101", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x0"]
elif RunNumber >= 611300 and RunNumber < 800000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x100", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x0"]
elif RunNumber >= 800000 and RunNumber < 2110000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x100", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x101", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x10"]
    if UseDemoCabling == 2016:
        UseDemoCabling = 2018
    if UseDemoCabling == 2017:
        UseDemoCabling = 2019
elif RunNumber >= 2110000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x100", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x101", "0x1"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x200", "0x0"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x201", "0x0"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x10"]
    ByteStreamCnvSvc.ROD2ROBmap += ["0x12", "0x500000"]

if RunNumber >= 2200000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x10", "0x500000"]
    if RunNumber >= 2210456:
        UseDemoCabling = 2018

if RunNumber >= 2310000:
    ByteStreamCnvSvc.ROD2ROBmap += ["0x15",  "0x500000"]

if TileFELIX:
    if RunNumber < 2310000:
        ByteStreamCnvSvc.ROD2ROBmap += ["0x203", "0x500006"]
    elif RunNumber < 2310439:
        ByteStreamCnvSvc.ROD2ROBmap += ["0x201", "0x2"]
    else:
        ByteStreamCnvSvc.ROD2ROBmap += ["0x201", "0x520002"]
        ByteStreamCnvSvc.ROD2ROBmap += ["0x402", "0x540003"]

if 'PublishName' in dir():
    doTileOptATLAS = False
    doTileOpt2 = False
    doTileFit = True
    doTileCalib = False

if not 'TileCorrectTime' in dir():
    TileCorrectTime = False
if not 'doTileOptATLAS' in dir():
    doTileOptATLAS = False

if TileFELIX:
    doTileOpt2 = False
    doTileFit = True
    topSequence += CfgMgr.TileDigitsGainFilter(HighGainThreshold = 4095)

# load conditions data
include( "TileRec/TileDefaults_jobOptions.py" )
include( "TileConditions/TileConditions_jobOptions.py" )


# set reconstruction flags and reconstruct data
from TileRecUtils.TileRecFlags import jobproperties
jobproperties.TileRecFlags.calibrateEnergy.set_Value_and_Lock(False) #don't need pC in raw channels, keep ADC counts
jobproperties.TileRecFlags.noiseFilter.set_Value_and_Lock(0) #Enable noise filter tool
jobproperties.TileRecFlags.correctTimeJumps.set_Value_and_Lock(False)
jobproperties.TileRecFlags.BestPhaseFromCOOL.set_Value_and_Lock(False) #Use best phase from COOL
jobproperties.TileRecFlags.doTileOverflowFit.set_Value_and_Lock(False)
include( "TileRec/TileRec_jobOptions.py" )


if doTileCells:
    # enable interpolation for dead cells
    doCaloNeighborsCorr = False
    if not 'MaskBadChannels' in dir():
        MaskBadChannels = False
    if TileBiGainRun:
        include( "TileRec/TileCellMaker_jobOptions_doublegain.py" )
        topSequence.CaloCellMakerHG.CaloCellMakerToolNames["TileCellBuilderHG"].UseDemoCabling = UseDemoCabling
        topSequence.CaloCellMakerLG.CaloCellMakerToolNames["TileCellBuilderLG"].UseDemoCabling = UseDemoCabling
        topSequence.CaloCellMakerHG.CaloCellMakerToolNames["TileCellBuilderHG"].mergeChannels = False
        topSequence.CaloCellMakerLG.CaloCellMakerToolNames["TileCellBuilderLG"].mergeChannels = False
    else:
        include('TileRec/TileCellMaker_jobOptions.py')
        topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].UseDemoCabling = UseDemoCabling
        topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].maskBadChannels = MaskBadChannels
        topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].mergeChannels = False

from TileRecUtils.TileDQstatusAlgDefault import TileDQstatusAlgDefault
TileDQstatusAlgDefault()

#----------------
# TileMonitoring
#----------------
if doMonitoring:

    if TileBiGainRun:
        CellContainerMonitored = 'AllCaloHG'
    else:
        CellContainerMonitored = 'AllCalo'

    topSequence += CfgMgr.AthenaMonManager( "TileTBMonManager"
                                            , ManualRunLBSetup    = True
                                            , ManualDataTypeSetup = True
                                            , Environment         = "online"
                                            , FileKey             = MonitorOutput
                                            , Run                 = RunNumber
                                            , LumiBlock           = 1)


    #-------------------------------
    #   Tile raw channel time monitoring
    #-------------------------------
    TileTBMonTool = CfgMgr.TileTBMonTool ( name                  = 'TileTBMonTool'
                                           , histoPathBase       = '/Tile/TestBeam'
                                           # , doOnline            = athenaCommonFlags.isOnline()
                                           , CellsContainerID  = '' # used to check if the current event is collision
                                           , MBTSCellContainerID = '' # used to check if the current event is collision
                                           # Masked format: 'module gain channel,channel' (channels are separated by comma)
                                           , Masked = ['LBC04 0 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47',
                                                       'LBC04 1 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47',
                                                       'LBA01 0 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47',
                                                       'LBA01 1 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47',
                                                       'LBC01 0 36,37,38,39,40,41',
                                                       'LBC01 1 36,37,38,39,40,41']
                                           , CellContainer       = CellContainerMonitored
                                           , MaxTotalEnergy      = MaxTotalEnergy
                                           , CellEnergyThreshold = CellEnergyThreshold)

    if 'beam_energy' in dir() and beam_energy > 0:
        TileTBMonTool.BeamEnergy = beam_energy * 1000

    topSequence.TileTBMonManager.AthenaMonTools += [ TileTBMonTool ]
    
    TileTBBeamMonTool = CfgMgr.TileTBBeamMonTool ( name                  = 'TileTBBeamMonTool'
                                                   , histoPathBase       = '/Tile/TestBeam/BeamElements'
                                                   , TBperiod = TileTBperiod
                                                   # , doOnline            = athenaCommonFlags.isOnline()
                                                   , CellsContainerID  = '' # used to check if the current event is collision
                                                   , MBTSCellContainerID = '' # used to check if the current event is collision
                                                   , CutEnergyMin = 40
                                                   , CutEnergyMax = 70
                                                   , CellContainer       = CellContainerMonitored )

    if TileTBperiod >= 2021:
        TileTBBeamMonTool.BC1Z = 17348.8
        TileTBBeamMonTool.BC2Z = 4404.2
        if RunNumber <= 2110503:
            # September TB2021
            TileTBBeamMonTool.BC1HorizontalOffset = -0.156736
            TileTBBeamMonTool.BC1HorizontalSlope = -0.178455
            TileTBBeamMonTool.BC1VerticalOffset = -0.452977
            TileTBBeamMonTool.BC1VerticalSlope = -0.17734
            TileTBBeamMonTool.BC2HorizontalOffset = 2.88152
            TileTBBeamMonTool.BC2HorizontalSlope = -0.187192
            TileTBBeamMonTool.BC2VerticalOffset = -1.79832
            TileTBBeamMonTool.BC2VerticalSlope = -0.190846
        elif RunNumber < 2200000:
            # November TB2021
            TileTBBeamMonTool.BC1HorizontalOffset = -0.0107769
            TileTBBeamMonTool.BC1HorizontalSlope = -0.178204
            TileTBBeamMonTool.BC1VerticalOffset = -0.625063
            TileTBBeamMonTool.BC1VerticalSlope = -0.178842
            TileTBBeamMonTool.BC2HorizontalOffset = 0.746517
            TileTBBeamMonTool.BC2HorizontalSlope = -0.176083
            TileTBBeamMonTool.BC2VerticalOffset = +0.49177
            TileTBBeamMonTool.BC2VerticalSlope = -0.18221
        elif RunNumber < 2310000:
            # June TB2022
            TileTBBeamMonTool.BC1HorizontalOffset = -0.566211
            TileTBBeamMonTool.BC1HorizontalSlope = -0.0513049
            TileTBBeamMonTool.BC1VerticalOffset = 1.15376
            TileTBBeamMonTool.BC1VerticalSlope = -0.0514167
            TileTBBeamMonTool.BC2HorizontalOffset = -0.163869
            TileTBBeamMonTool.BC2HorizontalSlope = -0.0523055
            TileTBBeamMonTool.BC2VerticalOffset = -0.0602012
            TileTBBeamMonTool.BC2VerticalSlope =  -0.0532108
        else:
            # July TB2023
            TileTBBeamMonTool.BC1HorizontalOffset = 2.465295
            TileTBBeamMonTool.BC1HorizontalSlope = -0.072135
            TileTBBeamMonTool.BC1VerticalOffset = 2.127854
            TileTBBeamMonTool.BC1VerticalSlope = -0.073442
            TileTBBeamMonTool.BC2HorizontalOffset = -0.317171
            TileTBBeamMonTool.BC2HorizontalSlope = -0.075384
            TileTBBeamMonTool.BC2VerticalOffset = 1.875657
            TileTBBeamMonTool.BC2VerticalSlope = -0.076717

            TileTBBeamMonTool.MaskMuonPMTs = [7] # Mask PMTs in total energy for Muon Wall


    topSequence.TileTBMonManager.AthenaMonTools += [ TileTBBeamMonTool ]
    

    TimeRange = [-100, 100]
    if 'TileFrameLength' in dir():
        if TileFrameLength == 9:
            TimeRange = [-125, 125]
        elif TileFrameLength == 15:
            TimeRange = [-200, 200]
    TileTBPulseMonTool = CfgMgr.TileTBPulseMonTool ( name                  = 'TileTBPulseMonTool'
                                                     , histoPathBase       = '/Tile/TestBeam/PulseShape'
                                                     , UseDemoCabling = UseDemoCabling
                                                     , TimeRange = TimeRange
                                                     , TileRawChannelContainer = jobproperties.TileRecFlags.TileRawChannelContainer() )

    topSequence.TileTBMonManager.AthenaMonTools += [ TileTBPulseMonTool ]

    if TileBiGainRun:
        TileTBCellMonToolHG = CfgMgr.TileTBCellMonTool ( name                  = 'TileTBCellMonToolHG'
                                                         , histoPathBase       = '/Tile/TestBeam'
                                                         # , doOnline            = athenaCommonFlags.isOnline()
                                                         , CellsContainerID  = '' # used to check if the current event is collision
                                                         , MBTSCellContainerID = '' # used to check if the current event is collision
                                                         , cellsContainerName       = 'AllCaloHG'
                                                         , FillTimeHistograms = True
                                                         , energyThresholdForTime = 1.0
                                                         , MaxEnergy = MaxEnergy
                                                         , MaxTotalEnergy = MaxTotalEnergy
                                                         , TimeRange = TimeRange)
        
        topSequence.TileTBMonManager.AthenaMonTools += [ TileTBCellMonToolHG ]
        
        
        TileTBCellMonToolLG = CfgMgr.TileTBCellMonTool ( name                  = 'TileTBCellMonToolLG'
                                                         , histoPathBase       = '/Tile/TestBeam/LG'
                                                         # , doOnline            = athenaCommonFlags.isOnline()
                                                         , CellsContainerID  = '' # used to check if the current event is collision
                                                         , MBTSCellContainerID = '' # used to check if the current event is collision
                                                         , cellsContainerName       = 'AllCaloLG'
                                                         , FillTimeHistograms = True
                                                         , energyThresholdForTime = 1.0
                                                         , MaxEnergy = MaxEnergy
                                                         , MaxTotalEnergy = MaxTotalEnergy
                                                         , TimeRange = TimeRange)
        
        topSequence.TileTBMonManager.AthenaMonTools += [ TileTBCellMonToolLG ]

    else:
        TileTBCellMonTool = CfgMgr.TileTBCellMonTool ( name                  = 'TileTBCellMonTool'
                                                       , histoPathBase       = '/Tile/TestBeam'
                                                       # , doOnline            = athenaCommonFlags.isOnline()
                                                       , CellsContainerID  = '' # used to check if the current event is collision
                                                       , MBTSCellContainerID = '' # used to check if the current event is collision
                                                       , cellsContainerName       = 'AllCalo'
                                                       , FillTimeHistograms = True
                                                       , energyThresholdForTime = 1.0
                                                       , MaxEnergy = MaxEnergy
                                                       , MaxTotalEnergy = MaxTotalEnergy
                                                       , TimeRange = TimeRange)

        topSequence.TileTBMonManager.AthenaMonTools += [ TileTBCellMonTool ]

    

    if (TileMonoRun or TileCisRun or TilePedRun) or True:
        TileRawChannelMon = CfgMgr.TileRawChannelMonTool ( name              = "TileRawChannelMon"
                                                           , OutputLevel     = WARNING
                                                           , histoPathBase   = "/Tile/RawChannel"
                                                           , book2D          = True if ('TileCisRun' in dir() and TileCisRun) else False
                                                           , PlotDSP         = False
                                                           , runType         = 9 if TileMonoRun else TileRunType
                                                           , TileRawChannelContainer = 'TileRawChannelFit' if (doTileFit and not TilePedRun) else 'TileRawChannelOpt2'
                                                           , SummaryUpdateFrequency = TileSummaryUpdateFrequency)
        topSequence.TileTBMonManager.AthenaMonTools += [ TileRawChannelMon ]


    if TilePedRun:
        TileDigiNoiseMon = CfgMgr.TileDigiNoiseMonTool(name               = 'TileDigiNoiseMon'
                                                       , OutputLevel        = WARNING
                                                       , TileDigitsContainer = "TileDigitsCnt"
                                                       , histoPathBase = "/Tile/DigiNoise"
                                                       , FillEmptyFromDB = False
                                                       , FillPedestalDifference = False
                                                       , CheckDCS           = TileUseDCS
                                                       , SummaryUpdateFrequency = TileSummaryUpdateFrequency );
        
        topSequence.TileTBMonManager.AthenaMonTools += [ TileDigiNoiseMon ];

    print(topSequence.TileTBMonManager)


import os
# -- use root histos --
# THistService for native root in Athena
if not  athenaCommonFlags.isOnline() or storeHisto or athenaCommonFlags.isOnlineStateless():
    if not hasattr(svcMgr, 'THistSvc'):
        from GaudiSvc.GaudiSvcConf import THistSvc
        svcMgr += THistSvc('THistSvc')
    if os.path.exists(RootHistOutputFileName):
        os.remove(RootHistOutputFileName)
    svcMgr.THistSvc.Output = [MonitorOutput + " DATAFILE='" + RootHistOutputFileName + "' OPT='RECREATE'"]
else:
    from TrigServices.TrigServicesConf import TrigMonTHistSvc
    trigmonTHistSvc = TrigMonTHistSvc("THistSvc")
    svcMgr += trigmonTHistSvc



#To read CTP RESULTS and DSP Raw Channels
if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
    from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc
    svcMgr += ByteStreamAddressProviderSvc()

svcMgr.ByteStreamAddressProviderSvc.TypeNames += [
                                                   "TileRawChannelContainer/TileRawChannelCnt",
                                                   "CTP_RDO/CTP_RDO",
                                                   "CTP_RIO/CTP_RIO",
                                                  ]


svcMgr.MessageSvc.defaultLimit= MsgLinesLimit
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.Format = "% F%35W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = useColors
#svcMgr.HistorySvc.OutputLevel = 3

theApp.EvtMax = EvtMax

from AthenaServices.AthenaServicesConf import AthenaEventLoopMgr
svcMgr += AthenaEventLoopMgr()
svcMgr.AthenaEventLoopMgr.EventPrintoutInterval = 100

if TileUseCOOL:
    from DBReplicaSvc.DBReplicaSvcConf import DBReplicaSvc
    svcMgr += DBReplicaSvc(UseCOOLSQLite=False)

if hasattr (svcMgr.ToolSvc, 'TileRawChannelBuilderOpt2Filter') and False:
    svcMgr.ToolSvc.TileRawChannelBuilderOpt2Filter.OutputLevel = DEBUG


if hasattr (svcMgr.ToolSvc, 'TileCellBuilder') and False:
    svcMgr.ToolSvc.TileCellBuilder.OutputLevel = DEBUG



if TileFELIX:
    ServiceMgr.TileInfoLoader.NSamples = 16
    ServiceMgr.TileInfoLoader.TrigSample = 3
    topSequence.TileRChMaker.TileDigitsContainer = 'TileDigitsFiltered'
    TileRawChannelBuilderFitFilter = topSequence.TileRChMaker.TileRawChannelBuilder['TileRawChannelBuilderFitFilter']
    TileRawChannelBuilderFitFilter.ExtraSamplesRight = 0
    TileRawChannelBuilderFitFilter.ExtraSamplesLeft = 0
    TileRawChannelBuilderFitFilter.MaxIterate = 9
    TileRawChannelBuilderFitFilter.SaturatedSample = 4095
    #TileRawChannelBuilderFitFilter.OutputLevel = DEBUG

#toolSvc += CfgMgr.TileROD_Decoder(OutputLevel = VERBOSE)
