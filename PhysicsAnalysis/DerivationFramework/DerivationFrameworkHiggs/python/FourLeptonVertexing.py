# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory



def FourLeptonVertexerCfg(flags, name="FourLeptonVertexAlg", **kwargs):
    result = ComponentAccumulator()
    ### Setup muon selection tool
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta=2.7,
                                                                DisablePtCuts=True,
                                                                MuQuality=2, ### Select the loose working point
                                                                )) )

    ### Electron selection tool, the Higgs group needs to decide
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))
    ###
    from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
    kwargs.setdefault("VertexFitter", result.popToolsAndMerge(
        TrkVKalVrtFitterCfg(flags, FirstMeasuredPoint = False)))
    kwargs.setdefault("MinMuonPt", 4000)
    kwargs.setdefault("MinElecPt", 6000)    
    vtx_tool = CompFactory.DerivationFramework.FourLeptonVertexingAlgorithm(name , **kwargs)
    result.addEventAlgo(vtx_tool, primary = True)
    return result

if __name__ == "__main__":
    from MuonConfig.MuonConfigUtils import SetupMuonStandaloneArguments
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    args = SetupMuonStandaloneArguments()
    ConfigFlags.Input.Files = args.input    
    ConfigFlags.Concurrency.NumThreads=args.threads
    ConfigFlags.Concurrency.NumConcurrentEvents=args.threads

    ConfigFlags.lock()
    ConfigFlags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(ConfigFlags))
    cfg.merge(FourLeptonVertexerCfg(ConfigFlags))

    sc = cfg.run(ConfigFlags.Exec.MaxEvents)
    if not sc.isSuccess():
        exit(1)




