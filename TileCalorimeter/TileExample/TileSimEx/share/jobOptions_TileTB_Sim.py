# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#==============================================================
#
# Job options file for Geant4 Simulation
#
# Standalone TileCal Testbeam in 2000-2003
#
#==============================================================

## Algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

from AthenaCommon.AppMgr import theApp
svcMgr = theApp.serviceMgr()

#---  Output printout level -----------------------------------
#output threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
if not 'OutputLevel' in dir():
    OutputLevel = 3
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False

#--- Number of events to be processed (default is 10) ---------
if not 'EvtMax' in dir():
    EvtMax = 10

#--- Suffix for output POOL file ------------------------------
if not 'FileSuffix' in dir():
    FileSuffix = ''

#--- AthenaCommon flags ---------------------------------------
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.PoolHitsOutput='tiletb%s.HITS.pool.root' % FileSuffix
athenaCommonFlags.EvtMax=EvtMax

#--- Detector flags -------------------------------------------
from AthenaCommon.DetFlags import DetFlags

# - Select detectors
DetFlags.ID_setOff()
DetFlags.Calo_setOff()
DetFlags.Muon_setOff()
DetFlags.LVL1_setOff()

DetFlags.Tile_setOn()
# - MCTruth
DetFlags.simulate.Truth_setOn()

#-- Geometry flags --------------------------------------------
from AthenaCommon.GlobalFlags import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-CTB-01"
from AtlasGeoModel import SetGeometryVersion
jobproperties.Global.DetGeo = "ctbh8"
from AtlasGeoModel import GeoModelInit

#--- Simulation flags -----------------------------------------
from G4AtlasApps.SimFlags import simFlags
if 'VP1' in dir():
    simFlags.ReleaseGeoModel=False

if not 'Geo' in dir():
    # TileCal standalone setup with 2 barrels and 1 ext.barrel on top
    #Geo = '2B1EB'
    # TileCal standalone setup with 2 barrels and 2 ext.barrels on top
    #Geo = '2B2EB'
    # TileCal standalone setup with 3 barrels
    #Geo = '3B'
    # TileCal standalone setup with 5 barrels - use for sampling fraction calculation
    Geo = '5B'
simFlags.SimLayout='tb_Tile2000_2003_%s' % Geo.split('-')[0]
simFlags.load_tbtile_flags()

# set eta only if you want eta-projective scan
if not 'Eta' in dir() and not 'Theta' in dir() and not 'Z' in dir():
    Eta = 0.35

if 'Eta' in dir():
    simFlags.Eta=Eta

else:
    if not 'Theta' in dir():
        Theta=-19.656205808169407

    if not 'Z' in dir():
        Z=817.96448041135261

    simFlags.Theta=Theta
    simFlags.Z=Z

# these two values Theta and Z can be set instead of eta
# negative theta corresponds to positive eta values
#simFlags.Theta=-19.656205808169407
# Z coordinate is calculated along front face of module at R=2290
# this is the distance from center of the modules to the desired impact point
# positive value - impact point at positive eta side
#simFlags.Z=817.96448041135261
#
# for 90 degrees scans put theta=+/-90
# positive theta - beam enters from positive eta side (as defined in CTB setup!)
#simFlags.Theta=90
# Z coordinate is the distance from ATLAS center to the desired impact point
# sensitive part starts at Z=2300, ends at Z=2300+3*100+3*130+3*150+2*190=3820
#simFlags.Z=2550.0

#
# put 5.625 or -5.625 if you want to rotate table to up/bottom module
#simFlags.Phi=5.625
#
# put non-zero value here if you want beam above (>0) or below (<0) center
#simFlags.Y=5.0

if 'Phi' in dir():
    simFlags.Phi=Phi
if 'Y' in dir():
    simFlags.Y=Y


# set this flag for simulation with calibration hits
if 'CalibrationRun' in dir():
    simFlags.CalibrationRun = 'Tile'

