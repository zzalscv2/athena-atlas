# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from MuonCombinedRecExample.MuonCombinedRecFlags import muonCombinedRecFlags
from AthenaCommon.CfgGetter import getPublicTool, getAlgorithm
from MuonRecExample.ConfiguredMuonRec import ConfiguredMuonRec
from MuonRecExample.MuonRecFlags import muonRecFlags
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
muonRecFlags.setDefaults()

from AthenaCommon.AlgSequence import AlgSequence
from AthenaCommon import CfgMgr
from MuonCombinedRecExample.MuonCombinedKeys import MuonCombinedKeys as MuonCbKeys
from InDetRecExample.InDetKeys import InDetKeys
from InDetRecExample.InDetJobProperties import InDetFlags
from TriggerJobOpts.TriggerFlags import TriggerFlags

def MuonCaloTagAlg(name="MuonCaloTagAlg",**kwargs):
    reco_cscs = MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs()
    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm =  muonRecFlags.doMicromegas() and MuonGeometryFlags.hasMM()  
  
    tools = [getPublicTool("MuonCaloTagTool")]
    kwargs.setdefault("MuonCombinedInDetExtensionTools", tools )
    kwargs.setdefault("TagMap","caloTagMap")
    kwargs.setdefault("CombinedTrackCollection","")
    kwargs.setdefault("METrackCollection","")
    kwargs.setdefault("HasCSC", reco_cscs )
    kwargs.setdefault("HasSTgc",reco_stgcs )
    kwargs.setdefault("HasMM", reco_mm )
    return CfgMgr.MuonCombinedInDetExtensionAlg(name,**kwargs)

def MuonCaloTagAlg_LRT(name="MuonCaloTagAlg_LRT", **kwargs):
    reco_cscs = MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs()
    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm =  muonRecFlags.doMicromegas() and MuonGeometryFlags.hasMM()  
    tools = [getPublicTool("MuonCaloTagTool")]
    kwargs.setdefault("MuonCombinedInDetExtensionTools", tools )
    kwargs.setdefault("TagMap","caloTagMap_LRT")
    kwargs.setdefault("InDetCandidateLocation", MuonCbKeys.InDetTrackParticlesLargeD0())  
    kwargs.setdefault("CombinedTrackCollection","")
    kwargs.setdefault("METrackCollection","")
    kwargs.setdefault("HasCSC", reco_cscs )
    kwargs.setdefault("HasSTgc", reco_stgcs )
    kwargs.setdefault("HasMM", reco_mm )
    return CfgMgr.MuonCombinedInDetExtensionAlg(name,**kwargs)

def MuonSegmentTagAlg( name="MuonSegmentTagAlg", **kwargs ):
    kwargs.setdefault("MuonSegmentTagTool", getPublicTool("MuonSegmentTagTool") )
    return CfgMgr.MuonSegmentTagAlg(name,**kwargs)

def MuonSegmentTagAlg_LRT( name="MuonSegmentTagAlg_LRT", **kwargs ):
    kwargs.setdefault("MuonSegmentTagTool", getPublicTool("MuonSegmentTagTool") )
    kwargs.setdefault("InDetCandidateLocation", MuonCbKeys.InDetTrackParticlesLargeD0())
    kwargs.setdefault("TagMap","segmentTagMap_LRT")
    kwargs.setdefault("MuonSegmentLocation","xaodMuonSegments")
    return CfgMgr.MuonSegmentTagAlg(name,**kwargs)

def MuonInsideOutRecoAlg( name="MuonInsideOutRecoAlg", **kwargs ):
    reco_cscs = MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs()
    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm =  muonRecFlags.doMicromegas() and MuonGeometryFlags.hasMM()  
    tools = [getPublicTool("MuonInsideOutRecoTool") ]
    kwargs.setdefault("MuonCombinedInDetExtensionTools", tools )
    kwargs.setdefault("usePRDs",True)
    kwargs.setdefault("HasCSC", reco_cscs )
    kwargs.setdefault("HasSTgc", reco_stgcs )
    kwargs.setdefault("HasMM", reco_mm )
    kwargs.setdefault("TagMap","muGirlTagMap")
    kwargs.setdefault("SegmentCollection","MuGirlSegments")
    return CfgMgr.MuonCombinedInDetExtensionAlg(name,**kwargs)

