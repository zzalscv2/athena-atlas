#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#     Contains algs/tools used by Inner Detector Trigger

#Global keys/names for collections
#from .InDetTrigCollectionKeys import TrigTRTKeys, TrigPixelKeys

from InDetRecExample.TrackingCommon import makePublicTool, setDefaults

from AthenaCommon.Logging import logging 
log = logging.getLogger("InDetTrigCommon")

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper

#--------------------------------------------------------------------------

def _prefix():
   #Too long? Do we need this to separate from InDet?
   return 'InDetTrigMT_'

def _suffix(signature=None):
   return '_%s'%signature

#Retrieve full name of the algorithm/tool which consist of
#1] Predefined PREFIX describing the scope where this tool is being used (Inner Detector Trigger)
#2] CORE name containing the actual name of the tool
#3] SUFFIX which is derived from signature name: _electron, _muonLate, _FS etc

def add_prefix( core, suffix=None ):
   return  '{}{}{}'.format( _prefix(), core, _suffix(suffix) )

#-------------------------------------------------------------------------------------------------
#                       Alg/Tools for offline pattern recognition tracking


@makePublicTool
def zVertexMakerTool_builder(name, trackingCuts, seedMakerTool ):
   from InDetRecExample.InDetJobProperties import InDetFlags

   kwargs = {}

   #Prepare default parameter settings for the tool
   kwargs = setDefaults( kwargs,
                         Zmax          = trackingCuts.maxZImpact(),
                         Zmin          = -trackingCuts.maxZImpact(),
                         minRatio      = 0.17,
                         SeedMakerTool = seedMakerTool )


   if InDetFlags.doHeavyIon():
      kwargs = setDefaults( kwargs,
                            HistSize   = 2000,
                            minContent = 30)

   from SiZvertexTool_xk.SiZvertexTool_xkConf import InDet__SiZvertexMaker_xk
   return InDet__SiZvertexMaker_xk(name  = name,
                                   **kwargs)


@makePublicTool
def siTrackMakerTool_builder( name, config, trackFinderTool, trackingCuts, usePrdAssociationTool ):
   from InDetRecExample.InDetJobProperties import InDetFlags

   if config.name == 'cosmics':
      trackPatternRecoInfo = 'SiSpacePointsSeedMaker_Cosmic'
   else:
      trackPatternRecoInfo = 'SiSPSeededFinder'

   useBremMode = trackingCuts.mode() == "Offline"

   kwargs = {}

   from InDetConfig.SiDetElementsRoadToolConfig import TrigSiDetElementsRoadMaker_xkCfg
   siDetElementsRoadMakerTool = CAtoLegacyPublicToolWrapper(TrigSiDetElementsRoadMaker_xkCfg)
   
   #FIXME:
   #Check compatibility of cosmic cuts from offline version with online run2
   #https://gitlab.cern.ch/atlas/athena/-/blob/21.2/InnerDetector/InDetExample/InDetTrigRecExample/python/InDetTrigConfigRecNewTracking.py#L167-172
   #Prepare default parameter settings for the tool
   kwargs = setDefaults( kwargs,
                         useSCT                        = trackingCuts.useSCT(),
                         usePixel                      = trackingCuts.usePixel(),
                         RoadTool                      = siDetElementsRoadMakerTool,
                         CombinatorialTrackFinder      = trackFinderTool,
                         pTmin                         = trackingCuts.minPT(),
                         pTminBrem                     = trackingCuts.minPTBrem(),
                         pTminSSS                      = InDetFlags.pT_SSScut(),                      #FIXME: revisit  ATR-22756
                         nClustersMin                  = trackingCuts.minClusters(),
                         nHolesMax                     = trackingCuts.nHolesMax(),
                         nHolesGapMax                  = trackingCuts.nHolesGapMax(),
                         SeedsFilterLevel              = trackingCuts.seedFilterLevel(),
                         Xi2max                        = trackingCuts.Xi2max(),
                         Xi2maxNoAdd                   = trackingCuts.Xi2maxNoAdd(),
                         nWeightedClustersMin          = trackingCuts.nWeightedClustersMin(),
                         CosmicTrack                   = (config.name == 'cosmics'),
                         Xi2maxMultiTracks             = trackingCuts.Xi2max(), # was 3.
                         useSSSseedsFilter             = InDetFlags.doSSSfilter(),                    #FIXME: revisit  ATR-22756
                         doMultiTracksProd             = True,
                         useBremModel                  = InDetFlags.doBremRecovery() and useBremMode, #FIXME: revisit  ATR-22756
                         doCaloSeededBrem              = InDetFlags.doCaloSeededBrem(),               #FIXME: revisit  ATR-22756
                         doHadCaloSeedSSS              = InDetFlags.doHadCaloSeededSSS(),             #FIXME: revisit  ATR-22756
                         phiWidth                      = trackingCuts.phiWidthBrem(),
                         etaWidth                      = trackingCuts.etaWidthBrem(),
                         EMROIPhiRZContainer           = "InDetCaloClusterROIPhiRZ0GeV",
                         HadROIPhiRZContainer          = "InDetHadCaloClusterROIPhiRZ",               #FIXME: revisit  ATR-22756
                         TrackPatternRecoInfo          = trackPatternRecoInfo,
                         UseAssociationTool            = usePrdAssociationTool)


   from SiTrackMakerTool_xk.SiTrackMakerTool_xkConf import InDet__SiTrackMaker_xk
   return  InDet__SiTrackMaker_xk(name = name,
                                  **kwargs)



