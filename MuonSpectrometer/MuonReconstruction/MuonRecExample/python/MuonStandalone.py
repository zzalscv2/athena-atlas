# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

__doc__ = """Configuration of Muon Spectrometer Standalone muon reconstruction"""

###############################################################
#
# Configuration for Standalone
#
#==============================================================
from AthenaCommon import CfgMgr

from .MuonStandaloneFlags import muonStandaloneFlags,MoorelikeStrategy
from .MuonRecFlags import muonRecFlags
from .ConfiguredMuonRec import ConfiguredMuonRec

from .MuonRecUtils import ExtraFlags

from AthenaCommon.CfgGetter import getPublicTool,getPrivateTool,getPublicToolClone
from RecExConfig.ObjKeyStore                  import cfgKeyStore

from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
#==============================================================

# call  setDefaults to update default flags
muonRecFlags.setDefaults()
muonStandaloneFlags.setDefaults()

#
# Tools for MuPat track building
#

def MuonTrackSteering(name="MuonTrackSteering", extraFlags=None, **kwargs):
    if extraFlags is None:
        extraFlags = ExtraFlags()
        
    extraFlags.setFlagDefault("namePrefix", "MuSt_")
    extraFlags.setFlagDefault("doSegmentPhiMatching", True)
    extraFlags.setFlagDefault(muonStandaloneFlags.optimiseMomentumResolutionUsingChi2) # take name & value from JobProperty
    extraFlags.setFlagDefault(muonStandaloneFlags.strategy)
    extraFlags.setFlagDefault(muonStandaloneFlags.trackBuilder)
    extraFlags.setFlagDefault(muonStandaloneFlags.printSummary)
    extraFlags.setFlagDefault(muonStandaloneFlags.refinementTool)
                  
    if extraFlags.strategy:
        kwargs.setdefault("StrategyList", extraFlags.strategy)
    else:
        kwargs.setdefault("StrategyList", MoorelikeStrategy)  

    kwargs.setdefault("DoSummary", extraFlags.printSummary)
    kwargs.setdefault("HoleRecoveryTool",getPublicTool("MuonEORecoveryTool"))
    if "TrackBuilderTool" not in kwargs:
        extraFlags.setFlagDefault('UseTrackingHistory',True)
        kwargs["TrackBuilderTool"] = getPublicToolClone("MooMuonTrackBuilder", "MooTrackBuilderTemplate",
                                                        extraFlags=extraFlags)
        if "TrackRefinementTool" not in kwargs:
            kwargs["TrackRefinementTool"] = getPublicTool("MooTrackBuilderTemplate")

    kwargs.setdefault("SegSeedQCut", 1)
    kwargs.setdefault("Seg2ndQCut", 1)
    return CfgMgr.Muon__MuonTrackSteering(name,**kwargs)

def MuonSegmentFilterAlg(name="MuonSegmentFilterAlg", **kwargs):
    kwargs.setdefault("SegmentCollectionName", "TrackMuonSegments")
    return CfgMgr.MuonSegmentFilterAlg(name, **kwargs)

def MuonSegmentFinderNCBAlg(name="MuonSegmentMaker_NCB", **kwargs):
    ### Only use the TGC measurements from the  current bunch crossing
    kwargs.setdefault("TGC_PRDs", "TGC_Measurements")
    reco_cscs = muonRecFlags.doCSCs() and MuonGeometryFlags.hasCSC()
    reco_mircomegas = muonRecFlags.doMMs() and MuonGeometryFlags.hasMM()
    reco_stgc = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    kwargs.setdefault("doStgcSegments", reco_stgc)
    kwargs.setdefault("doMMSegments", reco_mircomegas)
    kwargs.setdefault("doMdtSegments", False)
    kwargs.setdefault("CSC_clusterkey", "CSC_Clusters" if reco_cscs else "")
    ## Define the output container
    kwargs.setdefault("SegmentCollectionName", "NCB_TrackMuonSegments")
    ### Do not rebuild the NSW alignment segments
    kwargs.setdefault("NSWSegmentCollectionName", "")
    kwargs.setdefault("SegmentQuality", 1)

    ### Setup the CSC segment maker
    if reco_cscs:
        cscSegmentUtilTool = getPublicToolClone("CscSegmentUtilTool_NCB",
                                                "CscSegmentUtilTool",
                                                TightenChi2 = False, 
                                                IPconstraint=False) 
        kwargs.setdefault("Csc2dSegmentMaker", getPublicToolClone("Csc2dSegmentMaker_NCB","Csc2dSegmentMaker",
                                                                segmentTool = cscSegmentUtilTool))
        kwargs.setdefault("Csc4dSegmentMaker",getPublicToolClone("Csc4dSegmentMaker_NCB","Csc4dSegmentMaker",
                                                                segmentTool = getPublicTool("CscSegmentUtilTool_NCB")))
    ### Setup the NSW segment maker
    if reco_mircomegas or reco_stgc:
        kwargs.setdefault("MuonClusterCreator",  getPublicTool("MuonClusterOnTrackCreator"))
        Cleaner = getPublicToolClone("MuonTrackCleaner_seg","MuonTrackCleaner")
        Cleaner.Extrapolator = getPublicTool("MuonStraightLineExtrapolator")
        Cleaner.Fitter = getPublicTool("MCTBSLFitterMaterialFromTrack")
        Cleaner.PullCut = 3
        Cleaner.PullCutPhi = 3
        Cleaner.UseSLFit = True
        kwargs.setdefault("NSWSegmentMaker", getPublicToolClone("MuonNSWSegmentFinderTool_NCB",
                                                                "MuonNSWSegmentFinderTool",
                                                                TrackCleaner = Cleaner,
                                                                SeedMMStereos = False,
                                                                IPConstraint = False) )

    
    return CfgMgr.MuonSegmentFinderAlg(name, **kwargs)