def MuGirlAlg_LRT( name="MuGirlAlg_LRT", **kwargs ):
    reco_cscs = MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs()
    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm =  muonRecFlags.doMicromegas() and MuonGeometryFlags.hasMM()  
    tools = [getPublicTool("MuonInsideOutRecoTool") ]
    kwargs.setdefault("MuonCombinedInDetExtensionTools", tools )
    kwargs.setdefault("usePRDs",True)
    kwargs.setdefault("HasCSC", reco_cscs )
    kwargs.setdefault("HasSTgc", reco_stgcs )
    kwargs.setdefault("HasMM", reco_mm )
    kwargs.setdefault("TagMap","MuGirlMap_LRT")
    kwargs.setdefault("METrackCollection","MuGirlMETracks_LRT")
    kwargs.setdefault("SegmentCollection","MuGirlSegments_LRT")
    kwargs.setdefault("CombinedTrackCollection",MuonCbKeys.MuGirlMuonsLargeD0())
    kwargs.setdefault("InDetCandidateLocation",MuonCbKeys.InDetTrackParticlesLargeD0())
    return CfgMgr.MuonCombinedInDetExtensionAlg(name,**kwargs)

def MuGirlStauAlg(name="MuGirlStauAlg",**kwargs):
    reco_cscs = MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs()
    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm =  muonRecFlags.doMicromegas() and MuonGeometryFlags.hasMM()  
    tools = [getPublicTool("MuonStauRecoTool")]
    kwargs.setdefault("MuonCombinedInDetExtensionTools", tools )
    kwargs.setdefault("TagMap","stauTagMap")
    kwargs.setdefault("HasCSC", reco_cscs )
    kwargs.setdefault("HasSTgc", reco_stgcs )
    kwargs.setdefault("HasMM", reco_mm )
    kwargs.setdefault("CombinedTrackCollection","MuGirlStauCombinedTracks")
    kwargs.setdefault("METrackCollection","")
    kwargs.setdefault("SegmentCollection","MuGirlStauSegments")
    kwargs.setdefault("InDetCandidateLocation", "InDetCandidatesMerged" if InDetFlags.doR3LargeD0() else "InDetCandidates")
    return CfgMgr.MuonCombinedInDetExtensionAlg(name,**kwargs)

def MuonCombinedInDetCandidateAlg( name="MuonCombinedInDetCandidateAlg",**kwargs ):    
    kwargs.setdefault("TrackSelector",getPublicTool("MuonCombinedInDetDetailedTrackSelectorTool") )
    if muonCombinedRecFlags.doSiAssocForwardMuons() and InDetFlags.doForwardTracks():
        kwargs.setdefault("DoSiliconAssocForwardMuons", True )
        kwargs.setdefault("InDetForwardTrackSelector", getPublicTool("MuonCombinedInDetDetailedForwardTrackSelectorTool") )

    kwargs.setdefault("MuonSystemExtensionTool", getPublicTool("MuonSystemExtensionTool"))
    return CfgMgr.MuonCombinedInDetCandidateAlg(name,**kwargs)

def MuonCombinedInDetCandidateAlg_LRT( name="MuonCombinedInDetCandidateAlg_LRT",**kwargs ):
    kwargs.setdefault("TrackSelector",getPublicTool("MuonCombinedInDetDetailedTrackSelectorTool_LRT") )
    kwargs.setdefault("MuonSystemExtensionTool", getPublicTool("MuonSystemExtensionTool"))
    ### Use the Standard Track particle container in cases where no separate containters will be
    ### saved for the LRT tracking
    track_container = InDetKeys.xAODLargeD0TrackParticleContainer()
    if not InDetFlags.storeSeparateLargeD0Container(): track_container = InDetKeys.xAODTrackParticleContainer()
    kwargs.setdefault("TrackParticleLocation",[track_container])
    kwargs.setdefault("InDetCandidateLocation",MuonCbKeys.InDetTrackParticlesLargeD0())
    kwargs.setdefault("DoSiliconAssocForwardMuons", False)
    kwargs.setdefault("InDetForwardTrackSelector", getPublicTool("MuonCombinedInDetDetailedForwardTrackSelectorTool") )
    return CfgMgr.MuonCombinedInDetCandidateAlg(name,**kwargs)

def MuonInDetExtensionMergerAlg(name="MuonInDetExtensionMergerAlg", **kwargs):
    kwargs.setdefault("ToMerge", ["InDetCandidates", MuonCbKeys.InDetTrackParticlesLargeD0()]) # should be InDetCandidates and InDetCandidatesLRT?
    kwargs.setdefault("ToWrite", "InDetCandidatesMerged")
    return CfgMgr.MuonInDetExtensionMergerAlg(name, **kwargs)


def MuonCombinedMuonCandidateAlg( name="MuonCombinedMuonCandidateAlg", **kwargs ):
    kwargs.setdefault("MuonCandidateTool",getPublicTool("MuonCandidateTool"))
    return CfgMgr.MuonCombinedMuonCandidateAlg(name,**kwargs)

