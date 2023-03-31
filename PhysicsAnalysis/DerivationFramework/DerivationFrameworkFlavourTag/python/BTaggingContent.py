# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import LHCPeriod


JetStandardAux = \
    [ "pt"
    , "eta"
    , "btaggingLink"
    , "HadronConeExclTruthLabelID"
    , "HadronConeExclTruthLabelBarcode"
    , "HadronConeExclExtendedTruthLabelID"
    , "ConeExclBHadronsFinal"
    , "ConeExclCHadronsFinal"
    ]

BTaggingStandardRun3Aux = \
    [
      "DL1r_pu"
    , "DL1r_pc"
    , "DL1r_pb"

    , "DL1dv00_pu" #“recommended r22 tagger” which is DL1dLoose20210824r22 named DL1dv00
    , "DL1dv00_pc"
    , "DL1dv00_pb"
    , "DL1dv01_pu" # new recommended r22 tagger named DL1dv01 which is DL1dLoose20220509
    , "DL1dv01_pc"
    , "DL1dv01_pb"

    , "dipsLoose20210729_pu"
    , "dipsLoose20210729_pc"
    , "dipsLoose20210729_pb"
    , "dipsLoose20220314v2_pu"
    , "dipsLoose20220314v2_pc"
    , "dipsLoose20220314v2_pb"
    , "dipsLooseVR20230208_pu"
    , "dipsLooseVR20230208_pc"
    , "dipsLooseVR20230208_pb"

    , "SV1_NGTinSvx"
    , "SV1_masssvx"

    , "DL1dv00Flip_pu"
    , "DL1dv00Flip_pc"
    , "DL1dv00Flip_pb"

    , "DL1r20210824r22Flip_pu"
    , "DL1r20210824r22Flip_pc"
    , "DL1r20210824r22Flip_pb"

    , "DL1dv01Flip_pu"
    , "DL1dv01Flip_pc"
    , "DL1dv01Flip_pb"

    , "dipsLoose20220314v2flip_pu"
    , "dipsLoose20220314v2flip_pc"
    , "dipsLoose20220314v2flip_pb"

    , "GN120220509_pb"
    , "GN120220509_pc"
    , "GN120220509_pu"
    , "GN2v00_pb"
    , "GN2v00_pc"
    , "GN2v00_pu"

    , "GN120220509Flip_pb"
    , "GN120220509Flip_pc"
    , "GN120220509Flip_pu"
    , "GN2v00Flip_pb"
    , "GN2v00Flip_pc"
    , "GN2v00Flip_pu"

    , "GN120220509Neg_pb"
    , "GN120220509Neg_pc"
    , "GN120220509Neg_pu"
    , "GN2v00Neg_pb"
    , "GN2v00Neg_pc"
    , "GN2v00Neg_pu"
    ]

BTaggingStandardRun4Aux = [
    "SV1_NGTinSvx",
    "SV1_masssvx",

    "dipsrun420221008_pu",
    "dipsrun420221008_pc",
    "dipsrun420221008_pb",

    "DL1drun420221017_pu",
    "DL1drun420221017_pc",
    "DL1drun420221017_pb",

    "GN1run420221010_pu",
    "GN1run420221010_pc",
    "GN1run420221010_pb"
]


# These are the inputs to DL1rmu + SMT
BTaggingHighLevelAux = [
    "softMuon_dR",
    "softMuon_pTrel",
    "softMuon_scatteringNeighbourSignificance",
    "softMuon_momentumBalanceSignificance",
    "softMuon_qOverPratio",
    "softMuon_ip3dD0",
    "softMuon_ip3dD0Significance",
    "softMuon_ip3dZ0",
    "softMuon_ip3dZ0Significance",
    "JetFitter_mass",
    "JetFitter_isDefaults",
    "JetFitter_energyFraction",
    "JetFitter_significance3d",
    "JetFitter_nVTX",
    "JetFitter_nSingleTracks",
    "JetFitter_nTracksAtVtx",
    "JetFitter_N2Tpair",
    "JetFitter_deltaR",
    "SV1_isDefaults",
    "SV1_N2Tpair",
    "SV1_efracsvx",
    "SV1_deltaR",
    "SV1_Lxy",
    "SV1_L3d",
    "SV1_significance3d",
    "IP2D_bu",
    "IP2D_isDefaults",
    "IP2D_bc",
    "IP2D_cu",
    "IP3D_bu",
    "IP3D_isDefaults",
    "IP3D_bc",
    "IP3D_cu",
    "JetFitterSecondaryVertex_nTracks",
    "JetFitterSecondaryVertex_isDefaults",
    "JetFitterSecondaryVertex_mass",
    "JetFitterSecondaryVertex_energy",
    "JetFitterSecondaryVertex_energyFraction",
    "JetFitterSecondaryVertex_displacement3d",
    "JetFitterSecondaryVertex_displacement2d",
    "JetFitterSecondaryVertex_maximumTrackRelativeEta",
    "JetFitterSecondaryVertex_minimumTrackRelativeEta",
    "JetFitterSecondaryVertex_averageTrackRelativeEta",
    "JetFitterDMeson_mass",
    "JetFitterDMeson_isDefaults",
    "maximumTrackRelativeEta",
    "minimumTrackRelativeEta",
    "averageTrackRelativeEta",
    "rnnip_pb",
    "rnnip_pc",
    "rnnip_pu",
    "softMuon_pb",
    "softMuon_pc",
    "softMuon_pu",
    "softMuon_isDefaults"
]

