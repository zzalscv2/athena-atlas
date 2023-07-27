# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg, trigInDetLRTCfg
from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg

from AthenaCommon.Logging import logging
from AthenaCommon.CFElements import parOR

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from .FullScanDefs import trkFSRoI

@AccumulatorCache
def commonInDetFullScanCfg(flags):
    acc = ComponentAccumulator()
    seqname='TrigInDetFullScan'
    acc.addSequence(parOR(seqname),primary=True)

    flagsWithTrk = getFlagsForActiveConfig(flags, 'fullScan', log)
    acc.merge(
        trigInDetFastTrackingCfg(
            flagsWithTrk,
            trkFSRoI,
            signatureName='fullScan',
            in_view=False
        ),
        seqname
    )

    acc.merge(
        InDetTrigPriVxFinderCfg(
            flagsWithTrk,
            inputTracks = flagsWithTrk.Tracking.ActiveConfig.tracks_FTF,
            outputVtx = flagsWithTrk.Tracking.ActiveConfig.vertex_jet,
        ),
        seqname
    )

    return acc


def commonInDetLRTCfg(flags, std_cfg, lrt_cfg, rois=trkFSRoI):
    acc = ComponentAccumulator()
    seqname = 'TrigInDetLRT_'+lrt_cfg.name
    acc.addSequence(parOR(seqname),primary=True)
    flagsWithTrk = getFlagsForActiveConfig(flags, lrt_cfg.name, log)

    acc.merge(
        trigInDetLRTCfg(
            flagsWithTrk,
            std_cfg.trkTracks_FTF(),
            rois,
            in_view=False
        ),
        seqname
    )

    return acc
