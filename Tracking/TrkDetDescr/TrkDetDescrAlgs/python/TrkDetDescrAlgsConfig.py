# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

"""Define methods to configure TrkDetDescrAlgs"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ITkMaterialMappingCfg(flags, name="ITkMaterialMapping", **kwargs):
  """Return configured ComponentAccumulator and tool for MaterialMapping"""
  
  result=ComponentAccumulator()
  LayerMaterialName = 'LayerMaterialITK'    
  LayerMaterialDirectory = '/GLOBAL/TrackingGeo/'
  
  from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelGeometryCfg
  itkPixel = ITkPixelGeometryCfg(flags)
  result.merge(itkPixel)
   
  from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripGeometryCfg
  itkStrip = ITkStripGeometryCfg(flags)
  result.merge(itkStrip)
    
  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  result.merge(BeamPipeGeometryCfg(flags))
      
  # get the correct TrackingGeometry setup
  geom_svc=None
  geom_cond_key=''
  from InDetRecExample.TrackingCommon import use_tracking_geometry_cond_alg
  if not use_tracking_geometry_cond_alg :
     from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
     acc = TrackingGeometrySvcCfg(flags)
     geom_svc = acc.getPrimary()
     result.merge(acc)      
     kwargs.setdefault("TrackingGeometrySvc", geom_svc)
  else :
     from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
     result.merge( TrackingGeometryCondAlgCfg(flags) )
     geom_cond_key = 'AtlasTrackingGeometry'
     kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)    
    
  if 'MappingVolumeName' not in kwargs :
      kwargs.setdefault("MappingVolumeName", 'InDet::Containers::InnerDetector')
  
  if 'ExtrapolationEngine' not in kwargs :
      from TrkConfig.AtlasExtrapolationEngineConfig import AtlasExtrapolationEngineCfg
      kwargs.setdefault("ExtrapolationEngine", result.getPrimaryAndMerge(AtlasExtrapolationEngineCfg(flags)))
  
  if 'MaterialMapper' not in kwargs :
      MaterialMapper = CompFactory.Trk.MaterialMapper("MaterialMapper")
      kwargs.setdefault("MaterialMapper", MaterialMapper)
  
  if 'LayerMaterialRecordAnalyser' not in kwargs :
      LayerMaterialAnalyser = CompFactory.Trk.LayerMaterialAnalyser(name="LayerMaterialRecordAnalyser")
      kwargs.setdefault("LayerMaterialRecordAnalyser", LayerMaterialAnalyser)
  
  if 'LayerMaterialCreators' not in kwargs :
      BinnedLayerMaterialCreator = CompFactory.Trk.BinnedLayerMaterialCreator(name="BinnedLayerMaterialCreator")
      BinnedLayerMaterialCreator.LayerMaterialName         = LayerMaterialName
      BinnedLayerMaterialCreator.LayerMaterialDirectory    = LayerMaterialDirectory
      LayerMaterialCreators  = [BinnedLayerMaterialCreator]
      kwargs.setdefault("LayerMaterialCreators", LayerMaterialCreators)
      
  if 'LayerMaterialAnalysers' not in kwargs :
      BinnedLayerMaterialAnalyser = CompFactory.Trk.LayerMaterialAnalyser(name="BinnedLayerMaterialAnalyser")
      BinnedLayerMaterialAnalyser.LayerMaterialName         =  LayerMaterialName
      BinnedLayerMaterialAnalyser.ValidationTreeName        = 'BinnedLayerMaterialAnalyser'
      BinnedLayerMaterialAnalyser.ValidationTreeDescription = 'Output of the BinnedLayerMaterialAnalyser'
      BinnedLayerMaterialAnalyser.ValidationTreeFolder      = '/val/BinnedLayerMaterialAnalyser'
      LayerMaterialAnalysers = [ BinnedLayerMaterialAnalyser ]
      kwargs.setdefault("LayerMaterialAnalysers", LayerMaterialAnalysers)
      
  histSvc = CompFactory.THistSvc(Output = ["val DATAFILE='AtlasGeant4Geometry.root' TYPE='ROOT' OPT='RECREATE'"])
  result.addService( histSvc )
  
  algo = CompFactory.Trk.MaterialMapping(name=name, **kwargs)
  result.addEventAlgo(algo, primary = True)
  
  from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg    
  result.merge(OutputConditionsAlgCfg(flags, name = "CondAlg_Material",
                                      outputFile="AtlasLayerMaterial.pool.root",
                                      ObjectList=['Trk::LayerMaterialMap#'+LayerMaterialDirectory+LayerMaterialName],
                                      WriteIOV=True,IOVTagList=[flags.ITk.trackingGeometry.materialTag] ))
  
  result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True))
  
  return result
  
  
def ITkMaterialValidationCfg(flags, name="MaterialValidation", **kwargs):
  """Return configured ComponentAccumulator and tool for MaterialMapping"""
  
  result=ComponentAccumulator()
  
  from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelGeometryCfg
  itkPixel = ITkPixelGeometryCfg(flags)
  result.merge(itkPixel)
   
  from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripGeometryCfg
  itkStrip = ITkStripGeometryCfg(flags)
  result.merge(itkStrip)
    
  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  result.merge(BeamPipeGeometryCfg(flags))
  
  # get the correct TrackingGeometry setup
  geom_svc=None
  geom_cond_key=''
  from InDetRecExample.TrackingCommon import use_tracking_geometry_cond_alg
  if not use_tracking_geometry_cond_alg :
     from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
     acc = TrackingGeometrySvcCfg(flags)
     geom_svc = acc.getPrimary()
     result.merge(acc)      
     kwargs.setdefault("TrackingGeometrySvc", geom_svc)
  else :
     from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
     result.merge( TrackingGeometryCondAlgCfg(flags) )
     geom_cond_key = 'AtlasTrackingGeometry'
     kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)    
    
  
  if 'MaterialMapper' not in kwargs :
      MaterialMapper = CompFactory.Trk.MaterialMapper("MaterialMapper")
      kwargs.setdefault("MaterialMapper", MaterialMapper)
      
  if 'MinEta' not in kwargs :
      minEta = -6.
      kwargs.setdefault("MinEta", minEta)
      
  if 'MaxEta' not in kwargs :
      maxEta = 6.
      kwargs.setdefault("MaxEta", maxEta)
  
  histSvc = CompFactory.THistSvc(Output = ["val DATAFILE='AtlasTrackingGeometry.root' TYPE='ROOT' OPT='RECREATE'"])
  result.addService( histSvc )
  
  algo = CompFactory.Trk.MaterialValidation(name=name, **kwargs)
  result.addEventAlgo(algo, primary = True)
  
  return result
