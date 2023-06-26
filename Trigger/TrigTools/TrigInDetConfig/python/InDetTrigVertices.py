#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__author__ =   "Mark Sutton and Lukas Novotny"
__all__    = [ "makeInDetTrigVertices" ]


# old function for backwards compatability
# TODO remove after CA migration complete

def makeInDetTrigVertices( flags, whichSignature, inputTrackCollection, outputVtxCollection=None, config=None, adaptiveVertex=None ) :

        
    from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
    from TrigInDetConfig.TrigInDetConfig import trigInDetVertexingCfg
    return algorithmCAToGlobalWrapper(
        trigInDetVertexingCfg,
        flags,
        inputTracks = inputTrackCollection,
        outputVtx = outputVtxCollection,
    )

