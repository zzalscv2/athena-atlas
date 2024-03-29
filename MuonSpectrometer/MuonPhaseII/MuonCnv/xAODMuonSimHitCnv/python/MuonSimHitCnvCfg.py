#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MdtToxAODConvAlgCfg(flags,name="MdtSimHitToxAODConvAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("UseR4DetMgr", flags.Muon.setupGeoModelXML)
    the_alg = CompFactory.MdtSimHitToxAODCnvAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODtoMdtCnvAlgCfg(flags, name = "xAODtoMdtSimHitConvAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.xAODSimHitToMdtCnvAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODSimHitToMdtMeasCnvAlgCfg(flags,name = "SimHitToMdtMeasurementCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    result.merge(MdtCalibDbAlgCfg(flags))
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags))) 
    the_alg = CompFactory.xAODSimHitToMdtMeasCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODSimHitTosTGCMeasCnvAlgCfg(flags, name = "SimHitTosTGCMeasurementCnvAlg",**kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))

    the_alg = CompFactory.xAODSimHitTosTGCMeasCnvAlg(name,**kwargs)
    result.addEventAlgo(the_alg,primary=True)
    return result

def xAODSimHitToMmMeasCnvAlgCfg(flags, name = "SimHitToMmMeasurementCnvAlg",**kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))

    the_alg = CompFactory.xAODSimHitToMmMeasCnvAlg(name,**kwargs)
    result.addEventAlgo(the_alg,primary=True)
    return result