def MuonSegmentFinderAlg( name="MuonSegmentMaker", **kwargs):
    
    SegmentFinder = getPublicTool("MuonNSWSegmentFinderTool")
    Cleaner = getPublicToolClone("MuonTrackCleaner_seg","MuonTrackCleaner")
    Cleaner.Extrapolator = getPublicTool("MuonStraightLineExtrapolator")
    Cleaner.Fitter = getPublicTool("MCTBSLFitterMaterialFromTrack")
    Cleaner.PullCut = 3
    Cleaner.PullCutPhi = 3
    Cleaner.UseSLFit = True
    SegmentFinder.TrackCleaner = Cleaner
    # for test purposes allow parallel running of truth segment finding and new segment finder
    SegmentLocation = "TrackMuonSegments"
    if muonStandaloneFlags.segmentOrigin == 'TruthTracking':
        SegmentLocation = "ThirdChainSegments"
    reco_cscs = muonRecFlags.doCSCs() and MuonGeometryFlags.hasCSC()
    reco_mircomegas = muonRecFlags.doMMs() and MuonGeometryFlags.hasMM()
    reco_stgc = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()    
    kwargs.setdefault("CSC_clusterkey", "CSC_Clusters" if reco_cscs else "")
    kwargs.setdefault("doStgcSegments", reco_stgc)
    kwargs.setdefault("doMMSegments", reco_mircomegas)
    kwargs.setdefault("Csc2dSegmentMaker",  getPublicTool("Csc2dSegmentMaker") if reco_cscs else "")
    kwargs.setdefault("Csc4dSegmentMaker",  getPublicTool("Csc4dSegmentMaker") if reco_cscs else "")
    kwargs.setdefault("MuonClusterCreator", getPublicTool("MuonClusterOnTrackCreator") if reco_mircomegas or reco_stgc else "" )
    kwargs.setdefault("NSWSegmentMaker", getPublicTool("MuonNSWSegmentFinderTool") if reco_mircomegas or reco_stgc else "" )
    kwargs.setdefault("SegmentCollectionName", SegmentLocation)
    kwargs.setdefault("MuonPatternCalibration", getPublicTool("MuonPatternCalibration"))
    kwargs.setdefault("PrintSummary",  muonStandaloneFlags.printSummary())
    kwargs.setdefault("MuonClusterSegmentFinder", getPublicTool("MuonClusterSegmentFinder"))
    kwargs.setdefault("TGC_PRDs", 'TGC_MeasurementsAllBCs' if not muonRecFlags.useTGCPriorNextBC else 'TGC_Measurements')

    MuonSegmentFinderAlg = CfgMgr.MuonSegmentFinderAlg( name, **kwargs )   
   
    # we check whether the layout contains any CSC chamber and if yes, we check that the user also wants to use the CSCs in reconstruction
    if reco_cscs:
        getPublicTool("CscSegmentUtilTool")       
    else:       
        MuonSegmentFinderAlg.CSC_clusterkey = ""
    return MuonSegmentFinderAlg