def MuonCombinedAlg( name="MuonCombinedAlg",**kwargs ):
    kwargs.setdefault("MuonCombinedTool",getPublicTool("MuonCombinedTool"))
    tagmaps = []
    # CombinedTagMaps must be in a 1-1 correspondence
    # with MuonCombinedTagTools.
    for h in kwargs['MuonCombinedTool'].MuonCombinedTagTools:
        if h.getFullName().find('FitTagTool') >= 0:
            tagmaps.append ('muidcoTagMap')
        elif h.getFullName().find('StacoTagTool') >= 0:
            tagmaps.append ('stacoTagMap')
    kwargs.setdefault("CombinedTagMaps", tagmaps)
    return CfgMgr.MuonCombinedAlg(name,**kwargs)

def MuonCombinedAlg_LRT( name="MuonCombinedAlg_LRT",**kwargs ):
    kwargs.setdefault("MuonCombinedTool",getPublicTool("MuonCombinedTool"))
    kwargs.setdefault("InDetCandidateLocation",MuonCbKeys.InDetTrackParticlesLargeD0())
    kwargs.setdefault("CombinedTagMaps", ["muidcoTagMap_LRT","stacoTagMap_LRT"])
    kwargs.setdefault("MuidCombinedTracksLocation", "MuidCombinedTracks_LRT")
    kwargs.setdefault("MuidMETracksLocation", "MuidMETracks_LRT")    
    return CfgMgr.MuonCombinedAlg(name,**kwargs)

def recordMuonCreatorAlgObjs (kw):
    Alg = CfgMgr.MuonCreatorAlg
    def val (prop):
        d = kw.get (prop)
        if d is None:
            d = Alg.__dict__[prop].default
        return str(d)
    objs = {'xAOD::MuonContainer': val('MuonContainerLocation'),
            'xAOD::TrackParticleContainer': (val('CombinedLocation')+'TrackParticles',
                                             val('ExtrapolatedLocation')+'TrackParticles',
                                             val('MSOnlyExtrapolatedLocation')+'TrackParticles'),
            'xAOD::MuonSegmentContainer': val('SegmentContainerName'),
            }
    if val('BuildSlowMuon'):
        objs['xAOD::SlowMuonContainer'] = val('SlowMuonContainerLocation')
    if val('MakeClusters'):
        objs['CaloClusterCellLinkContainer'] =  val('CaloClusterCellLinkName') + '_links'
        objs['xAOD::CaloClusterContainer'] =  val('ClusterContainerName')

    from RecExConfig.ObjKeyStore import objKeyStore
    objKeyStore.addManyTypesTransient (objs)
    return

def MuonCreatorAlg( name="MuonCreatorAlg",**kwargs ):
    kwargs.setdefault("MuonCreatorTool",getPublicTool("MuonCreatorTool"))
    recordMuonCreatorAlgObjs (kwargs)
    # if muGirl is off, remove "muGirlTagMap" from "TagMaps"
    # but don't set this default in case the StauCreatorAlg is created (see below)
    if not muonCombinedRecFlags.doMuGirl() and not name=="StauCreatorAlg":
        kwargs.setdefault("TagMaps",["muidcoTagMap","stacoTagMap","caloTagMap","segmentTagMap"])
    if TriggerFlags.MuonSlice.doTrigMuonConfig:
        kwargs.setdefault("MakeClusters", False)
        kwargs.setdefault("ClusterContainerName", "")
        kwargs.setdefault("CopySegments", False)
    return CfgMgr.MuonCreatorAlg(name,**kwargs)


def MuonCreatorAlg_LRT( name="MuonCreatorAlg_LRT",**kwargs ):
    kwargs.setdefault("MuonCreatorTool",getPublicTool("MuonCreatorTool_LRT"))
    recordMuonCreatorAlgObjs (kwargs)
    tag_maps = ["muidcoTagMap_LRT","segmentTagMap_LRT","caloTagMap_LRT"]
    if muonCombinedRecFlags.doMuGirl():
        tag_maps += ["stacoTagMap_LRT","MuGirlMap_LRT"]
    kwargs.setdefault("TagMaps",tag_maps)
    kwargs.setdefault("MuonContainerLocation", MuonCbKeys.FinalMuonsLargeD0())
    kwargs.setdefault("InDetCandidateLocation", MuonCbKeys.InDetTrackParticlesLargeD0())
    kwargs.setdefault("ExtrapolatedLocation", "ExtraPolated"+MuonCbKeys.FinalMuonsLargeD0())
    kwargs.setdefault("MSOnlyExtrapolatedLocation", "MSOnlyExtraPolated"+MuonCbKeys.FinalMuonsLargeD0())
    kwargs.setdefault("CombinedLocation", "Combined"+MuonCbKeys.FinalMuonsLargeD0())
    kwargs.setdefault("SegmentContainerName", "MuonSegments_LRT")
    kwargs.setdefault("TrackSegmentContainerName", "TrkMuonSegments_LRT")
    kwargs.setdefault("BuildSlowMuon", False)
    kwargs.setdefault("MakeClusters", False)
    kwargs.setdefault("ClusterContainerName", "")
    return CfgMgr.MuonCreatorAlg(name,**kwargs)

