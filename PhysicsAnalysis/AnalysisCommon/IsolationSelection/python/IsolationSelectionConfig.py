#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def IsolationSelectionToolCfg(flags, name="IsolationSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CP.IsolationSelectionTool(name, **kwargs))
    return acc

def MuonPhysValIsolationSelCfg(flags, **kwargs):
    return IsolationSelectionToolCfg(flags,MuonWP="PflowTight_FixedRad")

def IsoCloseByCorrectionToolCfg(flags, name="IsoCloseByCorrectionTool", ttva_wp = "", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IsolationSelectionTool", acc.popToolsAndMerge(MuonPhysValIsolationSelCfg(flags)))
    from InDetConfig.InDetTrackSelectionToolConfig import isoTrackSelectionToolCfg
    from IsolationAlgs.IsoToolsConfig import isoTTVAToolCfg
    kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(isoTrackSelectionToolCfg(flags, minPt=500)) )
    if len(ttva_wp): 
        kwargs.setdefault("TTVASelectionTool", acc.popToolsAndMerge(isoTTVAToolCfg(flags, WorkingPoint = ttva_wp)))
    the_tool = CompFactory.CP.IsolationCloseByCorrectionTool(name, **kwargs)
    acc.setPrivateTools(the_tool)
    return acc


def IsoCloseByCorrSkimmingAlgCfg(flags, name="IsoCloseByCorrSkimmingAlg", ttva_wp = 'Nonprompt_All_MaxWeight',  **kwargs):
    result = ComponentAccumulator()
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta=2.7,
                                                                DisablePtCuts=True,
                                                                MuQuality=2, ### Select the loose working point
                                                                )))
    kwargs.setdefault("IsoCloseByCorrectionTool", result.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags)))    
    ### Photon selection needs to be still defined
    # kwargs.setdefault("PhotonSelectionTool", <blah>)   
    kwargs.setdefault("PhotContainer", "")
    the_alg = CompFactory.CP.IsoCloseByCorrectionTrkSelAlg(name+ttva_wp, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def IsoCloseByCaloDecorCfg(flags, name="IsoCloseByCaloDecor", containers =[], **kwargs):
    result = ComponentAccumulator()
    ## Configure the tool such that the calo & pflow clusters are decorated
    ## https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/IsolationSelection/IsolationCloseByCorrectionTool.h#L35-39
    kwargs.setdefault("IsoCloseByCorrectionTool", result.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags,
                                                                                                      CaloCorrectionModel = -1)))    
    for cont in containers:
        result.addEventAlgo(CompFactory.CP.IsoCloseByCaloDecorAlg(name = name + cont, 
                                                                  PrimaryContainer = cont, 
                                                                  **kwargs))
    return result
    
def TestIsoCloseByCorrectionCfg(flags, name="TestIsoCloseByAlg", **kwargs):
    result = ComponentAccumulator()
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta=2.7,
                                                                DisablePtCuts=True,
                                                                MuQuality=2, ### Select the loose working point
                                                                )))  
    the_alg = CompFactory.CP.TestIsolationCloseByCorrAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result