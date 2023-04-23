# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of HGTD_TrackingGeometry package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HGTD_LayerBuilderCondCfg(flags, name='HGTD_LayerBuilderCond', **kwargs):
    # for hgtd DetectorElement conditions data :
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    result = HGTD_ReadoutGeometryCfg(flags)
    
    kwargs.setdefault("Identification", 'HGTD')
    kwargs.setdefault("SetLayerAssociation", True)
    
    result.setPrivateTools(CompFactory.HGTD_LayerBuilderCond(name, **kwargs))
    return result


def HGTD_TrackingGeometryBuilderCfg(flags,
                                    name='HGTD_TrackingGeometryBuilderCond',
                                    **kwargs):
    result = ComponentAccumulator()

    HGTD_LayerBuilder = result.popToolsAndMerge(
        HGTD_LayerBuilderCondCfg(flags))
    result.addPublicTool(HGTD_LayerBuilder)
    
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import HGTD_CylinderVolumeCreatorCfg
    cylinderVolumeCreator = result.popToolsAndMerge(
        HGTD_CylinderVolumeCreatorCfg(flags))
    result.addPublicTool(cylinderVolumeCreator)
    
    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import (
        EnvelopeDefSvcCfg)
    envelopeDefinitionSvc = result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags))
    
    kwargs.setdefault("LayerBuilder", HGTD_LayerBuilder)
    kwargs.setdefault("EnvelopeDefinitionSvc", envelopeDefinitionSvc)
    kwargs.setdefault("TrackingVolumeCreator", cylinderVolumeCreator)
    
    result.setPrivateTools(
        CompFactory.HGTD_TrackingGeometryBuilderCond(name, **kwargs))
    return result
