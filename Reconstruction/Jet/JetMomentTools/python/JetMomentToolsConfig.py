# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

"""
                                                                      
 JetMomentToolsConfig: A helper module for configuring jet moment     
 tools, in support of JetRecConfig.JetModConfig. 
 Author: TJ Khoo                                                      


IMPORTANT : all the getXYZTool(jetdef, modspec) functions are meant to be used as callback from the main JetRecConfig module 
when we need to convert definitions of tools into the actual tools. At this point the functions are invoked as 
 func(jetdef, modspec)
Hence they have jetdef and modspec arguments even if not needed in every case.
"""

from AthenaCommon import Logging
jetmomentlog = Logging.logging.getLogger('JetMomentToolsConfig')

from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.StandardJetContext import jetContextDic

from xAODBase.xAODType import xAODType


def idTrackSelToolFromJetCtx(trkOpt="default"):
    """returns a InDetTrackSelectionTool configured with the jet context corresponding to trkOpt
      (technically wrapped inside a JetTrackSelectionTool) 
    """
    # JetTrackSelectionTool is still used by trk moment tools.
    # it should be deprecated in favor of simply the InDet tool
    from JetRecConfig.StandardJetContext import jetContextDic
    from JetRecTools.JetRecToolsConfig import getIDTrackSelectionTool #

    return  CompFactory.JetTrackSelectionTool(
        f"tracsel{trkOpt}",
        Selector        = getIDTrackSelectionTool(f"trackSel{trkOpt}", **jetContextDic[trkOpt]["trackSelOptions"])
    )



def getEMScaleMomTool(jetdef, modspec=""):
    # This may need updating e.g. for evolving trigger cluster container names
    # We do the non-trivial summation over constituents unless the jets were
    # built directly from EM-scale topoclusters, in which case we can just
    # copy the constituent scale
    useUncalibConstits = False
    if jetdef.inputdef.basetype==xAODType.CaloCluster:
        builtFromEMClusters = jetdef.inputdef.inputname in ["CaloCalTopoClusters","HLT_CaloTopoClustersFS"] and jetdef.inputdef.modifiers==["EM"]
        useUncalibConstits = not builtFromEMClusters
    elif (jetdef.inputdef.basetype==xAODType.ParticleFlow or jetdef.inputdef.basetype==xAODType.FlowElement):
        useUncalibConstits = True
    else:
        raise ValueError("EM scale momentum not defined for input type {}".format(jetdef.inputdef.basetype))

    emscalemom = CompFactory.JetEMScaleMomTool(
        "emscalemom_{}".format(jetdef.basename),
        UseUncalibConstits = useUncalibConstits,
        JetContainer = jetdef.fullname(),
    )

    return emscalemom

def getConstitFourMomTool(jetdef, modspec=""):
    ### Not ideal, but because CaloCluster.Scale is an internal class
    ### it makes the dict load really slow.
    ### So just copy the enum to a dict...
    ### Defined in Event/xAOD/xAODCaloEvent/versions/CaloCluster_v1.h
    CaloClusterStates = {
      "UNKNOWN"       : -1,
      "UNCALIBRATED"  :  0,
      "CALIBRATED"    :  1,
      "ALTCALIBRATED" :  2,
      "NSTATES"       :  3
      }

    cfourmom = CompFactory.JetConstitFourMomTool("constitfourmom_{0}".format(jetdef.basename))
    if "LCTopo" in jetdef.basename or "EMTopo" in jetdef.basename:
        cfourmom.JetScaleNames = ["DetectorEtaPhi"]
        if "HLT_" in jetdef.fullname():
            cfourmom.AltConstitColls = [""]
            cfourmom.AltConstitScales = [0]
            cfourmom.AltJetScales = ["JetConstitScaleMomentum"]
        else:
            clstate = "CALIBRATED" if "LCTopo" in jetdef.basename else "UNCALIBRATED"
            cfourmom.AltConstitColls = ["CaloCalTopoClusters"]
            cfourmom.AltConstitScales = [CaloClusterStates[clstate]]
            cfourmom.AltJetScales = [""]
    # Drop the LC-calibrated four-mom for EMTopo jets as we only wanted it as a possibility
    # in MET CST calculations but never used it
    elif "PFlow" in jetdef.basename or "UFO" in jetdef.basename:
        cfourmom.JetScaleNames = ["DetectorEtaPhi"]
        cfourmom.AltConstitColls = [""]
        cfourmom.AltConstitScales = [0]
        cfourmom.AltJetScales = ["JetConstitScaleMomentum"]

    return cfourmom

# Jet vertex fraction with selection.
def getJVFTool(jetdef, modspec):
    # retrieve the tracking keys to be used with modspec :
    trackingKeys = jetContextDic[modspec or jetdef.context]
    jvf = CompFactory.JetVertexFractionTool(
        "jvf",
        VertexContainer = trackingKeys["Vertices"],
        AssociatedTracks = trackingKeys["GhostTracksLabel"],
        TrackVertexAssociation = trackingKeys["TVA"],
        TrackParticleContainer  = trackingKeys["Tracks"],
        TrackSelector = idTrackSelToolFromJetCtx(modspec or jetdef.context),
        SuppressInputDependence = True
    )
    return jvf


# Jet vertex tagger with selection.
def getJVTTool(jetdef, modspec):
    jvt = CompFactory.JetVertexTaggerTool(
        "jvt",
        VertexContainer = jetContextDic[modspec or jetdef.context]["Vertices"],
        SuppressInputDependence = True
    )
    return jvt

