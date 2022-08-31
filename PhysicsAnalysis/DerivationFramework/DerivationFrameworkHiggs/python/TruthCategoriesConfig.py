# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TruthCategoriesDecoratorCfg(flags, name="TruthCategoriesDecorator", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC: return result
    from TruthConverters.TruthConvertersCfg import xAODtoHEPToolCfg
    from TruthRivetTools.TruthRivetToolsCfg import HiggsTruthCategoryToolCfg
    kwargs.setdefault("HepMCTool", result.popToolsAndMerge(xAODtoHEPToolCfg(flags)))
    kwargs.setdefault("CategoryTool", result.popToolsAndMerge(HiggsTruthCategoryToolCfg(flags)))
    the_alg = CompFactory.DerivationFramework.TruthCategoriesDecorator(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__ == "__main__":
    from MuonConfig.MuonConfigUtils import SetupMuonStandaloneArguments
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    args = SetupMuonStandaloneArguments()
    ConfigFlags.Input.Files = args.input    
    ConfigFlags.Concurrency.NumThreads=args.threads
    ConfigFlags.Concurrency.NumConcurrentEvents=args.threads
    from AthenaCommon.Constants import DEBUG
    
    ConfigFlags.lock()
    ConfigFlags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(ConfigFlags))
    cfg.merge(TruthCategoriesDecoratorCfg(ConfigFlags, OutputLevel = DEBUG))

    sc = cfg.run(ConfigFlags.Exec.MaxEvents)
    if not sc.isSuccess():
        exit(1)
