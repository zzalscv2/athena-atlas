# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

########################################################################
#                                                                      #
# JetModConfig: A helper module for configuring tools that support     #
# jet reconstruction                                                   #
# Author: TJ Khoo                                                      #
#                                                                      #
########################################################################

from AthenaCommon import Logging
jrtlog = Logging.logging.getLogger('JetRecToolsConfig')

from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.JetRecConfig import isAnalysisRelease



def getIDTrackSelectionTool(toolname, **toolProps):
    """returns a InDetTrackSelectionTool configured with toolProps
     (except in Analysis releases where some un-used (?) options are explicitely turned off)
    """
    idtracksel = CompFactory.getComp("InDet::InDetTrackSelectionTool")(toolname, **toolProps)

    if not isAnalysisRelease():
        # thes options can not be set in AnalysisBase/AthAnalysis. (but not setting them is equivalent to set them to False)
        idtracksel.UseTrkTrackTools = False
        idtracksel.Extrapolator     = ""
        idtracksel.TrackSummaryTool = ""
    return idtracksel

def getTrackSelAlg(trkOpt="default", trackSelOpt=False):
    from JetRecConfig.StandardJetContext import jetContextDic
    trkProperties = jetContextDic[trkOpt]

    trackToolProps = dict(**trkProperties["trackSelOptions"])

    if not trackSelOpt:
        # track selection from trkOpt but OVERWRITING the CutLevel for e.g. ghosts (typically, only a pt>500MeV cut remains): 
        trackToolProps.update( CutLevel=trkProperties['GhostTrackCutLevel'] )
        outContainerKey = "JetTracks"
        trkOpt= trkOpt+'ghost' # just so it gives a different tool name below
    else:
        outContainerKey = "JetTracksQualityCuts"

    # build the selection alg 
    trkSelAlg = CompFactory.JetTrackSelectionAlg( f"trackselalg_{trkOpt}_{trkProperties[outContainerKey]}",
                                                  TrackSelector = getIDTrackSelectionTool(f"tracksel{trkOpt}",**trackToolProps),
                                                  InputContainer = trkProperties["Tracks"],
                                                  OutputContainer = trkProperties[outContainerKey],
                                                  DecorDeps = ["TTVA_AMVFWeights_forReco", "TTVA_AMVFVertices_forReco"] # Hardcoded for now... we might want to have this context-dependent ??
                                                 )

    return trkSelAlg


def getJetTrackVtxAlg( trkOpt, algname="jetTVA", **ttva_overide):
    """  theSequence and ttva_overide are options used in trigger  (HLT/Jet/JetTrackingConfig.py)"""
    from TrackVertexAssociationTool.getTTVAToolForReco import getTTVAToolForReco
    from JetRecConfig.StandardJetContext import jetContextDic

    trkProperties = jetContextDic[trkOpt]

    ttva_options = dict(
        returnCompFactory = True,
        addDecoAlg = False, # We add it ourselves for sequence management
        TrackContName = trkProperties["Tracks"],
        VertexContName = trkProperties["Vertices"],
        HardScatterLinkDeco = ""
    )
    # allow client to overide options : 
    ttva_options.update(**ttva_overide)
    
    idtvassoc = getTTVAToolForReco( "jetTVatool", **ttva_options )

    alg = CompFactory.JetTrackVtxAssoAlg(algname,
                                         TrackParticleContainer  = trkProperties["Tracks"],
                                         TrackVertexAssociation  = trkProperties["TVA"],
                                         VertexContainer         = trkProperties["Vertices"],
                                         TrackVertexAssoTool     = idtvassoc                                         
                                         )
    return alg


def getPV0TrackVertexAssoAlg(trkOpt="", theSequence=None):
    if trkOpt: "_{}".format(trkOpt)
    from TrackVertexAssociationTool.getTTVAToolForReco import getTTVAToolForReco
    from JetRecConfig.StandardJetContext import jetContextDic

    trkProperties = jetContextDic[trkOpt]
    tvatool = getTTVAToolForReco("trackjettvassoc",
                                WorkingPoint = "Nonprompt_All_MaxWeight",
                                TrackContName = trkProperties["JetTracks"],
                                VertexContName = trkProperties["Vertices"],
                                returnCompFactory = True,
                                addDecoAlg = False, #setting this to True causes error in reconstruction because of there not being aux store associated with one of the decorations
                                                    #It has also been set to false in buildPV0TrackSel function in JetRecConfig/python/JetInputConfig.py 
                                )

    jettvassoc = CompFactory.JetTrackVtxAssoAlg("trackjetTVAAlg",
            TrackParticleContainer = trkProperties["JetTracks"],
            TrackVertexAssociation = "PV0"+trkProperties["TVA"],
            VertexContainer        = trkProperties["Vertices"],
            TrackVertexAssoTool    = tvatool
            )
    return jettvassoc, tvatool

def getPV0TrackSelAlg(tvaTool, trkOpt="default"):
    from JetRecConfig.StandardJetContext import jetContextDic
    trkProperties = jetContextDic[trkOpt]
    pv0trackselalg = CompFactory.PV0TrackSelectionAlg("pv0tracksel_trackjet",
            InputTrackContainer = trkProperties["JetTracks"],
            VertexContainer = trkProperties["Vertices"],
            OutputTrackContainer = "PV0"+trkProperties["JetTracks"],
            TVATool = tvaTool,
            )
    return pv0trackselalg

def getPFlowSelAlg():
    # PFlow objects matched to electrons/muons filtering algorithm 
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg",
                                              electronID = "LHMedium",
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalPFlowChargedParticleFlowObjects",
                                              NeutralPFlowOutputContainer = "GlobalPFlowNeutralParticleFlowObjects"
                                             )