# avoid reading CaloTTMap from COOL
include.block ( "CaloConditions/CaloConditions_jobOptions.py" )

#---- Needed for the U-shape (any value of Ushape > 0- means that tile size equal to size of master plate, for Ushape <=0 - size of the tile is like in old geometry )
if 'TileUshape' in dir():
    from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
    GeoModelSvc = GeoModelSvc()
    from AtlasGeoModel import TileGM
    GeoModelSvc.DetectorTools[ "TileDetectorTool" ].Ushape = TileUshape

#---- Select steel with 0.45% Manganse for absorber instead of pure Iron. Any value > 0 enables Steel
if 'TileSteel' in dir():
    from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
    GeoModelSvc = GeoModelSvc()
    GeoModelSvc.DetectorTools[ "TileDetectorTool" ].Steel = TileSteel

#---- Use PVT instead of PS for scintillator material. Any value > 0 enables PVT
if 'TilePVT' in dir():
    from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
    GeoModelSvc = GeoModelSvc()
    GeoModelSvc.DetectorTools[ "TileDetectorTool" ].PVT = TilePVT

#---- Special option to enable Cs tubes in simulation. Any value > 0 enables them
if 'TileCsTube' in dir():
    from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
    GeoModelSvc = GeoModelSvc()
    from AtlasGeoModel import TileGM
    GeoModelSvc.DetectorTools[ "TileDetectorTool" ].CsTube = TileCsTube

#---- If GDML variable is defined, full geometery is dumped to GDML file
if 'GDML' in dir():
    include("G4DebuggingTools/VolumeDebugger_options.py")
    #simFlags.OptionalUserActionList.addAction('G4UA::VolumeDebuggerTool')
    simFlags.UserActionConfig.addConfig('G4UA::VolumeDebuggerTool',"OutputPath","./TileTB_%s.gdml" % Geo)
    simFlags.UserActionConfig.addConfig('G4UA::VolumeDebuggerTool',"TargetVolume","") #You can set this to whatever volume you like

#--- Generator flags ------------------------------------------

## Run ParticleGun
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.PoolEvgenInput.set_Off()
import AthenaCommon.AtlasUnixGeneratorJob
import ParticleGun as PG
pg = PG.ParticleGun(randomStream="SINGLE", randomSeed = simFlags.RandomSeedOffset.get_Value())

# 50 GeV pions
#pg.sampler.pid = 211
#pg.sampler.pos = PG.PosSampler(x=-27500, y=[-20,20], z=[-20,20], t=-27500)
#pg.sampler.mom = PG.EEtaMPhiSampler(energy=50000, eta=0, phi=0)

# 100 GeV electrons - use for sampling faction calculation
#pg.sampler.pid = 11
#pg.sampler.pos = PG.PosSampler(x=-27500, y=[-20,20], z=[-20,20], t=-27500)
#pg.sampler.mom = PG.EEtaMPhiSampler(energy=100000, eta=0, phi=0)

if not 'PID' in dir():
    PID=11
if not 'E' in dir():
    E=100000
if not 'Xbeam' in dir():
    Xbeam=-27500
if not 'Ybeam' in dir():
    Ybeam=[-20,20]
if not 'Zbeam' in dir():
    Zbeam=[-20,20]
if not 'Tbeam' in dir():
    Tbeam=[-31250,-23750]
pg.sampler.pid = PID
pg.sampler.pos = PG.PosSampler(x=Xbeam, y=Ybeam, z=Zbeam, t=Tbeam)
pg.sampler.mom = PG.EEtaMPhiSampler(energy=E, eta=0, phi=0)

topSeq += pg

include("G4AtlasApps/fragment.SimCopyWeights.py")

try:
    from AthenaCommon.CfgGetter import getAlgorithm
    topSeq += getAlgorithm("BeamEffectsAlg")
except:
    printfunc ("can not import BeamEffectsAlg algorithm")

