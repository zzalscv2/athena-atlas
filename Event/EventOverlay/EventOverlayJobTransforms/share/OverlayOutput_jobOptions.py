# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

include.block('EventOverlayJobTransforms/OverlayOutput_jobOptions.py')

from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
from OverlayCommonAlgs.OverlayFlags import overlayFlags
from ISF_Config.ISF_jobProperties import ISF_Flags

from AthenaCommon.ConcurrencyFlags import jobproperties as jp
nThreads = jp.ConcurrencyFlags.NumThreads()

outStream = AthenaPoolOutputStream('StreamRDO', athenaCommonFlags.PoolRDOOutput(), asAlg=True)
outStream.ItemList = []

# Event info
outStream.ItemList += [ 'xAOD::EventInfo#EventInfo', 'xAOD::EventAuxInfo#EventInfoAux.' ]

# Truth
if DetFlags.overlay.Truth_on():
    outStream.ItemList += ['McEventCollection#TruthEvent']

    if athenaCommonFlags.DoFullChain():
        outStream.ItemList += ['TrackRecordCollection#CaloEntryLayer']
        outStream.ItemList += ['TrackRecordCollection#MuonEntryLayer']
        outStream.ItemList += ['TrackRecordCollection#MuonExitLayer']
    else:
        if 'TrackRecordCollection' in overlayFlags.optionalContainerMap():
            for collection in overlayFlags.optionalContainerMap()['TrackRecordCollection']:
                outStream.ItemList += ['TrackRecordCollection#' + collection]

    if not overlayFlags.isDataOverlay():
        if 'xAOD::JetContainer' in overlayFlags.optionalContainerMap():
            outStream.ItemList += ['xAOD::JetContainer#InTimeAntiKt4TruthJets']
            outStream.ItemList += ['xAOD::AuxContainerBase!#InTimeAntiKt4TruthJetsAux.-constituentLinks.-constituentWeights']
            outStream.ItemList += ['xAOD::JetContainer#OutOfTimeAntiKt4TruthJets']
            outStream.ItemList += ['xAOD::AuxContainerBase!#OutOfTimeAntiKt4TruthJetsAux.-constituentLinks.-constituentWeights']
            outStream.ItemList += ['xAOD::JetContainer#InTimeAntiKt6TruthJets']
            outStream.ItemList += ['xAOD::AuxContainerBase!#InTimeAntiKt6TruthJetsAux.-constituentLinks.-constituentWeights']
            outStream.ItemList += ['xAOD::JetContainer#OutOfTimeAntiKt6TruthJets']
            outStream.ItemList += ['xAOD::AuxContainerBase!#OutOfTimeAntiKt6TruthJetsAux.-constituentLinks.-constituentWeights']
        if 'xAOD::TruthParticleContainer' in overlayFlags.optionalContainerMap():
            outStream.ItemList += ["xAOD::TruthParticleContainer#TruthPileupParticles"]
            outStream.ItemList += ["xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."]

    if DetFlags.overlay.BCM_on():
        if not athenaCommonFlags.DoFullChain() or ("ATLFASTIIF" not in ISF_Flags.Simulator() and "ATLFAST3F" not in ISF_Flags.Simulator()):
            outStream.ItemList += [ 'InDetSimDataCollection#BCM_SDO_Map' ]

    if DetFlags.overlay.pixel_on():
        outStream.ItemList += [ 'InDetSimDataCollection#PixelSDO_Map' ]
        if overlayFlags.isDataOverlay():
            outStream.ItemList += [ 'IDCInDetBSErrContainer#PixelByteStreamErrs' ]

    if DetFlags.overlay.SCT_on():
        outStream.ItemList += ['InDetSimDataCollection#SCT_SDO_Map']
        if overlayFlags.isDataOverlay():
            outStream.ItemList += [ 'IDCInDetBSErrContainer#SCT_ByteStreamErrs' ]

    if DetFlags.overlay.TRT_on():
        outStream.ItemList += [ 'InDetSimDataCollection#TRT_SDO_Map' ]
        if overlayFlags.isDataOverlay():
            outStream.ItemList += [ 'TRT_BSErrContainer#TRT_ByteStreamErrs' ]

    if DetFlags.overlay.LAr_on() or DetFlags.overlay.Tile_on():
        if 'CaloCalibrationHitContainer' in overlayFlags.optionalContainerMap():
            for collection in overlayFlags.optionalContainerMap()['CaloCalibrationHitContainer']:
                outStream.ItemList += ['CaloCalibrationHitContainer#' + collection]

    if DetFlags.overlay.CSC_on():
        outStream.ItemList += [ 'CscSimDataCollection#CSC_SDO' ]

    if DetFlags.overlay.MDT_on():
        outStream.ItemList += [ 'MuonSimDataCollection#MDT_SDO' ]

    if DetFlags.overlay.RPC_on():
        outStream.ItemList += [ 'MuonSimDataCollection#RPC_SDO' ]

    if DetFlags.overlay.TGC_on():
        outStream.ItemList += [ 'MuonSimDataCollection#TGC_SDO' ]

    if DetFlags.overlay.sTGC_on():
        outStream.ItemList += [ 'MuonSimDataCollection#sTGC_SDO' ]

    if DetFlags.overlay.MM_on():
        outStream.ItemList += [ 'MuonSimDataCollection#MM_SDO' ]

