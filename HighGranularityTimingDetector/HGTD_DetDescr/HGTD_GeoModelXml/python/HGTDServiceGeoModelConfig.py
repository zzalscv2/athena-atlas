# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def HGTDServiceGeoModelCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    HGTDServiceTool = CompFactory.ITk.ServiceExtensionTool('HGTDservices')  
    HGTDServiceTool.GmxFilename = "HGTDservices.gmx"
    HGTDServiceTool.ContainingDetector = "LArBarrel"
    HGTDServiceTool.EnvelopeVolume = ""
    HGTDServiceTool.DataBaseTable = "HGTDSERVICESXDD"
    HGTDServiceTool.ServiceExtensionManagerName = "HGTDServices"
    geoModelSvc.DetectorTools += [ HGTDServiceTool ]
    return acc