def MuonStandaloneTrackParticleCnvAlg( name="MuonStandaloneTrackParticleCnvAlg",**kwargs):
    from AthenaCommon.Include import include
    include("BeamSpotConditions/BeamCondAlgSetup.py" )
    from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackParticleCnvAlg, xAODMaker__TrackCollectionCnvTool, xAODMaker__RecTrackParticleContainerCnvTool

    muonParticleCreatorTool = getPublicTool("MuonParticleCreatorTool")
    muonTrackCollectionCnvTool = xAODMaker__TrackCollectionCnvTool( name = "MuonTrackCollectionCnvTool", TrackParticleCreator = muonParticleCreatorTool )
    muonRecTrackParticleContainerCnvTool = xAODMaker__RecTrackParticleContainerCnvTool(name = "MuonRecTrackParticleContainerCnvTool", TrackParticleCreator = muonParticleCreatorTool )

    kwargs.setdefault("TrackParticleCreator", muonParticleCreatorTool)
    kwargs.setdefault("RecTrackParticleContainerCnvTool", muonRecTrackParticleContainerCnvTool)
    kwargs.setdefault("TrackCollectionCnvTool", muonTrackCollectionCnvTool)
    kwargs.setdefault("RecTrackParticleContainerCnvTool", muonRecTrackParticleContainerCnvTool)
    kwargs.setdefault("TrackContainerName", "MuonSpectrometerTracks")
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "MuonSpectrometerTrackParticles")
    kwargs.setdefault("AODContainerName", "")
    kwargs.setdefault("AODTruthContainerName", "")
    kwargs.setdefault("xAODTruthLinkVector",  "")
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("ConvertTracks", True)

    return xAODMaker__TrackParticleCnvAlg(name,**kwargs)



def MuonStationsInterSectAlg(**kwargs):
    from MuonStationIntersectCond.MuonStationIntersectCondConf import MuonStationIntersectCondAlg
    from AthenaCommon.AlgSequence import AthSequencer
    condSequence = AthSequencer("AthCondSeq")
    if hasattr(condSequence, "MuonStationIntersectCondAlg"): return
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    if athenaCommonFlags.isOnline:
        kwargs.setdefault("MdtCondKey", "")
    condSequence += MuonStationIntersectCondAlg("MuonStationIntersectCondAlg",**kwargs)

def MuonLayerHoughAlg(name="MuonLayerHoughAlg", **kwargs):
    from AthenaCommon.BeamFlags import jobproperties
    beamFlags = jobproperties.Beam
    
    reco_stgc = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mircomegas = muonRecFlags.doMMs() and MuonGeometryFlags.hasMM()
    reco_cscs = muonRecFlags.doCSCs() and MuonGeometryFlags.hasCSC()
    
    if ( beamFlags.beamType() == 'collisions'  ): 
        kwargs.setdefault( "MuonLayerScanTool", getPublicTool("MuonLayerHoughTool") )
    else:
        kwargs.setdefault( "MuonLayerScanTool", getPublicTool("MuonHoughPatternFinderTool" ))
    
    kwargs.setdefault("PrintSummary", muonStandaloneFlags.printSummary())
    kwargs.setdefault("CscPrepDataContainer" ,"CSC_Clusters" if reco_cscs else "")
    kwargs.setdefault("sTgcPrepDataContainer" , "STGC_Measurements" if reco_stgc else "")
    kwargs.setdefault("MMPrepDataContainer" , "MM_Measurements" if reco_mircomegas else "")
    kwargs.setdefault("TgcPrepDataContainer" , 'TGC_MeasurementsAllBCs' if not muonRecFlags.useTGCPriorNextBC else 'TGC_Measurements')
    return CfgMgr.MuonLayerHoughAlg(name,  **kwargs)