BTaggingHighLevelRun3Aux = BTaggingHighLevelAux \
                           + [ "DL1r20210519r22_pu"
                               , "DL1r20210519r22_pc"
                               , "DL1r20210519r22_pb"
                               , "DL1r20210824r22_pu"
                               , "DL1r20210824r22_pc"
                               , "DL1r20210824r22_pb"

                               , "dips20210729_pu"
                               , "dips20210729_pc"
                               , "dips20210729_pb"

                               , "DL1d20210824r22_pu"
                               , "DL1d20210824r22_pc"
                               , "DL1d20210824r22_pb" ]

BTaggingHighLevelRun4Aux = BTaggingHighLevelAux

JetGhostLabelAux = [
    "GhostBHadronsFinalCount",
    "GhostCHadronsFinalCount",
    "GhostTausFinalCount",
]

BTaggingExtendedAux = [
    "BTagTrackToJetAssociator",
]

JetExtendedAux = [
    "GhostBHadronsFinalCount",
    "GhostBHadronsFinalPt",
    "GhostCHadronsFinalCount",
    "GhostCHadronsFinalPt",
    "GhostTausFinalCount",
    "GhostTausFinalPt",
    "GhostTrack",
]

def BTaggingExpertContent(jetcol, ConfigFlags = None):

    btaggingtmp = "BTagging_" + jetcol.split('Jets')[0]
    if 'BTagging' in jetcol:
         stamp = jetcol.split('BTagging')[1]
         btaggingtmp += '_'+stamp

    # deal with name mismatch between PV0TrackJets and BTagging_Track
    btagging = btaggingtmp.replace("PV0Track", "Track")

    jetAllAux = JetStandardAux + JetExtendedAux
    jetcontent = [ ".".join( [ jetcol + "Aux" ] + jetAllAux ) ]

    isRun4 = False
    if ConfigFlags is not None:
        isRun4 = ConfigFlags.GeoModel.Run >= LHCPeriod.Run4

    # add aux variables
    btaggingAllAux = ( (BTaggingHighLevelRun4Aux if isRun4 else BTaggingHighLevelRun3Aux)
                      + (BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux)
                      + BTaggingExtendedAux)
    btagcontent = [ ".".join( [ btagging + "Aux" ] + btaggingAllAux ) ]

    return [jetcol] + jetcontent + [ btagging ] + btagcontent


def BTaggingStandardContent(jetcol, ConfigFlags = None):

    btaggingtmp = "BTagging_" + jetcol.split('Jets')[0]
    if 'BTagging' in jetcol:
         stamp = jetcol.split('BTagging')[1]
         btaggingtmp += '_'+stamp
    # deal with name mismatch between PV0TrackJets and BTagging_Track
    btagging = btaggingtmp.replace("PV0Track", "Track")

    jetcontent = [ jetcol ] + [
        ".".join( [ jetcol + "Aux" ] + JetStandardAux )
    ]

    isRun4 = False
    if ConfigFlags is not None:
        isRun4 = ConfigFlags.GeoModel.Run >= LHCPeriod.Run4

    aux = BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux
    btagcontent = [ btagging ] + [ ".".join([ btagging + "Aux" ] + aux) ]

    return jetcontent + btagcontent


def BTaggingXbbContent(jetcol, ConfigFlags = None):

    btaggingtmp = "BTagging_" + jetcol.split('Jets')[0]
    if 'BTagging' in jetcol:
         stamp = jetcol.split('BTagging')[1]
         btaggingtmp += '_'+stamp

    # deal with name mismatch between PV0TrackJets and BTagging_Track
    btagging = btaggingtmp.replace("PV0Track", "Track")

    jetAllAux = JetStandardAux + JetGhostLabelAux
    jetcontent = [ ".".join( [ jetcol + "Aux" ] + jetAllAux ) ]

    isRun4 = False
    if ConfigFlags is not None:
        isRun4 = ConfigFlags.GeoModel.Run >= LHCPeriod.Run4

    hl_aux = BTaggingHighLevelRun4Aux if isRun4 else BTaggingHighLevelRun3Aux
    aux = BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux
    # add aux variables
    btaggingAllAux = aux + hl_aux
    btagcontent = [ ".".join( [ btagging + "Aux" ] + btaggingAllAux ) ]

    return [jetcol] + jetcontent + [ btagging ] + btagcontent