def StauCreatorAlg( name="StauCreatorAlg", **kwargs ):
    kwargs.setdefault("MuonCreatorTool",getPublicTool("StauCreatorTool"))
    kwargs.setdefault("MuonContainerLocation","Staus")
    kwargs.setdefault("CombinedLocation","CombinedStau")
    kwargs.setdefault("ExtrapolatedLocation","ExtrapolatedStau")
    kwargs.setdefault("MSOnlyExtrapolatedLocation","MSOnlyExtrapolatedStau")
    kwargs.setdefault("MuonCandidateLocation","")
    kwargs.setdefault("SegmentContainerName","StauSegments")
    kwargs.setdefault("TrackSegmentContainerName","TrkStauSegments")
    kwargs.setdefault("BuildSlowMuon",1)
    kwargs.setdefault("ClusterContainerName", "SlowMuonClusterCollection")
    kwargs.setdefault("InDetCandidateLocation", "InDetCandidatesMerged" if InDetFlags.doR3LargeD0() else "InDetCandidates")
    kwargs.setdefault("TagMaps",["stauTagMap"])
    kwargs.setdefault("CopySegments", False)
    if not TriggerFlags.MuonSlice.doTrigMuonConfig:
        recordMuonCreatorAlgObjs (kwargs)
    return MuonCreatorAlg(name,**kwargs)

class MuonCombinedReconstruction(ConfiguredMuonRec):
    def __init__(self,**kwargs):
        ConfiguredMuonRec.__init__(self,"MuonCombinedReconstruction",**kwargs)
        self.configure()

    def configure(self,keys=None):
        super(MuonCombinedReconstruction,self).configure(keys)
        if not self.isEnabled(): return

        topSequence = AlgSequence()

        #if jobproperties.Beam.beamType()=='collisions':
            # creates input collections for ID and MS candidates
        topSequence += getAlgorithm("MuonCombinedInDetCandidateAlg")
        topSequence += getAlgorithm("MuonCombinedMuonCandidateAlg")
             
        if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonCombinedInDetCandidateAlg_LRT")

        # runs ID+MS combinations (fit, staco, mugirl, ID-taggers)
        if muonCombinedRecFlags.doStatisticalCombination() or muonCombinedRecFlags.doCombinedFit():
            topSequence += getAlgorithm("MuonCombinedAlg")
            if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonCombinedAlg_LRT")



        if muonCombinedRecFlags.doMuGirl():
            topSequence += getAlgorithm("MuonInsideOutRecoAlg")
            if muonCombinedRecFlags.doMuGirlLowBeta():
                if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonInDetExtensionMergerAlg")
                topSequence += getAlgorithm("MuGirlStauAlg")
            if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuGirlAlg_LRT")

        if muonCombinedRecFlags.doCaloTrkMuId():
            topSequence += getAlgorithm("MuonCaloTagAlg")
            if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonCaloTagAlg_LRT")

        if muonCombinedRecFlags.doMuonSegmentTagger():
            getPublicTool("MuonSegmentTagTool")
            topSequence += getAlgorithm("MuonSegmentTagAlg")
            if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonSegmentTagAlg_LRT")

        # runs over outputs and create xAODMuon collection
        topSequence += getAlgorithm("MuonCreatorAlg")
        if InDetFlags.doR3LargeD0(): topSequence += getAlgorithm("MuonCreatorAlg_LRT")
 
        # setting this here, in the new configuration it will be easier to do
        topSequence.MuonCreatorAlg.MuonCreatorTool.ParticleCaloExtensionTool.StartFromPerigee=True
        topSequence.MuonCreatorAlg.MuonCreatorTool.ParticleCaloExtensionToolID.StartFromPerigee=False

        if muonCombinedRecFlags.doMuGirl() and muonCombinedRecFlags.doMuGirlLowBeta():
            topSequence += getAlgorithm("StauCreatorAlg")
