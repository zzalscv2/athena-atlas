# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format

def getATLASVersion():
    import os

    if "AtlasVersion" in os.environ:
        return os.environ["AtlasVersion"]
    if "AtlasBaseVersion" in os.environ:
        return os.environ["AtlasBaseVersion"]
    return "Unknown"


def getDataTypes(flags, haveRDO=False, readAOD=False):
    data_types = []  # These need to match the tools added later
    if flags.Detector.EnableID:
        # General ID types
        data_types += ["JiveXML::SiSpacePointRetriever/SiSpacePointRetriever"]
        data_types += ["JiveXML::SiClusterRetriever/SiClusterRetriever"]
        data_types += ["JiveXML::VertexRetriever/VertexRetriever"]
        # These options will retrieve any available collection of segments/tracks from storegate
        data_types += ["JiveXML::SegmentRetriever/SegmentRetriever"]
        data_types += ["JiveXML::TrackRetriever/TrackRetriever"]

    if flags.Detector.EnablePixel:
        data_types += ["JiveXML::PixelClusterRetriever/PixelClusterRetriever"]
        if haveRDO:
            data_types += ["JiveXML::PixelRDORetriever/PixelRDORetriever"]
    if flags.Detector.EnableTRT:
        data_types += ["JiveXML::TRTRetriever/TRTRetriever"]
    if haveRDO and flags.Detector.EnableSCT:
        data_types += ["JiveXML::SCTRDORetriever/SCTRDORetriever"]

    # Truth (from TruthJiveXML_DataTypes.py)
    if not readAOD:
        data_types += ["JiveXML::TruthTrackRetriever/TruthTrackRetriever"]
    data_types += ["JiveXML::TruthMuonTrackRetriever/TruthMuonTrackRetriever"]

    if flags.Detector.EnableCalo:
        # TODO
        pass

    if flags.Detector.EnableMuon:
        # Taken from MuonJiveXML_DataTypes.py
        if flags.Detector.EnableMDT:
            data_types += ["JiveXML::MdtPrepDataRetriever/MdtPrepDataRetriever"]
        if flags.Detector.EnableTGC:
            data_types += ["JiveXML::TgcPrepDataRetriever/TgcPrepDataRetriever"]
        if flags.Detector.EnableRPC:
            data_types += ["JiveXML::RpcPrepDataRetriever/RpcPrepDataRetriever"]
        if flags.Detector.EnableCSC:
            data_types += ["JiveXML::CSCClusterRetriever/CSCClusterRetriever"]
            data_types += ["JiveXML::CscPrepDataRetriever/CscPrepDataRetriever"]
        if flags.Detector.EnablesTGC:
            data_types += ["JiveXML::sTgcPrepDataRetriever/sTgcPrepDataRetriever"]
        if flags.Detector.EnableMM:
            data_types += ["JiveXML::MMPrepDataRetriever/MMPrepDataRetriever"]

    # Taken from xAODJiveXML_DataTypes.py
    data_types += ["JiveXML::xAODCaloClusterRetriever/xAODCaloClusterRetriever"]
    data_types += ["JiveXML::xAODElectronRetriever/xAODElectronRetriever"]
    data_types += ["JiveXML::xAODMissingETRetriever/xAODMissingETRetriever"]
    data_types += ["JiveXML::xAODMuonRetriever/xAODMuonRetriever"]
    data_types += ["JiveXML::xAODPhotonRetriever/xAODPhotonRetriever"]
    data_types += ["JiveXML::xAODJetRetriever/xAODJetRetriever"]
    data_types += ["JiveXML::xAODTauRetriever/xAODTauRetriever"]
    data_types += ["JiveXML::xAODTrackParticleRetriever/xAODTrackParticleRetriever"]
    data_types += ["JiveXML::xAODVertexRetriever/xAODVertexRetriever"]

    return data_types


def InDetRetrieversCfg(flags):
    result = ComponentAccumulator()
    # Do we need to add equivalent of InDetFlags.doSlimming=False (in JiveXML_RecEx_config.py)? If so, why?
    # Following is based on InDetJiveXML_DataTypes.py and TrkJiveXML_DataTypes.py
    if flags.Detector.EnablePixel:
        result.merge(PixelClusterRetrieverCfg(flags))

    if flags.Detector.EnableID:
        result.merge(SiClusterRetrieverCfg(flags))
        result.merge(SiSpacePointRetrieverCfg(flags))
        result.merge(TrackRetrieverCfg(flags))

    if flags.Detector.EnableTRT:
        result.merge(TRTRetrieverCfg(flags))
    return result


