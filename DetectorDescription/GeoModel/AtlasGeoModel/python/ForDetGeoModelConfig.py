# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline


def ForDetGeometryCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc=acc.getPrimary()
    # LUCID
    if flags.Detector.GeometryLucid:
        LUCID_DetectorTool=CompFactory.LUCID_DetectorTool
        geoModelSvc.DetectorTools += [ LUCID_DetectorTool() ]
    # ALFA
    if flags.Detector.GeometryALFA:
        #from ALFA_GeoModel.ALFA_GeoModelConf import ALFA_DetectorTool 
        ALFA_DetectorTool=CompFactory.ALFA_DetectorTool
        theALFA_DetectorTool=ALFA_DetectorTool(name="ALFA_DetectorTool")
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
        geoModelSvc.DetectorTools += [ theALFA_DetectorTool ]
        acc.merge(addFoldersSplitOnline(flags,'FWD','/FWD/Onl/ALFA/position_calibration','/FWD/ALFA/position_calibration'))
    # ForwardRegion
    if flags.Detector.GeometryFwdRegion:
        ForwardRegionGeoModelTool=CompFactory.ForwardRegionGeoModelTool
        geoModelSvc.DetectorTools += [ ForwardRegionGeoModelTool() ]
    # ZDC
    if flags.Detector.GeometryZDC:
        ZDC_DetTool=CompFactory.ZDC_DetTool
        geoModelSvc.DetectorTools += [ ZDC_DetTool() ]
    # AFP
    if flags.Detector.GeometryAFP:
        AFP_GeoModelTool=CompFactory.AFP_GeoModelTool
        geoModelSvc.DetectorTools += [ AFP_GeoModelTool() ]
    acc.addService(geoModelSvc)
    return acc
