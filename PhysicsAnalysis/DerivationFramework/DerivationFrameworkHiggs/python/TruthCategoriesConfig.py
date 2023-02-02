# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TruthCategoriesDecoratorCfg(ConfigFlags, name="TruthCategoriesDecorator", **kwargs):
    acc = ComponentAccumulator()
    from TruthConverters.TruthConvertersCfg import xAODtoHEPToolCfg
    from TruthRivetTools.TruthRivetToolsCfg import HiggsTruthCategoryToolCfg
    kwargs.setdefault("HepMCTool", acc.popToolsAndMerge(xAODtoHEPToolCfg(ConfigFlags)))
    kwargs.setdefault("CategoryTool", acc.popToolsAndMerge(HiggsTruthCategoryToolCfg(ConfigFlags)))
    the_alg = CompFactory.DerivationFramework.TruthCategoriesDecorator(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    args = flags.fillFromArgs()
    flags.Input.Files = args.input    
    flags.Concurrency.NumThreads=args.threads
    flags.Concurrency.NumConcurrentEvents=args.threads
    from AthenaCommon.Constants import DEBUG
    
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TruthCategoriesDecoratorCfg(flags, OutputLevel = DEBUG))

    sc = cfg.run(flags.Exec.MaxEvents)
    if not sc.isSuccess():
        exit(1)
