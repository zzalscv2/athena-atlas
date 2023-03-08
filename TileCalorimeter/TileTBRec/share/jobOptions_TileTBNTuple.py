#*****************************************************************
#
# """topOptions file for Tile Testbeam Reconstruciton and Monitoring in Athena""" 
# """This topOptions is intended to test the monitoring code"""
#=================================================================

#xxx configure TileDQstatusAlg

from AthenaCommon.Logging import logging
log_tbntuple = logging.getLogger( 'jobOptions_TileTBNtuple.py' )

if not 'EvtMin' in dir():
    EvtMin = 0

if not 'EvtMax' in dir():
    EvtMax = 5000000

if not 'TestOnline' in dir():
    TestOnline = True

if not 'InputDirectory' in dir():
    InputDirectory = "/data1/daq"

if not 'FileName' in dir():
    FileName = 'ALL'

if not 'RunFromLocal' in dir():
    RunFromLocal = True

if not 'RunNumber' in dir():
    RunNumber = 0

if not 'Version' in dir():
    Version = ''

if 'FileFilter' in dir() and len(FileFilter)>0 and FileFilter != '.' and FileFilter != 'data' and FileFilter != '.data':
    Version += '_'+FileFilter

if not "histo_name" in dir():
    histo_name = "tilemon_%s%s.root" % (RunNumber,Version)

if not "ntuple_name" in dir():
    ntuple_name = "tiletb_%s%s.root" % (RunNumber,Version)

if len(Version)==0:
    Version = '0'

if not 'TileCisRun' in dir():
    TileCisRun = False

if not 'TileMonoRun' in dir():
    TileMonoRun = False

if not 'TilePedRun' in dir():
    TilePedRun = False

if not 'TilePhysRun' in dir():
    TilePhysRun = not (TileCisRun or TileMonoRun or TilePedRun)

if not 'TileFrameLength' in dir():
    TileFrameLength = 7

if not 'doTileOpt2' in dir():
    doTileOpt2 = not (TileCisRun or TileMonoRun)
    doTileOpt2 = (TileFrameLength == 7)

if not 'doTileFit' in dir():
    doTileFit = (TileCisRun or TileMonoRun)
    doTileFit = True

if not 'doTileOptATLAS' in dir():
    doTileOptATLAS = TilePedRun and (TileFrameLength == 7)

if not 'TileFELIX' in dir():
    TileFELIX = False

if not 'TileOfflineUnits' in dir():
    TileOfflineUnits = 0 if (TileCisRun or TileMonoRun or TilePedRun) else 3

TileCalibrateEnergy = (TileOfflineUnits!=0)
TileCalibMode = (TilePedRun or TileCisRun)
TilePMTOrder = not (TileCisRun or TileMonoRun or TilePedRun)
TileTBperiod = 2015 if (RunNumber/100000 == 5) else 2016

if TileFELIX:
    TileTBperiod = 2017

if RunNumber>800000:
    TileTBperiod += 2

if RunNumber >= 2200000:
    TileTBperiod = 2022
elif RunNumber >= 2110000:
    TileTBperiod = 2021

#---  Output printout level ----------------------------------- 
#output threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
if not 'OutputLevel' in dir():
    OutputLevel = 3
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False

include('TileMonitoring/jobOptions_TileTBMon.py')

if TileTBperiod < 2016:
    ByteStreamCnvSvc.ROD2ROBmap = [ "-1" ]

TileTBAANtuple = CfgMgr.TileTBAANtuple( TBperiod = TileTBperiod, CalibrateEnergy=TileCalibrateEnergy, OfflineUnits=TileOfflineUnits, CalibMode=TileCalibMode, PMTOrder=TilePMTOrder, NSamples = TileFrameLength, TileRawChannelContainerFlat = "", TileRawChannelContainerOpt="TileRawChannelOpt2" if doTileOpt2 else "", TileRawChannelContainerFit="TileRawChannelFit" if doTileFit else "", TileDigitsContainer = 'TileDigitsCnt' if not TileFELIX else 'TileDigitsFiltered', TileHitVector="", TileHitContainer="" )

