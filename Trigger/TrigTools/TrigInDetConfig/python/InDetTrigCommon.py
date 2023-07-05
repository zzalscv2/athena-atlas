#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#     Contains algs/tools used by Inner Detector Trigger

#Global keys/names for collections
#from .InDetTrigCollectionKeys import TrigTRTKeys, TrigPixelKeys

from InDetRecExample.TrackingCommon import makePublicTool, setDefaults

from AthenaCommon.Logging import logging 
log = logging.getLogger("InDetTrigCommon")



#--------------------------------------------------------------------------
#                    Track Ambiguity Solver algs/tools


#-------------------------------
#TODO:

#Make loader for extrapolator

#-------------------------------

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


def trigPropagator_getter():
   from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigPropagator
   return InDetTrigPropagator

#--------------------------------------------------------------------------------------

def trackMonitoringTool_builder(flags, suffix):
  #First load the generic monitoring tool with set of histograms for Particle Cnv
  from TrigInDetMonitoringTools.TrigInDetTrackingMonitoring import TrigInDetTrackCnvMonitoring
  genericMonTool = TrigInDetTrackCnvMonitoring(flags, name = 'GenericMonitoring_{}'.format(suffix))



  #Now pass this tool to the Track Monitoring tool
  from TrigInDetMonitoringTools.TrigInDetMonitoringToolsConf import TrigInDetTrackMonitoringTool
  return TrigInDetTrackMonitoringTool( name           = 'xAODParticleCreatorAlg_{}'.format(suffix),
                                       MonitoringTool = genericMonTool)



#Returns suffix of tracking type from a given alg name
def getTrackingSuffix( name ):
   if 'IDTrig' in name:
         return '_IDTrig'
   elif 'FTF' in name:
         return '_FTF'
   elif 'EFID' in name:
         return '_EFID'
   else:
      return ''



def trackPRD_Association_builder(name, inTrackCollections, associationMapName):
      from InDetRecExample.TrackingCommon import getInDetTrackPRD_Association
      associationTool = associationTool_getter()
      return getInDetTrackPRD_Association(name,
                                          TracksName = inTrackCollections,
                                          AssociationTool = associationTool,
                                          AssociationMapName = associationMapName)



#--------------------------------------------------------------------------
#                    Track Ambiguity algs/tools
def associationTool_getter():
      #TODO double check this!
      from InDetRecExample.TrackingCommon import getInDetTrigPRDtoTrackMapToolGangedPixels
      return getInDetTrigPRDtoTrackMapToolGangedPixels()

def trackFitterTool_getter(config):
      #For now load from RecLoadTools where the config is based on: InDetTrigFlags.trackFitterType()  (gaussian, kalman, globalChi2, ...)
      #There are also variations of cosmic/TRT fitters -> Decision which fitter to return  has to be adapted based on the signature as well
      if config.name == 'cosmics':
         from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackFitterCosmics
         return InDetTrigTrackFitterCosmics
      else:
         from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackFitter
         return  InDetTrigTrackFitter

def trackSelectionTool_getter(config):
      #TODO this might need to be revisited!

      if config.name == 'cosmics':
        from InDetTrigRecExample.InDetTrigConfigRecLoadToolsCosmics import  InDetTrigAmbiTrackSelectionToolCosmicsN
        return InDetTrigAmbiTrackSelectionToolCosmicsN

      else:
        from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigAmbiTrackSelectionTool
        return InDetTrigAmbiTrackSelectionTool



#-------------------------------------------------------------------------------------------------
#                       Alg/Tools for offline pattern recognition tracking

