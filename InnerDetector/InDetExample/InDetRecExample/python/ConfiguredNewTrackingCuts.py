# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#########################################################################
# ConfiguredNewtrackingCuts class
#########################################################################
# Python Setup Class for NewTracking reconstruction
#
# Author: Markus Elsing
#
#########################################################################

import AthenaCommon.SystemOfUnits as Units
class ConfiguredNewTrackingCuts :

  def __init__ (self, mode = "Offline"):

    self.__mode      = mode
    self.__extension = ""
    self.__set_indetflags()     #pointer to InDetFlags, don't use them directly
                                #to allow sharing this code with the trigger

    from AthenaCommon.DetFlags import DetFlags

    # --- put defaults to run Pixel/SCT/TRT
    self.__usePixel = DetFlags.haveRIO.pixel_on()
    self.__useSCT   = DetFlags.haveRIO.SCT_on()
    self.__useTRT   = DetFlags.haveRIO.TRT_on()
    self.__useSCTSeeding = True

    # --------------------------------------
    # --- NEW TRACKING cuts
    # --------------------------------------
    #
    # --- start with is cutLevel() == 1, defaults for 2010 data
    self.__minPT                   = 0.1 * Units.GeV
    self.__minSecondaryPt          = 0.4 * Units.GeV  # Pt cut for back tracking + segment finding for these
    self.__minTRTonlyPt            = 0.4 * Units.GeV  # Pt cut for TRT only

    # --- first set kinematic defaults
    self.__maxPT                   = None            # off !
    self.__minEta                  = -1              # off !
    self.__maxEta                  = 2.7

    # --- cluster cuts
    self.__minClusters             = 7                # Igor 6, was 7
    self.__minSiNotShared          = 6
    self.__maxShared               = 1                # cut is now on number of shared modules
    self.__minPixel                = 0
    self.__maxHoles                = 3                # was 5
    self.__maxPixelHoles           = 2                # was 5
    self.__maxSctHoles             = 2                # was 5
    self.__maxDoubleHoles          = 1                # was 2
    self.__maxPrimaryImpact        = 10.0 * Units.mm  # low lumi
    self.__maxEMImpact             = 50.0 * Units.mm  # Ambiguity solver default
    self.__maxZImpact              = 320. * Units.mm  # Was 250 mm

    # --- this is for the TRT-extension
    self.__minTRTonTrk             = 9
    self.__minTRTPrecFrac          = 0.3

    # --- general pattern cuts for NewTracking
    self.__radMax                  = 600. * Units.mm # default R cut for SP in SiSpacePointsSeedMaker
    self.__roadWidth               = 20.
    self.__nHolesMax               = self.__maxHoles
    self.__nHolesGapMax            = self.__maxHoles # not as tight as 2*maxDoubleHoles
    self.__Xi2max                  = 15.0
    self.__Xi2maxNoAdd             = 35.0
    self.__nWeightedClustersMin    = 6

    # --- seeding
    self.__useSeedFilter           = True
    self.__maxdImpactPPSSeeds      = 1.7
    self.__maxdImpactSSSSeeds      = 1000.0
    self.__maxSeedsPerSP_Pixels    = 5
    self.__maxSeedsPerSP_Strips    = 5
    self.__keepAllConfirmedPixelSeeds   = False
    self.__keepAllConfirmedStripSeeds   = False

    # --- min pt cut for brem
    self.__minPTBrem               = 1. * Units.GeV # off
    self.__phiWidthBrem            = 0.3 # default is 0.3
    self.__etaWidthBrem            = 0.2 # default is 0.3


    # --- Z Boundary Seeding
    self.__doZBoundary             = False

    # --------------------------------------
    # --- BACK TRACKING cuts
    # --------------------------------------

    # --- settings for segment finder
    self.__TRTSegFinderPtBins        = 70
    self.__maxSegTRTShared           = 0.7
    self.__excludeUsedTRToutliers    = False

    # --- triggers SegmentFinder and BackTracking
    self.__useParameterizedTRTCuts   = False
    self.__useNewParameterizationTRT = False
    self.__maxSecondaryTRTShared     = 0.7

    # --- defaults for secondary tracking
    self.__maxSecondaryImpact        = 100.0 * Units.mm # low lumi
    self.__minSecondaryClusters      = 4
    self.__minSecondarySiNotShared   = 4
    self.__maxSecondaryShared        = 1   # cut is now on number of shared modules
    self.__minSecondaryTRTonTrk      = 10
    self.__minSecondaryTRTPrecFrac   = 0.
    self.__maxSecondaryHoles         = 2
    self.__maxSecondaryPixelHoles    = 2
    self.__maxSecondarySCTHoles      = 2
    self.__maxSecondaryDoubleHoles   = 1
    self.__SecondarynHolesMax        = self.__maxSecondaryHoles
    self.__SecondarynHolesGapMax     = self.__maxSecondaryHoles

    self.__rejectShortExtensions     = False # extension finder in back tracking
    self.__SiExtensionCuts           = False # cut in Si Extensions before fit

    # --- pattern cuts for back tracking
    self.__SecondaryXi2max           = 15.0
    self.__SecondaryXi2maxNoAdd      = 50.0

    # --- run back tracking and TRT only in RoI seed regions
    self.__RoISeededBackTracking     = False
    self.__minRoIClusterEt            = 0.
    # --------------------------------------
    # --- TRT Only TRACKING cuts
    # --------------------------------------

    # --- TRT only
    self.__minTRTonly                = 15
    self.__maxTRTonlyShared          = 0.7
    self.__useTRTonlyParamCuts       = False
    self.__useTRTonlyOldLogic        = True
    self.__TrkSel_TRTTrksEtaBins                  = [ 999, 999, 999, 999, 999, 999, 999, 999, 999, 999] # eta bins (10) for eta-dep cuts on TRT conversion tracks
    self.__TrkSel_TRTTrksMinTRTHitsThresholds     = [   0,   0,   0,   0,   0,   0,   0,   0,   0,   0] # eta-dep nTRT for TRT conversion tracks (> 15 is applied elsewhere)
    self.__TrkSel_TRTTrksMinTRTHitsMuDependencies = [   0,   0,   0,   0,   0,   0,   0,   0,   0,   0] # eta-dep nTRT, mu dependence for TRT conversion tracks

    #
    # --------------------------------------
    # --- now start tighening cuts level by level
    # --------------------------------------
    #
    if self.__indetflags.cutLevel() >= 2:
      # --- cutLevel() == 3, defaults for 2011 first processing
      self.__minPT                     = 0.4 * Units.GeV
      self.__minSecondaryPt            = 1.0 * Units.GeV  # Pt cut for back tracking + segment finding for these
      self.__minTRTonlyPt              = 1.0 * Units.GeV  # Pt cut for TRT only
      self.__TRTSegFinderPtBins        = 50

    if self.__indetflags.cutLevel() >= 3:
      # --- cutLevel() == 3, defaults for 2011 reprocessing
      self.__useParameterizedTRTCuts   = True # toggles BackTracking and TRT only
      self.__useNewParameterizationTRT = True

    if self.__indetflags.cutLevel() >= 4:
      # --- PUTF cuts
      self.__maxdImpactPPSSeeds        = 2.0     # loosen cut on PPS seeds
      self.__maxdImpactSSSSeeds        = 20.0    # apply cut on SSS seeds

    if self.__indetflags.cutLevel() >= 6:
      # --- stop using TRT outlies from failed extension fits to create BackTracks or TRT Only
      self.__excludeUsedTRToutliers    = True             # TRT outliers are added to the exclusion list
      # --- TRT only cuts
      self.__minTRTonlyPt              = 2.0 * Units.GeV  # increase Pt cut for TRT only to the value of egamma for 1 track conversions
      self.__useTRTonlyOldLogic        = False            # turn off ole overlap logic to reduce number of hits
      # self.__maxTRTonlyShared          = 0.2              # reduce number of shared hits

    if self.__indetflags.cutLevel() >= 7:
      # --- more BackTracking cuts
      self.__minSecondaryTRTonTrk      = 15               # let's not allow for short overlap tracks
      self.__maxSecondaryHoles         = 1                # tighten hole cuts
      self.__maxSecondaryPixelHoles    = 1                # tighten hole cuts
      self.__maxSecondarySCTHoles      = 1                # tighten hole cuts
      self.__maxSecondaryDoubleHoles   = 0                # tighten hole cuts
      self.__minSecondaryTRTPrecFrac   = 0.3              # default for all tracking now, as well for BackTracking. See "TRTStandalone" for standalone TRT tracks.
      self.__rejectShortExtensions     = True             # fall back onto segment if TRT extension is short
      self.__SiExtensionCuts           = True             # use cuts from ambi scoring already early
      # self.__maxSecondaryTRTShared     = 0.2              # tighen shared hit cut for segment maker ?

    if self.__indetflags.cutLevel() >= 8:
      # --- slightly tighen NewTracking cuts
      self.__maxHoles                  = 2                # was 3
      self.__maxPixelHoles             = 1                # was 2


    if self.__indetflags.cutLevel() >= 9:
      # --- tighten maxZ for the IP parameter
      self.__maxZImpact              = 250 * Units.mm

    if self.__indetflags.cutLevel() >= 10:
      # --- turn on Z Boundary seeding
      self.__doZBoundary              = True

    if self.__indetflags.cutLevel() >= 11:
      # --- turn on eta dependent cuts
      self.__useTRTonlyParamCuts      = True

    if self.__indetflags.cutLevel() >= 12:
      # --- Tighten track reconstruction criteria
      self.__Xi2max                  = 9.0  # was 15
      self.__Xi2maxNoAdd             = 25.0 # was 35
      self.__nHolesMax               = 2 # was 3
      self.__nHolesGapMax            = 2 # was 3

    if self.__indetflags.cutLevel() >= 13 and DetFlags.detdescr.Calo_allOn():
      # --- turn on RoI seeded for Back Tracking and TRT only
      self.__RoISeededBackTracking   = True
      self.__minRoIClusterEt         = 4500. * Units.MeV #Default cut to mimic rel21-ish


    if self.__indetflags.cutLevel() >= 14 :
      self.__minPT                   = 0.5 * Units.GeV

    if self.__indetflags.cutLevel() >= 15 :
      self.__minClusters             = 8 #based on studies by R.Jansky

    if self.__indetflags.cutLevel() >= 16 :
      self.__maxPrimaryImpact        = 5.0 * Units.mm #based on studies by T.Strebler

    if self.__indetflags.cutLevel() >= 17:
      # Tuning of the search road and strip seed IP in the track finder.
      # Designed to speed up reconstruction at minimal performance impact. 
      self.__roadWidth              = 12
      self.__maxdImpactSSSSeeds     = 5.0 * Units.mm
      self.__maxZImpact              = 200

    if self.__indetflags.cutLevel() >= 18:
      # Further tuning of the pattern recognition designed to 
      # speed up reconstruction compared to 17 with minimal additional 
      # impact. Kept as separate level pending cross-check of 
      # seed confirmation robustness with end-of-run-3 radiation
      # damage. 
      self.__keepAllConfirmedPixelSeeds  = True
      self.__maxSeedsPerSP_Pixels          = 1
      self.__maxSeedsPerSP_Strips          = 5
    
    if self.__indetflags.cutLevel() >= 19:
      # Calo cluster Et for RoI seeded backtracking for TRT segment finding
      # and for TRT-si extensions
      self.__minRoIClusterEt         = 6000. * Units.MeV
      self.__minSecondaryPt          = 3.0 * Units.GeV  # Increase pT cut used for back-tracking to match calo-RoI

    assert self.__indetflags.cutLevel() < 20 , "default cut level is wrong {}.format(self.__indetflags.cutLevel())"

    # --------------------------------------
    # --- now the overwrites for special setups
    # --------------------------------------

    # --- do robust reconstruction

    if self.__indetflags.doRobustReco():
      # ---- new tracking
      self.__minClusters             = 7                # Igor 6, was 7
      self.__maxHoles                = 5                # was 5
      self.__maxPixelHoles           = 2                # was 5
      self.__maxSctHoles             = 5                # was 5
      self.__maxDoubleHoles          = 4                # was 2
      self.__maxZImpact              = 500.0 * Units.mm
      # ---- back tracking
      self.__maxSecondaryHoles       = 5
      self.__maxSecondaryPixelHoles  = 5
      self.__maxSecondarySCTHoles    = 5
      self.__maxSecondaryDoubleHoles = 2

    if self.__indetflags.doInnerDetectorCommissioning():
      self.__minClusters             = 6
      self.__nWeightedClustersMin    = 6
      self.__minSiNotShared   = 5
      self.__rejectShortExtensions = False

    # --- IBL setup
    if mode == "IBL" :
      self.__extension               = "IBL"
      self.__minPT                   = 0.900 * Units.GeV
      self.__minClusters             = 10
      self.__maxPixelHoles           = 1

    # --- High pile-up setup
    if mode == "HighPileup" :
      self.__extension               = "HighPileup"
      self.__minPT                   = 0.900 * Units.GeV
      self.__minClusters             = 9
      self.__maxPixelHoles           = 0

    # --- mode for min bias, commissioning or doRobustReco
    if mode == 'MinBias' or self.__indetflags.doRobustReco():
      if self.__indetflags.doHIP300():
        self.__minPT                     = 0.300 * Units.GeV
      else:
        self.__minPT                     = 0.100 * Units.GeV
      self.__minClusters               = 5
      self.__minSecondaryPt            = 0.4 * Units.GeV  # Pt cut for back tracking + segment finding for these
      self.__minTRTonlyPt              = 0.4 * Units.GeV  # Pt cut for TRT only
      self.__TRTSegFinderPtBins        = 50
      self.__maxdImpactSSSSeeds        = 20.0    # apply cut on SSS seeds
      self.__excludeUsedTRToutliers    = False   # TRT outliers are added to the exclusion list
      self.__useTRTonlyOldLogic        = True    # turn off ole overlap logic to reduce number of hits
      self.__maxSecondaryImpact        = 100.0 * Units.mm # low lumi

    # --- mode for high-d0 tracks
    if mode == "LargeD0":
      self.__extension          = "LargeD0" # this runs parallel to NewTracking
      self.__maxPT              = 1.0 * Units.TeV
      self.__minPT              = 900 * Units.MeV
      self.__maxEta             = 5
      self.__maxPrimaryImpact   = 300.0 * Units.mm
      self.__maxEMImpact        = 300.0 * Units.mm  # Ambiguity solver
      self.__maxZImpact         = 1500.0 * Units.mm
      self.__maxSecondaryImpact = 300.0 * Units.mm
      self.__minSecondaryPt     = 500.0 * Units.MeV
      self.__minClusters        = 7
      self.__minSiNotShared     = 5
      self.__maxShared          = 2   # cut is now on number of shared modules
      self.__minPixel           = 0
      self.__maxHoles           = 2
      self.__maxPixelHoles      = 1
      self.__maxSctHoles        = 2
      self.__maxDoubleHoles     = 1
      self.__radMax             = 600. * Units.mm
      self.__nHolesMax          = self.__maxHoles
      self.__nHolesGapMax       = self.__maxHoles # not as tight as 2*maxDoubleHoles
      self.__maxTracksPerSharedPRD = 2

    # --- mode for high-d0 tracks (re-optimisation for Run 2 by M.Danninger)
    if mode == "R3LargeD0":
      self.__extension          = "R3LargeD0" # this runs parallel to NewTracking                             
      self.__maxPT              = 1.0 * Units.TeV
      self.__minPT              = 1.0 * Units.GeV                                                                                    
      self.__maxEta             = 3                                                                                                        
      self.__maxPrimaryImpact   = 300.0 * Units.mm
      self.__maxEMImpact        = 300.0 * Units.mm  # Ambiguity solver
      self.__maxZImpact         = 500 * Units.mm    
      self.__maxSecondaryImpact = 300.0 * Units.mm  
      self.__minSecondaryPt     = 1000.0 * Units.MeV 
      self.__minClusters        = 8                  
      self.__minSiNotShared     = 6                 
      self.__maxShared          = 2   # cut is now on number of shared modules   
      self.__minPixel           = 0
      self.__maxHoles           = 2
      self.__maxPixelHoles      = 1
      self.__maxSctHoles        = 1  
      self.__maxDoubleHoles     = 0  
      self.__radMax             = 600. * Units.mm
      self.__nHolesMax          = self.__maxHoles
      self.__nHolesGapMax       = 1
      self.__maxTracksPerSharedPRD   = 2
      self.__Xi2max                  = 9.0  
      self.__Xi2maxNoAdd             = 25.0 
      self.__roadWidth               = 5. 
      self.__nWeightedClustersMin    = 8   
      self.__maxdImpactSSSSeeds      = 300.0
      self.__doZBoundary             = True
      self.__keepAllConfirmedStripSeeds   = True
      self.__maxSeedsPerSP_Strips    = 1


    # --- mode for high-d0 tracks down to 100 MeV (minPT, minClusters, minSecondaryPt cuts loosened to MinBias level)
    if mode == "LowPtLargeD0":
      self.__extension          = "LowPtLargeD0" # this runs parallel to NewTracking
      self.__maxPT              = 1.0 * Units.TeV
      self.__minPT              = 100 * Units.MeV
      self.__maxEta             = 5
      self.__maxPrimaryImpact   = 300.0 * Units.mm
      self.__maxEMImpact        = 300.0 * Units.mm  # Ambiguity solver
      self.__maxZImpact         = 1500.0 * Units.mm
      self.__maxSecondaryImpact = 300.0 * Units.mm
      self.__minSecondaryPt     = 400.0 * Units.MeV
      self.__minClusters        = 5
      self.__minSiNotShared     = 5
      self.__maxShared          = 2   # cut is now on number of shared modules
      self.__minPixel           = 0
      self.__maxHoles           = 2
      self.__maxPixelHoles      = 1
      self.__maxSctHoles        = 2
      self.__maxDoubleHoles     = 1
      self.__radMax             = 600. * Units.mm
      self.__nHolesMax          = self.__maxHoles
      self.__nHolesGapMax       = self.__maxHoles # not as tight as 2*maxDoubleHoles
      self.__maxTracksPerSharedPRD = 2

    # --- change defaults for low pt tracking
    if mode == "LowPt":
      self.__extension        = "LowPt" # this runs parallel to NewTracking
      self.__maxPT            = self.__minPT + 0.3 * Units.GeV # some overlap
      self.__minPT            = 0.050 * Units.GeV
      self.__minClusters      = 5
      self.__minSiNotShared   = 4
      self.__maxShared        = 1   # cut is now on number of shared modules
      self.__minPixel         = 2   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
      self.__maxHoles         = 2
      self.__maxPixelHoles    = 1
      self.__maxSctHoles      = 2
      self.__maxDoubleHoles   = 1
      self.__radMax           = 600. * Units.mm
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles # not as tight as 2*maxDoubleHoles

      if self.__indetflags.doMinBias():
        self.__maxPT            = 1000000 * Units.GeV # Won't accept None *NEEDS FIXING*
        self.__maxPrimaryImpact = 100.0 * Units.mm

    # --- change defaults for very low pt tracking
    if mode == "VeryLowPt":
      self.__extension        = "VeryLowPt" # this runs parallel to NewTracking
      self.__maxPT            = self.__minPT + 0.3 * Units.GeV # some overlap
      self.__minPT            = 0.050 * Units.GeV
      self.__minClusters      = 3
      self.__minSiNotShared   = 3
      self.__maxShared        = 1   # cut is now on number of shared modules
      self.__minPixel         = 3   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
      self.__maxHoles         = 1
      self.__maxPixelHoles    = 1
      self.__maxSctHoles      = 1
      self.__maxDoubleHoles   = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles # not as tight as 2*maxDoubleHoles
      self.__radMax           = 600. * Units.mm # restrivt to pixels

      if self.__indetflags.doMinBias():
        self.__maxPT            = 100000 * Units.GeV # Won't accept None *NEEDS FIXING*

    # --- change defaults for forward muon tracking
    if mode == "ForwardTracks":
      self.__extension        = "ForwardTracks" # this runs parallel to NewTracking
      self.__minEta           = 2.4 # restrict to minimal eta
      self.__maxEta           = 2.7
      self.__minPT            = 2 * Units.GeV
      self.__minClusters      = 3
      self.__minSiNotShared   = 3
      self.__maxShared        = 1
      self.__minPixel         = 3
      self.__maxHoles         = 1
      self.__maxPixelHoles    = 1
      self.__maxSctHoles      = 1
      self.__maxDoubleHoles   = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      self.__radMax           = 600. * Units.mm

      self.__useTRT           = False # no TRT for forward tracks

    # --- change defauls for beam gas tracking
    if mode == "BeamGas":
      self.__extension        = "BeamGas" # this runs parallel to NewTracking
      self.__minPT            = 0.500 * Units.GeV
      self.__maxPrimaryImpact = 300. * Units.mm
      self.__maxZImpact       = 2000. * Units.mm
      self.__minClusters      = 6
      self.__maxHoles         = 3
      self.__maxPixelHoles    = 3
      self.__maxSctHoles      = 3
      self.__maxDoubleHoles   = 1
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles # not as tight as 2*maxDoubleHoles

    # --- setup for lumi determination based on vertices
    if mode == "VtxLumi" :
      self.__extension               = "VtxLumi"
      self.__minPT                   = 0.900 * Units.GeV
      self.__minClusters             = 7
      self.__maxPixelHoles           = 1
      self.__radMax                  = 600. * Units.mm
      self.__nHolesMax               = 2
      self.__nHolesGapMax            = 1
      self.__useTRT                  = False

 # --- setup for beamspot determination based on vertices
    if mode == "VtxBeamSpot" :
      self.__extension               = "VtxBeamSpot"
      self.__minPT                   = 0.900 * Units.GeV
      self.__minClusters             = 9
      self.__maxPixelHoles           = 0
      self.__radMax                  = 320. * Units.mm
      self.__nHolesMax               = 2
      self.__nHolesGapMax            = 1
      self.__useTRT                  = False

  # --- changes for cosmics
    if mode == "Cosmics":
      self.__minPT            = 0.500 * Units.GeV
      self.__maxPrimaryImpact = 1000. * Units.mm
      self.__maxZImpact       = 10000. * Units.mm
      self.__minClusters      = 4
      self.__minSiNotShared   = 4
      self.__maxHoles         = 3
      self.__maxPixelHoles    = 3
      self.__maxSctHoles      = 3
      self.__maxDoubleHoles   = 1
      self.__minTRTonTrk      = 15
      self.__minTRTonly       = 15
      self.__roadWidth        = 60.
      self.__Xi2max           = 60.0
      self.__Xi2maxNoAdd      = 100.0
      self.__nWeightedClustersMin = 8
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles # not as tight as 2*maxDoubleHoles

    # --- changes for heavy ion
    if mode == "HeavyIon":
      self.__maxZImpact       = 200. * Units.mm
      self.__minPT            = 0.500 * Units.GeV
      self.__minClusters      = 9
      self.__minSiNotShared   = 7
      self.__maxShared        = 2 # was 1, cut is now on number of shared modules
      self.__maxHoles         = 0
      self.__maxPixelHoles    = 0
      self.__maxSctHoles      = 0
      self.__maxDoubleHoles   = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      self.__Xi2max           = 6.
      self.__Xi2maxNoAdd      = 10.
      if self.__indetflags.cutLevel() == 2:
        self.__maxdImpactSSSSeeds        = 20.0 # apply cut on SSS seeds
      elif self.__indetflags.cutLevel() == 3: # This is for MB data
        self.__minPT            = 0.300 * Units.GeV
        self.__maxdImpactSSSSeeds        = 20.0 # apply cut on SSS seeds
        self.__useParameterizedTRTCuts   = False #Make these false on all HI cut levels >=3, since standard cut levels set it true from levels >=3
        self.__useNewParameterizationTRT = False
      elif self.__indetflags.cutLevel() == 4: # ==CutLevel 2 with loosened hole cuts and chi^2 cuts
        self.__maxdImpactSSSSeeds        = 20.0 # apply cut on SSS seeds
        self.__maxdImpactPPSSeeds      = 1.7 #set this to 1.7 for all HI cut levels >=4, since standard cut levels set it to 2.0 from levels >=4. Not sure it has any effect, since we don't usually run mixed seeds (also true for HI?)
        self.__useParameterizedTRTCuts   = False
        self.__useNewParameterizationTRT = False
        self.__maxHoles               = 2
        self.__maxPixelHoles       = 1
        self.__maxSctHoles         = 1
        self.__maxDoubleHoles   = 0
        self.__Xi2max                   = 9.
        self.__Xi2maxNoAdd        = 25.
      elif self.__indetflags.cutLevel() == 5: # ==CutLevel 3 with loosened hole cuts and chi^2 cuts
        self.__minPT            = 0.300 * Units.GeV
        self.__maxdImpactSSSSeeds        = 20.0 # apply cut on SSS seeds
        self.__maxdImpactPPSSeeds      = 1.7
        self.__useParameterizedTRTCuts   = False
        self.__useNewParameterizationTRT = False
        self.__maxHoles               = 2
        self.__maxPixelHoles       = 1
        self.__maxSctHoles         = 1
        self.__maxDoubleHoles   = 0
        self.__Xi2max                   = 9.
        self.__Xi2maxNoAdd        = 25.

        self.__radMax           = 600. * Units.mm # restrict to pixels + first SCT layer
        self.__useTRT           = False

    # --- changes for Pixel/SCT segments
    from AthenaCommon.DetFlags    import DetFlags
    if ( DetFlags.haveRIO.pixel_on() and not DetFlags.haveRIO.SCT_on() ):
      self.__minClusters = 3
    elif ( DetFlags.haveRIO.SCT_on() and not DetFlags.haveRIO.pixel_on() ):
      self.__minClusters = 6

    # --- changes for Pixel segments
    if mode == "Pixel":
      self.__extension        = "Pixel" # this runs parallel to NewTracking
      self.__minPT            = 0.1 * Units.GeV
      self.__minClusters      = 3
      self.__maxHoles         = 1
      self.__maxPixelHoles    = 1
      self.__maxSctHoles      = 0
      self.__maxDoubleHoles   = 0
      self.__minSiNotShared   = 3
      self.__maxShared        = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      self.__useSCT           = False
      self.__useTRT           = False

      if self.__indetflags.doMinBias():
        if self.__indetflags.doHIP300():
          self.__minPT            = 0.300 * Units.GeV
        else:
          self.__minPT            = 0.05 * Units.GeV
        self.__maxPT            = 100000 * Units.GeV # Won't accept None *NEEDS FIXING*

      if self.__indetflags.doHeavyIon():
        self.__minPT            = 0.1 * Units.GeV
        self.__maxHoles         = 0
        self.__maxPixelHoles    = 0
        self.__minSiNotShared   = 3
        self.__maxShared        = 0
        self.__nHolesMax        = self.__maxHoles
        self.__nHolesGapMax     = self.__maxHoles
        self.__useSCT           = False
        self.__useTRT           = False

      if self.__indetflags.doCosmics():
        self.__minPT            = 0.500 * Units.GeV
        self.__maxPrimaryImpact = 1000. * Units.mm
        self.__maxZImpact       = 10000. * Units.mm
        self.__maxHoles         = 3
        self.__maxPixelHoles    = 3
        self.__maxShared        = 0    # no shared hits in cosmics
        self.__roadWidth        = 60.
        self.__nHolesMax        = self.__maxHoles
        self.__nHolesGapMax     = self.__maxHoles
        self.__Xi2max           = 60.0
        self.__Xi2maxNoAdd      = 100.0
        self.__nWeightedClustersMin = 6


    if mode == "Disappearing":
      self.__extension        = "Disappearing" # this runs after NewTracking
      self.__minPT            = 5.0 * Units.GeV
      self.__minClusters      = 4
      self.__maxHoles         = 0
      self.__maxPixelHoles    = 0
      self.__maxSctHoles      = 0
      self.__maxDoubleHoles   = 0
      self.__minSiNotShared   = 3
      self.__maxShared        = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      # self.__useSCT           = False
      self.__useSCT           = True
      # self.__useTRT           = False
      self.__useTRT           = True
      self.__useSCTSeeding    = False
      self.__maxEta           = 2.2


    # --- changes for SCT segments
    if mode == "SCT":
      self.__extension        = "SCT" # this runs parallel to NewTracking
      self.__minPT            = 0.1 * Units.GeV
      self.__minClusters      = 7
      self.__maxHoles         = 2
      self.__maxPixelHoles    = 0
      self.__maxSctHoles      = 2
      self.__maxDoubleHoles   = 1
      self.__minSiNotShared   = 5
      self.__maxShared        = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      self.__usePixel         = False
      self.__useTRT           = False

      if self.__indetflags.doMinBias():
        if self.__indetflags.doHIP300():
           self.__minPT            = 0.3 * Units.GeV
        else:
           self.__minPT            = 0.1 * Units.GeV

      if self.__indetflags.doCosmics():
        self.__minPT            = 0.500 * Units.GeV
        self.__maxPrimaryImpact = 1000. * Units.mm
        self.__maxZImpact       = 10000. * Units.mm
        self.__maxHoles         = 3
        self.__maxPixelHoles    = 0
        self.__maxSctHoles      = 3
        self.__maxShared        = 0   # no shared hits in cosmics
        self.__roadWidth        = 60.
        self.__nHolesMax        = self.__maxHoles
        self.__nHolesGapMax     = self.__maxHoles
        self.__Xi2max           = 60.0
        self.__Xi2maxNoAdd      = 100.0
        self.__nWeightedClustersMin = 6

        if self.__indetflags.doInnerDetectorCommissioning():
          self.__minClusters      = 4
          self.__minSiNotShared   = 4
          self.__nWeightedClustersMin = 4

    # --- TRT subdetector tracklet cuts
    if mode == "TRT":
      self.__minPT                   = 0.4 * Units.GeV
      self.__minTRTonly              = 15
      self.__maxTRTonlyShared        = 0.7

    # --- TRT Standalone tracks (used by conversion finding)
    if mode == "TRTStandalone":
      # minSecondaryTRTPrecFrac is fed into ConfiguredTRTStandalone and eventually
      # into InDet__InDetTrtTrackScoringTool:
      self.__minSecondaryTRTPrecFrac = 0.15
      # Mu- and eta- dependent cuts on nTRT
      self.__TrkSel_TRTTrksEtaBins                  = [  0.7,   0.8,   0.9,  1.2,  1.3,  1.6,  1.7,  1.8,  1.9,  999] # eta bins (10) for eta-dep cuts on TRT conversion tracks
      self.__TrkSel_TRTTrksMinTRTHitsThresholds     = [   27,    18,    18,   18,   26,   28,   26,   24,   22,    0] # eta-dep nTRT for TRT conversion tracks (> 15 is applied elsewhere)
      self.__TrkSel_TRTTrksMinTRTHitsMuDependencies = [  0.2,  0.05,  0.05, 0.05, 0.15, 0.15, 0.15, 0.15, 0.15,    0] # eta-dep nTRT, mu dependence for TRT conversion tracks

    # --- mode for SCT and TRT
    if mode == "SCTandTRT":
      self.__extension        = "SCTandTRT" # this runs parallel to NewTracking
      self.__minPT            = 0.4 * Units.GeV
      self.__minClusters      = 7
      self.__maxHoles         = 2
      self.__maxPixelHoles    = 0
      self.__maxSctHoles      = 2
      self.__maxDoubleHoles   = 0
      self.__minSiNotShared   = 5
      self.__maxShared        = 0
      self.__nHolesMax        = self.__maxHoles
      self.__nHolesGapMax     = self.__maxHoles
      self.__usePixel         = False
      self.__useTRT           = True

      if self.__indetflags.doCosmics():
        self.__minPT            = 0.500 * Units.GeV
        self.__maxPrimaryImpact = 1000. * Units.mm
        self.__maxZImpact       = 10000. * Units.mm
        self.__maxHoles         = 3
        self.__maxPixelHoles    = 0
        self.__maxSctHoles      = 3
        self.__maxShared        = 0   # no shared hits in cosmics
        self.__roadWidth        = 60.
        self.__nHolesMax        = self.__maxHoles
        self.__nHolesGapMax     = self.__maxHoles
        self.__Xi2max           = 60.0
        self.__Xi2maxNoAdd      = 100.0
        self.__nWeightedClustersMin = 6

        if self.__indetflags.doInnerDetectorCommissioning():
          self.__minClusters      = 4
          self.__nWeightedClustersMin = 4
          self.__minSiNotShared   = 4
          self.__rejectShortExtensions     = False

    # using d0<10mm instead of d0<5mm. This is desired for B-physics
    if mode == "BLS":
      self.__maxPrimaryImpact   = 10.0 * Units.mm 
      self.__maxdImpactSSSSeeds = 10.0 * Units.mm


