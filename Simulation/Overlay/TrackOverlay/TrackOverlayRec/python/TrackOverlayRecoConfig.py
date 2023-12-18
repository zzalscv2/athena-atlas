# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND, parOR
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrackOverlayRec.TrackOverlayEventFilterConfig import TrackOverlayDecisionAlgCfg, InvertedTrackOverlayDecisionAlgCfg
from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg

def TrackOverlayRecoCfg(flags):
    acc = ComponentAccumulator()  
    acc.addSequence(seqAND("MOSequence"), parentName='AthAlgSeq')
    acc.merge(TrackOverlayDecisionAlgCfg(flags), sequenceName='MOSequence')
    acc.addSequence(parOR('WorkMOSequence'), parentName='MOSequence')
    flagsMO = flags.cloneAndReplace("TrackOverlay.ActiveConfig","TrackOverlay.MCOverlayConfig")
    acc.merge(InDetTrackRecoCfg(flagsMO), sequenceName='WorkMOSequence')

    acc.addSequence(seqAND("TOSequence"), parentName='AthAlgSeq')
    acc.merge(InvertedTrackOverlayDecisionAlgCfg(flags), sequenceName='TOSequence')
    acc.addSequence(parOR('WorkTOSequence'), parentName='TOSequence')
    flagsTO = flags.cloneAndReplace("TrackOverlay.ActiveConfig","TrackOverlay.TrackOverlayConfig")
    acc.merge(InDetTrackRecoCfg(flagsTO), sequenceName='WorkTOSequence')
    return acc
