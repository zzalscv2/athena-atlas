# Author: N. Gollub (nils.gollub@cern.ch)
# main job option to setup TileConditions
#
from __future__ import division
include.block ("TileConditions/TileConditions_jobOptions.py")
from AthenaCommon.Logging import logging
msg = logging.getLogger( 'TileConditions_jobOptions.py' )

from TileConditions.TileInfoConfigurator import TileInfoConfigurator
tileInfoConfigurator = TileInfoConfigurator()

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
if athenaCommonFlags.isOnline():
    rn = None
elif 'RunNumber' in dir():
    rn = RunNumber
elif '_run_number' in dir():
    rn = _run_number
else:
    rn = None

import re
from AthenaCommon.GlobalFlags import globalflags
gbltg=globalflags.DetDescrVersion()
patt = re.compile(r'-([A-Z]+.)-')
ss = patt.search(gbltg)
if (type(ss) != type(None)):
    version = ss.group(1)
    if (version=='CSC'):
        msg.warning("Old geometry tag detected %s - will not use COOL DB for TileCal calibration" % gbltg)
        TileUseCOOL = False
        TileFrameLength = 9

if not 'TileCablingType' in dir():
    if not athenaCommonFlags.isOnline():
        if rn is None:
            try:
                from Digitization.DigitizationFlags import digitizationFlags
                if digitizationFlags.dataRunNumber.statusOn:
                    rn = digitizationFlags.dataRunNumber()
                if rn is None and digitizationFlags.RunAndLumiOverrideList.statusOn:
                    rn = digitizationFlags.RunAndLumiOverrideList.getMinMaxRunNumbers()[0]
            except:
                msg.info("No DigitizationFlags available - looks like HLT job")
        if rn is None:
            try:
                from G4AtlasApps.SimFlags import simFlags
                if simFlags.RunNumber.statusOn:
                    rn = simFlags.RunNumber()
                if rn is None and simFlags.RunAndLumiOverrideList.statusOn:
                    rn = simFlags.RunAndLumiOverrideList.getMinMaxRunNumbers()[0]
            except:
                msg.info("No SimFlags available - looks like HLT job")
        if rn is None:
            try:
                from RecExConfig.AutoConfiguration import GetRunNumber
                rn=GetRunNumber()
            except:
                msg.info("No Run Number available - assume latest cabling")

    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as geoFlags
    if geoFlags.Run()=="RUN1":
        if rn>219651: # choose RUN2 cabling for old geometry tags starting from 26-MAR-2013 
            TileCablingType = 4 
            msg.warning("Forcing RUN2 cabling for run %s with geometry %s" % (rn,gbltg) )
    elif geoFlags.Run()=="RUN2":
        if rn==None or (globalflags.DataSource()!='data' and rn>=310000) or rn>=343000 or rn<1: # choose RUN2a cabling for R2 geometry tags starting from 31-Jan-2018
            TileCablingType = 5
            msg.info("Forcing RUN2a (2018) cabling for run %s with geometry %s" % (rn,gbltg) )
        else:
            TileCablingType = 4
            msg.info("Forcing RUN2 (2014-2017) cabling for run %s with geometry %s" % (rn,gbltg) )
    elif geoFlags.Run()=="RUN3":
        TileCablingType = 6
        msg.info("Forcing RUN3 cabling for run %s with geometry %s" % (rn,gbltg) )

if 'TileCablingType' in dir():
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    svcMgr.TileCablingSvc.CablingType = TileCablingType

if not 'TileFrameLength' in dir():
    TileFrameLength = 7; # unique default value for simulation and reconstruction

if not 'TileUseCOOL' in dir():
    TileUseCOOL=True; # use COOL DB by default for everything

if not 'TileUseDCS' in dir():
    TileUseDCS = TileUseCOOL and (not athenaCommonFlags.isOnline()) and globalflags.DataSource()=='data'

    if TileUseDCS:
        from AthenaCommon.DetFlags import DetFlags
        from RecExConfig.RecFlags import rec
        TileUseDCS = DetFlags.dcs.Tile_on() and rec.doESD and (not rec.readESD)
        TileCheckOFC=True

    if TileUseDCS:
        if rn is None:
            from RecExConfig.AutoConfiguration import GetRunNumber
            rn=GetRunNumber()
        TileUseDCS = ((rn>171194 and rn<222222) or rn>232498); # use DCS only for 2011 data and later, excluding shutdown period