include('G4AtlasApps/Tile2000_2003.flat.configuration.py')#HACK - has to be here for TBDetDescrLoader
if Geo.count('-')>0:
    GeoModelSvc.TileVersionOverride='TileTB-%s' % Geo
else:
    GeoModelSvc.TileVersionOverride='TileTB-%s-00' % Geo

#--- Geant4 flags ---------------------------------------------

## Select G4 physics list or use the default one
#simFlags.PhysicsList.set_Value('QGSP_EMV')
#simFlags.PhysicsList.set_Value('QGSP_BERT_EMV')
#simFlags.PhysicsList.set_Value('QGSP_BERT')
#simFlags.PhysicsList.set_Value('FTFP_BERT')
#simFlags.PhysicsList.set_Value('FTFP_BERT_ATL_VALIDATION')

## Use verbose G4 tracking
if 'VerboseTracking' in dir():
    simFlags.G4Commands+= ['/tracking/verbose 1']

## Set non-standard range cut
if 'RangeCut' in dir():
    svcMgr.ToolSvc.PhysicsListToolBase.GeneralCut=RangeCut

#--- Final step -----------------------------------------------

## Populate alg sequence
from AthenaCommon.CfgGetter import getAlgorithm
topSeq += getAlgorithm("G4AtlasAlg",tryDefaultConfigurable=True)

# uncomment and modify any of options below to have non-standard simulation 
SD = svcMgr.TileCTBGeoG4SDCalc
SD.TileTB=True
# SD.DeltaTHit = [ 50. ]
# SD.TimeCut = 200.5
# SD.PlateToCell = True
# SD.DoTileRow = False
# SD.DoTOFCorrection = True
# Birks' law
if 'DoBirk' in dir():
    SD.DoBirk = DoBirk
if 'OldBirk' in dir() or 'Birk1' in dir() or 'Birk2' in dir():
    import AthenaCommon.SystemOfUnits as Units
    gramsPerMeVcmSq = Units.g/(Units.MeV*Units.cm2)
    if 'OldBirk' in dir() and OldBirk:
        ## exp. values from NIM 80 (1970) 239-244
        SD.birk1 = 0.0130 * gramsPerMeVcmSq
        SD.birk2 = 9.6e-6 * gramsPerMeVcmSq * gramsPerMeVcmSq
    if 'Birk1' in dir():
        SD.birk1 = Birk1 * gramsPerMeVcmSq
    if 'Birk2' in dir():
        SD.birk2 = Birk2 * gramsPerMeVcmSq * gramsPerMeVcmSq
if 'TileUshape' in dir():
    SD.Ushape=TileUshape
printfunc (SD)

## VP1 algorithm for visualization
if 'VP1' in dir():
    from VP1Algs.VP1AlgsConf import VP1Alg
    topSeq += VP1Alg()

## Write Calibration Hit ID and hit coordinates into Ntuple
if 'CalibrationRun' in dir() and 'HitInfo' in dir():
    ## Root Ntuple output
    theApp.HistogramPersistency = "ROOT"
    svcMgr = theApp.serviceMgr()
    if not hasattr(svcMgr,"THistSvc"):
        from GaudiSvc.GaudiSvcConf import THistSvc
        svcMgr+=THistSvc()
    svcMgr.THistSvc.Output += [ "AANT DATAFILE='Hits.Ntup.root' OPT='RECREATE' " ]
    svcMgr.THistSvc.MaxFileSize = 32768
    #svcMgr.THistSvc.CompressionLevel = 5

    ## Tool to create TileCalibHitCnt Ntuple
    from AthenaCommon.AppMgr import ToolSvc
    from TileSimUtils.TileSimUtilsConf import TileCalibHitCntNtup
    theTileCalibHitCntNtup = TileCalibHitCntNtup()
    ToolSvc += theTileCalibHitCntNtup

#--- End of jobOptions_TileTB_Sim.py --------------------------
