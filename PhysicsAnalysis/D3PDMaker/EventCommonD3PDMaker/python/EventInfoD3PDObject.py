# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file EventCommonD3PDMaker/python/EventInfoD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date Aug, 2009
# @brief Define EventInfo D3PD object.
#


from D3PDMakerCoreComps.D3PDObject import make_SG_D3PDObject
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


# Create the object type.
EventInfoD3PDObject = make_SG_D3PDObject( 'xAOD::EventInfo_v1',
                                          D3PDMakerFlags.EventInfoSGKey,
                                          '',
                                          'EventInfoD3PDObject' )


######
# Define blocks.

# Event ID information.
EventInfoD3PDObject.defineBlock (
    0, 'eventID',
    D3PD.AuxDataFillerTool, #D3PDMakerCoreComps
    Vars = ['RunNumber    = runNumber', 
            'EventNumber  = eventNumber',
            'timestamp    = timeStamp',
            'timestamp_ns = timeStampNSOffset',
            'lbn          = lumiBlock',
            'bcid',
            'detmask0     = detectorMask0',
            'detmask1     = detectorMask1',
            ])

EventInfoD3PDObject.defineBlock (
    0, 'eventMu',
    D3PD.AuxDataFillerTool, #D3PDMakerCoreComps
    Vars = ['actualIntPerXing  = actualInteractionsPerCrossing',
            'averageIntPerXing = averageInteractionsPerCrossing',
            ])

# Since this filler tool only fills MC specific info at the moment:
if D3PDMakerFlags.DoTruth:
    EventInfoD3PDObject.defineBlock (
        0, 'eventType',
        D3PD.AuxDataFillerTool, #D3PDMakerCoreComps
        Vars = ['mc_channel_number = mcChannelNumber',
                'mc_event_number   = mcEventNumber',
                ])
    EventInfoD3PDObject.defineBlock (0, 'eventWeight',
                                     #EventCommonD3PDMaker
                                     D3PD.EventWeightFillerTool)

EventInfoD3PDObject.defineBlock (
    1, 'eventFlags',
    D3PD.AuxDataFillerTool, #D3PDMakerCoreComps
    Vars = ['pixelFlags',
            'sctFlags',
            'trtFlags',
            'larFlags',
            'tileFlags',
            'muonFlags',
            'fwdFlags = forwardDetFlags',
            'coreFlags',
            'backgroundFlags',
            'lumiFlags',
            ])
EventInfoD3PDObject.defineBlock (1, 'eventErrorState',
                                 #EventCommonD3PDMaker
                                 D3PD.EventErrorStateFillerTool)

# The trigger information is only interesting for data events, so let's only
# add it for those:
if not D3PDMakerFlags.DoTruth:
    EventInfoD3PDObject.defineBlock (2, 'eventTrigInfo',
                                     #EventCommonD3PDMaker
                                     D3PD.EventTrigInfoFillerTool,
                                     Streams = [ "Egamma", "Muons", "JetTauEtmiss" ])

# The event type flags should only be saved at a higher detail level:
EventInfoD3PDObject.defineBlock (5, 'eventTypeFlags',
                                 #EventCommonD3PDMaker
                                 D3PD.EventTypeFillerTool)
