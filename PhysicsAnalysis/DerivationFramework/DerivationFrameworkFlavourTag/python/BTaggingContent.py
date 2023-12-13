"""
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

Define sets of standard variables to save in output files
"""

from AthenaConfiguration.Enums import LHCPeriod

def _getVars(name, extra_flavours=None, flip_modes=None):
    """Convenience function for getting output variable names"""
    if extra_flavours is None:
        extra_flavours = []
    if flip_modes is None:
        flip_modes = [""]
    flavors = list("cub") + extra_flavours
    variants = [""] + flip_modes
    return [f'{name}{v}_p{f}' for v in variants for f in flavors]

# some jet variables we always want to save
JetStandardAux = [
    "pt",
    "eta",
    "btaggingLink",
    "HadronConeExclTruthLabelID",
    "HadronConeExclTruthLabelBarcode",
    "HadronConeExclExtendedTruthLabelID",
    "ConeExclBHadronsFinal",
    "ConeExclCHadronsFinal",
    "jetFoldHash",
]

# standard outputs for Run 3
BTaggingStandardRun3Aux = [
    "SV1_NGTinSvx",
    "SV1_masssvx",
]
BTaggingStandardRun3Aux += _getVars("dipsLoose20210729")
BTaggingStandardRun3Aux += _getVars("dipsLoose20220314v2")
BTaggingStandardRun3Aux += _getVars("dipsLooseVR20230208")
BTaggingStandardRun3Aux += _getVars("DL1r20210824r22Flip") # flipped version of DL1r retrained in r22
BTaggingStandardRun3Aux += _getVars("dipsLoose20220314v2flip")
BTaggingStandardRun3Aux += _getVars("DL1dv00", flip_modes=['Flip']) # preliminary r22 tagger which used DL1dLoose20210824r22
BTaggingStandardRun3Aux += _getVars("DL1dv01", flip_modes=['Flip']) # summer 2023 recommended r22 tagger which uses DL1dLoose20220509
BTaggingStandardRun3Aux += _getVars("GN2v00", flip_modes=['Simple']) # preliminary GN2 tagger
BTaggingStandardRun3Aux += _getVars("GN2v01", extra_flavours=['tau'], flip_modes=['Simple']) # planned GN2 tagger for first 2024 recommendations

# standard outputs for Run 4
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


# more involved outputs we might not want to save (ExpertContent)
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
    "softMuon_pb",
    "softMuon_pc",
    "softMuon_pu",
    "softMuon_isDefaults"
]

# ExpertContent for Run 3
BTaggingHighLevelRun3Aux = BTaggingHighLevelAux
BTaggingHighLevelRun3Aux += _getVars("DL1r20210824r22") # r22 retraining of DL1r
BTaggingHighLevelRun3Aux += _getVars("dips20210729")
BTaggingHighLevelRun3Aux += _getVars("DL1d20210824r22")
BTaggingHighLevelRun3Aux += _getVars("GN120220509", flip_modes=['Simple'])

# ExpertContent for Run 4
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


def _getBtagging(jetcol):
    """Convenience function for getting btagging names"""
    btaggingtmp = "BTagging_" + jetcol.split('Jets')[0]
    if 'BTagging' in jetcol:
         stamp = jetcol.split('BTagging')[1]
         btaggingtmp += '_'+stamp
    # deal with name mismatch between PV0TrackJets and BTagging_Track
    btagging = btaggingtmp.replace("PV0Track", "Track")
    return btagging


def _isRun4(ConfigFlags):
    """Convenience function for checking if we are in Run4"""
    return ConfigFlags is not None and ConfigFlags.GeoModel.Run >= LHCPeriod.Run4


def BTaggingExpertContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)
    jetAllAux = JetStandardAux + JetExtendedAux
    jetcontent = [ ".".join( [ jetcol + "Aux" ] + jetAllAux ) ]
    isRun4 = _isRun4(ConfigFlags)

    # add aux variables
    btaggingAllAux = ( (BTaggingHighLevelRun4Aux if isRun4 else BTaggingHighLevelRun3Aux)
                      + (BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux)
                      + BTaggingExtendedAux)
    btagcontent = [ ".".join( [ btagging + "Aux" ] + btaggingAllAux ) ]

    return [jetcol] + jetcontent + [ btagging ] + btagcontent


def BTaggingStandardContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)
    jetcontent = [ jetcol ] + [".".join( [ jetcol + "Aux" ] + JetStandardAux )]
    isRun4 = _isRun4(ConfigFlags)

    aux = BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux
    btagcontent = [ btagging ] + [ ".".join([ btagging + "Aux" ] + aux) ]

    return jetcontent + btagcontent


def BTaggingXbbContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)
    jetAllAux = JetStandardAux + JetGhostLabelAux
    jetcontent = [ ".".join( [ jetcol + "Aux" ] + jetAllAux ) ]
    isRun4 = _isRun4(ConfigFlags)

    hl_aux = BTaggingHighLevelRun4Aux if isRun4 else BTaggingHighLevelRun3Aux
    aux = BTaggingStandardRun4Aux if isRun4 else BTaggingStandardRun3Aux
    # add aux variables
    btaggingAllAux = aux + hl_aux
    btagcontent = [ ".".join( [ btagging + "Aux" ] + btaggingAllAux ) ]

    return [jetcol] + jetcontent + [ btagging ] + btagcontent
