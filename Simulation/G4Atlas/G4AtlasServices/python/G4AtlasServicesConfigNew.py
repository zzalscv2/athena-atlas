# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon import CfgGetter
#from G4AtlasApps.SimFlags import simFlags
#from AthenaCommon.BeamFlags import jobproperties

from G4AtlasServices.G4AtlasServicesConf import DetectorGeometrySvc, StandardFieldSvc, G4AtlasSvc, G4GeometryNotifierSvc
#the physics region tools
from G4AtlasTools.G4PhysicsRegionConfigNew import SX1PhysicsRegionToolCfg, BedrockPhysicsRegionToolCfg, CavernShaftsConcretePhysicsRegionToolCfg, PixelPhysicsRegionToolCfg, SCTPhysicsRegionToolCfg, TRTPhysicsRegionToolCfg, TRT_ArPhysicsRegionToolCfg, BeampipeFwdCutPhysicsRegionToolCfg, FWDBeamLinePhysicsRegionToolCfg, EMBPhysicsRegionToolCfg, EMECPhysicsRegionToolCfg, HECPhysicsRegionToolCfg, FCALPhysicsRegionToolCfg, DriftWallPhysicsRegionToolCfg, DriftWall1PhysicsRegionToolCfg, DriftWall2PhysicsRegionToolCfg, MuonSystemFastPhysicsRegionToolCfg

"""FwdRegionPhysicsRegionToolCfg,  
 TRT_KrPhysicsRegionToolCfg,  
 EMECParaPhysicsRegionToolCfg, 
 FCALParaPhysicsRegionToolCfg, 
 FCAL2ParaPhysicsRegionToolCfg, 
 PreSampLArPhysicsRegionToolCfg, 
 DeadMaterialPhysicsRegionToolCfg, 
 MuonPhysicsRegionToolCfg, 
 SX1PhysicsRegionToolCfg,  
 SCTSiliconPhysicsRegionToolCfg"""

from G4AtlasTools.G4GeometryToolConfig import MaterialDescriptionToolCfg, G4AtlasDetectorConstructionToolCfg
#from G4AtlasTools.G4FieldConfigNew import ATLASFieldManagerToolCfg, TightMuonsATLASFieldManagerToolCfg, BeamPipeFieldManagerToolCfg, InDetFieldManagerToolCfg, MuonsOnlyInCaloFieldManagerToolCfg, MuonFieldManagerToolCfg, Q1FwdFieldMangerToolCfg, Q2FwdFieldMangerToolCfg, Q3FwdFieldMangerToolCfg, D1FwdFieldMangerToolCfg, D2FwdFieldMangerToolCfg, Q4FwdFieldMangerToolCfg, Q5FwdFieldMangerToolCfg, Q6FwdFieldMangerToolCfg, Q7FwdFieldMangerToolCfg, Q1HKickFwdFieldMangerToolCfg, Q1VKickFwdFieldMangerToolCfg, Q2HKickFwdFieldMangerToolCfg, Q2VKickFwdFieldMangerToolCfg, Q3HKickFwdFieldMangerToolCfg, Q3VKickFwdFieldMangerToolCfg, Q4VKickAFwdFieldMangerToolCfg, Q4HKickFwdFieldMangerToolCfg, Q4VKickBFwdFieldMangerToolCfg, Q5HKickFwdFieldMangerToolCfg,  Q6VKickFwdFieldMangerToolCfg, FwdRegionFieldMangerToolCfg
"""ClassicFieldManagerToolCfg
BasicDetectorFieldManagerToolCfg
BasicFwdFieldMangerToolCfg"""

#already imported - is this correct?
from MagFieldServices.MagFieldServicesConfig import MagneticFieldSvcCfg

