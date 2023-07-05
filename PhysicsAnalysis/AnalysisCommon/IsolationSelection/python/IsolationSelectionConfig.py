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
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    kwargs.setdefault("ParticleCaloExtensionTool", acc.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)))   
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

def IsoCloseByCorrAlgCfg(flags, name="IsoCloseByCorrAlg", isPhysLite = False, containerNames = [ "Muons", "Electrons", "Photons"], useSelTools = False, **kwargs):

    result = ComponentAccumulator()
    # Configure the CloseBy isolation correction alg - only need two WPs each for all iso variables
    elIsoWPs   = [ "Loose_VarRad", "TightTrackOnly_FixedRad" ]
    muIsoWPs   = [ "PflowLoose_VarRad", "Loose_VarRad" ] 
    phIsoWPs   = [ "FixedCutLoose", "TightCaloOnly" ]
    isoTool   = result.popToolsAndMerge( IsolationSelectionToolCfg( flags,
                                                                    ElectronWPVec = elIsoWPs,
                                                                    MuonWPVec     = muIsoWPs,
                                                                    PhotonWPVec   = phIsoWPs
                                                                    ))
    # Set suffix for writing corrected isolation values
    isoDecSuffix = "CloseByCorr"
    selectionDecorator = "isoSelIsOK"
    kwargs.setdefault("IsoCloseByCorrectionTool", 
                       result.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags, 
                                                                           IsolationSelectionTool = isoTool,
                                                                           SelectionDecorator     = selectionDecorator,
                                                                           IsoDecSuffix           = isoDecSuffix
                                                                           )))  
    
    # Need muon selection tool to apply Loose - no pt cuts
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta        = 2.7,
                                                                DisablePtCuts = True,
                                                                MuQuality     = 2, ### Select the loose working point
                                                                )))  

    # For PhysLite, add in electron and photon selection tools for el LH very loose and ph isEM loose
    if isPhysLite or useSelTools:
        from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
        from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
        from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
        from AthenaConfiguration.Enums import LHCPeriod
        kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))

        from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import AsgPhotonIsEMSelectorCfg
        from ROOT import egammaPID
        kwargs.setdefault("PhotonSelectionTool", result.popToolsAndMerge(AsgPhotonIsEMSelectorCfg(flags,
                                                                         name= "PhotonSelTool",
                                                                         quality = egammaPID.PhotonIDLoose)))


    # Set selection for muons, electrons and photons to contribute to overlap
    kwargs.setdefault("ParticleContainerKeys",    containerNames)
    
    # For Phys, use the already defined DFCommon id variables for electron and photon, for loose muons, use tool above
    # For PhysLite, the tools above are used
    if not isPhysLite and not useSelTools:
        kwargs.setdefault("ElecSelectionKey",  "Electrons.DFCommonElectronsLHVeryLoose")
        kwargs.setdefault("PhotSelectionKey",  "Photons.DFCommonPhotonsIsEMLoose")
    # No default pt cuts for the moment
    kwargs.setdefault("MinElecPt", 0.)
    kwargs.setdefault("MinMuonPt", 0.)
    kwargs.setdefault("MinPhotPt", 0.)

      
    the_alg = CompFactory.CP.IsoCloseByCorrectionAlg(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def IsoCloseByCaloDecorCfg(flags, name="IsoCloseByCaloDecor", containers =[], **kwargs):
    result = ComponentAccumulator()
    ## Configure the tool such that the calo & pflow clusters are decorated
    ## https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/IsolationSelection/IsolationCloseByCorrectionTool.h#L35-39
    kwargs.setdefault("IsoCloseByCorrectionTool", result.popToolsAndMerge(
                      IsoCloseByCorrectionToolCfg(flags,
                                                  CaloCorrectionModel = -1
                                                  )))    
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

def IsoCloseByAlgsCfg(flags, isPhysLite = False, containerNames = [ "Muons", "Electrons", "Photons"], useSelTools = False, stream_name="", ttva_wp = "Nonprompt_All_MaxWeight"):

    # Add in two ways to do IsoCloseBy correction:
    #   - use IsoCloseByCorrAlg to modify the <iso_value>s for close by lepton/photon. 
    #     These can be used directly reading a derivation.
    #   - Also add in extra information to run IsolationCloseByTool on the derivation
    # For closeByIso correction, only one way is needed. The other way can be a cross check.
    # The second way will be eventually depricated and is not used for PhysLite to minimize the 
    # information on PhysLite.
    acc = ComponentAccumulator()

    # Add additional information to derivation output to be able to run IsoCloseByCorrectionTool on it 
    if not isPhysLite:
        from IsolationSelection.IsolationSelectionConfig import IsoCloseByCorrSkimmingAlgCfg, IsoCloseByCaloDecorCfg
        ### Add the tracks that potentially polute the isolation cones of others to the collection. 
        ### Question: Is the list of recommended TTVA working points used for isolation available somewhere?
        acc.merge(IsoCloseByCorrSkimmingAlgCfg(flags, ttva_wp = "Nonprompt_All_MaxWeight",
                                                            OutputStream = stream_name))
        
        ### Associate the close-by pflow objects and the calorimeter clusters
        acc.merge(IsoCloseByCaloDecorCfg(flags,
                                         containers = containerNames ))

    # Setup the isolation close-by correction algorithm sequence to correct the isolation of near-by el, mu, ph
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByCorrAlgCfg
    acc.merge(IsoCloseByCorrAlgCfg(flags, isPhysLite = isPhysLite, containerNames = containerNames, useSelTools = useSelTools))


    return acc

def setupIsoCloseBySlimmingVariables(slimmingHelper, isLLP1 = False):

    ### Extra variables needed to make the isolation close-by correction work when running on derivation
    ### https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/Root/IsolationCloseByCorrectionTool.cxx#L23-30
    iso_corr_vars = [ "IsoCloseByCorr_assocClustEta", "IsoCloseByCorr_assocClustPhi", "IsoCloseByCorr_assocClustEnergy",
                "IsoCloseByCorr_assocClustDecor", "IsoCloseByCorr_assocPflowEta", "IsoCloseByCorr_assocPflowPhi", "IsoCloseByCorr_assocPflowEnergy",
                "IsoCloseByCorr_assocPflowDecor"]

    if isLLP1:
        slimmingHelper.ExtraVariables += ["Electrons."+(".".join(iso_corr_vars)),
                                          "LRTElectrons."+(".".join(iso_corr_vars)),
                                          "Muons."+(".".join(iso_corr_vars)),
                                          "MuonsLRT."+(".".join(iso_corr_vars)) ]
    else:
        slimmingHelper.ExtraVariables += ["Electrons."+(".".join(iso_corr_vars)),
                                          "Muons."+(".".join(iso_corr_vars)) ]