#
# The top level configurator
#
class MuonStandalone(ConfiguredMuonRec):
    def __init__(self,**kwargs):
        ConfiguredMuonRec.__init__(self,"MuonStandalone",**kwargs)
        # setup minimum config needed to get Storegate keys
        # full setup is done in configure()

        # keys for output of segment building stage
        self.addOutputKey("MuonSegments", "MuonSegments")

    def configure(self,keys=None):
        super(MuonStandalone,self).configure(keys)
        if not self.isEnabled(): return        
        # do the following in case of (at least one) NSW
        reco_stgc = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
        reco_mircomegas = muonRecFlags.doMMs() and MuonGeometryFlags.hasMM()        
        
        MuonStationsInterSectAlg()

        self.addAlg( MuonLayerHoughAlg("MuonLayerHoughAlg"))
        if not muonStandaloneFlags.patternsOnly():
            self.addAlg(MuonSegmentFinderAlg("MuonSegmentMaker"))
                
            self.addAlg(MuonSegmentFinderNCBAlg("MuonSegmentMaker_NCB"))
            if not cfgKeyStore.isInInput ('xAOD::MuonSegmentContainer', 'MuonSegments_NCB'):
                self.addAlg( CfgMgr.xAODMaker__MuonSegmentCnvAlg("MuonSegmentCnvAlg_NCB",
                                                                SegmentContainerName="NCB_TrackMuonSegments",
                                                                xAODContainerName="NCB_MuonSegments") )

        if reco_stgc or reco_mircomegas:
            self.addAlg( CfgMgr.xAODMaker__MuonSegmentCnvAlg("QuadNSW_MuonSegmentCnvAlg",
                                                             SegmentContainerName="TrackMuonNSWSegments",
                                                             xAODContainerName="xAODNSWSegments") )

        if muonStandaloneFlags.doSegmentsOnly():
            return	                    
        # Tracks builder
        #
        # add the algorithm (which uses the MuonTrackSteering)
        # 
        TrackBuilder = CfgMgr.MuPatTrackBuilder("MuPatTrackBuilder", 
                                                TrackSteering=getPrivateTool("MuonTrackSteering"), 
                                                SpectrometerTrackOutputLocation="MuonSpectrometerTracks", 
                                                MuonSegmentCollection="TrackMuonSegments")

        self.addAlg( TrackBuilder )
        #### Add a segment collection only containing only EM and EO hits
        if muonRecFlags.runCommissioningChain():
            self.addAlg(MuonSegmentFilterAlg(FilteredCollectionName="TrackMuonSegmentsEMEO"))
  
            chamberRecovery_EMEO = getPublicToolClone("MuonChamberRecovery_EMEO", "MuonChamberHoleRecoveryTool", 
                                                               sTgcPrepDataContainer="",
                                                               MMPrepDataContainer="")

            MooTrackBuilder_EMEO = getPublicToolClone("MooMuonTrackBuilder_EMEO", 
                                               "MooTrackBuilderTemplate",
                                                ChamberHoleRecoveryTool = chamberRecovery_EMEO)
            TrackSteeringTool_EMEO = getPublicToolClone("MuonTrackSteering_EMEO", "MuonTrackSteering", TrackBuilderTool = MooTrackBuilder_EMEO)
            
            
            TrackBuilder_EMEO = CfgMgr.MuPatTrackBuilder("MuPatTrackBuilder_EMEO", 
                                                TrackSteering=TrackSteeringTool_EMEO, 
                                                SpectrometerTrackOutputLocation="EMEO_MuonSpectrometerTracks", 
                                                MuonSegmentCollection="TrackMuonSegmentsEMEO")
            self.addAlg(TrackBuilder_EMEO)
            if muonStandaloneFlags.createTrackParticles():
                xAODTrackParticleCnvAlg_EMEO = MuonStandaloneTrackParticleCnvAlg("MuonStandaloneTrackParticleCnvAlg_EMEO",
                                                                           TrackContainerName = "EMEO_MuonSpectrometerTracks",
                                                                           xAODTrackParticlesFromTracksContainerName="EMEO_MuonSpectrometerTrackParticles")
                self.addAlg( xAODTrackParticleCnvAlg_EMEO )
               

        if muonStandaloneFlags.createTrackParticles():
            xAODTrackParticleCnvAlg = MuonStandaloneTrackParticleCnvAlg("MuonStandaloneTrackParticleCnvAlg")
            self.addAlg( xAODTrackParticleCnvAlg )



    def getCalibConfig(self):
        """Get the configuration to configure Calibration Ntuple output"""
        doTracks = not muonStandaloneFlags.doSegmentsOnly()
        if doTracks:
            tracksKey = "MuonSpectrometerTracks"
        else:
            tracksKey = ""
            
        return { 'eventTag'         : muonStandaloneFlags.segmentOrigin(), 
                 'patternsKey'      : '',#self.dataKey("MuonPatterns"), 
                 'segmentsKey'      : self.dataKey("MuonSegments"), 
                 'tracksKey'        : tracksKey,
                 'doPhi'            : muonStandaloneFlags.trackBuilder == 'Moore',
                 'segmentAuthor'    : 5,
                 'trackAuthor'     : 200
                 }
                  
