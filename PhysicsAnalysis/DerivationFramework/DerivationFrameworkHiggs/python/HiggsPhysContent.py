 # Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def setupHiggsAugmentationAlgs(ConfigFlags, stream_name="", ttva_wp = "Nonprompt_All_MaxWeight"):
    acc = ComponentAccumulator()
    from DerivationFrameworkHiggs.FourLeptonVertexing import FourLeptonVertexerCfg
    acc.merge(FourLeptonVertexerCfg(ConfigFlags))
    if ConfigFlags.Input.isMC:
        from DerivationFrameworkHiggs.TruthCategoriesConfig import TruthCategoriesDecoratorCfg
        acc.merge(TruthCategoriesDecoratorCfg(ConfigFlags))

    from IsolationSelection.IsolationSelectionConfig import IsoCloseByCorrSkimmingAlgCfg, IsoCloseByCaloDecorCfg
    ### Add the tracks that potentially polute the isolation cones of others to the collection. 
    ### Question: Is the list of recommended TTVA working points used for isolation available somewhere?
    acc.merge(IsoCloseByCorrSkimmingAlgCfg(ConfigFlags, ttva_wp = "Nonprompt_All_MaxWeight",
                                                        OutputStream = stream_name))

    ### Associate the close-by pflow objects and the calorimeter clusters
    acc.merge(IsoCloseByCaloDecorCfg(ConfigFlags,
                                    containers = ["Electrons", "Muons", "Photons"] ))
    return acc

def setupHiggsSlimmingVariables(ConfigFlags, slimmingHelper):
    ### Extra variables needed by the H->ZZ->4L analysis to make the isolation close-by correction work
    ### https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/Root/IsolationCloseByCorrectionTool.cxx#L23-30
    iso_corr_vars = [ "IsoCloseByCorr_assocClustEta", "IsoCloseByCorr_assocClustPhi", "IsoCloseByCorr_assocClustEnergy",
                "IsoCloseByCorr_assocClustDecor", "IsoCloseByCorr_assocPflowEta", "IsoCloseByCorr_assocPflowPhi", "IsoCloseByCorr_assocPflowEnergy",
                "IsoCloseByCorr_assocPflowDecor"]

    slimmingHelper.ExtraVariables += ["Electrons."+(".".join(iso_corr_vars)),
                                           "Muons."+(".".join(iso_corr_vars)) ]
    if ConfigFlags.Input.isMC:
        htxs_vars = [
                "HTXS_prodMode",
            "HTXS_errorCode",
            "HTXS_Stage0_Category",
            "HTXS_Stage1_Category_pTjet25",
            "HTXS_Stage1_Category_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet25",
            "HTXS_Stage1_2_Category_pTjet25",
            "HTXS_Stage1_2_Category_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet25",
            "HTXS_Stage1_2_Fine_Category_pTjet25",
            "HTXS_Stage1_2_Fine_Category_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet25",
            "HTXS_Njets_pTjet25",
            "HTXS_Njets_pTjet30",
            "HTXS_isZ2vvDecay","HTXS_Higgs_pt"]
        for p4_var in ["HTXS_Higgs", "HTXS_V", "HTXS_V_jets25", "HTXS_V_jets30", "HTXS_Higgs_decay", "HTXS_V_decay" ]:
           htxs_vars += [p4_var+"_eta", p4_var+"_phi", p4_var+"_pt",p4_var+"_m"]
        slimmingHelper.ExtraVariables += ["EventInfo." + ".".join(htxs_vars)] 
    slimmingHelper.StaticContent += ["xAOD::VertexContainer#FourLeptonVertices"]
    slimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#FourLeptonVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"]
    
 