# Jet vertex tagger with neural network.
def getNNJvtTool(jetdef, modspec):
    nnjvt = CompFactory.getComp("JetPileupTag::JetVertexNNTagger")(
        "nnjvt",
        VertexContainer = jetContextDic[modspec or jetdef.context]["Vertices"],
        SuppressInputDependence = True
    )
    return nnjvt


def getTrackMomentsTool(jetdef, modspec):
    # retrieve the tracking keys to be used with modspec : 
    trackingKeys = jetContextDic[modspec or jetdef.context]

    trackmoments = CompFactory.JetTrackMomentsTool(
        "trkmoms",
        VertexContainer = trackingKeys["Vertices"],
        AssociatedTracks = trackingKeys["GhostTracksLabel"],
        TrackVertexAssociation = trackingKeys["TVA"],
        TrackMinPtCuts = [500, 1000],
        TrackSelector = idTrackSelToolFromJetCtx(modspec or jetdef.context), 
        DoPFlowMoments = 'PFlow' in jetdef.fullname() or 'UFO' in jetdef.fullname() ,
    )
    return trackmoments

def getTrackSumMomentsTool(jetdef, modspec):
    jettrackselloose = idTrackSelToolFromJetCtx(modspec or jetdef.context)
    # retrieve the tracking keys to be used with modspec : 
    trackingKeys = jetContextDic[modspec or jetdef.context]
    tracksummoments = CompFactory.JetTrackSumMomentsTool(
        "trksummoms",
        VertexContainer = trackingKeys["Vertices"],
        AssociatedTracks = trackingKeys["GhostTracksLabel"],
        TrackVertexAssociation = trackingKeys["TVA"],
        RequireTrackPV = True,
        TrackSelector = jettrackselloose
    )
    return tracksummoments

# This tool sets a decoration saying which the nominal HS PV was.
# Historically it did the origin correction, but now we do this to constituents
def getOriginCorrVxTool(jetdef, modspec):
    origin_setpv = CompFactory.JetOriginCorrectionTool(
      "jetorigin_setpv",
      VertexContainer = jetContextDic[modspec or jetdef.context]["Vertices"],
      OriginCorrectedName = "",
      OnlyAssignPV = True,
    )
    return origin_setpv


def getJetPtAssociationTool(jetdef, modspec):

    from JetRecConfig.JetDefinition import buildJetAlgName

    truthJetAlg = buildJetAlgName(jetdef.algorithm, jetdef.radius)+'Truth'+str(modspec)+'Jets'

    jetPtAssociation = CompFactory.JetPtAssociationTool('jetPtAssociation',
                                                        MatchingJetContainer = truthJetAlg,
                                                        AssociationName = "GhostTruth")

    return jetPtAssociation


def getQGTaggingTool(jetdef, modspec):

    trackingKeys = jetContextDic[modspec or jetdef.context]

    qgtagging = CompFactory.JetQGTaggerVariableTool('qgtagging',
                                                    VertexContainer = trackingKeys["Vertices"],
                                                    TrackVertexAssociation = trackingKeys["TVA"],
                                                    TrackSelector = idTrackSelToolFromJetCtx(modspec or jetdef.context),
                                                   )

    return qgtagging


def getPFlowfJVTTool(jetdef, modspec):

    from JetCalibTools import JetCalibToolsConfig
    calibString = "AnalysisLatest:mc:JetArea_Residual_EtaJES"
    if( modspec and modspec == "CustomVtx" ) :
      calibString = "AnalysisLatest:mc:JetArea_Residual_EtaJES:Kt4EMPFlowCustomVtxEventShape:HggPrimaryVertices"
    jetCalibrationTool = JetCalibToolsConfig.getJetCalibToolFromString(jetdef, calibString)

    wPFOTool = CompFactory.getComp('CP::WeightPFOTool')("fJVT__wPFO")

    trackingKeys = jetContextDic[modspec or jetdef.context]

    fJVTTool = CompFactory.JetForwardPFlowJvtTool("fJVT",
                                                  verticesName = trackingKeys["Vertices"],
                                                  TrackVertexAssociation = trackingKeys["TVA"],
                                                  WeightPFOTool = wPFOTool,
                                                  JetCalibrationTool = jetCalibrationTool,
                                                  FEName = jetdef.inputdef.containername,
                                                  ORName = "",
                                                  FjvtRawName = "DFCommonJets_fJvt",
                                                  includePV = False)

    return fJVTTool


def getPFlowbJVTTool(jetdef, modspec):

    from JetCalibTools import JetCalibToolsConfig
    jetCalibrationTool = JetCalibToolsConfig.getJetCalibToolFromString(jetdef, "AnalysisLatest:mc:JetArea_Residual_EtaJES")

    wPFOTool = CompFactory.getComp('CP::WeightPFOTool')("bJVT__wPFO")

    trackingKeys = jetContextDic[modspec or jetdef.context]

    bJVTTool = CompFactory.JetBalancePFlowJvtTool('bJVT',
                                                  verticesName = trackingKeys["Vertices"],
                                                  TrackVertexAssociation = trackingKeys["TVA"],
                                                  WeightPFOTool = wPFOTool,
                                                  JetCalibrationTool = jetCalibrationTool,
                                                  FEName = jetdef.inputdef.containername,
                                                  ORNameFE = "",
                                                  BjvtRawName = 'DFCommonJets_bJvt',
                                                  includePV = True)

    return bJVTTool
