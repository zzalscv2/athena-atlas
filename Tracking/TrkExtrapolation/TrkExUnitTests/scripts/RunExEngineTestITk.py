# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
from AthenaCommon.Logging import log
import sys

if(len(sys.argv[1:])):
  cmdargs=dict(arg.split('=') for arg in sys.argv[1:])
  if not 'MisalignMode' in cmdargs.keys():
    MisalignMode = -1
  else:
    MisalignMode=int(cmdargs.get('MisalignMode',11))
    print("Looking for misalignment files for mode "+str(MisalignMode))
else:
  MisalignMode = -1

## Just enable ID for the moment.
ConfigFlags.Input.isMC             = True

ConfigFlags.Input.Files = []

ConfigFlags.ITk.Geometry.AllLocal = False
if ConfigFlags.ITk.Geometry.AllLocal:
  detectors = [
    "ITkPixel",
    "ITkStrip",
    "Bpipe"
  ]
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(ConfigFlags, detectors, toggle_geometry=True)
ConfigFlags.TrackingGeometry.MaterialSource = "Input"

ConfigFlags.Detector.GeometryHGTD = False

ConfigFlags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-00"
ConfigFlags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
ConfigFlags.GeoModel.Align.Dynamic = False
if(MisalignMode!=-1):
  tag=""
  BFile=""
  DBFile="MisalignmentSet"+str(MisalignMode)+".db"
  if(MisalignMode==0):
    tag="InDetSi_MisalignmentMode_no Misalignment"
  elif(MisalignMode==1):
    tag="InDetSi_MisalignmentMode_misalignment by 6 parameters"
  elif(MisalignMode==2):
    tag="InDetSi_MisalignmentMode_random misalignment"
  elif(MisalignMode==3):
    tag="InDetSi_MisalignmentMode_IBL-stave temperature dependent bowing"
  elif(MisalignMode==11):
    tag="InDetSi_MisalignmentMode_R deltaR (radial expansion)"
  elif(MisalignMode==21):
    tag="InDetSi_MisalignmentMode_R deltaPhi (curl)"
  elif(MisalignMode==31):
    tag="InDetSi_MisalignmentMode_R deltaZ (telescope)"
  elif(MisalignMode==99):
    tag="InDetSi_MisalignmentMode_99"
    DBFile="MisalignmentSet99.db"
  DBName="OFLCOND"
  ConfigFlags.IOVDb.DBConnection ="sqlite://;schema="+DBFile+";dbname="+DBName

ConfigFlags.Detector.GeometryCalo = False
ConfigFlags.Detector.GeometryMuon = False

# This should run serially for the moment.
ConfigFlags.Concurrency.NumThreads = 1
ConfigFlags.Concurrency.NumConcurrentEvents = 1

log.debug('Lock config flags now.')
ConfigFlags.lock()

log.debug('dumping config flags now.')
ConfigFlags.dump()

cfg=MainServicesCfg(ConfigFlags)    
if(MisalignMode!=-1):
  from IOVDbSvc.IOVDbSvcConfig import addFolders
  cfg.merge(addFolders(ConfigFlags,"/Indet/AlignITk",db=DBName,detDb=DBFile,tag=tag))

from TrkExUnitTests.TrkExUnitTestsConfig import ExtrapolationEngineTestCfg
topoAcc=ExtrapolationEngineTestCfg(ConfigFlags,
                                   NumberOfTestsPerEvent   = 100,
                                   # parameters mode: 0 - neutral tracks, 1 - charged particles
                                   ParametersMode          = 1,
                                   # do the full test backwards as well
                                   BackExtrapolation       = False,
                                   # Smear the production vertex - standard primary vertex paramters
                                   SmearOrigin             = True,
                                   SimgaOriginD0           = 2./3.,
                                   SimgaOriginZ0           = 50.,
                                   # pT range for testing
                                   PtMin                   = 1000,
                                   PtMax                   = 1000,
                                   # The test range in Eta
                                   EtaMin                  =  -5.,
                                   EtaMax                  =   5.,
                                   # Configure how you wanna run
                                   CollectSensitive        = True,
                                   CollectPassive          = True,
                                   CollectBoundary         = True,
                                   CollectMaterial         = True,
                                   UseHGTD                 = ConfigFlags.Detector.GeometryHGTD,
                                   # the path limit to test
                                   PathLimit               = -1.,
                                   )

cfg.merge(topoAcc)

cfg.printConfig()
if(MisalignMode!=-1):
  cfg.getService("GeoModelSvc").DetectorTools["ITk::PixelDetectorTool"].Alignable=True
  print("Pixel manager alignability has been set to: "+ str(cfg.getService("GeoModelSvc").DetectorTools["ITk::PixelDetectorTool"].Alignable))
  cfg.getService("GeoModelSvc").DetectorTools["ITk::StripDetectorTool"].Alignable=True
  print("Strip manager alignability has been set to:"+str(cfg.getService("GeoModelSvc").DetectorTools["ITk::StripDetectorTool"].Alignable))

cfg.run(10)
