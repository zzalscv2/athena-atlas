# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# PhysCommonThinningConfig
# Contains the configuration for the thinning for PHYS(LITE)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysCommonThinningCfg(ConfigFlags, StreamName = "StreamDAOD_PHYS", **kwargs):
    """Configure the common augmentation"""
    acc = ComponentAccumulator()

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, TauTrackParticleThinningCfg, DiTauTrackParticleThinningCfg, TauJetLepRMParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    if "TrackParticleThinningToolName" in kwargs:
        tp_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
        acc.merge(TrackParticleThinningCfg(
            ConfigFlags,
            name                    = kwargs['TrackParticleThinningToolName'],
            StreamName              = StreamName, 
            SelectionString         = tp_thinning_expression,
            InDetTrackParticlesKey  = "InDetTrackParticles"))
        
    # Include inner detector tracks associated with muons
    if "MuonTPThinningToolName" in kwargs:
        acc.merge(MuonTrackParticleThinningCfg(
            ConfigFlags,
            name                    = kwargs['MuonTPThinningToolName'],
            StreamName              = StreamName,
            MuonKey                 = "Muons",
            InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # disable tau thinning for now
    if "TauJetThinningToolName" in kwargs:
        tau_thinning_expression = "(TauJets.ptFinalCalib >= 0)"
        acc.merge(GenericObjectThinningCfg(ConfigFlags,
            name            = kwargs['TauJetThinningToolName'],
            StreamName      = StreamName,
            ContainerName   = "TauJets",
            SelectionString = tau_thinning_expression))
    
    # Only keep tau tracks (and associated ID tracks) classified as charged tracks
    if "TauTPThinningToolName" in kwargs:
        acc.merge(TauTrackParticleThinningCfg(
            ConfigFlags,
            name                   = kwargs['TauTPThinningToolName'],
            StreamName             = StreamName,
            TauKey                 = "TauJets",
            InDetTrackParticlesKey = "InDetTrackParticles",
            DoTauTracksThinning    = True,
            TauTracksKey           = "TauTracks"))
    
    if "TauJets_MuonRMThinningToolName" in kwargs:
        tau_murm_thinning_expression = tau_thinning_expression.replace('TauJets', 'TauJets_MuonRM')
        acc.merge(TauJetLepRMParticleThinningCfg(
            ConfigFlags,
            name                   = kwargs["TauJets_MuonRMThinningToolName"],
            StreamName             = StreamName,
            originalTauKey         = "TauJets",
            LepRMTauKey            = "TauJets_MuonRM",
            InDetTrackParticlesKey = "InDetTrackParticles",
            TauTracksKey           = "TauTracks_MuonRM",
            SelectionString        = tau_murm_thinning_expression))
 
    # ID tracks associated with high-pt di-tau
    if "DiTauTPThinningToolName" in kwargs:
        acc.merge(DiTauTrackParticleThinningCfg(
            ConfigFlags,
            name                    = kwargs['DiTauTPThinningToolName'],
            StreamName              = StreamName,
            DiTauKey                = "DiTauJets",
            InDetTrackParticlesKey  = "InDetTrackParticles"))
 
    ## Low-pt di-tau thinning
    if "DiTauLowPtThinningToolName" in kwargs:
        acc.merge(GenericObjectThinningCfg(
            ConfigFlags,
            name            = kwargs['DiTauLowPtThinningToolName'],
            StreamName      = StreamName,
            ContainerName   = "DiTauJetsLowPt",
            SelectionString = "DiTauJetsLowPt.nSubjets > 1"))
    
    # ID tracks associated with low-pt ditau
    if "DiTauLowPtTPThinningToolName" in kwargs:
        acc.merge(DiTauTrackParticleThinningCfg(
            ConfigFlags,
            name                    = kwargs['DiTauLowPtTPThinningToolName'],
            StreamName              = StreamName,
            DiTauKey                = "DiTauJetsLowPt",
            InDetTrackParticlesKey  = "InDetTrackParticles",
            SelectionString         = "DiTauJetsLowPt.nSubjets > 1"))
 
    # keep calo clusters around electrons
    if "ElectronCaloClusterThinningToolName" in kwargs:
        acc.merge(CaloClusterThinningCfg(
            ConfigFlags,
            name                  = kwargs['ElectronCaloClusterThinningToolName'],
            StreamName            = StreamName,
            SGKey                 = "AnalysisElectrons",
            CaloClCollectionSGKey = "egammaClusters",
            ConeSize              = -1.0))
 
    # keep calo clusters around photons
    if "PhotonCaloClusterThinningToolName" in kwargs:
        acc.merge(CaloClusterThinningCfg(
            ConfigFlags,
            name                  = kwargs['PhotonCaloClusterThinningToolName'],
            StreamName            = StreamName,
            SGKey                 = "AnalysisPhotons",
            CaloClCollectionSGKey = "egammaClusters",
            ConeSize=-1.0))
 
    # GSF tracks associated to electrons
    if "ElectronGSFTPThinningToolName" in kwargs:
        ElectronGSFTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name                   = kwargs['ElectronGSFTPThinningToolName'],
            StreamName             = StreamName,
            SGKey                  = "AnalysisElectrons",
            GSFTrackParticlesKey   = "GSFTrackParticles",
            InDetTrackParticlesKey = "",
            BestMatchOnly          = True)
        acc.addPublicTool(ElectronGSFTPThinningTool)
 
    # GSF tracks associated to photons
    if "PhotonGSFTPThinningToolName" in kwargs:
        PhotonGSFTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name                   = kwargs['PhotonGSFTPThinningToolName'],
            StreamName             = StreamName,
            SGKey                  = "AnalysisPhotons",
            GSFTrackParticlesKey   = "GSFTrackParticles",
            InDetTrackParticlesKey = "",
            BestMatchOnly          = True)
        acc.addPublicTool(PhotonGSFTPThinningTool)

    return acc

