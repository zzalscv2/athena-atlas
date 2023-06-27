
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from JetRecTools import JetRecToolsConfig
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, conf2toConfigurable

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg
from InDetUsedInVertexFitTrackDecorator.UsedInVertexFitTrackDecoratorCfg import getUsedInVertexFitTrackDecoratorAlg

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from ..Config.MenuComponents import parOR

def retrieveJetContext(trkopt):
    """Tell the standard jet config about the specific track related options we are using here.

     This is done by defining a new jet context into jetContextDic.
     Then, later, passing this context name in the JetDefinition and standard helper functions will ensure
    these options will consistently be used everywhere.

    returns the context dictionary and the list of keys related to tracks in this dic.
    """

    from JetRecConfig.StandardJetContext import jetContextDic
    if trkopt not in jetContextDic:
        # *****************
        # Set the options corresponding to trkopt to a new entry in jetContextDic 
        trksig = {
            'ftf':    'jet',
            'roiftf': 'jetSuper',
        }[trkopt]
        IDTrigConfig = getInDetTrigConfig( trksig )

        tracksname = IDTrigConfig.tracks_FTF()
        verticesname = IDTrigConfig.vertex
            
        tvaname = f"JetTrackVtxAssoc_{trkopt}"
        label = f"GhostTrack_{trkopt}"
        ghosttracksname = f"PseudoJet{label}"
        
        jetContextDic[trkopt] = jetContextDic['default'].clone(
            Tracks           = tracksname,
            Vertices         = verticesname,
            TVA              = tvaname,
            GhostTracks      = ghosttracksname,
            GhostTracksLabel = label ,
            JetTracks        = f'JetSelectedTracks_{trkopt}',
        )

        # also declare some JetInputExternal corresponding to trkopt
        # This ensures the JetRecConfig helpers know about them.
        # We declare simplistic JetInputExternal, without algoBuilder, because the rest of the trigger config is in charge of producing these containers.
        from JetRecConfig.JetDefinition import JetInputExternal
        from xAODBase.xAODType import xAODType
        from JetRecConfig.StandardJetConstits import stdInputExtDic
        stdInputExtDic[tracksname] = JetInputExternal( tracksname, xAODType.TrackParticle )
        stdInputExtDic[verticesname] = JetInputExternal( verticesname, xAODType.Vertex )
        
    return jetContextDic[trkopt], jetContextDic["trackKeys"]

@AccumulatorCache
def JetFSTrackingCfg(flags, trkopt, RoIs):
    """ Create the tracking CA and return it as well as the output name dictionary """
    seqname = f"JetFSTracking_{trkopt}_RecoSequence"
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname))

    IDTrigConfig = getInDetTrigConfig( 'jet' )

    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flagsWithTrk = getFlagsForActiveConfig(flags, 'jet', log)

    assert trkopt == "ftf"
    acc.merge(
        trigInDetFastTrackingCfg(
            flagsWithTrk,
            RoIs,
            signatureName="jet",
            in_view=False
        ),
        seqname
    )

    # get the jetContext for trkopt (and build it if not existing yet)
    jetContext, trkKeys = retrieveJetContext(trkopt)

    acc.merge(
        InDetTrigPriVxFinderCfg(
            flagsWithTrk,
            inputTracks = jetContext["Tracks"],
            outputVtx = jetContext["Vertices"],
            name="InDetTrigPriVxFinder_jetFS",
        ),
        seqname
    )

    acc.addEventAlgo(
        getUsedInVertexFitTrackDecoratorAlg(
            trackCont = jetContext["Tracks"],
            vtxCont   = jetContext["Vertices"]
        ),
        seqname
    )

    # Create the TTVA
    acc.addEventAlgo(
        JetRecToolsConfig.getJetTrackVtxAlg(
            trkopt, algname="jetalg_TrackPrep"+trkopt,
            # # parameters for the CP::TrackVertexAssociationTool (or the TrackVertexAssociationTool.getTTVAToolForReco function) :
            #WorkingPoint = "Nonprompt_All_MaxWeight", # this is the new default in offline (see also CHS configuration in StandardJetConstits.py)
            WorkingPoint = "Custom",
            d0_cut       = 2.0, 
            dzSinTheta_cut = 2.0, 
            doPVPriority = IDTrigConfig.adaptiveVertex,
        ),
        seqname
    )
    
    # Add the pseudo-jet creator
    acc.addEventAlgo(
        CompFactory.PseudoJetAlgorithm(
            f"pjgalg_{jetContext['GhostTracksLabel']}",
            InputContainer=jetContext["Tracks"],
            OutputContainer=jetContext["GhostTracks"],
            Label=jetContext["GhostTracksLabel"],
            SkipNegativeEnergy=True,
        ),
        seqname
    )

    return acc