if TileUseDCS or ('TileCheckOFC' in dir() and TileCheckOFC) or ('RunOflOFC' in dir()):
    if rn is None:
        from RecExConfig.AutoConfiguration import GetRunNumber
        rn=GetRunNumber()  # This may return None
    if not 'RunOflOFC' in dir():
        RunOflOFC=314450
    if rn and rn<RunOflOFC: # use OFC stored in online folder for all runs before 2017
        from TileConditions.TileCoolMgr import tileCoolMgr
        tileCoolMgr.addSource('OfcOf2Phy', '/TILE/ONL01/FILTER/OF2/PHY', 'TILE', "", '/TILE/ONL01/FILTER/OF2/PHY', 'SplitMC')
        tileCoolMgr.addSource('OfcOf1Phy', '/TILE/ONL01/FILTER/OF1/PHY', 'TILE', "", '/TILE/ONL01/FILTER/OF1/PHY', 'SplitMC')

msg.info("Adjusting TileInfo for %s samples" % TileFrameLength )
tileInfoConfigurator.NSamples = TileFrameLength
tileInfoConfigurator.TrigSample = (TileFrameLength-1)//2 # Floor division

if TileUseCOOL or athenaCommonFlags.isOnline():
    #=== setup reading from COOL DB
    msg.info("setting up COOL for TileCal conditions data")
    TileGapTiming=""
    if (('TileCisRun' in dir()) and (TileCisRun) or
        ('TileMonoRun' in dir()) and (TileMonoRun) or
        ('TileRampRun' in dir()) and (TileRampRun)):
        TilePulse="CIS"
    elif ('TileLasRun' in dir()) and (TileLasRun):
        TilePulse="LAS"
        if ('TilePhysTiming' in dir()) and (TilePhysTiming):
            TileGapTiming="GAP"
    else:
        TilePulse="PHY"
    if not 'TileUseCOOLOFC' in dir():
        TileUseCOOLOFC=True; # read OFC from COOL
    if not 'TileUseCOOLPULSE' in dir() or not TileUseCOOLOFC:
        TileUseCOOLPULSE=True; # read pulse from COOL

    tileInfoConfigurator.setupCOOL(type=(TileGapTiming+TilePulse))

    if TileUseDCS:
        tileInfoConfigurator.setupCOOLDCS();
else:
    msg.warning("COOL is not used for TileCal conditions data")
    TileUseCOOLOFC=False
    tileInfoConfigurator.setupCOOLPHYPULSE()
    tileInfoConfigurator.setupCOOLAutoCr()

    msg.info("Reading TileCal bad channel list from TileDefault.onlBch and TileDefault.oflBch files")
    from TileConditions.TileCondToolConf import getTileBadChanTool
    getTileBadChanTool('FILE')

# fine-tune CellNoise values depending on beam type
if not 'TileCommissioning' in dir():
    from AthenaCommon.BeamFlags import jobproperties
    if jobproperties.Beam.beamType != 'collisions':
        TileCommissioning = True
    else:
        TileCommissioning = False
        
if TileCommissioning:
    msg.info("Adjusting TileInfo to return cell noise for Opt.Filter with iterations")
    tileInfoConfigurator.NoiseScaleIndex = 2; # Noise for Optimal Filter with iterations
else:
    msg.info("Adjusting TileInfo to return cell noise for Opt.Filter without iterations")
    tileInfoConfigurator.NoiseScaleIndex = 1; # Noise for Optimal Filter without iterations

# setup for 12-bit ADCs
TileUse12bit = False
if not TileUse12bit:
    msg.info("Setting 10-bit ADC configuration")
    tileInfoConfigurator.setupAdcRange(10)
else:
    msg.info("Setting 12-bit ADC configuration")
    tileInfoConfigurator.setupAdcRange(12)

from AthenaCommon.GlobalFlags import globalflags
if globalflags.DataSource() != 'data':
    # Set up Tile samping fraction for MC jobs
    tileInfoConfigurator.setupCOOLSFR()
    from AthenaCommon.DetFlags import DetFlags
    if not DetFlags.simulate.Tile_on():
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        condSeq.TileSamplingFractionCondAlg.G4Version = -1