if TileTBperiod >= 2021:
    # Starting from 2021 calibration constants for beam chambers should be provided via JO
    TileTBAANtuple.BC1Z  = 17348.8
    TileTBAANtuple.BC1Z_0  = 17348.8
    TileTBAANtuple.BC1Z_90  = 15594.05
    TileTBAANtuple.BC1Z_min90  = 15571.8

    TileTBAANtuple.BC2Z  = 4404.2
    TileTBAANtuple.BC2Z_0  = 4420.7
    TileTBAANtuple.BC2Z_90  = 2649.45
    TileTBAANtuple.BC2Z_min90  = 2627.2

    if RunNumber <= 2110503:
        # September TB2021
        TileTBAANtuple.BC1X1 = -0.156736
        TileTBAANtuple.BC1X2 = -0.178455
        TileTBAANtuple.BC1Y1 = -0.452977
        TileTBAANtuple.BC1Y2 = -0.17734

        TileTBAANtuple.BC2X1 = 2.88152
        TileTBAANtuple.BC2X2 = -0.187192
        TileTBAANtuple.BC2Y1 = -1.79832
        TileTBAANtuple.BC2Y2 = -0.190846
    elif RunNumber <= 2200000:
        # November TB2021
        TileTBAANtuple.BC1X1 = -0.0107769
        TileTBAANtuple.BC1X2 = -0.178204
        TileTBAANtuple.BC1Y1 = -0.625063
        TileTBAANtuple.BC1Y2 = -0.178842

        TileTBAANtuple.BC2X1 = 0.746517
        TileTBAANtuple.BC2X2 = -0.176083
        TileTBAANtuple.BC2Y1 = 0.49177
        TileTBAANtuple.BC2Y2 = -0.18221
    else:
        # June TB2022
        TileTBAANtuple.BC1X1 = -0.566211
        TileTBAANtuple.BC1X2 = -0.0513049
        TileTBAANtuple.BC1Y1 = 1.15376
        TileTBAANtuple.BC1Y2 = -0.0514167

        TileTBAANtuple.BC2X1 = -0.163869
        TileTBAANtuple.BC2X2 = -0.0523055
        TileTBAANtuple.BC2Y1 = -0.0602012
        TileTBAANtuple.BC2Y2 =  -0.0532108


topSequence += TileTBAANtuple
print(topSequence.TileTBAANtuple)

from GaudiSvc.GaudiSvcConf import THistSvc
svcMgr += THistSvc()
svcMgr.THistSvc.Output += [ f"AANT DATAFILE='{OutputDirectory}/{ntuple_name}' OPT='RECREATE' " ]
OutputDirectorysvcMgr.THistSvc.MaxFileSize = 32768

if TileCisRun or TileMonoRun:
    # CIS calibration using top calib alg
    from TileCalibAlgs.TileCalibAlgsConf import TileTopCalibAlg
    from TileCalibAlgs.TileCalibAlgsConf import TileCisDefaultCalibTool

    TileCalibAlg = TileTopCalibAlg()
    TileCalibAlg.RunNumber        = RunNumber
    TileCalibAlg.RunType          = 8
    TileCalibAlg.FileName = f'{OutputDirectory}/tileCalibCIS_{RunNumber}_CIS.{Version}.root'

    # declare CIS tool(s) and set jobOptions if necessary
    TileCisTool = TileCisDefaultCalibTool()

    TileCisTool.removePed = True
    from AthenaCommon.AppMgr import ToolSvc
    TileCalibAlg.TileCalibTools += [ TileCisTool ]
    TileCalibAlg.OutputLevel = WARNING

    topSequence += TileCalibAlg

if TilePedRun:
    #Add Noise Calib Tool
    from TileCalibAlgs.TileCalibAlgsConf import TileDigiNoiseCalibAlg
    theTileDigiNoiseCalibAlg = TileDigiNoiseCalibAlg( "theTileDigiNoiseCalibAlg" )
    theTileDigiNoiseCalibAlg.DoAvgCorr = False # False=> Full AutoCorr matrix calculation
    topSequence += theTileDigiNoiseCalibAlg

topSequence.TileRChMaker.OutputLevel = ERROR
print(topSequence)

svcMgr.EventSelector.SkipEvents = EvtMin
svcMgr.AthenaEventLoopMgr.EventPrintoutInterval = 100
theApp.EvtMax=EvtMax

# special settings for TB 2015, not needed in 2016
#ToolSvc.TileRawChannelBuilderFitFilter.FirstSample = 1
#ToolSvc.TileRawChannelBuilderFitFilter.FrameLength = 7
