# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline

def ALFA_DetectorToolCfg(flags, name="ALFA_DetectorTool", **kwargs):
    result = ComponentAccumulator()
    theALFA_DetectorTool = CompFactory.ALFA_DetectorTool(name="ALFA_DetectorTool")
    theALFA_DetectorTool.MetrologyType=3
    theALFA_DetectorTool.B7L1U_MDGeometryType = 2
    theALFA_DetectorTool.B7L1U_ODGeometryType = 2
    theALFA_DetectorTool.B7L1L_MDGeometryType = 2
    theALFA_DetectorTool.B7L1L_ODGeometryType = 2
    theALFA_DetectorTool.A7L1U_MDGeometryType = 2
    theALFA_DetectorTool.A7L1U_ODGeometryType = 2
    theALFA_DetectorTool.A7L1L_MDGeometryType = 2
    theALFA_DetectorTool.A7L1L_ODGeometryType = 2
    theALFA_DetectorTool.A7R1U_MDGeometryType = 2
    theALFA_DetectorTool.A7R1U_ODGeometryType = 2
    theALFA_DetectorTool.A7R1L_MDGeometryType = 2
    theALFA_DetectorTool.A7R1L_ODGeometryType = 2
    theALFA_DetectorTool.B7R1U_MDGeometryType = 2
    theALFA_DetectorTool.B7R1U_ODGeometryType = 2
    theALFA_DetectorTool.B7R1L_MDGeometryType = 2
    theALFA_DetectorTool.B7R1L_ODGeometryType = 2
    result.merge(addFoldersSplitOnline(flags,'FWD','/FWD/Onl/ALFA/position_calibration','/FWD/ALFA/position_calibration'))
    result.setPrivateTools(theALFA_DetectorTool)
    return result


def ForDetGeometryCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    result = GeoModelCfg(flags)
    geoModelSvc=result.getPrimary()
    geoModelSvc.DetectorTools += [ CompFactory.ForDetEnvelopeTool() ]
    # LUCID
    if flags.Detector.GeometryLucid:
        geoModelSvc.DetectorTools += [ CompFactory.LUCID_DetectorTool() ]
    # ALFA
    if flags.Detector.GeometryALFA:
        geoModelSvc.DetectorTools += [ result.popToolsAndMerge(ALFA_DetectorToolCfg(flags)) ]
    # ForwardRegion
    if flags.Detector.GeometryFwdRegion:
        # ForwardRegionGeoModelFactory (created by
        # ForwardRegionGeoModelTool) has a PublicToolHandle to
        # IForwardRegionProperties. Outside of simulation jobs the
        # default version of the tool seems to be used.
        from AthenaConfiguration.Enums import ProductionStep
        if flags.Common.ProductionStep in [ProductionStep.Simulation, ProductionStep.FastChain]:
            from ForwardRegionProperties.ForwardRegionPropertiesConfig import ForwardRegionPropertiesCfg
            tool = result.popToolsAndMerge(ForwardRegionPropertiesCfg(flags))
            result.addPublicTool(tool)
        geoModelSvc.DetectorTools += [ CompFactory.ForwardRegionGeoModelTool() ]
    # ZDC
    if flags.Detector.GeometryZDC:
        geoModelSvc.DetectorTools += [ CompFactory.ZDC_DetTool() ]
    # AFP
    if flags.Detector.GeometryAFP:
        geoModelSvc.DetectorTools += [ CompFactory.AFP_GeoModelTool() ]
    return result