def getATLAS_RegionCreatorList(ConfigFlags):
    regionCreatorList = []
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as commonGeoFlags
    from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags as geoFlags


    isUpgrade = ConfigFlags.GeoModel.Run =="RUN4" 
    isRUN2 = (ConfigFlags.GeoModel.Run in ["RUN2", "RUN3"]) or (commonGeoFlags.Run()=="UNDEFINED" and geoFlags.isIBL())

    from G4AtlasApps.SimFlags import simFlags
    from AthenaCommon.DetFlags import DetFlags
    from AthenaCommon.BeamFlags import jobproperties
    if ConfigFlags.Beam.Type == 'cosmics' or ConfigFlags.Sim.CavernBG != 'Signal':
        regionCreatorList += [SX1PhysicsRegionToolCfg(ConfigFlags), BedrockPhysicsRegionToolCfg(ConfigFlags), CavernShaftsConcretePhysicsRegionToolCfg(ConfigFlags)]
        #regionCreatorList += ['CavernShaftsAirPhysicsRegionTool'] # Not used currently
    if ConfigFlags.Detector.SimulateID:
        if ConfigFlags.Detector.SimulatePixel:
            regionCreatorList += [PixelPhysicsRegionToolCfg(ConfigFlags)]
        if ConfigFlags.Detector.SimulateSCT:
            regionCreatorList += [SCTPhysicsRegionToolCfg(ConfigFlags)]
        if ConfigFlags.Detector.SimulateTRT and not isUpgrade:
            regionCreatorList += [TRTPhysicsRegionToolCfg(ConfigFlags)]
            if isRUN2:
                regionCreatorList += [TRT_ArPhysicsRegionToolCfg(ConfigFlags)] #'TRT_KrPhysicsRegionTool'
        # FIXME dislike the ordering here, but try to maintain the same ordering as in the old configuration.
        if ConfigFlags.Detector.SimulateBpipe:
            #if simFlags.BeamPipeSimMode.statusOn and simFlags.BeamPipeSimMode() != "Normal":
            if True:
                regionCreatorList += [BeampipeFwdCutPhysicsRegionToolCfg(ConfigFlags)]
            #if simFlags.ForwardDetectors.statusOn and simFlags.ForwardDetectors() == 2:
            if True:
                regionCreatorList += [FWDBeamLinePhysicsRegionToolCfg(ConfigFlags)]
    if ConfigFlags.Detector.SimulateCalo:
        if ConfigFlags.Detector.GeometryLAr:
            ## Shower parameterization overrides the calibration hit flag
            if simFlags.LArParameterization.statusOn and simFlags.LArParameterization() > 0 \
                    and ConfigFlags.Sim.CalibrationRun in ['LAr','LAr+Tile','DeadLAr']:
                print 'You requested both calibration hits and frozen showers / parameterization in the LAr.'
                print '  Such a configuration is not allowed, and would give junk calibration hits where the showers are modified.'
                print '  Please try again with a different value of either simFlags.LArParameterization (' + str(simFlags.LArParameterization()) + ') or simFlags.CalibrationRun ('+str(ConfigFlags.Sim.CalibrationRun)+')'
                raise RuntimeError('Configuration not allowed')
            if simFlags.LArParameterization() > 0:
                regionCreatorList += [EMBPhysicsRegionTool(ConfigFlags), EMECPhysicsRegionTool(ConfigFlags),
                                      HECPhysicsRegionTool(ConfigFlags), FCALPhysicsRegionTool(ConfigFlags)]
                # FIXME 'EMBPhysicsRegionTool' used for parametrization also - do we need a second instance??
                regionCreatorList += [EMECParaPhysicsRegionTool(ConfigFlags),
                                      FCALParaPhysicsRegionTool(ConfigFlags), FCAL2ParaPhysicsRegionTool(ConfigFlags)]
                if simFlags.LArParameterization.get_Value() > 1:
                    regionCreatorList += [PreSampLArPhysicsRegionTool(ConfigFlags), DeadMaterialPhysicsRegionTool(ConfigFlags)]
            elif simFlags.LArParameterization() is None or simFlags.LArParameterization() == 0:
                regionCreatorList += [EMBPhysicsRegionTool(ConfigFlags), EMECPhysicsRegionTool(ConfigFlags),
                                      HECPhysicsRegionTool(ConfigFlags), FCALPhysicsRegionTool(ConfigFlags)]
    ## FIXME _initPR never called for FwdRegion??
    #if simFlags.ForwardDetectors.statusOn:
    #    if DetFlags.geometry.FwdRegion_on():
    #        regionCreatorList += ['FwdRegionPhysicsRegionTool']
    if ConfigFlags.Detector.GeometryMuon:
        regionCreatorList += [DriftWallPhysicsRegionTool(ConfigFlags), DriftWall1PhysicsRegionTool(ConfigFlags), DriftWall2PhysicsRegionTool(ConfigFlags)]
        if ConfigFlags.Sim.CavernBG != 'Read' and not (simFlags.RecordFlux.statusOn and simFlags.RecordFlux()):
            regionCreatorList += [MuonSystemFastPhysicsRegionTool(ConfigFlags)]
    return regionCreatorList

