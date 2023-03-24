#!/usr/bin/env python
"""Main steering for the digitization jobs

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from BCM_Digitization.BCM_DigitizationConfig import BCM_DigitizationCfg
from Digitization.DigitizationParametersConfig import writeDigitizationMetadata
from LArDigitization.LArDigitizationConfig import LArTriggerDigitizationCfg
from MuonConfig.CSC_DigitizationConfig import CSC_DigitizationDigitToRDOCfg
from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
from PixelDigitization.PLR_DigitizationConfig import PLR_DigitizationCfg
from PixelDigitization.ITkPixelDigitizationConfig import ITkPixelDigitizationCfg
from PixelDigitization.PixelDigitizationConfig import PixelDigitizationCfg
from SCT_Digitization.SCT_DigitizationConfig import SCT_DigitizationCfg
from StripDigitization.StripDigitizationConfig import ITkStripDigitizationCfg
from HGTD_Digitization.HGTD_DigitizationConfig import HGTD_DigitizationCfg
from TileSimAlgs.TileDigitizationConfig import TileDigitizationCfg, TileTriggerDigitizationCfg
from TRT_Digitization.TRT_DigitizationConfig import TRT_DigitizationCfg
from ALFA_Digitization.ALFA_DigitizationConfig import ALFA_DigitizationCfg
from AFP_Digitization.AFP_DigitizationConfig import AFP_DigitizationCfg
from LUCID_Digitization.LUCID_DigitizationConfig import LUCID_DigitizationCfg
from ZDC_SimuDigitization.ZDC_SimuDigitizationConfig import ZDC_DigitizationCfg
from RunDependentSimComps.PileUpUtils import pileupInputCollections

from AthenaCommon.Logging import logging
logDigiSteering = logging.getLogger('DigitizationSteering')

def DigitizationMainServicesCfg(flags):
    """Configure main digitization services"""
    if flags.Digitization.PileUp:
        if flags.Concurrency.NumThreads > 0:
            logDigiSteering.info("DigitizationMainServicesCfg: Attempting to run pile-up digitization AthenaMT using %s threads!", str(flags.Concurrency.NumThreads))
            logDigiSteering.info("DigitizationMainServicesCfg: Using new PileUpMT code.")
            # raise RuntimeError("DigitizationSteering.DigitizationMainServicesCfg: Running pile-up digitization with AthenaMT is not supported. Please update your configuration.")
            from Digitization.PileUpMTConfig import PileUpMTAlgCfg
            acc = MainServicesCfg(flags)
            acc.merge(PileUpMTAlgCfg(flags))
        else:
            from Digitization.PileUpConfig import PileUpEventLoopMgrCfg
            acc = MainServicesCfg(flags, LoopMgr="PileUpEventLoopMgr")
            acc.merge(PileUpEventLoopMgrCfg(flags))
    else:
        acc = MainServicesCfg(flags)

    acc.merge(PoolReadCfg(flags))

    return acc


def DigitizationMainCfg(flags):
    # Construct main services
    acc = DigitizationMainServicesCfg(flags)

    acc.merge(DigitizationMainContentCfg(flags))

    return acc

def DigitizationMainContentCfg(flags):

    acc = ComponentAccumulator()

    acc.merge(writeDigitizationMetadata(flags))

    if not flags.Digitization.PileUp:
        # Old EventInfo conversion
        if "EventInfo" not in flags.Input.Collections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
            acc.merge(EventInfoCnvAlgCfg(flags,
                                        inputKey="McEventInfo",
                                        outputKey="Input_EventInfo"))

        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
        acc.merge(EventInfoUpdateFromContextAlgCfg(flags))

        # Decorate pile-up values
        from Digitization.PileUpConfig import NoPileUpMuWriterCfg
        acc.merge(NoPileUpMuWriterCfg(flags))

    # Signal-only truth information
    if flags.Digitization.PileUp:
        from MCTruthSimAlgs.MCTruthSimAlgsConfig import (
            MergeMcEventCollCfg,
            InTimeOnlyMcEventCollCfg,
            SignalOnlyMcEventCollCfg,
            MergeAntiKt4TruthJetsCfg,
            MergeAntiKt6TruthJetsCfg,
            MergeTruthParticlesCfg,
            MergeMuonEntryLayerCfg,
            MergeCalibHitsCfg,
        )
        if flags.Common.ProductionStep is not ProductionStep.FastChain and getEnabledDetectors(flags):
            if flags.Digitization.DigiSteeringConf=="StandardPileUpToolsAlg":
                acc.merge(MergeMcEventCollCfg(flags))
            elif flags.Digitization.DigiSteeringConf=="StandardInTimeOnlyTruthPileUpToolsAlg":
                acc.merge(InTimeOnlyMcEventCollCfg(flags))
            else:
                acc.merge(SignalOnlyMcEventCollCfg(flags))
        if flags.Digitization.EnableTruth:
            puCollections = pileupInputCollections(flags.Digitization.PU.LowPtMinBiasInputCols)
            if "AntiKt4TruthJets" in puCollections:
                acc.merge(MergeAntiKt4TruthJetsCfg(flags))
            if "AntiKt6TruthJets" in puCollections:
                acc.merge(MergeAntiKt6TruthJetsCfg(flags))
            if "TruthPileupParticles" in puCollections:
                acc.merge(MergeTruthParticlesCfg(flags))
            acc.merge(MergeMuonEntryLayerCfg(flags))
            acc.merge(MergeCalibHitsCfg(flags))

    from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
    acc.merge(TruthDigitizationOutputCfg(flags))

    # Beam spot reweighting
    if flags.Common.ProductionStep != ProductionStep.PileUpPresampling and flags.Digitization.InputBeamSigmaZ > 0:
        from BeamEffects.BeamEffectsAlgConfig import BeamSpotReweightingAlgCfg
        acc.merge(BeamSpotReweightingAlgCfg(flags))

    # Inner Detector
    if flags.Detector.EnableBCM:
        acc.merge(BCM_DigitizationCfg(flags))
    if flags.Detector.EnablePixel:
        acc.merge(PixelDigitizationCfg(flags))
    if flags.Detector.EnableSCT:
        acc.merge(SCT_DigitizationCfg(flags))
    if flags.Detector.EnableTRT:
        acc.merge(TRT_DigitizationCfg(flags))

    # ITk
    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelDigitizationCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripDigitizationCfg(flags))
    if flags.Detector.EnablePLR:
        acc.merge(PLR_DigitizationCfg(flags))

    # HGTD
    if flags.Detector.EnableHGTD:
        acc.merge(HGTD_DigitizationCfg(flags))

    # Calorimeter
    if flags.Detector.EnableLAr:
        acc.merge(LArTriggerDigitizationCfg(flags))
    if flags.Detector.EnableTile:
        acc.merge(TileDigitizationCfg(flags))
        acc.merge(TileTriggerDigitizationCfg(flags))

    # Muon Spectrometer
    if flags.Detector.EnableMDT:
        acc.merge(MDT_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableTGC:
        acc.merge(TGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableRPC:
        acc.merge(RPC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableCSC:
        acc.merge(CSC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnablesTGC:
        acc.merge(sTGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableMM:
        acc.merge(MM_DigitizationDigitToRDOCfg(flags))

    # LUCID
    if flags.Detector.EnableLucid:
        acc.merge(LUCID_DigitizationCfg(flags))

    # AFP
    if flags.Detector.EnableAFP:
        acc.merge(AFP_DigitizationCfg(flags))

    # ALFA
    if flags.Detector.EnableALFA:
        acc.merge(ALFA_DigitizationCfg(flags))

    # ZDC
    if flags.Detector.EnableZDC:
        acc.merge(ZDC_DigitizationCfg(flags))

    # Add MT-safe PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        acc.merge(PerfMonMTSvcCfg(flags))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
    acc.merge(InfileMetaDataCfg(flags, "RDO"))

    return acc


def DigitizationMessageSvcCfg(flags):
    """MessageSvc for digitization and overlay"""
    MessageSvc = CompFactory.MessageSvc
    acc = ComponentAccumulator()
    acc.addService(MessageSvc(setError=["HepMcParticleLink"]))
    return acc


def DigitizationTestingPostInclude(flags, acc):
    """Testing digitization post-include"""
    # dump config
    configName = "DigiPUConfigCA" if flags.Digitization.PileUp else "DigiConfigCA"
    from AthenaConfiguration.JobOptsDumper import JobOptsDumperCfg
    acc.merge(JobOptsDumperCfg(flags, FileName=f"{configName}.txt"))

    # dump pickle
    with open(f"{configName}.pkl", "wb") as f:
        acc.store(f)