def siSPSeededTrackFinder_builder( name, config, outputTracks, trackingCuts, usePrdAssociationTool, nameSuffix, trackSummaryTool ):

   #FIXME: ATR-22756, ATR-22755
   # 1] Currently some flags are copy paste from offline configuration, might need to switch those to trigger flags
   # 2] trackingCuts are adapted from offline version as well, ideally we would want to have these from ConfigSettings.py in the end
   # 3] This code requires proper validation (currently ongoing using minBias and cosmic signatures)


   from .InDetTrigCollectionKeys           import TrigPixelKeys, TrigSCTKeys
   from InDetRecExample.InDetJobProperties import InDetFlags



   #Load subtools of the TrackFinder
   from InDetConfig.SiSpacePointsSeedToolConfig import TrigSiSpacePointsSeedMakerCfg
   siSpacePointsSeedMakerTool = CAtoLegacyPublicToolWrapper(TrigSiSpacePointsSeedMakerCfg)

   
   # --- Z-coordinates primary vertices finder (only for collisions)
   zVertexMakerTool = None
   #FIXME:Switch to trig flags? ATR-22756
   if InDetFlags.useZvertexTool():
      zVertexMakerTool =  zVertexMakerTool_builder(name, trackingCuts, siSpacePointsSeedMakerTool )


   from InDetConfig.SiCombinatorialTrackFinderToolConfig import SiCombinatorialTrackFinder_xk_Trig_Cfg
   siCombinatorialTrackFinderTool = CAtoLegacyPublicToolWrapper(SiCombinatorialTrackFinder_xk_Trig_Cfg, name=name+'_Tool')

   siTrackMakerTool =  siTrackMakerTool_builder( name                       = add_prefix( 'siTrackMaker', nameSuffix),
                                                 config                     = config,
                                                 trackFinderTool            = siCombinatorialTrackFinderTool,
                                                 trackingCuts               = trackingCuts,
                                                 usePrdAssociationTool      = usePrdAssociationTool)

   #-----------------------------------------------------
   #  Configure parameters

   kwargs = {}
   #Prepare default parameter settings for the tool
   kwargs = setDefaults( kwargs,
                         TrackTool           = siTrackMakerTool,
                         PRDtoTrackMap       = TrigPixelKeys.PRDtoTrackMap if usePrdAssociationTool else '', #TODO: if prd is enabled this needs to be tested, ATR-22756
                         SpacePointsPixelName= TrigPixelKeys.SpacePoints,
                         SpacePointsSCTName  = TrigSCTKeys.SpacePoints,
                         TrackSummaryTool    = trackSummaryTool,
                         TracksLocation      = outputTracks, 
                         SeedsTool           = siSpacePointsSeedMakerTool,
                         ZvertexTool         = zVertexMakerTool,
                         useZvertexTool      = InDetFlags.useZvertexTool(),         #FIXME: revisit  ATR-22756
                         useNewStrategy      = InDetFlags.useNewSiSPSeededTF(),     #FIXME: revisit  ATR-22756
                         useMBTSTimeDiff     = InDetFlags.useMBTSTimeDiff(),                                         #FIXME: revisit  ATR-22756
                         useZBoundFinding    = trackingCuts.doZBoundary())         
      
   # FIXME: revisit HI option ATR-22756
   #     if InDetFlags.doHeavyIon():
   #           kwargs = setDefaults( kwargs, FreeClustersCut = 2) #Heavy Ion optimization from Igor

   from SiSPSeededTrackFinder.SiSPSeededTrackFinderConf import InDet__SiSPSeededTrackFinder

   return InDet__SiSPSeededTrackFinder( name=name, **kwargs )
                                    

