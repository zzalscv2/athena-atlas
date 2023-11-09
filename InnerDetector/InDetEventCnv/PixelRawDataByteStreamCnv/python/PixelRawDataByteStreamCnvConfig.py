#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg, PixelHitDiscCnfgAlgCfg

def PixelRawDataProviderAlgCfg(flags, RDOKey="PixelRDOs", **kwargs):
    """ Main function to configure Pixel raw data decoding """
    acc = PixelCablingCondAlgCfg(flags)
    acc.merge(PixelHitDiscCnfgAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    acc.merge (PixelReadoutManagerCfg(flags))

    from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
    regSelTool = acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags))

    suffix = kwargs.pop("suffix","")
    decoder = CompFactory.PixelRodDecoder(name="PixelRodDecoder"+suffix,
                                          CheckDuplicatedPixel = False if "data15" in flags.Input.ProjectName else True
                                          )
    
    providerTool =  CompFactory.PixelRawDataProviderTool(name="PixelRawDataProviderTool"+suffix,
                                                         Decoder = decoder)

    acc.addEventAlgo(CompFactory.PixelRawDataProvider(RDOKey = RDOKey,
                                                      RegSelTool = regSelTool, 
                                                      ProviderTool = providerTool,
                                                      **kwargs))
    return acc


def TrigPixelRawDataProviderAlgCfg(flags, suffix, RoIs):
    trigargs = {
        'name' : 'TrigPixelRawDataProvider'+suffix,
        'suffix' : suffix,
        'RoIs' : RoIs,   
        'isRoI_Seeded': True,
        'RDOCacheKey' : 'PixRDOCache',
        'BSErrorsCacheKey' : 'PixBSErrCache'
    }
    return PixelRawDataProviderAlgCfg(flags, **trigargs)