@makePublicTool
def siSpacePointsSeedMakerTool_builder(name, config, trackingCuts, usePrdAssociationTool, trackSummaryTool ):
   from InDetRecExample.InDetKeys  import  InDetKeys
   from .InDetTrigCollectionKeys   import  TrigPixelKeys, TrigSCTKeys

   kwargs = {}
   kwargs = setDefaults( kwargs,
                         pTmin                  = trackingCuts.minPT(),
                         maxdImpact             = trackingCuts.maxPrimaryImpact(),
                         maxZ                   = trackingCuts.maxZImpact(),
                         minZ                   = -trackingCuts.maxZImpact(),
                         usePixel               = trackingCuts.usePixel(),
                         SpacePointsPixelName   = TrigPixelKeys.SpacePoints,
                         useSCT                 = trackingCuts.useSCT(), #Note: this is false for dissappearing tracks in offline
                         SpacePointsSCTName     = TrigSCTKeys.SpacePoints,
                         useOverlapSpCollection = trackingCuts.useSCT(), #Note: this is false for dissappearing tracks in offline
                         SpacePointsOverlapName = InDetKeys.OverlapSpacePoints(), #FIXME: Switch to trigger flags? ATR-22756
                         radMax                 = trackingCuts.radMax(),
                         RapidityCut            = trackingCuts.maxEta())


   #FIXME: revisit HI ATR-22756
   #Change/add tracking  parameters based on the different tracking mode
   #if config.name == 'HI':
   #   kwargs = setDefaults( kwargs,
   #                         maxdImpactPPS = trackingCuts.maxdImpactPPSSeeds(),
   #                         maxdImpactSSS = trackingCuts.maxdImpactSSSSeeds())

   if usePrdAssociationTool:
      kwargs = setDefaults( kwargs,
                            PRDtoTrackMap      = TrigPixelKeys.PRDtoTrackMap)

   #FIXME: switch to TrigFlags? ATR-22756
   if config.name != 'cosmics':
      kwargs = setDefaults( kwargs,
                            maxRadius1     = 0.75*trackingCuts.radMax(),
                            maxRadius2     = trackingCuts.radMax(),
                            maxRadius3     = trackingCuts.radMax())


   if config.name == 'cosmics':
      from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_Cosmic as SiSpacePointsSeedMaker
   #FIXME: revisit HI ATR-22756
   #elif config.name == 'HI':
   #   from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_HeavyIon as SiSpacePointsSeedMaker
   else:
    from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_ATLxk as SiSpacePointsSeedMaker

   return SiSpacePointsSeedMaker ( name    =  name,
                                   **kwargs)



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




def prdAssociation_builder( InputCollections ):
   import InDetRecExample.TrackingCommon as TrackingCommon
   #FIXME: If so: ATR-22756
   # 1] Get a new association tool
   #associationTool = TrackingCommon.getInDetTrigPRDtoTrackMapToolGangedPixels(),

   # 2] Use the ganged pixel from here?
   #from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigPrdAssociationTool

   # 3] Create the new one as in offline tracking?:
   prefix     = 'TrigInDet'
   suffix     = ''#NewTrackingCuts.extension()
   return TrackingCommon.getInDetTrackPRD_Association(namePrefix   = prefix,
                                                      nameSuffix   = suffix,
                                                      TracksName   = list(InputCollections))#This is readHandle #What input collection Thought there are no tracks at this point??!
   # 4] if so do I use normal or ganged?
   #from InDetAssociationTools.InDetAssociationToolsConf import InDet__InDetPRD_AssociationToolGangedPixels
   #InDetTrigPrdAssociationl = InDet__InDetPRD_AssociationToolGangedPixels(name = "%sPrdAssociationTool%s"%(prefix,suffix),
   #                                                                          PixelClusterAmbiguitiesMapName = TrigPixelKeys.PRDtoTrackMap )

@makePublicTool
def siDetectorElementRoadMakerTool_builder( name, trackingCuts ):
   from InDetRecExample.InDetKeys  import  InDetKeys

   from SiDetElementsRoadTool_xk.SiDetElementsRoadTool_xkConf import InDet__SiDetElementsRoadMaker_xk
   return  InDet__SiDetElementsRoadMaker_xk(name               = name,
                                            PropagatorTool     = trigPropagator_getter(),
                                            usePixel           = trackingCuts.usePixel(),
                                            PixManagerLocation = InDetKeys.PixelManager(), #FIXME: revisit  ATR-22756
                                            useSCT             = trackingCuts.useSCT(),
                                            SCTManagerLocation = InDetKeys.SCT_Manager(),  #FIXME: revisit   ATR-22756
                                            RoadWidth          = trackingCuts.RoadWidth())



