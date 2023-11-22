# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

def ZDC_DetToolCfg(flags):
     from AtlasGeoModel.GeoModelConfig import GeoModelCfg
     result = GeoModelCfg(flags)
     result.getPrimary().DetectorTools += [ CompFactory.ZDC_DetTool() ]
     return result 
