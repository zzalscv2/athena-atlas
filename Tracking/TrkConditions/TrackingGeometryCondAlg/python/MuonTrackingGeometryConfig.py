# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of MuonTrackingGeometry packages

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonStationTypeBuilderCfg(flags, name='MuonStationTypeBuilder', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.Muon.MuonStationTypeBuilder(name, **kwargs))
    return result

def MuonStationBuilderCfg(flags, name='MuonStationBuilder', useCond=True, **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = MuonGeoModelCfg(flags)

    if "StationTypeBuilder" not in kwargs:
        kwargs.setdefault("StationTypeBuilder", result.popToolsAndMerge(
            MuonStationTypeBuilderCfg(flags)))

    if useCond:
        name = name + 'Cond'
    muonStationBuilder = CompFactory.Muon.MuonStationBuilderCond(name, **kwargs) if useCond else \
                         CompFactory.Muon.MuonStationBuilder(name, **kwargs)
    result.setPrivateTools(muonStationBuilder)
    return result

def MuonInertMaterialBuilderCfg(flags, name='MuonInertMaterialBuilder',
                                useCond = True,
                                **kwargs):
   from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
   result = MuonGeoModelCfg(flags)

   if useCond:
       name = name + 'Cond'
   muonInertMaterialBuilder = CompFactory.Muon.MuonInertMaterialBuilderCond(name, **kwargs) if useCond else \
                              CompFactory.Muon.MuonInertMaterialBuilder(name, **kwargs)
   result.setPrivateTools(muonInertMaterialBuilder)
   return result

def MuonTrackingGeometryBuilderCfg(flags, name='MuonTrackingGeometryBuilder',
                                   useCond = True,
                                   **kwargs):
    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import EnvelopeDefSvcCfg
    result = ComponentAccumulator()

    kwargs.setdefault("EnvelopeDefinitionSvc", result.getPrimaryAndMerge(
        EnvelopeDefSvcCfg(flags)))

    kwargs.setdefault("MuonStationBuilder", result.popToolsAndMerge(
        MuonStationBuilderCfg(flags,
                              name = 'MuonStationBuilder',
                              useCond = useCond)))

    kwargs.setdefault("InertMaterialBuilder", result.popToolsAndMerge(
        MuonInertMaterialBuilderCfg(flags,
                                    name = 'MuonInertMaterialBuilder',
                                    useCond = useCond)))

    kwargs.setdefault("EntryVolumeName", 'MuonSpectrometerEntrance')
    kwargs.setdefault("ExitVolumeName", 'Muon::Containers::MuonSystem')

    if useCond:
        name = name + 'Cond'
    geometryBuilder = CompFactory.Muon.MuonTrackingGeometryBuilderCond(name, **kwargs) if useCond else \
                      CompFactory.Muon.MuonTrackingGeometryBuilder(name, **kwargs)
    result.setPrivateTools(geometryBuilder)
    return result

    