#not called anywhere?
def getCTB_RegionCreatorList(ConfigFlags):
    regionCreatorList = []
    from G4AtlasApps.SimFlags import simFlags
    ## FIXME _initPR never called for SCT??
    #if DetFlags.ID_on():
    #    if DetFlags.geometry.SCT_on():
    #        regionCreatorList += ['SCTSiliconPhysicsRegionTool']
    if ConfigFlags.Detector.SimulateCalo:
        eta=simFlags.Eta.get_Value()
        if eta>=0 and eta<1.201:
            if ConfigFlags.Detector.SimulateLAr:
                regionCreatorList += [EMBPhysicsRegionTool(ConfigFlags)]
    if ConfigFlags.Detector.SimulateMuon:
        regionCreatorList += [DriftWallPhysicsRegionTool(ConfigFlags), DriftWall1PhysicsRegionTool(ConfigFlags), DriftWall2PhysicsRegionTool(ConfigFlags)]
    return regionCreatorList

def getTB_RegionCreatorList(ConfigFlags):
    regionCreatorList = []
    from G4AtlasApps.SimFlags import simFlags

    if (ConfigFlags.GeoModel.AtlasVersion=="tb_LArH6_2003"):
        if (ConfigFlags.Detector.SimulateLAr):
            regionCreatorList += [FCALPhysicsRegionTool(ConfigFlags)]
    elif (ConfigFlags.GeoModel.AtlasVersion=="tb_LArH6_2002"):
        if (ConfigFlags.Detector.SimulateLAr):
            regionCreatorList += [HECPhysicsRegionTool(ConfigFlags)]
    elif (ConfigFlags.GeoModel.AtlasVersion=="tb_LArH6EC_2002"):
        if (ConfigFlags.Detector.SimulateLAr):
            regionCreatorList += [EMECPhysicsRegionTool(ConfigFlags)]
    elif (ConfigFlags.GeoModel.AtlasVersion=="tb_LArH6_2004"):
        if (simFlags.LArTB_H6Hec.get_Value()):
            regionCreatorList += [HECPhysicsRegionTool(ConfigFlags)]
        if (simFlags.LArTB_H6Emec.get_Value()):
            regionCreatorList += [EMECPhysicsRegionTool(ConfigFlags)]
        if (simFlags.LArTB_H6Fcal.get_Value()):
            regionCreatorList += [FCALPhysicsRegionTool(ConfigFlags)]
    return regionCreatorList

#########################################################################
def StandardFieldSvcCfg(ConfigFlags,name="StandardField", **kwargs):
    import MagFieldServices.SetupField
    kwargs.setdefault("MagneticFieldSvc", "AtlasFieldSvc") # TODO This should probably be based on simFlags.MagneticField?
    #kwargs.setdefault("FieldOn", True)
    return StandardFieldSvc(name, **kwargs)

def ForwardFieldSvcCfg(ConfigFlags, name="ForwardField", **kwargs):
    #FIXME Once it exists this version should use the new MagField Service defined in ForwardRegionMgField
    kwargs.setdefault("MagneticFieldSvc", "AtlasFieldSvc")
    #kwargs.setdefault("FieldOn", True)
    return StandardFieldSvc(name, **kwargs)