# Inner detector
if DetFlags.overlay.BCM_on():
    if not athenaCommonFlags.DoFullChain() or ("ATLFASTIIF" not in ISF_Flags.Simulator() and "ATLFAST3F" not in ISF_Flags.Simulator()):
        outStream.ItemList+=['BCM_RDO_Container#BCM_RDOs']

if DetFlags.overlay.pixel_on():
    outStream.ItemList += ['PixelRDO_Container#PixelRDOs']

if DetFlags.overlay.SCT_on():
    outStream.ItemList += ['SCT_RDO_Container#SCT_RDOs']

if DetFlags.overlay.TRT_on():
    outStream.ItemList += ['TRT_RDO_Container#TRT_RDOs']

if DetFlags.overlay.LAr_on():
    outStream.ItemList+=['LArRawChannelContainer#LArRawChannels']
    if 'AddCaloDigi' in digitizationFlags.experimentalDigi():
        outStream.ItemList+=['LArDigitContainer#LArDigitContainer_MC']
    elif 'AddCaloDigiThinned' in digitizationFlags.experimentalDigi():
        outStream.ItemList+=['LArDigitContainer#LArDigitContainer_MC_Thinned']

if DetFlags.overlay.Tile_on():
    outStream.ItemList += [ 'TileRawChannelContainer#TileRawChannelCnt' ]
    if 'AddCaloDigi' in digitizationFlags.experimentalDigi():
        outStream.ItemList += [ 'TileDigitsContainer#TileDigitsCnt', 'TileDigitsContainer#TileDigitsFlt' ]
    else:
        outStream.ItemList += [ 'TileDigitsContainer#TileDigitsFlt' ]
    outStream.ItemList += [ 'TileL2Container#TileL2Cnt' ]

if DetFlags.overlay.CSC_on():
    outStream.ItemList += [ 'CscRawDataContainer#CSCRDO' ]

if DetFlags.overlay.MDT_on():
    outStream.ItemList += [ 'MdtCsmContainer#MDTCSM' ]

if DetFlags.overlay.RPC_on():
    outStream.ItemList += [ 'RpcPadContainer#RPCPAD' ]
    from MuonRecExample.MuonRecFlags import muonRecFlags
    if muonRecFlags.doNRPCs():
        outStream.ItemList += [ 'xAOD::NRPCRDOContainer#NRPCRDO' , 'xAOD::NRPCRDOAuxContainer#NRPCRDOAux.' ]

if DetFlags.overlay.TGC_on():
    outStream.ItemList += [ 'TgcRdoContainer#TGCRDO' ]

if DetFlags.overlay.sTGC_on():
    outStream.ItemList += [ 'Muon::STGC_RawDataContainer#sTGCRDO' ]

if DetFlags.overlay.MM_on():
    outStream.ItemList += [ 'Muon::MM_RawDataContainer#MMRDO' ]

if DetFlags.overlay.LVL1_on():
    if DetFlags.simulateLVL1.LAr_on():
        outStream.ItemList += [ 'LArTTL1Container#LArTTL1EM' ]
        outStream.ItemList += [ 'LArTTL1Container#LArTTL1HAD' ]
        from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as commonGeoFlags
        if commonGeoFlags.Run()=="RUN3":
            outStream.ItemList+=["CaloCellContainer#SCell"]

    if DetFlags.simulateLVL1.Tile_on():
        outStream.ItemList += [ 'TileTTL1Container#TileTTL1Cnt' ]
        outStream.ItemList += [ 'TileTTL1Container#TileTTL1MBTS' ]
        outStream.ItemList += [ 'TileDigitsContainer#MuRcvDigitsCnt' ]
        outStream.ItemList += [ 'TileRawChannelContainer#MuRcvRawChCnt' ]
        outStream.ItemList += [ 'TileMuonReceiverContainer#TileMuRcvCnt' ]

#add the tracks
if overlayFlags.doTrackOverlay():
    outStream.ItemList += [ 'TrackCollection#'+overlayFlags.bkgPrefix()+'CombinedInDetTracks' ]
    outStream.ItemList += [ 'TrackCollection#'+overlayFlags.bkgPrefix()+'DisappearingTracks' ]
    outStream.ItemList += [ 'TrackCollection#'+overlayFlags.bkgPrefix()+'ResolvedForwardTracks' ]
    outStream.ItemList += [ 'TrackCollection#'+overlayFlags.bkgPrefix()+'ResolvedLargeD0Tracks' ]
    outStream.ItemList += [ 'InDet::TRT_DriftCircleContainer#'+overlayFlags.bkgPrefix()+'TRT_DriftCircles' ]
    outStream.ItemList += [ 'InDet::PixelClusterContainer#'+overlayFlags.bkgPrefix()+'PixelClusters' ]
    outStream.ItemList += [ 'InDet::SCT_ClusterContainer#'+overlayFlags.bkgPrefix()+'SCT_Clusters' ]

# Temporary to ensure the output is stored
outStream.TransientItems = outStream.ItemList
