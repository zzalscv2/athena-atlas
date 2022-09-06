# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def ITkServiceExtensionGeoModelCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    ITkServiceExtensionTool = CompFactory.ITk.ServiceExtensionTool()
    ITkServiceExtensionTool.GmxFilename = "ITKLayouts/Common/Type2Services.gmx"
    ITkServiceExtensionTool.ContainingDetector = "LArBarrel"
    ITkServiceExtensionTool.EnvelopeVolume = "LAr::Barrel::Cryostat::ITkServices"
    geoModelSvc.DetectorTools += [ ITkServiceExtensionTool ]
    return acc