def Q1FwdG4FieldSvcCfg(ConfigFlags, name='Q1FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q1")
    return StandardFieldSvc(name, **kwargs)
def Q2FwdG4FieldSvcCfg(ConfigFlags, name='Q2FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q2")
    return StandardFieldSvc(name, **kwargs)
def Q3FwdG4FieldSvcCfg(ConfigFlags, name='Q3FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q3")
    return StandardFieldSvc(name, **kwargs)
def D1FwdG4FieldSvcCfg(ConfigFlags, name='D1FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "D1")
    return StandardFieldSvc(name, **kwargs)
def D2FwdG4FieldSvcCfg(ConfigFlags, name='D2FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "D2")
    return StandardFieldSvc(name, **kwargs)
def Q4FwdG4FieldSvcCfg(ConfigFlags, name='Q4FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q4")
    return StandardFieldSvc(name, **kwargs)
def Q5FwdG4FieldSvcCfg(ConfigFlags, name='Q5FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q5")
    return StandardFieldSvc(name, **kwargs)
def Q6FwdG4FieldSvcCfg(ConfigFlags, name='Q6FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q6")
    return StandardFieldSvc(name, **kwargs)
def Q7FwdG4FieldSvcCfg(ConfigFlags, name='Q7FwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q7")
    return StandardFieldSvc(name, **kwargs)
def Q1HKickFwdG4FieldSvcCfg(ConfigFlags, name='Q1HKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q1HKick")
    return StandardFieldSvc(name, **kwargs)
def Q1VKickFwdG4FieldSvcCfg(ConfigFlags, name='Q1VKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q1VKick")
    return StandardFieldSvc(name, **kwargs)
def Q2HKickFwdG4FieldSvcCfg(ConfigFlags, name='Q2HKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q2HKick")
    return StandardFieldSvc(name, **kwargs)
def Q2VKickFwdG4FieldSvcCfg(ConfigFlags, name='Q2VKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q2VKick")
    return StandardFieldSvc(name, **kwargs)
def Q3HKickFwdG4FieldSvcCfg(ConfigFlags, name='Q3HKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q3HKick")
    return StandardFieldSvc(name, **kwargs)
def Q3VKickFwdG4FieldSvcCfg(ConfigFlags, name='Q3VKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q3VKick")
    return StandardFieldSvc(name, **kwargs)
def Q4VKickAFwdG4FieldSvcCfg(ConfigFlags, name='Q4VKickAFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q4VKickA")
    return StandardFieldSvc(name, **kwargs)
def Q4HKickFwdG4FieldSvcCfg(ConfigFlags, name='Q4HKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q4HKick")
    return StandardFieldSvc(name, **kwargs)
def Q4VKickBFwdG4FieldSvcCfg(ConfigFlags, name='Q4VKickBFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q4VKickB")
    return StandardFieldSvc(name, **kwargs)
def Q5HKickFwdG4FieldSvcCfg(ConfigFlags, name='Q5HKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q5HKick")
    return StandardFieldSvc(name, **kwargs)
def Q6VKickFwdG4FieldSvcCfg(ConfigFlags, name='Q6VKickFwdG4FieldSvc', **kwargs):
    kwargs.setdefault("MagneticFieldSvc",           "Q6VKick")
    return StandardFieldSvc(name, **kwargs)

def getATLAS_FieldMgrList(ConfigFlags):
    fieldMgrList = []
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.TightMuonStepping.statusOn or\
       not simFlags.TightMuonStepping():
        fieldMgrList += [ATLASFieldManagerToolCfg(ConfigFlags)]
    else:
        fieldMgrList += [TightMuonsATLASFieldManagerToolCfg(ConfigFlags)]

    from AthenaCommon.DetFlags import DetFlags
    if ConfigFlags.Detector.SimulateBpipe:
        fieldMgrList += [BeamPipeFieldManagerToolCfg(ConfigFlags)]
    if ConfigFlags.Detector.SimulateID:
        fieldMgrList += [InDetFieldManagerToolCfg(ConfigFlags)]
    if ConfigFlags.Detector.SimulateCalo and simFlags.MuonFieldOnlyInCalo.statusOn and simFlags.MuonFieldOnlyInCalo():
        fieldMgrList += [MuonsOnlyInCaloFieldManager(ConfigFlags)]
    if ConfigFlags.Detector.SimulateMuon:
        fieldMgrList += [MuonFieldManager(ConfigFlags)]
    if ConfigFlags.Detector.SimulateForward: #needed?
        if ConfigFlags.Detector.GeometryFwdRegion: #or forward?
            fieldMgrList += [Q1FwdFieldManager(ConfigFlags),
                             Q2FwdFieldManager(ConfigFlags),
                             Q3FwdFieldManager(ConfigFlags),
                             D1FwdFieldManager(ConfigFlags),
                             D2FwdFieldManager(ConfigFlags),
                             Q4FwdFieldManager(ConfigFlags),
                             Q5FwdFieldManager(ConfigFlags),
                             Q6FwdFieldManager(ConfigFlags),
                             Q7FwdFieldManager(ConfigFlags),
                             Q1HKickFwdFieldManager(ConfigFlags),
                             Q1VKickFwdFieldManager(ConfigFlags),
                             Q2HKickFwdFieldManager(ConfigFlags),
                             Q2VKickFwdFieldManager(ConfigFlags),
                             Q3HKickFwdFieldManager(ConfigFlags),
                             Q3VKickFwdFieldManager(ConfigFlags),
                             Q4VKickAFwdFieldManager(ConfigFlags),
                             Q4HKickFwdFieldManager(ConfigFlags),
                             Q4VKickBFwdFieldManager(ConfigFlags),
                             Q5HKickFwdFieldManager(ConfigFlags),
                             Q6VKickFwdFieldManager(ConfigFlags),
                             FwdRegionFieldManager(ConfigFlags)]
    return fieldMgrList

#called?
def getCTB_FieldMgrList(ConfigFlags):
    fieldMgrList = []
    return fieldMgrList

def getTB_FieldMgrList(ConfigFlags):
    fieldMgrList = []
    return fieldMgrList

def getGeometryConfigurationTools(ConfigFlags):
    geoConfigToolList = []
    # The methods for these tools should be defined in the
    # package containing each tool, so G4AtlasTools in this case
    geoConfigToolList += [MaterialDescriptionToolCfg(ConfigFlags)]
    return geoConfigToolList

def DetectorGeometrySvcCfg(ConfigFlags, name="DetectorGeometrySvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorConstruction", G4AtlasDetectorConstructionToolCfg(ConfigFlags))
    ## For now just have the same geometry configurations tools loaded for ATLAS and TestBeam
    kwargs.setdefault("GeometryConfigurationTools", getGeometryConfigurationTools(ConfigFlags))
    from G4AtlasApps.SimFlags import simFlags
    if hasattr(simFlags,"Eta"): #FIXME ugly hack
        kwargs.setdefault("World", 'TileTB_World')
        kwargs.setdefault("RegionCreators", getTB_RegionCreatorList(ConfigFlags))
        kwargs.setdefault("FieldManagers", getTB_FieldMgrList(ConfigFlags))
    elif hasattr(simFlags,"LArTB_H1TableYPos"): #FIXME ugly hack
        kwargs.setdefault("World", 'LArTB_World')
        kwargs.setdefault("RegionCreators", getTB_RegionCreatorList(ConfigFlags))
        kwargs.setdefault("FieldManagers", getTB_FieldMgrList(ConfigFlags))
    else:
        if ConfigFlags.Beam.Type == 'cosmics' or ConfigFlags.Sim.CavernBG != 'Signal':
            kwargs.setdefault("World", 'Cavern')
        else:
            kwargs.setdefault("World", 'Atlas')
        kwargs.setdefault("RegionCreators", getATLAS_RegionCreatorList(ConfigFlags))
        if hasattr(simFlags, 'MagneticField') and simFlags.MagneticField.statusOn:
            kwargs.setdefault("FieldManagers", getATLAS_FieldMgrList(ConfigFlags))
    return result, DetectorGeometrySvc(name, **kwargs)

def G4AtlasSvcCfg(ConfigFlags, name="G4AtlasSvc", **kwargs):
    if ConfigFlags.Concurrency.NumThreads > 0:
        is_hive = True
    else:
        is_hive = False
    kwargs.setdefault("isMT", is_hive)
    kwargs.setdefault("DetectorGeometrySvc", 'DetectorGeometrySvc')
    return G4AtlasSvc(name, **kwargs)

def G4GeometryNotifierSvcCfg(ConfigFlags, name="G4GeometryNotifierSvc", **kwargs):
    kwargs.setdefault("ActivateLVNotifier", True)
    kwargs.setdefault("ActivatePVNotifier", False)
    return G4GeometryNotifierSvc(name, **kwargs)






if __name__ == '__main__':
  from AthenaConfiguration.MainServicesConfig import MainServicesSerialCfg
  import os

  # Set up logging and config behaviour
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  from AthenaCommon.Configurable import Configurable
  log.setLevel(DEBUG)
  Configurable.configurableRun3Behavior = 1


  #import config flags
  from AthenaConfiguration.AllConfigFlags import ConfigFlags
  
  from AthenaConfiguration.TestDefaults import defaultTestFiles
  inputDir = defaultTestFiles.d
  ConfigFlags.Input.Files = defaultTestFiles.EVNT


  ConfigFlags.Detector.SimulateBpipe = True
  ConfigFlags.Detector.SimulateID = True
  ConfigFlags.Detector.SimulateCalo = True 
  ConfigFlags.Detector.SimulateMuon = True
  ConfigFlags.Detector.SimulateForward = True
  ConfigFlags.Detector.GeometryFwdRegion = True
  # Finalize 
  ConfigFlags.lock()

  ## Initialize a new component accumulator
  cfg = MainServicesSerialCfg()

  #add the algorithm
  acc1, Svc1 = DetectorGeometrySvcCfg(ConfigFlags)
  Svc2 = StandardFieldSvcCfg(ConfigFlags)
  Svc3 = ForwardFieldSvcCfg(ConfigFlags)
  Svc4 = Q1FwdG4FieldSvcCfg(ConfigFlags)
  Svc5 = G4AtlasSvcCfg(ConfigFlags)
  Svc6 = G4GeometryNotifierSvcCfg(ConfigFlags)

  cfg.addService(Svc1)
  cfg.addService(Svc2)
  cfg.addService(Svc3)
  cfg.addService(Svc4)

  cfg.merge(acc1)

  #cfg.addEventAlgo(Alg) #Event algo?
  #cfg.merge(acc)
  #cfg.addPublicTool(Alg)

  # Dump config
  #cfg.getService("StoreGateSvc").Dump = True
  #cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  ConfigFlags.dump()


  f=open("test.pkl","w")
  cfg.store(f) 
  f.close()



  print cfg._publicTools
  print "-----------------finished----------------------"
