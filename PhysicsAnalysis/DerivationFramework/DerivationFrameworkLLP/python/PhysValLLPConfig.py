# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the LLP tools used in DAOD_PHYSVAL
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PhysValLLPCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    # LRT track merge
    from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
    acc.merge(InDetLRTMergeCfg(flags))

    # LRT muons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    acc.merge(LRTMuonMergerAlg( flags,
                                PromptMuonLocation    = "Muons",
                                LRTMuonLocation       = "MuonsLRT",
                                OutputMuonLocation    = "StdWithLRTMuons",
                                CreateViewCollection  = True))

    # LRT electrons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
    acc.merge(LRTElectronMergerAlg( flags,
                                    PromptElectronLocation = "Electrons",
                                    LRTElectronLocation    = "LRTElectrons",
                                    OutputCollectionName   = "StdWithLRTElectrons",
                                    isDAOD                 = False,
                                    CreateViewCollection   = True))


    # LLP Secondary Vertexing
    from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg

    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive",
                                 AugmentingVersionString  = "",
                                 FillIntermediateVertices = False,
                                 TrackLocation            = "InDetWithLRTTrackParticles"))

    # leptons-only VSI
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet_"+"_LeptonsMod_LRTR3_1p0",
                                 AugmentingVersionString     = "_LeptonsMod_LRTR3_1p0",
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = "InDetWithLRTTrackParticles",
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectTracksWithLRTCuts   = True,
                                 doSelectTracksFromMuons     = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doSelectTracksFromElectrons = True,
                                 MuonLocation                = "StdWithLRTMuons",
                                 ElectronLocation            = "StdWithLRTElectrons"))

    return acc