@makePublicTool
def siCombinatorialTrackFinderTool_builder( name, trackingCuts ):
   from .InDetTrigCollectionKeys   import TrigPixelKeys, TrigSCTKeys
   from AthenaCommon.DetFlags      import DetFlags
   import InDetRecExample.TrackingCommon as TrackingCommon

   #FIXME: quick hack to try running ID, remove later, ATR-22756
   DetFlags.ID_setOn()

   #Are we happy with these settings?
   from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigSCTConditionsSummaryTool, InDetTrigPatternUpdator, InDetTrigBoundaryCheckTool
   # @TODO ensure that PRD association map is used if usePrdAssociationTool is set ATR-22756

   kwargs = {}
   #Prepare default parameter settings for the tool
   kwargs = setDefaults( kwargs,
                         PropagatorTool        = trigPropagator_getter(),
                         UpdatorTool           = InDetTrigPatternUpdator,
                         SctSummaryTool        = InDetTrigSCTConditionsSummaryTool, #Any reason for this to be turned off? None,
                         RIOonTrackTool        = TrackingCommon.getInDetRotCreatorDigital(),
                         usePixel              = DetFlags.haveRIO.pixel_on(),
                         useSCT                = DetFlags.haveRIO.SCT_on(),
                         PixelClusterContainer = TrigPixelKeys.Clusters,
                         SCT_ClusterContainer  = TrigSCTKeys.Clusters)


   #Add SCT condition summary if specified
   #FIXME: Use TriggerFlags instead? ATR-22756
   #if (DetFlags.haveRIO.SCT_on()):
   #   kwargs = setDefaults( kwargs,
   #                         SctSummaryTool = InDetTrigSCTConditionsSummaryTool )

   from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiCombinatorialTrackFinder_xk
   return InDet__SiCombinatorialTrackFinder_xk(name  = name,
                                               BoundaryCheckTool = InDetTrigBoundaryCheckTool,
                                               **kwargs)


@makePublicTool
def siTrackMakerTool_builder( name, config, siDetElementsRoadMakerTool, trackFinderTool, trackingCuts, usePrdAssociationTool ):
   from InDetRecExample.InDetJobProperties import InDetFlags
   from InDetRecExample.InDetKeys          import InDetKeys

   if config.name == 'cosmics':
      trackPatternRecoInfo = 'SiSpacePointsSeedMaker_Cosmic'
   #FIXME: Add HI option once implemented ATR-2275
   #elif config.name == 'HI':
   #  trackPatternRecoInfo = 'SiSpacePointsSeedMaker_HeavyIon'

   else:
      trackPatternRecoInfo = 'SiSPSeededFinder'

   useBremMode = trackingCuts.mode() == "Offline"

   kwargs = {}


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


   if InDetFlags.doStoreTrackSeeds():
      from SeedToTrackConversionTool.SeedToTrackConversionToolConf import InDet__SeedToTrackConversionTool
      InDet_SeedToTrackConversion = InDet__SeedToTrackConversionTool(name       = "InDet_SeedToTrackConversion"+trackingCuts.extension(),
                                                                     OutputName = InDetKeys.SiSPSeedSegments()+trackingCuts.extension())
      kwargs = setDefaults( kwargs,
                            SeedToTrackConversion = InDet_SeedToTrackConversion,
                            SeedSegmentsWrite = True )



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
   siSpacePointsSeedMakerTool = siSpacePointsSeedMakerTool_builder(name                  = add_prefix( 'siSPSeedMaker', nameSuffix),
                                                                   config                = config,
                                                                   trackingCuts          = trackingCuts,
                                                                   usePrdAssociationTool = usePrdAssociationTool, 
                                                                   trackSummaryTool      = trackSummaryTool )
   
   # --- Z-coordinates primary vertices finder (only for collisions)
   zVertexMakerTool = None
   #FIXME:Switch to trig flags? ATR-22756
   if InDetFlags.useZvertexTool():
      zVertexMakerTool =  zVertexMakerTool_builder(name, trackingCuts, siSpacePointsSeedMakerTool )

   # --- SCT and Pixel detector elements road builder
   #FIXME: use cosmic version of RMaker as for Run2?
   #https://gitlab.cern.ch/atlas/athena/-/blob/21.2/InnerDetector/InDetExample/InDetTrigRecExample/python/InDetTrigConfigRecNewTracking.py#L167
   siDetectorElementRoadMaker = siDetectorElementRoadMakerTool_builder( name         = add_prefix( 'SiDetectorElementRoadMaker', nameSuffix),
                                                                        trackingCuts = trackingCuts )

   # --- Local track finding using sdCaloSeededSSSpace point seed
   siCombinatorialTrackFinderTool = siCombinatorialTrackFinderTool_builder( name         = add_prefix( 'SiCombinatorialTrackFinder', nameSuffix),
                                                                            trackingCuts = trackingCuts)

   siTrackMakerTool =  siTrackMakerTool_builder( name                       = add_prefix( 'siTrackMaker', nameSuffix),
                                                 config                     = config,
                                                 siDetElementsRoadMakerTool = siDetectorElementRoadMaker,
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
                                    

