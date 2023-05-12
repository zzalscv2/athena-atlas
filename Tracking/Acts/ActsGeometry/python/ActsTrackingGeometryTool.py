# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

##################################################################################
# The ActsTrackingGeometryTool fragment
#
# usage: 
#   include('ActsGeometry/ActsTrackingGeometryTool.py')
##################################################################################

from __future__ import print_function

# import the DetFlags for the setting
from AthenaCommon.DetFlags import DetFlags

# import the Extrapolator configurable
from ActsGeometry.ActsGeometryConf import ActsTrackingGeometryTool
   
class ConfiguredActsTrackingGeometry( ActsTrackingGeometryTool ) :   
  # constructor
  def __init__(self,name):
    
    subDetectors = []
    if DetFlags.pixel_on():
      subDetectors += ["Pixel"]
    from ActsGeometry.ActsGeometryConf import ActsAlignmentCondAlg
    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
    if not hasattr(condSeq, "NominalAlignmentCondAlg"):
      condSeq += ActsAlignmentCondAlg(name = "NominalAlignmentCondAlg")
    from ActsGeometry.ActsGeometryConf import ActsTrackingGeometrySvc
    from ROOT.ActsTrk import DetectorType
    actsTrackingGeometrySvc = ActsTrackingGeometrySvc(name = "ActsTrackingGeometrySvc",
                                                      BuildSubDetectors = subDetectors,
                                                      NotAlignDetectors = [DetectorType.Pixel])
    
    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr += actsTrackingGeometrySvc
    
    
    
    ActsTrackingGeometryTool.__init__(self,
                                      name,
                                      ActsTrackingGeometrySvc=actsTrackingGeometrySvc)
    
##################################################################################    

# now create the instance
from AthenaCommon.AppMgr import ToolSvc 
if not hasattr(ToolSvc, "ActsTrackingGeometryTool"):
  actsTrackingGeometryTool = ConfiguredActsTrackingGeometry(name = "ActsTrackingGeometryTool")
  # add it to the ServiceManager
  ToolSvc += actsTrackingGeometryTool

