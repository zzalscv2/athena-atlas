
# Blocking the include for after first inclusion
include.block ('InDetRecExample/ConfiguredNewTrackingSiPattern.py')

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------

class  ConfiguredNewTrackingSiPattern:

   def __init__(self,
                InputCollections = None,
                ResolvedTrackCollectionKey = None,
                SiSPSeededTrackCollectionKey = None ,
                NewTrackingCuts = None,
                TrackCollectionKeys=[] ,
                TrackCollectionTruthKeys=[],
                ClusterSplitProbContainer='',
                doTrackOverlay=False):

      from InDetRecExample.InDetJobProperties import InDetFlags
      from InDetRecExample.InDetKeys          import InDetKeys

      import InDetRecExample.TrackingCommon   as TrackingCommon
      #
      # --- get ToolSvc and topSequence
      #
      from AthenaCommon.AppMgr                import ToolSvc
      from AthenaCommon.AlgSequence           import AlgSequence
      topSequence = AlgSequence()

      #
      # --- decide if use the association tool
      #
      if (len(InputCollections) > 0) and (NewTrackingCuts.mode() == "LowPt" or NewTrackingCuts.mode() == "VeryLowPt" or NewTrackingCuts.mode() == "LargeD0" or NewTrackingCuts.mode() == "R3LargeD0" or NewTrackingCuts.mode() == "LowPtLargeD0" or NewTrackingCuts.mode() == "BeamGas" or NewTrackingCuts.mode() == "ForwardTracks" or NewTrackingCuts.mode() == "Disappearing"):
         usePrdAssociationTool = True
      else:
         usePrdAssociationTool = False

      #
      # --- get list of already associated hits (always do this, even if no other tracking ran before)
      #
      asso_tool = None
      if usePrdAssociationTool:
         prefix     = 'InDet'
         suffix     = NewTrackingCuts.extension()
         InDetPRD_Association = TrackingCommon.getInDetTrackPRD_Association(namePrefix     = prefix,
                                                                            nameSuffix     = suffix,
                                                                            TracksName = list(InputCollections))

         # asso_tool = TrackingCommon.getConstPRD_AssociationTool(namePrefix     = prefix, nameSuffix = suffix)

         topSequence += InDetPRD_Association
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetPRD_Association)

      # ------------------------------------------------------------
      #
      # ----------- SiSPSeededTrackFinder
      #
      # ------------------------------------------------------------

      doSeedMakerValidation = InDetFlags.writeSeedValNtuple()

      if InDetFlags.doSiSPSeededTrackFinder():

         #
         # --- Space points seeds maker, use different ones for cosmics and collisions
         #
         if InDetFlags.doCosmics():
            from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_Cosmic as SiSpacePointsSeedMaker
         elif InDetFlags.doHeavyIon():
            from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_HeavyIon as SiSpacePointsSeedMaker
         elif NewTrackingCuts.mode() == "LowPt" or NewTrackingCuts.mode() == "VeryLowPt" or (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias()) :
            from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_LowMomentum as SiSpacePointsSeedMaker
         elif NewTrackingCuts.mode() == "BeamGas":
            from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_BeamGas as SiSpacePointsSeedMaker
         else:
            from SiSpacePointsSeedTool_xk.SiSpacePointsSeedTool_xkConf import InDet__SiSpacePointsSeedMaker_ATLxk as SiSpacePointsSeedMaker

         InDetSiSpacePointsSeedMaker = SiSpacePointsSeedMaker (name                   = "InDetSpSeedsMaker"+NewTrackingCuts.extension(),
                                                               pTmin                  = NewTrackingCuts.minPT(),
                                                               usePixel               = NewTrackingCuts.usePixel(),
                                                               SpacePointsPixelName   = InDetKeys.PixelSpacePoints(),
                                                               useSCT                 = (NewTrackingCuts.useSCT() and NewTrackingCuts.useSCTSeeding()),
                                                               SpacePointsSCTName     = InDetKeys.SCT_SpacePoints(),
                                                               useOverlapSpCollection = (NewTrackingCuts.useSCT() and NewTrackingCuts.useSCTSeeding()),
                                                               SpacePointsOverlapName = InDetKeys.OverlapSpacePoints(),
                                                               radMax                 = NewTrackingCuts.radMax(),
                                                               etaMax                 = NewTrackingCuts.maxEta())

         if doSeedMakerValidation:

           InDetSiSpacePointsSeedMaker.WriteNtuple = True

           from AthenaCommon.AppMgr import ServiceMgr
           if not hasattr(ServiceMgr, 'THistSvc'):
             from GaudiSvc.GaudiSvcConf import THistSvc
             ServiceMgr += THistSvc()

           ServiceMgr.THistSvc.Output  = ["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]

         if not InDetFlags.doCosmics():
            InDetSiSpacePointsSeedMaker.maxdImpact = NewTrackingCuts.maxPrimaryImpact()
            InDetSiSpacePointsSeedMaker.maxZ = NewTrackingCuts.maxZImpact()
            InDetSiSpacePointsSeedMaker.minZ = -NewTrackingCuts.maxZImpact()

         if InDetFlags.doHeavyIon():
            InDetSiSpacePointsSeedMaker.maxdImpactPPS = NewTrackingCuts.maxdImpactPPSSeeds()
            InDetSiSpacePointsSeedMaker.maxdImpactSSS = NewTrackingCuts.maxdImpactSSSSeeds()

         if NewTrackingCuts.mode() in ["Offline", "BLS", "ForwardTracks"]:
            InDetSiSpacePointsSeedMaker.maxdImpactSSS = NewTrackingCuts.maxdImpactSSSSeeds()
            InDetSiSpacePointsSeedMaker.maxSeedsForSpacePointStrips = NewTrackingCuts.MaxSeedsPerSP_Strips()
            InDetSiSpacePointsSeedMaker.maxSeedsForSpacePointPixels = NewTrackingCuts.MaxSeedsPerSP_Pixels()
            InDetSiSpacePointsSeedMaker.alwaysKeepConfirmedStripSeeds = NewTrackingCuts.KeepAllConfirmedStripSeeds()
            InDetSiSpacePointsSeedMaker.alwaysKeepConfirmedPixelSeeds = NewTrackingCuts.KeepAllConfirmedPixelSeeds()
            InDetSiSpacePointsSeedMaker.mindRadius  = 10.
            # limit size of space-point vector, uses auto-grow mechanism
            # to avoid exceeding bounds (should rarely happen)
            InDetSiSpacePointsSeedMaker.maxSizeSP  = 200
            InDetSiSpacePointsSeedMaker.dImpactCutSlopeUnconfirmedSSS  = 1.25
            InDetSiSpacePointsSeedMaker.dImpactCutSlopeUnconfirmedPPP  = 2.0

         elif NewTrackingCuts.mode() == "R3LargeD0":
            InDetSiSpacePointsSeedMaker.optimisePhiBinning = False
            InDetSiSpacePointsSeedMaker.usePixel = False
            InDetSiSpacePointsSeedMaker.maxSeedsForSpacePointStrips = NewTrackingCuts.MaxSeedsPerSP_Strips()
            InDetSiSpacePointsSeedMaker.alwaysKeepConfirmedStripSeeds = NewTrackingCuts.KeepAllConfirmedStripSeeds()
            InDetSiSpacePointsSeedMaker.maxdRadius = 150
            InDetSiSpacePointsSeedMaker.seedScoreBonusConfirmationSeed = -2000  #let's be generous

         if usePrdAssociationTool:
            # not all classes have that property !!!
            InDetSiSpacePointsSeedMaker.PRDtoTrackMap      = prefix+'PRDtoTrackMap'+suffix \
                                                                if usePrdAssociationTool else ''

         if (NewTrackingCuts.mode() in ["BeamGas", "LowPt", "VeryLowPt"] or
             (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias())):
            InDetSiSpacePointsSeedMaker.maxRadius1         = 0.75*NewTrackingCuts.radMax()
            InDetSiSpacePointsSeedMaker.maxRadius2         = NewTrackingCuts.radMax()
            if NewTrackingCuts.mode() == "BeamGas":
               InDetSiSpacePointsSeedMaker.maxRadius3      = NewTrackingCuts.radMax()

         if (NewTrackingCuts.mode() in ["LowPt", "VeryLowPt"] or
             (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias())):
            try :
               InDetSiSpacePointsSeedMaker.pTmax              = NewTrackingCuts.maxPT()
            except:
               pass 
            InDetSiSpacePointsSeedMaker.mindRadius         = 4.0

         if NewTrackingCuts.mode() == "ForwardTracks":
            InDetSiSpacePointsSeedMaker.checkEta           = True
            InDetSiSpacePointsSeedMaker.etaMin             = NewTrackingCuts.minEta()
            InDetSiSpacePointsSeedMaker.etaMax             = NewTrackingCuts.maxEta()
                    
         #InDetSiSpacePointsSeedMaker.OutputLevel = VERBOSE
         ToolSvc += InDetSiSpacePointsSeedMaker
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSiSpacePointsSeedMaker)
            
         #
         # --- Z-coordinates primary vertices finder (only for collisions)
         #
         if InDetFlags.useZvertexTool():
            from SiZvertexTool_xk.SiZvertexTool_xkConf import InDet__SiZvertexMaker_xk
            InDetZvertexMaker = InDet__SiZvertexMaker_xk(name          = 'InDetZvertexMaker'+NewTrackingCuts.extension(),
                                                         Zmax          = NewTrackingCuts.maxZImpact(),
                                                         Zmin          = -NewTrackingCuts.maxZImpact(),
                                                         minRatio      = 0.17) # not default
            InDetZvertexMaker.SeedMakerTool = InDetSiSpacePointsSeedMaker

            if InDetFlags.doHeavyIon():
               InDetZvertexMaker.HistSize = 2000
               ###InDetZvertexMaker.minContent = 200 
               InDetZvertexMaker.minContent = 30
               
            ToolSvc += InDetZvertexMaker
            if (InDetFlags.doPrintConfigurables()):
               printfunc (InDetZvertexMaker)

         else:
            InDetZvertexMaker = None

         #
         # --- SCT and Pixel detector elements road builder
         #
         from SiDetElementsRoadTool_xk.SiDetElementsRoadTool_xkConf import InDet__SiDetElementsRoadMaker_xk
         InDetSiDetElementsRoadMaker = InDet__SiDetElementsRoadMaker_xk(name               = 'InDetSiRoadMaker'+NewTrackingCuts.extension(),
                                                                        PropagatorTool     = InDetPatternPropagator,
                                                                        usePixel           = NewTrackingCuts.usePixel(),
                                                                        PixManagerLocation = InDetKeys.PixelManager(),
                                                                        useSCT             = NewTrackingCuts.useSCT(), 
                                                                        SCTManagerLocation = InDetKeys.SCT_Manager(),         
                                                                        RoadWidth          = NewTrackingCuts.RoadWidth())
         if (InDetFlags.doPrintConfigurables()):
            printfunc (     InDetSiDetElementsRoadMaker)
         # Condition algorithm for InDet__SiDetElementsRoadMaker_xk
         if DetFlags.haveRIO.pixel_on():
             # Condition algorithm for SiCombinatorialTrackFinder_xk
            from AthenaCommon.AlgSequence import AthSequencer
            condSeq = AthSequencer("AthCondSeq")
            if not hasattr(condSeq, "InDetSiDetElementBoundaryLinksPixelCondAlg"):
                from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiDetElementBoundaryLinksCondAlg_xk
                condSeq += InDet__SiDetElementBoundaryLinksCondAlg_xk(name = "InDetSiDetElementBoundaryLinksPixelCondAlg",
                                                                      ReadKey = "PixelDetectorElementCollection",
                                                                      WriteKey = "PixelDetElementBoundaryLinks_xk",
                                                                      UsePixelDetectorManager = True)

         if NewTrackingCuts.useSCT():
            from AthenaCommon.AlgSequence import AthSequencer
            condSeq = AthSequencer("AthCondSeq")
            if not hasattr(condSeq, "InDet__SiDetElementsRoadCondAlg_xk"):
               from SiDetElementsRoadTool_xk.SiDetElementsRoadTool_xkConf import InDet__SiDetElementsRoadCondAlg_xk
               condSeq += InDet__SiDetElementsRoadCondAlg_xk(name = "InDet__SiDetElementsRoadCondAlg_xk")

            if not hasattr(condSeq, "InDetSiDetElementBoundaryLinksSCTCondAlg"):
               from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiDetElementBoundaryLinksCondAlg_xk
               condSeq += InDet__SiDetElementBoundaryLinksCondAlg_xk(name = "InDetSiDetElementBoundaryLinksSCTCondAlg",
                                                                     ReadKey = "SCT_DetectorElementCollection",
                                                                     WriteKey = "SCT_DetElementBoundaryLinks_xk")


         #
         # --- Local track finding using sdCaloSeededSSSpace point seed
         #
         # @TODO ensure that PRD association map is used if usePrdAssociationTool is set
         rot_creator_digital = TrackingCommon.getInDetRotCreatorDigital()
         boundary_check_tool = TrackingCommon.getInDetBoundaryCheckTool()
         from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiCombinatorialTrackFinder_xk
         track_finder = InDet__SiCombinatorialTrackFinder_xk(name                  = 'InDetSiComTrackFinder'+NewTrackingCuts.extension(),
                                                             PropagatorTool        = InDetPatternPropagator,
                                                             UpdatorTool           = InDetPatternUpdator,
                                                             BoundaryCheckTool     = boundary_check_tool,
                                                             RIOonTrackTool        = rot_creator_digital,
                                                             usePixel              = DetFlags.haveRIO.pixel_on(),
                                                             useSCT                = DetFlags.haveRIO.SCT_on(),
                                                             PixelClusterContainer = InDetKeys.PixelClusters(),
                                                             SCT_ClusterContainer  = InDetKeys.SCT_Clusters(),
                                                             PixelDetElStatus        = 'PixelDetectorElementStatus' if DetFlags.haveRIO.pixel_on() else '',
                                                             SCTDetElStatus          = 'SCTDetectorElementStatus'   if DetFlags.haveRIO.SCT_on() else '')
         if NewTrackingCuts.mode() == "Offline" or NewTrackingCuts.mode() == "BLS": 
             track_finder.writeHolesFromPattern = InDetFlags.useHolesFromPattern()

         if (DetFlags.haveRIO.SCT_on()):
            track_finder.SctSummaryTool = InDetSCT_ConditionsSummaryTool
         else:
            track_finder.SctSummaryTool = None

         ToolSvc += track_finder

         useBremMode = NewTrackingCuts.mode() == "Offline" or NewTrackingCuts.mode() == "BLS"
         from SiTrackMakerTool_xk.SiTrackMakerTool_xkConf import InDet__SiTrackMaker_xk as SiTrackMaker
         InDetSiTrackMaker = SiTrackMaker(name                      = 'InDetSiTrackMaker'+NewTrackingCuts.extension(),
                                          useSCT                    = NewTrackingCuts.useSCT(),
                                          usePixel                  = NewTrackingCuts.usePixel(),
                                          RoadTool                  = InDetSiDetElementsRoadMaker,
                                          CombinatorialTrackFinder  = track_finder,
                                          pTmin                     = NewTrackingCuts.minPT(),
                                          pTminBrem                 = NewTrackingCuts.minPTBrem(),
                                          pTminSSS                  = InDetFlags.pT_SSScut(),
                                          nClustersMin              = NewTrackingCuts.minClusters(),
                                          nHolesMax                 = NewTrackingCuts.nHolesMax(),
                                          nHolesGapMax              = NewTrackingCuts.nHolesGapMax(),
                                          SeedsFilterLevel          = NewTrackingCuts.seedFilterLevel(),
                                          Xi2max                    = NewTrackingCuts.Xi2max(),
                                          Xi2maxNoAdd               = NewTrackingCuts.Xi2maxNoAdd(),
                                          nWeightedClustersMin      = NewTrackingCuts.nWeightedClustersMin(),
                                          CosmicTrack               = InDetFlags.doCosmics(),
                                          Xi2maxMultiTracks         = NewTrackingCuts.Xi2max(), # was 3.
                                          useSSSseedsFilter         = InDetFlags.doSSSfilter(),
                                          doMultiTracksProd         = True,
                                          useBremModel              = InDetFlags.doBremRecovery() and useBremMode, # only for NewTracking the brem is debugged !!!
                                          doCaloSeededBrem          = InDetFlags.doCaloSeededBrem(),
                                          doHadCaloSeedSSS          = InDetFlags.doHadCaloSeededSSS(),
                                          phiWidth                  = NewTrackingCuts.phiWidthBrem(),
                                          etaWidth                  = NewTrackingCuts.etaWidthBrem(),
                                          EMROIPhiRZContainer       = "InDetCaloClusterROIPhiRZ0GeV",
                                          HadROIPhiRZContainer      = "InDetHadCaloClusterROIPhiRZ",
                                          UseAssociationTool        = usePrdAssociationTool)

         if InDetFlags.doCosmics():
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_Cosmic'
	   
         elif InDetFlags.doHeavyIon():
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_HeavyIon'
         
         elif NewTrackingCuts.mode() == "LowPt":
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_LowMomentum'

         elif NewTrackingCuts.mode() == "VeryLowPt" or (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias()):
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_VeryLowMomentum'           

         elif NewTrackingCuts.mode() == "BeamGas":
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_BeamGas'
 
         elif NewTrackingCuts.mode() == "ForwardTracks":
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_ForwardTracks'

         elif NewTrackingCuts.mode() == "LargeD0" or NewTrackingCuts.mode() == "R3LargeD0" or NewTrackingCuts.mode() == "LowPtLargeD0":
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSpacePointsSeedMaker_LargeD0'
        
         else:
           InDetSiTrackMaker.TrackPatternRecoInfo = 'SiSPSeededFinder'
					  
         if InDetFlags.doStoreTrackSeeds():
              from SeedToTrackConversionTool.SeedToTrackConversionToolConf import InDet__SeedToTrackConversionTool
              InDet_SeedToTrackConversion = InDet__SeedToTrackConversionTool(name = "InDet_SeedToTrackConversion",
                                                                             Extrapolator = TrackingCommon.getInDetExtrapolator(),
                                                                             OutputName = InDetKeys.SiSPSeedSegments()+NewTrackingCuts.extension())
              InDetSiTrackMaker.SeedToTrackConversion = InDet_SeedToTrackConversion
              InDetSiTrackMaker.SeedSegmentsWrite = True

         #InDetSiTrackMaker.OutputLevel = VERBOSE				  
         ToolSvc += InDetSiTrackMaker
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSiTrackMaker)
         #
         # set output track collection name
        #
         self.__SiTrackCollection = SiSPSeededTrackCollectionKey
         #
         # --- Setup Track finder using space points seeds
         #

         from SiSPSeededTrackFinder.SiSPSeededTrackFinderConf import InDet__SiSPSeededTrackFinder

         if NewTrackingCuts.mode() == "ForwardTracks":

          InDetSiSPSeededTrackFinder = InDet__SiSPSeededTrackFinder(name             = 'InDetSiSpTrackFinder'+NewTrackingCuts.extension(),
                                                                    TrackTool        = InDetSiTrackMaker,
                                                                    PRDtoTrackMap    = prefix+'PRDtoTrackMap'+suffix \
                                                                                       if usePrdAssociationTool else '',
                                                                    TrackSummaryTool = TrackingCommon.getInDetTrackSummaryToolNoHoleSearch(),
                                                                    TracksLocation   = self.__SiTrackCollection,
                                                                    SeedsTool        = InDetSiSpacePointsSeedMaker,
                                                                    useZvertexTool   = InDetFlags.useZvertexTool(),
                                                                    ZvertexTool      = InDetZvertexMaker,
                                                                    useNewStrategy   = False,
                                                                    useMBTSTimeDiff  = InDetFlags.useMBTSTimeDiff(),
                                                                    useZBoundFinding = False)
          if InDetFlags.doHeavyIon() :
           InDetSiSPSeededTrackFinder.FreeClustersCut = 2 #Heavy Ion optimization from Igor
         
         else:
          InDetSiSPSeededTrackFinder = InDet__SiSPSeededTrackFinder(name             = 'InDetSiSpTrackFinder'+NewTrackingCuts.extension(),
                                                                    TrackTool        = InDetSiTrackMaker,
                                                                    PRDtoTrackMap    = prefix+'PRDtoTrackMap'+suffix \
                                                                                       if usePrdAssociationTool else '',
                                                                    TrackSummaryTool = TrackingCommon.getInDetTrackSummaryToolNoHoleSearch(),
                                                                    TracksLocation   = self.__SiTrackCollection,
                                                                    SeedsTool        = InDetSiSpacePointsSeedMaker,
                                                                    useZvertexTool   = InDetFlags.useZvertexTool(),
                                                                    ZvertexTool      = InDetZvertexMaker,
                                                                    useNewStrategy   = InDetFlags.useNewSiSPSeededTF(),
                                                                    useMBTSTimeDiff  = InDetFlags.useMBTSTimeDiff(),
                                                                    useZBoundFinding = NewTrackingCuts.doZBoundary())   

          if InDetFlags.doHeavyIon() :
           InDetSiSPSeededTrackFinder.FreeClustersCut = 2 #Heavy Ion optimization from Igor
          if NewTrackingCuts.mode() == "Offline" or NewTrackingCuts.mode() == "BLS": 
             InDetSiSPSeededTrackFinder.writeHolesFromPattern = InDetFlags.useHolesFromPattern()

         #InDetSiSPSeededTrackFinder.OutputLevel =VERBOSE 
         topSequence += InDetSiSPSeededTrackFinder
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSiSPSeededTrackFinder)

         if not InDetFlags.doSGDeletion():
            if InDetFlags.doTruth():
               #
               # set up the truth info for this container
               #
               include ("InDetRecExample/ConfiguredInDetTrackTruth.py")
               InDetTracksTruth = ConfiguredInDetTrackTruth(self.__SiTrackCollection,
                                                            self.__SiTrackCollection+"DetailedTruth",
                                                            self.__SiTrackCollection+"TruthCollection")
               #
               # add final output for statistics
               #
               TrackCollectionKeys      += [ InDetTracksTruth.Tracks() ]
               TrackCollectionTruthKeys += [ InDetTracksTruth.TracksTruth() ]
            else:
               TrackCollectionKeys      += [ self.__SiTrackCollection ]
               
      # ------------------------------------------------------------
      #
      # ---------- Ambiguity solving
      #
      # ------------------------------------------------------------

      if InDetFlags.doAmbiSolving():
         #
         # --- load InnerDetector TrackSelectionTool
         #
         
         prob1 = InDetFlags.pixelClusterSplitProb1()
         prob2 = InDetFlags.pixelClusterSplitProb2()
         nhitsToAllowSplitting = 9
         use_parameterization = False

         from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
         if CommonGeometryFlags.Run() == 1:
            prob1 = InDetFlags.pixelClusterSplitProb1_run1()
            prob2 = InDetFlags.pixelClusterSplitProb2_run1() 
            nhitsToAllowSplitting = 8

         drift_circle_cut_tool = TrackingCommon.getInDetTRTDriftCircleCutForPatternReco() if use_parameterization else ''

         if InDetFlags.doTIDE_Ambi() and not NewTrackingCuts.mode() == "ForwardTracks":
           from InDetAmbiTrackSelectionTool.InDetAmbiTrackSelectionToolConf import InDet__InDetDenseEnvAmbiTrackSelectionTool as AmbiTrackSelectionTool
         else:
           from InDetAmbiTrackSelectionTool.InDetAmbiTrackSelectionToolConf import InDet__InDetAmbiTrackSelectionTool as AmbiTrackSelectionTool
         InDetAmbiTrackSelectionTool = AmbiTrackSelectionTool(name                = 'InDetAmbiTrackSelectionTool'+NewTrackingCuts.extension(),
                                                              DriftCircleCutTool  = drift_circle_cut_tool,
                                                              AssociationTool     = TrackingCommon.getInDetPRDtoTrackMapToolGangedPixels(),
                                                              minHits             = NewTrackingCuts.minClusters(),
                                                              minNotShared        = NewTrackingCuts.minSiNotShared(),
                                                              maxShared           = NewTrackingCuts.maxShared(),
                                                              minTRTHits          = 0, # used for Si only tracking !!!
                                                              sharedProbCut       = 0.10,
                                                              UseParameterization = use_parameterization,
                                                              Cosmics             = InDetFlags.doCosmics(),
                                                              doPixelSplitting    = InDetFlags.doPixelClusterSplitting())
         if InDetFlags.doTIDE_Ambi() and not NewTrackingCuts.mode() == "ForwardTracks":
           InDetAmbiTrackSelectionTool.sharedProbCut             = prob1
           InDetAmbiTrackSelectionTool.sharedProbCut2            = prob2
           InDetAmbiTrackSelectionTool.minSiHitsToAllowSplitting = nhitsToAllowSplitting
           InDetAmbiTrackSelectionTool.minUniqueSCTHits          = 4
           InDetAmbiTrackSelectionTool.minTrackChi2ForSharedHits = 3
           InDetAmbiTrackSelectionTool.HadROIPhiRZContainer      = "InDetHadCaloClusterROIPhiRZBjet"
           InDetAmbiTrackSelectionTool.doHadCaloSeed             = InDetFlags.doCaloSeededAmbi()   #Do special cuts in region of interest
           InDetAmbiTrackSelectionTool.minPtSplit                = InDetFlags.pixelClusterSplitMinPt()       #Only allow split clusters on track withe pt greater than this MeV
           InDetAmbiTrackSelectionTool.phiWidth                  = 0.05     #Split cluster ROI size
           InDetAmbiTrackSelectionTool.etaWidth                  = 0.05     #Split cluster ROI size
           InDetAmbiTrackSelectionTool.EMROIPhiRZContainer       = "InDetCaloClusterROIPhiRZ10GeV"
           InDetAmbiTrackSelectionTool.minPtBjetROI              = 10000
           InDetAmbiTrackSelectionTool.doEmCaloSeed              = InDetFlags.doCaloSeededAmbi()   #Only split in cluster in region of interest
           InDetAmbiTrackSelectionTool.phiWidthEM                = 0.05     #Split cluster ROI size
           InDetAmbiTrackSelectionTool.etaWidthEM                = 0.05     #Split cluster ROI size

         if InDetFlags.doTIDE_AmbiTrackMonitoring() and InDetFlags.doTIDE_Ambi() and not (NewTrackingCuts.mode() == "ForwardSLHCTracks" or
                                                                                          NewTrackingCuts.mode() == "ForwardTracks" or
                                                                                          NewTrackingCuts.mode() == "PixelPrdAssociation" or
                                                                                          NewTrackingCuts.mode() == "PixelFourLayer" or
                                                                                          NewTrackingCuts.mode() == "PixelThreeLayer"):
           InDetAmbiTrackSelectionTool.ObserverTool = TrackingCommon.getTrackObserverTool()
         # if NewTrackingCuts.mode() == "ForwardTracks":
         #    InDetAmbiTrackSelectionTool.OutputLevel = VERBOSE

         ToolSvc += InDetAmbiTrackSelectionTool
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetAmbiTrackSelectionTool)
         #
         # --- set up different Scoring Tool for collisions and cosmics
         #
         if InDetFlags.doCosmics():
            InDetAmbiScoringTool = TrackingCommon.getInDetCosmicsScoringTool(NewTrackingCuts)
         elif(NewTrackingCuts.mode() == "R3LargeD0" and InDetFlags.nnCutLargeD0Threshold > 0):
            # Set up NN config
            InDetAmbiScoringTool = TrackingCommon.getInDetNNScoringToolSi(NewTrackingCuts)
         else:
            InDetAmbiScoringTool = TrackingCommon.getInDetAmbiScoringToolSi(NewTrackingCuts)
  
         #
         # --- load Ambiguity Processor
         #
         useBremMode = NewTrackingCuts.mode() == "Offline" or NewTrackingCuts.mode() == "BLS"

         # @TODO is the cluster split probability container needed here ?
         ambi_track_summary_tool = TrackingCommon.getInDetTrackSummaryTool(namePrefix                 = 'InDetAmbiguityProcessorSplitProb',
                                                                           nameSuffix                 = NewTrackingCuts.extension(),
                                                                           ClusterSplitProbabilityName= 'InDetAmbiguityProcessorSplitProb'+NewTrackingCuts.extension())
         if InDetFlags.doTIDE_Ambi() and not NewTrackingCuts.mode() == "ForwardTracks":
           # DenseEnvironmentsAmbiguityScoreProcessorTool
           from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__DenseEnvironmentsAmbiguityScoreProcessorTool as ScoreProcessorTool
           InDetAmbiguityScoreProcessor = ScoreProcessorTool(name               = 'InDetAmbiguityScoreProcessor'+NewTrackingCuts.extension(),
                                                             ScoringTool        = InDetAmbiScoringTool,
                                                             SplitProbTool      = NnPixelClusterSplitProbTool if InDetFlags.doPixelClusterSplitting() and 'NnPixelClusterSplitProbTool' in globals() else None,
                                                             AssociationTool    = TrackingCommon.getInDetPRDtoTrackMapToolGangedPixels(),
                                                             AssociationToolNotGanged  = TrackingCommon.getPRDtoTrackMapTool(),
                                                             AssociationMapName = 'PRDToTrackMap'+NewTrackingCuts.extension(),
                                                             InputClusterSplitProbabilityName = ClusterSplitProbContainer,
                                                             OutputClusterSplitProbabilityName = 'SplitProb'+NewTrackingCuts.extension())
           
            # DenseEnvironmentsAmbiguityProcessorTool
           from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__DenseEnvironmentsAmbiguityProcessorTool as ProcessorTool
           use_low_pt_fitter =  True if NewTrackingCuts.mode() == "LowPt" or NewTrackingCuts.mode() == "VeryLowPt" or (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias()) else False

           from AthenaCommon import CfgGetter
           from InDetRecExample.TrackingCommon import setDefaults
           fitter_args = setDefaults({},
                                     nameSuffix                   = 'Ambi'+NewTrackingCuts.extension(),
                                     SplitClusterMapExtension     = NewTrackingCuts.extension(),
                                     ClusterSplitProbabilityName  = 'InDetAmbiguityProcessorSplitProb'+NewTrackingCuts.extension())
           if InDetFlags.holeSearchInGX2Fit():
               fitter_args = setDefaults(fitter_args,
               DoHoleSearch                 = True,
               BoundaryCheckTool            = TrackingCommon.getInDetBoundaryCheckTool())
               
           fitter_list=[     CfgGetter.getPublicToolClone('InDetTrackFitter'+'Ambi'+NewTrackingCuts.extension(), 'InDetTrackFitter',**fitter_args)    if not use_low_pt_fitter \
                             else CfgGetter.getPublicToolClone('InDetTrackFitterLowPt'+NewTrackingCuts.extension(), 'InDetTrackFitterLowPt',**fitter_args)]
                      
           if InDetFlags.doRefitInvalidCov() :
              from AthenaCommon import CfgGetter
              if len(NewTrackingCuts.extension()) > 0 :
                 fitter_args = setDefaults({}, SplitClusterMapExtension = NewTrackingCuts.extension() )
                 fitter_list.append(CfgGetter.getPublicToolClone('KalmanFitter'         +NewTrackingCuts.extension(),'KalmanFitter',         **fitter_args))
                 fitter_list.append(CfgGetter.getPublicToolClone('ReferenceKalmanFitter'+NewTrackingCuts.extension(),'ReferenceKalmanFitter',**fitter_args))
              else :
                 fitter_list.append(CfgGetter.getPublicTool('KalmanFitter'))
                 fitter_list.append(CfgGetter.getPublicTool('ReferenceKalmanFitter'))

           InDetAmbiguityProcessor = ProcessorTool(name               = 'InDetAmbiguityProcessor'+NewTrackingCuts.extension(),
                                                   Fitter             = fitter_list ,
                                                   AssociationTool    = TrackingCommon.getInDetPRDtoTrackMapToolGangedPixels(),
                                                   AssociationMapName = 'PRDToTrackMap'+NewTrackingCuts.extension(),
                                                   TrackSummaryTool   = ambi_track_summary_tool,
                                                   ScoringTool        = InDetAmbiScoringTool,
                                                   SelectionTool      = InDetAmbiTrackSelectionTool,
                                                   InputClusterSplitProbabilityName = 'SplitProb'+NewTrackingCuts.extension(),
                                                   OutputClusterSplitProbabilityName = 'InDetAmbiguityProcessorSplitProb'+NewTrackingCuts.extension(),
                                                   SuppressHoleSearch = False,
                                                   tryBremFit         = InDetFlags.doBremRecovery() and useBremMode,
                                                   caloSeededBrem     = InDetFlags.doCaloSeededBrem(),
                                                   pTminBrem          = NewTrackingCuts.minPTBrem(),
                                                   RefitPrds          = True,
                                                   KeepHolesFromBeforeRefit = InDetFlags.useHolesFromPattern())

         else:
           from AthenaCommon import CfgGetter
           from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__SimpleAmbiguityProcessorTool as ProcessorTool
           InDetAmbiguityProcessor = ProcessorTool(name               = 'InDetAmbiguityProcessor'+NewTrackingCuts.extension(),
                                                 Fitter             = CfgGetter.getPublicTool('InDetTrackFitter'),
                                                 AssociationTool    = TrackingCommon.getInDetPRDtoTrackMapToolGangedPixels(),
                                                 TrackSummaryTool   = ambi_track_summary_tool,
                                                 ScoringTool        = InDetAmbiScoringTool,
                                                 SelectionTool      = InDetAmbiTrackSelectionTool,
                                                 InputClusterSplitProbabilityName = ClusterSplitProbContainer,
                                                 OutputClusterSplitProbabilityName = 'InDetAmbiguityProcessorSplitProb'+NewTrackingCuts.extension(),
                                                 SuppressHoleSearch = False,
                                                 tryBremFit         = InDetFlags.doBremRecovery() and useBremMode,
                                                 caloSeededBrem     = InDetFlags.doCaloSeededBrem(),
                                                 pTminBrem          = NewTrackingCuts.minPTBrem(),
                                                 RefitPrds          = True)
           InDetAmbiguityScoreProcessor = None

         if InDetFlags.doTIDE_Ambi() and not NewTrackingCuts.mode() == "ForwardTracks" and 'NnPixelClusterSplitProbTool' in globals() :
           if InDetAmbiguityScoreProcessor is not None :
              InDetAmbiguityScoreProcessor.sharedProbCut             = prob1
              InDetAmbiguityScoreProcessor.sharedProbCut2            = prob2
              if NewTrackingCuts.extension() == "":
                 InDetAmbiguityScoreProcessor.SplitClusterMap_old  = ""
              InDetAmbiguityScoreProcessor.SplitClusterMap_new  = InDetKeys.SplitClusterAmbiguityMap()+NewTrackingCuts.extension()

         if NewTrackingCuts.mode() == "Pixel":
            InDetAmbiguityProcessor.SuppressHoleSearch = True
         if NewTrackingCuts.mode() == "LowPt" or NewTrackingCuts.mode() == "VeryLowPt" or (NewTrackingCuts.mode() == "Pixel" and InDetFlags.doMinBias()):
            if InDetAmbiguityProcessor.getName().find('Dense') :
               pass
            else :
               from AthenaCommon import CfgGetter
               InDetAmbiguityProcessor.Fitter = CfgGetter.getPublicTool('InDetTrackFitterLowPt')
             
         if InDetFlags.materialInteractions():
            InDetAmbiguityProcessor.MatEffects = InDetFlags.materialInteractionsType()
         else:
            InDetAmbiguityProcessor.MatEffects = 0

         # if NewTrackingCuts.mode() == "ForwardTracks":
         #    InDetAmbiguityProcessor.OutputLevel = VERBOSE
         if InDetFlags.doTIDE_AmbiTrackMonitoring() and InDetFlags.doTIDE_Ambi() and not (NewTrackingCuts.mode() == "ForwardSLHCTracks" or
                                                                                          NewTrackingCuts.mode() == "ForwardTracks" or
                                                                                          NewTrackingCuts.mode() == "PixelPrdAssociation" or
                                                                                          NewTrackingCuts.mode() == "PixelFourLayer" or
                                                                                          NewTrackingCuts.mode() == "PixelThreeLayer"):
            InDetAmbiguityProcessor.ObserverTool = TrackingCommon.getTrackObserverTool()
            InDetAmbiguityProcessor.ObserverToolWriter = TrackingCommon.getTrackObserverTool(name = 'TrackObserverToolWriter', write_tracks = True)
            InDetAmbiguityScoreProcessor.ObserverTool = TrackingCommon.getTrackObserverTool()

         ToolSvc += InDetAmbiguityProcessor
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetAmbiguityProcessor)

         # add InDetAmbiguityScoreProcessor
         if InDetAmbiguityScoreProcessor is not None :
            ToolSvc += InDetAmbiguityScoreProcessor

         #
         # --- set input and output collection
         #
         InputTrackCollection     = self.__SiTrackCollection
         from OverlayCommonAlgs.OverlayFlags import overlayFlags
         self.__SiTrackCollection = ResolvedTrackCollectionKey
         #if track overlay, this isn't the final track collection
         if doTrackOverlay:
            self.__SiTrackCollection = overlayFlags.sigPrefix()+self.__SiTrackCollection
         #
         # --- configure Ambiguity (score) solver
         #
         from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguityScore
         # from RecExConfig.hideInput import hideInput
         # hideInput ('TrackCollection', self.__SiTrackCollection)
         InDetAmbiguityScore = Trk__TrkAmbiguityScore(name               = 'InDetAmbiguityScore'+NewTrackingCuts.extension(),
                                                        TrackInput         = [ InputTrackCollection ],
                                                        TrackOutput        = 'ScoredMap'+'InDetAmbiguityScore'+NewTrackingCuts.extension(),
                                                        AmbiguityScoreProcessor =  InDetAmbiguityScoreProcessor ) ## TODO: check the case when it is None object
         topSequence += InDetAmbiguityScore
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetAmbiguityScore)


         #
         # --- configure Ambiguity solver
         #
         from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguitySolver
         from RecExConfig.hideInput import hideInput
         hideInput ('TrackCollection', self.__SiTrackCollection)
         InDetAmbiguitySolver = Trk__TrkAmbiguitySolver(name               = 'InDetAmbiguitySolver'+NewTrackingCuts.extension(),
                                                        TrackInput         = 'ScoredMap'+'InDetAmbiguityScore'+NewTrackingCuts.extension(),
                                                        TrackOutput        = self.__SiTrackCollection,
                                                        AmbiguityProcessor = InDetAmbiguityProcessor)
         topSequence += InDetAmbiguitySolver
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetAmbiguitySolver)

         if doTrackOverlay:
            MergingTrackCollections=[self.__SiTrackCollection, overlayFlags.bkgPrefix()+ResolvedTrackCollectionKey]
            from InDetRecExample.TrackingCommon                        import getInDetPRDtoTrackMapToolGangedPixels
            from TrkTrackCollectionMerger.TrkTrackCollectionMergerConf import Trk__TrackCollectionMerger
            TrkTrackCollectionMerger = Trk__TrackCollectionMerger(name                    = "InDetTrackCollectionMerger_"+NewTrackingCuts.extension(),
                                                                  TracksLocation          = MergingTrackCollections,
                                                                  OutputTracksLocation    = ResolvedTrackCollectionKey,
                                                                  AssociationTool         = getInDetPRDtoTrackMapToolGangedPixels(),
                                                                  DoTrackOverlay          = True)
            topSequence += TrkTrackCollectionMerger
            self.__SiTrackCollection = ResolvedTrackCollectionKey
            

         #
         # --- Delete Silicon Sp-Seeded tracks
         #
         from InDetRecExample.ConfiguredInDetSGDeletion import InDetSGDeletionAlg
         InDetSGDeletionAlg(key = SiSPSeededTrackCollectionKey)

         if ( ( NewTrackingCuts.mode() in ["Pixel", "SCT"] ) or not InDetFlags.doSGDeletion()):
            if InDetFlags.doTruth():
               #
               # set up the truth info for this container
               #
               include ("InDetRecExample/ConfiguredInDetTrackTruth.py")
               InDetTracksTruth = ConfiguredInDetTrackTruth(self.__SiTrackCollection,
                                                            self.__SiTrackCollection+"DetailedTruth",
                                                            self.__SiTrackCollection+"TruthCollection")
               #
               # add final output for statistics
               #
               TrackCollectionKeys      += [ InDetTracksTruth.Tracks() ]
               TrackCollectionTruthKeys += [ InDetTracksTruth.TracksTruth() ]
            else:
               TrackCollectionKeys      += [ self.__SiTrackCollection ]

         if InDetFlags.doTIDE_AmbiTrackMonitoring() and InDetFlags.doTIDE_Ambi() and not (NewTrackingCuts.mode() == "ForwardSLHCTracks" or
                                                                                          NewTrackingCuts.mode() == "ForwardTracks" or
                                                                                          NewTrackingCuts.mode() == "PixelPrdAssociation" or
                                                                                          NewTrackingCuts.mode() == "PixelFourLayer" or
                                                                                          NewTrackingCuts.mode() == "PixelThreeLayer"):
           # add truth and trackParticles
           include ("InDetRecExample/ConfiguredInDetTrackTruth.py")
           InDetTracksTruth = ConfiguredInDetTrackTruth(InDetKeys.ObservedTracks(),
                                                        InDetKeys.ObservedDetailedTracksTruth(),
                                                        InDetKeys.ObservedTracksTruth())
           include ("InDetRecExample/ConfiguredxAODTrackParticleCreation.py")
           InDetxAOD = ConfiguredxAODTrackParticleCreation(InDetKeys.ObservedTracks(),
                                                           InDetKeys.ObservedTracksTruth(),
                                                           InDetKeys.xAODObservedTrackParticleContainer())
   def SiTrackCollection ( self ):
      try:
         return self.__SiTrackCollection
      except AttributeError:
         raise AttributeError("ConfiguredNewTrackingSiPattern: "\
                  " Neither InDetFlags.doSiSPSeededTrackFinder nor InDetFlags.doAmbiSolving"\
                  " are True, meaning the __SiTrackCollection data member has not been declared.")
               