@AccumulatorCache
def JetRoITrackingCfg(flags, jetsIn, trkopt, RoIs):
    """ Create the tracking CA and return it as well as the output name dictionary """

    acc = ComponentAccumulator()

    acc.addEventAlgo(
        CompFactory.AthViews.ViewDataVerifier(
            name='VDVInDetFTF_jetsuper',
            DataObjects=[
                ('TrigRoiDescriptorCollection' , f'StoreGateSvc+{RoIs}'),
                ('xAOD::JetContainer' , f'StoreGateSvc+{jetsIn}'),
            ]
        )
    )

    assert trkopt == "roiftf"
    IDTrigConfig = getInDetTrigConfig( 'jetSuper' )

    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    flagsWithTrk = getFlagsForActiveConfig(flags, 'jet', log)

    acc.merge(
        trigInDetFastTrackingCfg(
            flagsWithTrk,
            RoIs,
            signatureName="jetSuper",
            in_view=True
        )
    )

    acc.merge(
        InDetTrigPriVxFinderCfg(
            flagsWithTrk,
            inputTracks = IDTrigConfig.tracks_FTF(),
            outputVtx =   IDTrigConfig.vertex,
            name="InDetTrigPriVxFinderjetSuper",
        )
    )

    # make sure we output only the key,value related to tracks (otherwise, alg duplication issues)
    jetContext, trkKeys = retrieveJetContext(trkopt)
    outmap = { k:jetContext[k] for k in trkKeys }

    return acc, outmap


def addJetTTVA( flags, jetseq, trkopt, config, verticesname=None, adaptiveVertex=None, selector=None ):

    tracksname = config.tracks_FTF()

    label = f"GhostTrack_{trkopt}"

    # get the jetContext for trkopt (and build it if not existing yet)
    jetContext, trkKeys = retrieveJetContext(trkopt)

    vtxFitDecoAlg = getUsedInVertexFitTrackDecoratorAlg(
        trackCont = jetContext["Tracks"],
        vtxCont   = jetContext["Vertices"]
    )

    # *****************************
    # Track-vtx association.
    custom_ttva = {}
    if flags.Trigger.Jet.TrackVtxAssocWP=="Custom":
        custom_ttva = dict(
            d0_cut       = 2.0, 
            dzSinTheta_cut = 2.0, 
        )

    jettrkprepalg = JetRecToolsConfig.getJetTrackVtxAlg(
        trkopt, algname="jetalg_TrackPrep"+trkopt,
        # # parameters for the CP::TrackVertexAssociationTool (or the TrackVertexAssociationTool.getTTVAToolForReco function) :
        WorkingPoint = flags.Trigger.Jet.TrackVtxAssocWP, # e.g. "Custom", or "Nonprompt_All_MaxWeight" (new default in offline - see also CHS configuration in StandardJetConstits.py)
        doPVPriority = adaptiveVertex,
        # schedules track decoration alg with used-in-fit links
        add2Seq = jetseq,
        # Option to set custom TTVA cuts
        **custom_ttva
    )

    # Pseudojets for ghost tracks
    pjgalg = CompFactory.PseudoJetAlgorithm(
        "pjgalg_"+label,
        InputContainer=tracksname,
        OutputContainer=jetContext["GhostTracks"],
        Label=label,
        SkipNegativeEnergy=True
    )

    # Add the 3 algs to the sequence :
    jetseq += vtxFitDecoAlg
    jetseq += jettrkprepalg
    jetseq += pjgalg

    if flags.Trigger.Jet.doVRJets:
        pv0_jettvassoc, pv0_ttvatool = JetRecToolsConfig.getPV0TrackVertexAssoAlg(trkopt, jetseq)
        pv0trackselalg = JetRecToolsConfig.getPV0TrackSelAlg(pv0_ttvatool, trkopt)
        jetseq += conf2toConfigurable( pv0_jettvassoc )
        jetseq += conf2toConfigurable( pv0trackselalg )