# ----------------------------------------------------------------------------
# --- private method
  def __set_indetflags(self):
    from InDetRecExample.InDetJobProperties import InDetFlags
    self.__indetflags = InDetFlags

# ----------------------------------------------------------------------------
# --- return methods for the cut values - the main purpose of this class
# ----------------------------------------------------------------------------

  def mode( self ) :
    return self.__mode

  def extension( self ) :
    return self.__extension

  def minPT( self ) :
    return self.__minPT

  def maxPT( self ) :
    return self.__maxPT

  def minPTBrem( self ) :
    return self.__minPTBrem

  def phiWidthBrem( self ) :
    return self.__phiWidthBrem

  def etaWidthBrem( self ) :
    return self.__etaWidthBrem

  def maxPrimaryImpact( self ) :
    return self.__maxPrimaryImpact

  def maxEMImpact ( self ):
    return self.__maxEMImpact

  def maxSecondaryImpact( self ) :
    return self.__maxSecondaryImpact

  def maxZImpact( self ) :
    return self.__maxZImpact

  def minEta( self ) :
    return self.__minEta

  def maxEta( self ) :
    return self.__maxEta

  def minClusters( self ) :
    return self.__minClusters

  def minPixel( self ) :
    return self.__minPixel

  def minSecondaryClusters( self ) :
    return self.__minSecondaryClusters

  def minSiNotShared( self ) :
    return self.__minSiNotShared

  def minSecondarySiNotShared( self ) :
    return self.__minSecondarySiNotShared

  def maxShared( self ) :
    return self.__maxShared

  def maxSecondaryShared( self ) :
    return self.__maxSecondaryShared

  def maxHoles( self ) :
    return self.__maxHoles

  def maxPixelHoles( self ) :
    return self.__maxPixelHoles

  def maxSCTHoles( self ) :
    return self.__maxSctHoles

  def maxSecondaryHoles( self ) :
    return self.__maxSecondaryHoles

  def maxSecondaryPixelHoles( self ) :
    return self.__maxSecondaryPixelHoles

  def maxSecondarySCTHoles( self ) :
    return self.__maxSecondarySCTHoles

  def maxDoubleHoles( self ) :
    return self.__maxDoubleHoles

  def maxSecondaryDoubleHoles( self ) :
    return self.__maxSecondaryDoubleHoles

  def maxSecondaryTRTShared( self ) :
    return self.__maxSecondaryTRTShared

  def minTRTonTrk( self ) :
    return self.__minTRTonTrk

  def TrkSel_TRTTrksEtaBins( self ) :
    return self.__TrkSel_TRTTrksEtaBins

  def TrkSel_TRTTrksMinTRTHitsThresholds( self ) :
    return self.__TrkSel_TRTTrksMinTRTHitsThresholds

  def TrkSel_TRTTrksMinTRTHitsMuDependencies( self ) :
    return self.__TrkSel_TRTTrksMinTRTHitsMuDependencies

  def minTRTPrecFrac( self ) :
    return self.__minTRTPrecFrac

  def useParameterizedTRTCuts( self ) :
    return self.__useParameterizedTRTCuts

  def useNewParameterizationTRT( self ) :
    return self.__useNewParameterizationTRT

  def minSecondaryTRTonTrk ( self ) :
    return self.__minSecondaryTRTonTrk

  def minSecondaryTRTPrecFrac( self ) :
    return self.__minSecondaryTRTPrecFrac

  def TRTSegFinderPtBins ( self ) :
    return self.__TRTSegFinderPtBins

  def minTRTonly( self ) :
    return self.__minTRTonly

  def maxTRTonlyShared( self ) :
    return self.__maxTRTonlyShared

  def excludeUsedTRToutliers ( self ) :
    return self.__excludeUsedTRToutliers

  def rejectShortExtensions ( self ) :
    return self.__rejectShortExtensions

  def SiExtensionCuts ( self ) :
    return self.__SiExtensionCuts

  def useTRTonlyParamCuts ( self ) :
    return self.__useTRTonlyParamCuts

  def useTRTonlyOldLogic ( self ) :
    return self.__useTRTonlyOldLogic

  def minSecondaryPt( self ) :
    return self.__minSecondaryPt

  def minTRTonlyPt( self ) :
    return self.__minTRTonlyPt

  def RoadWidth( self ) :
    return self.__roadWidth

  def MaxSeedsPerSP_Strips( self ) :
    return self.__maxSeedsPerSP_Strips

  def MaxSeedsPerSP_Pixels( self ) :
    return self.__maxSeedsPerSP_Pixels
    
  def KeepAllConfirmedPixelSeeds( self ) :
    return self.__keepAllConfirmedPixelSeeds
    
  def KeepAllConfirmedStripSeeds( self ) :
    return self.__keepAllConfirmedStripSeeds

  def useSeedFilter( self ) :
    return self.__useSeedFilter

  def radMax( self ) :
    return self.__radMax

  def nHolesMax( self ) :
    return self.__nHolesMax

  def nHolesGapMax( self ) :
    return self.__nHolesGapMax

  def Xi2max( self ) :
    return self.__Xi2max

  def Xi2maxNoAdd( self ) :
    return self.__Xi2maxNoAdd

  def SecondarynHolesMax( self ) :
    return self.__SecondarynHolesMax

  def SecondarynHolesGapMax( self ) :
    return self.__SecondarynHolesGapMax

  def SecondaryXi2max( self ) :
    return self.__SecondaryXi2max

  def SecondaryXi2maxNoAdd( self ) :
    return self.__SecondaryXi2maxNoAdd

  def nWeightedClustersMin( self ) :
    return self.__nWeightedClustersMin

  def maxdImpactPPSSeeds( self ) :
    return self.__maxdImpactPPSSeeds

  def maxdImpactSSSSeeds( self ) :
    return self.__maxdImpactSSSSeeds

  def usePixel( self ) :
    return self.__usePixel

  def useSCT( self ) :
    return self.__useSCT

  def useSCTSeeding( self ) :
    return self.__useSCTSeeding

  def useTRT( self ) :
    return self.__useTRT

  def doZBoundary( self ) :
    return self.__doZBoundary

  def RoISeededBackTracking( self ) :
    return self.__RoISeededBackTracking

  def minRoIClusterEt( self ) :
    return self.__minRoIClusterEt

  def printInfo( self ) :
    print('****** Inner Detector Track Reconstruction Cuts ************************************')
    print('*')
    print('* SETUP is  : ',self.__mode)
    print('* extension : ',self.__extension)
    print('*')
    print('* InDetFlags.cutLevel() = ', self.__indetflags.cutLevel())
    print('*')
    print('* Pixel used                  :  ', self.__usePixel)
    print('* SCT used                    :  ', self.__useSCT)
    print('* TRT used                    :  ', self.__useTRT)
    print('*'  )
    print('* min pT                      :  ', self.__minPT, ' MeV')
    print('* max Z IP                    :  ', self.__maxZImpact, ' mm')
    print('* min eta                     :  ', self.__minEta)
    print('* max eta                     :  ', self.__maxEta)
    if self.__mode=="LowPt":
      print('* max PT                      :  ', self.__maxPT, ' MeV')
    print('*')
    print('* NewTracking cuts:')
    print('* -----------------')
    print('* max Rphi IP (primaries)     :  ', self.__maxPrimaryImpact, ' mm')
    print('* max Rphi IP (EM)            :  ', self.__maxEMImpact, ' mm')
    print('* min number of clusters      :  ', self.__minClusters)
    print('* min number of pixel hits    :  ', self.__minPixel)
    print('* min number of NOT shared    :  ', self.__minSiNotShared)
    print('* max number of shared        :  ', self.__maxShared)
    print('* max number of Si holes      :  ', self.__maxHoles)
    print('* max number of Pixel holes   :  ', self.__maxPixelHoles)
    print('* max number of SCT holes     :  ', self.__maxSctHoles)
    print('* max number of double holes  :  ', self.__maxDoubleHoles)
    print('*')
    print('* use seed filter             :  ', self.__useSeedFilter)
    print('* maximal R of SP for seeding :  ', self.__radMax)
    print('* max holes in pattern        :  ', self.__nHolesMax)
    print('* max holes gap in pattern    :  ', self.__nHolesGapMax)
    print('* Xi2 max                     :  ', self.__Xi2max)
    print('* Xi2 max no add              :  ', self.__Xi2maxNoAdd)
    print('* max impact on seeds PPS/SSS :  ', self.__maxdImpactPPSSeeds,', ',self.__maxdImpactSSSSeeds)
    print('* nWeightedClustersMin        :  ', self.__nWeightedClustersMin)
    if self.__useTRT:
      print('* min TRT on track extension  :  ', self.__minTRTonTrk)
      print('* min TRT precision fraction  :  ', self.__minTRTPrecFrac)
    if self.__indetflags.doBremRecovery() and self.__mode == "Offline":
      print('*')
      print("* -> Brem recovery enabled !")
      print('* min pT for brem reocvery    :  ', self.__minPTBrem, ' MeV')
      if self.__indetflags.doCaloSeededBrem():
        print("* -> in Calo seeded mode !!!")
        print('* phi Width of road for brem  :  ', self.__phiWidthBrem)
        print('* eta Width of road for brem  :  ', self.__etaWidthBrem)
    print('*')
    if self.__useTRT and self.__RoISeededBackTracking:
      print('*')
      print('* RoI seeded BackTracking and TRT only !!!')
      print('*')
    if self.__useSCT and self.__useTRT:
      print('* BackTracking cuts:')
      print('* ------------------')
      print('* min pt                      :  ', self.__minSecondaryPt, ' MeV')
      print('* max Rphi IP (secondaries)   :  ', self.__maxSecondaryImpact, ' mm')
      print('* min number of clusters      :  ', self.__minSecondaryClusters)
      print('* min number of NOT shared    :  ', self.__minSecondarySiNotShared)
      print('* max number of shared        :  ', self.__maxSecondaryShared)
      print('* max number of Si holes      :  ', self.__maxSecondaryHoles)
      print('* max number of Pixel holes   :  ', self.__maxSecondaryPixelHoles)
      print('* max number of SCT holes     :  ', self.__maxSecondarySCTHoles)
      print('* max number of double holes  :  ', self.__maxSecondaryDoubleHoles)
      print('* min TRT on track            :  ', self.__minSecondaryTRTonTrk)
      print('* min TRT precision fraction  :  ', self.__minSecondaryTRTPrecFrac)
      print('* max TRT shared fraction     :  ', self.__maxSecondaryTRTShared)
      print('* max holes in pattern        :  ', self.__SecondarynHolesMax)
      print('* max holes gap in pattern    :  ', self.__SecondarynHolesGapMax)
      print('* Xi2 max                     :  ', self.__SecondaryXi2max)
      print('* Xi2 max no add              :  ', self.__SecondaryXi2maxNoAdd)
      print('* TRT segment finder pt bins  :  ', self.__TRTSegFinderPtBins)
      print('* rejectShortExtensions       :  ', self.__rejectShortExtensions)
      print('* SiExtensionsCuts            :  ', self.__SiExtensionCuts)
      if self.__RoISeededBackTracking:
        print('* min CaloCluster Et          :  ', self.__minRoIClusterEt)
      print('*')
    if self.__useTRT:
      print('* useParameterizedTRTCuts     :  ', self.__useParameterizedTRTCuts)
      print('* useNewParameterizationTRT   :  ', self.__useNewParameterizationTRT)
      print('* - TRT Trks Eta Bins                 : ',self.__TrkSel_TRTTrksEtaBins                 )
      print('* - TRT Trks MinTRTHits Thresholds    : ',self.__TrkSel_TRTTrksMinTRTHitsThresholds    )
      print('* - TRT Trks MinTRTHits Mu Dependency : ',self.__TrkSel_TRTTrksMinTRTHitsMuDependencies)
      print('* excludeUsedTRToutliers      :  ', self.__excludeUsedTRToutliers)
      print('*')
      print('* TRT only cuts:')
      print('* --------------')
      print('* min pt                      :  ', self.__minTRTonlyPt, ' MeV')
      print('* min TRT only hits           :  ', self.__minTRTonly)
      print('* max TRT shared fraction     :  ', self.__maxTRTonlyShared)
      print('* useTRTonlyParamCuts         :  ', self.__useTRTonlyParamCuts)
      print('* old transition hit logic    :  ', self.__useTRTonlyOldLogic)
      print('*')
    print('************************************************************************************')


