# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file EventCommonD3PDMaker/python/LBMetadataConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Nov, 2009
# @brief Return a configured LBMetadataTool.
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def LBMetadataCfg (flags):
    acc = ComponentAccumulator()

    lbtool = CompFactory.LumiBlockMetaDataTool( 'LumiBlockMetaDataTool' )
    acc.addPublicTool (lbtool)

    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    acc.merge (MetaDataSvcCfg (flags, tools = [lbtool]))
    
    acc.addPrivateTool (CompFactory.D3PD.LBMetadataTool (Metakey = 'Lumi/'))
    return acc