def PixelClusterRetrieverCfg(flags, name="PixelClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PixelTruthMap", "")
    the_tool = CompFactory.JiveXML.PixelClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiClusterRetrieverCfg(flags, name="SiClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("SCT_TruthMap", "")
    the_tool = CompFactory.JiveXML.SiClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiSpacePointRetrieverCfg(flags, name="SiSpacePointRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PRD_TruthPixel", "")
        kwargs.setdefault("PRD_TruthSCT", "")
    the_tool = CompFactory.JiveXML.SiSpacePointRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TRTRetrieverCfg(flags, name="TRTRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("TRTTruthMap", "")
    the_tool = CompFactory.JiveXML.TRTRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TrackRetrieverCfg(flags, name="TrackRetriever", **kwargs):
    # Based on TrkJiveXML_DataTypes
    result = ComponentAccumulator()
    # FIXME - this is copied from TrkJiveXML_DataTypes.py, but we can do better
    kwargs.setdefault("PriorityTrackCollection", "Tracks")
    kwargs.setdefault(
        "OtherTrackCollections",
        [
            "CombinedMuonTracks",
            "MuonSpectrometerTracks",
            "ConvertedStacoTracks",
            "ConvertedMuIdCBTracks",
            "CombinedInDetTracks",
            "GSFTracks",
        ],
    )
    ### The Event Filter track collections are not written to XML by default.
    ### To write them out, you must uncomment the following line:
    # kwargs.setdefault("DoWriteHLT", True)
    ### switch residual data off:
    kwargs.setdefault("DoWriteResiduals", False)
    the_tool = CompFactory.JiveXML.TrackRetrieve(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TruthTrackRetrieverCfg(flags, name="TruthTrackRetriever", **kwargs):
    # Based on TruthJiveXML_DataTypes.py (and JiveXML_RecEx_config.py)
    result = ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

    extrap = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    result.addPublicTool(extrap)

    kwargs.setdefault("StoreGateKey", "TruthEvent")
    the_tool = CompFactory.JiveXML.TruthTrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def CaloRetrieversCfg(flags):
    result = ComponentAccumulator()
    # TODO
    return result


def MuonRetrieversCfg(flags):
    result = ComponentAccumulator()
    # TODO
    return result


def xAODRetrieversCfg(flags):
    # Based on xAODJiveXML_DataTypes.py
    result = ComponentAccumulator()
    # It's not really necessary to configure these, since nothing depends on flags.
    # We could just make all this the default in cpp (if it is not already)
    result.addPublicTool(
        CompFactory.JiveXML.xAODElectronRetriever(
            name="xAODElectronRetriever",
            StoreGateKey="Electrons",
            OtherCollections=["Electrons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMissingETRetriever(
            name="xAODMissingETRetriever",
            FavouriteMETCollection="MET_Reference_AntiKt4EMPFlow",
            OtherMETCollections=[
                "MET_Reference_AntiKt4EMTopo",
                "MET_Calo",
                "MET_LocHadTopo",
                "MET_Core_AntiKt4LCTopo",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMuonRetriever(
            name="xAODMuonRetriever", StoreGateKey="Muons", OtherCollections=["Muons"]
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODPhotonRetriever(
            name="xAODPhotonRetriever",
            StoreGateKey="Photons",
            OtherCollections=["Photons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODJetRetriever(
            name="xAODJetRetriever",
            FavouriteJetCollection="AntiKt4EMPFlowJets",
            OtherJetCollections=[
                "AntiKt4EMTopoJets",
                "AntiKt4LCTopoJets",
                "AntiKt10LCTopoJets",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTauRetriever(
            name="xAODTauRetriever", StoreGateKey="TauJets"
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTrackParticleRetriever(
            name="xAODTrackParticleRetriever",
            StoreGateKey="InDetTrackParticles",
            OtherTrackCollections=[
                "InDetLargeD0TrackParticles",
                "CombinedMuonTrackParticles",
                "GSFTrackParticles",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODVertexRetriever(
            name="xAODVertexRetriever",
            PrimaryVertexCollection="PrimaryVertices",
            SecondaryVertexCollection="BTagging_AntiKt2TrackSecVtx",
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODCaloClusterRetriever(
            name="xAODCaloClusterRetriever",
            FavouriteClusterCollection="egammaClusters",
            OtherClusterCollections=["CaloCalTopoClusters"],
        )
    )
    return result

def TriggerRetrieversCfg(flags):
    result = ComponentAccumulator()
    # TODO
    return result

def AlgoJiveXMLCfg(flags, name="MuonCombinePatternTool", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    kwargs.setdefault("AtlasRelease", getATLASVersion())
    kwargs.setdefault("WriteToFile", True)
    kwargs.setdefault("OnlineMode", False)
    ### Enable this to recreate the geometry XML files for Atlantis
    kwargs.setdefault("WriteGeometry", False)

    # This next bit sets the data types, then we set the associated public tools
    readAOD = False  # FIXME - set this properly
    readESD = False  # FIXME - set this properly
    haveRDO = (
        flags.Input.Format is Format.BS or "StreamRDO" in flags.Input.ProcessingTags
    )
    kwargs.setdefault("DataTypes", getDataTypes(flags, haveRDO, readAOD))

    if not readAOD:
        result.merge(TruthTrackRetrieverCfg(flags))

    if haveRDO or readESD:
        if flags.Detector.EnableID:
            result.merge(InDetRetrieversCfg(flags))

        if flags.Detector.EnableCalo:
            result.merge(CaloRetrieversCfg(flags))

        if flags.Detector.EnableMuon:
            result.merge(MuonRetrieversCfg(flags))

    result.merge(xAODRetrieversCfg(flags))

    if flags.Trigger.doHLT: #FIXME - is this the right flag?
        result.merge(TriggerRetrieversCfg(flags))

    the_alg = CompFactory.JiveXML.AlgoJiveXML(name="AlgoJiveXML", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
