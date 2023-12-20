# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonCalibStreamReadCfg(flags):
    """Set up to read from a muon calibration stream bytestream file

    The function adds the components required to read events and metadata from
    bytestream input. May be used to read events from a secondary input as well
    primary input file.

    Args:
        flags:      Job configuration flags
        Fix the type_names for MDT, TGC and RPC in this configuration

    Services:
        MuonCalibStreamCnvSvcCfg
        MuonCalibStreamDataProviderSvcCfg
        MuonCalibStreamFileInputSvcCfg
        MuonCalibStreamAddressProviderSvcCfg (comment out after changing from converter to alg)
        EventSelectorMuonCalibStreamCfg
        MuonCalibRunLumiBlockCoolSvcCfg (to be fixed)

    Returns:
        A component accumulator fragment containing the components required to read 
        from Muon calibration stream bytestream data. Should be merged into main job configuration.
    """
    result = ComponentAccumulator()

    # import the basic fragment
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg,MuonGeoModelCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    result.merge(MuonGeoModelCfg(flags))
    
    from MuonCalibStreamCnvSvc.MuonCalibStreamCnvSvcConfig import MuonCalibStreamCnvSvcCfg,\
                                                                MuonCalibStreamDataProviderSvcCfg,\
                                                                MuonCalibStreamFileInputSvcCfg,\
                                                                EventSelectorMuonCalibStreamCfg,\
                                                                MuonCalibRunLumiBlockCoolSvcCfg
    result.merge(MuonCalibStreamCnvSvcCfg(flags))

    # set MuonCalibStream EvtPersistencySvc
    event_persistency = CompFactory.EvtPersistencySvc(name="EventPersistencySvc", CnvServices=["MuonCalibStreamCnvSvc"])
    result.addService(event_persistency)

    # set MuonCalibrationStream event selector 
    result.merge(EventSelectorMuonCalibStreamCfg(flags, name = 'EventSelector'))
    result.setAppProperty("EvtSel", "EventSelector")

    # import other MuonCalibStreamCnv Services 
    result.merge(MuonCalibStreamFileInputSvcCfg(flags))
    result.merge(MuonCalibStreamDataProviderSvcCfg(flags))
    result.merge(MuonCalibRunLumiBlockCoolSvcCfg(flags))

    return result



def MuonCalibStreamTestAlgCfg(flags, name = "MuonCalibStreamTestAlg", **kwargs):
    result = ComponentAccumulator()
    
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg,MuonGeoModelCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    result.merge(MuonGeoModelCfg(flags))

    result.addEventAlgo(CompFactory.MuonCalibStreamTestAlg(name, **kwargs))
    
    return result

def EventInfoCalibRawDataProviderCfg(flags, name = "EventInfoCalibRawDataProviderAlg", **kwargs):
    result = ComponentAccumulator()

    result.addEventAlgo(CompFactory.EventInfoCalibRawDataProvider(name, lb_from_cool = False, **kwargs))
    
    return result


def MdtCalibRawDataProviderCfg(flags, name = "MdtCalibRawDataProviderAlg", **kwargs):
    result = ComponentAccumulator()
    
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg,MuonGeoModelCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    result.merge(MuonGeoModelCfg(flags))

    result.addEventAlgo(CompFactory.MdtCalibRawDataProvider(name,**kwargs))
    
    return result

def RpcCalibRawDataProviderCfg(flags, name = "RpcCalibRawDataProviderAlg", **kwargs):
    result = ComponentAccumulator()
    
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg,MuonGeoModelCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    result.merge(MuonGeoModelCfg(flags))

    result.addEventAlgo(CompFactory.RpcCalibRawDataProvider(name,**kwargs))
    
    return result

def TgcCalibRawDataProviderCfg(flags, name = "TgcCalibRawDataProviderAlg", **kwargs):
    result = ComponentAccumulator()
    
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg,MuonGeoModelCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    result.merge(MuonGeoModelCfg(flags))

    result.addEventAlgo(CompFactory.TgcCalibRawDataProvider(name,**kwargs))
    
    return result

