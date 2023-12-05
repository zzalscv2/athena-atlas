# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonCalibStreamFileInputSvcCfg(flags, name = 'MuonCalibStreamFileInputSvc'):

    result = ComponentAccumulator()

    result.addService(CompFactory.MuonCalibStreamFileInputSvc(name, InputFiles = flags.Input.Files, DumpStream   = 1),primary =True)
    
    return result

def MuonCalibStreamCnvSvcCfg(flags, name = 'MuonCalibStreamCnvSvc'):

    result = ComponentAccumulator()

    result.addService(CompFactory.MuonCalibStreamCnvSvc(name), primary = True)
    
    return result

def MuonCalibStreamAddressProviderSvcCfg(flags, name = 'MuonCalibStreamAddressProviderSvc'):

    result = ComponentAccumulator()
    
    result.addService(CompFactory.MuonCalibStreamAddressProviderSvc(name), primary = True)
    
    return result

def MuonCalibStreamDataProviderSvcCfg(flags, name = 'MuonCalibStreamDataProviderSvc'):

    result = ComponentAccumulator()
    result.addService(CompFactory.MuonCalibStreamDataProviderSvc(name,RunNumber = flags.Input.RunNumbers[0],LumiBlockNumberFromCool = False, RunNumberFromCool=False), primary = True)
    
    return result

def EventSelectorMuonCalibStreamCfg(flags, name = 'EventSelectorMuonCalibStream'):

    result = ComponentAccumulator()
    result.addService(CompFactory.EventSelectorMuonCalibStream(name), primary = True)
    
    return result

def MuonCalibRunLumiBlockCoolSvcCfg(flags, name = 'MuonCalibRunLumiBlockCoolSvc'):

    result = ComponentAccumulator()
    result.addService(CompFactory.MuonCalibRunLumiBlockCoolSvc(name), primary = True)
    
    return result

