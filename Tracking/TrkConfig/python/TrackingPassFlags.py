#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import BeamType
from AthenaConfiguration.Enums import LHCPeriod


def select( selInd, valuesmap ):
    for k,v in valuesmap.items():    
        ranges = [int(x) for x in k.split('-') if x != '']
        if len(ranges) == 2:
            if ranges[0] <= selInd and selInd <= ranges[1]: return v
        if len(ranges) == 1 and k.startswith('-'):
            if selInd <= ranges[0]: return v
        if len(ranges) == 1 and k.endswith('-'):
            if ranges[0] <= selInd: return v
    raise RuntimeError("No value can be selected from ranges {} given key {}".format( valuesmap.keys(), selInd ))

def minPT_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-1':   0.1 * Units.GeV,
    '2-13': 0.4 * Units.GeV,
    '14-':  0.5 * Units.GeV } )

def minSecondaryPT_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-1':   0.4 * Units.GeV,
    '2-18': 1.0 * Units.GeV,
    '19-': 3.0 * Units.GeV } )

def minTRTonlyPt_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-1':   0.4 * Units.GeV,
    '2-5': 1.0 * Units.GeV,
    '6-': 2.0 * Units.GeV, } )

def minClusters_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-14':  7,
    '15-':  8 } )

def maxHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-7':  3,
    '8-':  2 } )

def maxPixelHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-7':  2,
    '8-':  1 } )

def maxPrimaryImpact_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-15':  10.0 * Units.mm,
    '16-':  5.0 * Units.mm } )

def maxZImpact_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-8':  320.0 * Units.mm,
    '9-16':  250 * Units.mm,
    '17-':  200.0 * Units.mm } )

def nHolesMax_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-11':  3,
    '12-':  2 } )

def nHolesGapMax_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-11':  3,
    '12-':  2 } )

def Xi2max_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-11':  15.0,
    '12-':  9.0 } )

def Xi2maxNoAdd_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-11':  35.0,
    '12-':  25.0 } )

def seedFilterLevel_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-4':  1,
    '5-':  2 } )

def maxdImpactPPSSeeds_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-3':  1.7,
    '4-':  2.0 } )

def maxdImpactSSSSeeds_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-3':  1000.0,
    '4-16':  20.0,
    '17-': 5.0 * Units.mm } )

def doZBoundary_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-9':  False,
    '10-':  True } )

def TRTSegFinderPtBins_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-1':  70,
    '2-':  50 } )

def excludeUsedTRToutliers_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-5':  False,
    '6-':  True } )

def useParameterizedTRTCuts_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
                   {'-2':  False,
                    '3-':  True } )

def useNewParameterizationTRT_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-2':  False,
    '3-':  True } )

def minSecondaryTRTonTrk_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  10,
    '7-':  15 } )

def minSecondaryTRTPrecFrac_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  0.0,
    '7-':  0.3 } )

def maxSecondaryHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  2,
    '7-':  1 } )

def maxSecondaryPixelHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  2,
    '7-':  1 } )

def maxSecondarySCTHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  2,
    '7-':  1 } )

def maxSecondaryDoubleHoles_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  1,
    '7-':  0 } )

def rejectShortExtensions_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  False,
    '7-':  True } )

def SiExtensionCuts_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-6':  False,
    '7-':  True } )

def RoISeededBackTracking_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-12':  False,
    '13-':  True } )

def roadWidth_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-16':  20.0,
    '17-':  12.0 } )

def keepAllConfirmedPixelSeeds_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-17':  False,
    '18-':  True } )

def minRoIClusterEt_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-18':  0.0,
    '19-':  6000. * Units.MeV } )

def maxSeedsPerSP_Pixels_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-17':  5,
    '18-':   1 } )

def maxSeedsPerSP_Strips_ranges( inflags ):
    return select( inflags.Tracking.cutLevel,
    {'-17':  5,
    '18-':   5 } )

################################################################
    ## create set of tracking cut flags
################################################################
def createTrackingPassFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("extension", "" ) ### for extension

    icf.addFlag("usePrdAssociationTool", False)
    icf.addFlag("isLowPt", False)
    icf.addFlag("useTIDE_Ambi", lambda pcf: pcf.Tracking.doTIDE_Ambi)
    icf.addFlag("useTRTExtension", lambda pcf: pcf.Tracking.doTRTExtension)
    icf.addFlag("storeSeparateContainer", False)
    icf.addFlag("doAmbiguityProcessorTrackFit", True)

    icf.addFlag("minPT", minPT_ranges )
    icf.addFlag("minSecondaryPt", minSecondaryPT_ranges ) #Pt cut for back tracking + segment finding for these
    icf.addFlag("minTRTonlyPt", minTRTonlyPt_ranges ) #Pt cut for TRT only
    icf.addFlag("pT_SSScut", -1. ) # off

    # --- first set kinematic defaults
    icf.addFlag("maxPT", 1000.0 * Units.TeV) # off!
    icf.addFlag("minEta", -1) # off!
    icf.addFlag("maxEta", 2.7)


    # --- cluster cuts
    icf.addFlag("minClusters", lambda pcf:
                3 if (pcf.Detector.EnablePixel and not pcf.Detector.EnableSCT) else
                6  if (pcf.Detector.EnableSCT and not pcf.Detector.EnablePixel) else
                6 if pcf.Beam.Type is BeamType.Cosmics else
                minClusters_ranges( pcf ) )

    icf.addFlag("minSiNotShared", lambda pcf:
                5 if pcf.Beam.Type is BeamType.Cosmics else 6)
    
    icf.addFlag("maxShared", 1) # cut is now on number of shared modules
    icf.addFlag("minPixel", 0)
    icf.addFlag("maxHoles", maxHoles_ranges )
    icf.addFlag("maxPixelHoles", maxPixelHoles_ranges )
    icf.addFlag("maxSctHoles", 2)
    icf.addFlag("maxDoubleHoles", 1)

    icf.addFlag("maxPrimaryImpact", lambda pcf:
                10.0 * Units.mm if pcf.Tracking.doBLS
                else maxPrimaryImpact_ranges(pcf))
    icf.addFlag("maxEMImpact", 50.0 * Units.mm)
    icf.addFlag("maxZImpact", maxZImpact_ranges )

    # --- this is for the TRT-extension
    icf.addFlag("minTRTonTrk", 9)
    icf.addFlag("minTRTPrecFrac", 0.3)

    # --- general pattern cuts for NewTracking

    icf.addFlag("radMax", 600.0 * Units.mm) # default R cut for SP in SiSpacePointsSeedMaker
    icf.addFlag("roadWidth", roadWidth_ranges )
    icf.addFlag("nHolesMax", nHolesMax_ranges )
    icf.addFlag("nHolesGapMax", nHolesGapMax_ranges ) # not as tight as 2*maxDoubleHoles
    icf.addFlag("Xi2max", Xi2max_ranges )
    icf.addFlag("Xi2maxNoAdd", Xi2maxNoAdd_ranges )
    icf.addFlag("nWeightedClustersMin", 6)

    # --- seeding
    icf.addFlag("seedFilterLevel", seedFilterLevel_ranges )
    icf.addFlag("maxTracksPerSharedPRD", 0)  ## is 0 ok for default??
    icf.addFlag("maxdImpactPPSSeeds", 2)
    icf.addFlag("maxdImpactSSSSeeds", lambda pcf:
                10.0 * Units.mm if pcf.Tracking.doBLS
                else maxdImpactSSSSeeds_ranges(pcf))
    icf.addFlag("maxZSpacePointsPPPSeeds", 2700.0 * Units.mm )
    icf.addFlag("maxZSpacePointsSSSSeeds", 2700.0 * Units.mm )
    icf.addFlag("maxSeedsPerSP_Pixels", maxSeedsPerSP_Pixels_ranges )
    icf.addFlag("maxSeedsPerSP_Strips", maxSeedsPerSP_Strips_ranges )
    icf.addFlag("keepAllConfirmedPixelSeeds", keepAllConfirmedPixelSeeds_ranges )
    icf.addFlag("keepAllConfirmedStripSeeds", False)

    # --- min pt cut for brem
    icf.addFlag("minPTBrem", 1. * Units.GeV) # off
    icf.addFlag("phiWidthBrem", 0.3 ) # default is 0.3
    icf.addFlag("etaWidthBrem", 0.2 ) # default is 0.3

    # --- Z Boundary Seeding
    icf.addFlag("doZBoundary", doZBoundary_ranges)
    
    # --------------------------------------
    # --- BACK TRACKING cuts
    # --------------------------------------

    # --- settings for segment finder
    icf.addFlag("TRTSegFinderPtBins", TRTSegFinderPtBins_ranges)
    icf.addFlag("maxSegTRTShared", 0.7)
    icf.addFlag("excludeUsedTRToutliers", excludeUsedTRToutliers_ranges)

    # --- triggers SegmentFinder and BackTracking
    icf.addFlag("useParameterizedTRTCuts", useParameterizedTRTCuts_ranges )
    icf.addFlag("useNewParameterizationTRT", useNewParameterizationTRT_ranges )
    icf.addFlag("maxSecondaryTRTShared", 0.7)

    # --- defaults for secondary tracking
    icf.addFlag("maxSecondaryImpact", 100.0 * Units.mm) # low lumi
    
    icf.addFlag("minSecondaryClusters"      , 4)
    icf.addFlag("minSecondarySiNotShared"   , 4)
    icf.addFlag("maxSecondaryShared"        , 1)  # cut is now on number of shared modules
    icf.addFlag("minSecondaryTRTonTrk"      , minSecondaryTRTonTrk_ranges)
    icf.addFlag("minSecondaryTRTPrecFrac"   , minSecondaryTRTPrecFrac_ranges)
    
    icf.addFlag("maxSecondaryHoles"         , maxSecondaryHoles_ranges)
    icf.addFlag("maxSecondaryPixelHoles"    , maxSecondaryPixelHoles_ranges)
    icf.addFlag("maxSecondarySCTHoles"      , maxSecondarySCTHoles_ranges)
    icf.addFlag("maxSecondaryDoubleHoles"   , maxSecondaryDoubleHoles_ranges)
    icf.addFlag("SecondarynHolesMax"        , 2 )
    icf.addFlag("SecondarynHolesGapMax"     , 2 )

    icf.addFlag("rejectShortExtensions"     , lambda pcf:
                False if pcf.Beam.Type is BeamType.Cosmics else
                rejectShortExtensions_ranges( pcf ) ) # extension finder in back tracking
                    
    icf.addFlag("SiExtensionCuts"           , SiExtensionCuts_ranges) # cut in Si Extensions before fit

    # --- pattern cuts for back tracking
    icf.addFlag("SecondaryXi2max"           , 15.0)
    icf.addFlag("SecondaryXi2maxNoAdd"      , 50.0)

    # --- run back tracking and TRT only in RoI seed regions
    icf.addFlag("RoISeededBackTracking"     , RoISeededBackTracking_ranges and ( lambda pcf : pcf.Detector.EnableCalo ) )
    icf.addFlag("minRoIClusterEt"           , minRoIClusterEt_ranges)

    icf.addFlag("usePixel"       		  , lambda pcf : pcf.Detector.EnablePixel )
    icf.addFlag("useTRT"        		  , lambda pcf : pcf.Detector.EnableTRT )
    icf.addFlag("useSCT"        		  , lambda pcf : pcf.Detector.EnableSCT )
    icf.addFlag("useSCTSeeding"        	  	  , True )

    # --------------------------------------
    # --- TRT Only TRACKING cuts
    # --------------------------------------
    
    # --- TRT only
    icf.addFlag("minTRTonly"                , 15)
    icf.addFlag("maxTRTonlyShared"          , 0.7)
    icf.addFlag("useTRTonlyParamCuts"       , True)
    icf.addFlag("useTRTonlyOldLogic"        , False)
    icf.addFlag("TrkSel.TRTTrksEtaBins"                 , [999, 999, 999, 999, 999, 999, 999, 999, 999, 999])  # eta bins (10) for eta-dep cuts on TRT conversion tracks
    icf.addFlag("TrkSel.TRTTrksMinTRTHitsThresholds"    , [  0,   0,   0,   0,   0,   0,   0,   0,   0,   0])  # eta-dep nTRT for TRT conversion tracks (> 15 is applied elsewhere)
    icf.addFlag("TrkSel.TRTTrksMinTRTHitsMuDependencies", [  0,   0,   0,   0,   0,   0,   0,   0,   0,   0])  # eta-dep nTRT, mu dependence for TRT conversion tracks

    # --- Pixel and TRT particle ID during particle creation
    icf.addFlag("RunPixelPID", True)
    icf.addFlag("RunTRTPID", True)
    return icf

### RobustReco mode ####################
def createRobustRecoTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = ""

    icf.minClusters             = 7
    icf.maxHoles                = 5
    icf.maxPixelHoles           = 2
    icf.maxSctHoles             = 5
    icf.maxDoubleHoles          = 4

    icf.maxZImpact              = 500*Units.mm

    icf.maxSecondaryHoles       = 5
    icf.maxSecondaryPixelHoles  = 5
    icf.maxSecondarySCTHoles    = 5
    icf.maxSecondaryDoubleHoles = 2

    return icf

### ITk mode ####################
def createITkTrackingPassFlags():

    # Set ITk flags from scratch to avoid relying on InDet flags through lambda functions
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("extension", "" ) ### for extension

    icf.addFlag("useITkPixel"       		  , lambda pcf : pcf.Detector.EnableITkPixel )
    icf.addFlag("useITkStrip"        		  , lambda pcf : pcf.Detector.EnableITkStrip )
    icf.addFlag("useITkPixelSeeding"        	  , True )
    icf.addFlag("useITkStripSeeding"        	  , True )

    icf.addFlag("usePrdAssociationTool"     , False)
    icf.addFlag("storeSeparateContainer"    , False)
    icf.addFlag("doZBoundary"               , True)
    icf.addFlag("doAmbiguityProcessorTrackFit", True)

    icf.addFlag("useEtaDepCuts"             , True)
    # Maximum bin set to 9999 instead of four to prevent out of bounds lookups
    icf.addFlag("etaBins"                   , [-1.0, 2.0, 2.6, 9999.0])
    icf.addFlag("maxEta"                    , 4.0)
    icf.addFlag("minPT"                     , lambda pcf :
                [0.2 * Units.GeV] if pcf.Tracking.doLowMu
                else [0.9 * Units.GeV, 0.4 * Units.GeV, 0.4 * Units.GeV])

    icf.addFlag("minPTSeed"                 , lambda pcf :
                0.2 * Units.GeV if pcf.Tracking.doLowMu
                else 0.9 * Units.GeV)
    icf.addFlag("maxPrimaryImpactSeed"      , 2.0 * Units.mm)
    icf.addFlag("maxZImpactSeed"            , 200.0 * Units.mm)
    icf.addFlag("seedFilterLevel"           , 2)

    # --- cluster cuts
    icf.addFlag("minClusters"             , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [9, 8, 7])
    icf.addFlag("minSiNotShared"          , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [7, 6, 5])
    icf.addFlag("maxShared"               , [2])
    icf.addFlag("minPixel"                , [1])
    icf.addFlag("maxHoles"                , [2])
    icf.addFlag("maxPixelHoles"           , [2])
    icf.addFlag("maxSctHoles"             , [2])
    icf.addFlag("maxDoubleHoles"          , [1])
    icf.addFlag("maxPrimaryImpact"        , [2.0 * Units.mm, 2.0 * Units.mm, 10.0 * Units.mm])
    icf.addFlag("maxZImpact"              , [200.0 * Units.mm])

    # --- general pattern cuts for NewTracking
    icf.addFlag("roadWidth"               , 20.)
    icf.addFlag("nHolesMax"               , icf.maxHoles)
    icf.addFlag("nHolesGapMax"            , icf.maxHoles)

    icf.addFlag("Xi2max"                  , [9.0])
    icf.addFlag("Xi2maxNoAdd"             , [25.0])
    icf.addFlag("nWeightedClustersMin"    , [6])

    # --- seeding
    icf.addFlag("maxdImpactSSSSeeds"      , [20.0 * Units.mm])
    icf.addFlag("radMax"                  , 1100. * Units.mm)

    # --- min pt cut for brem
    icf.addFlag("minPTBrem"               , [1000.0 * Units.mm])
    icf.addFlag("phiWidthBrem"            , [0.3])
    icf.addFlag("etaWidthBrem"            , [0.2])

    return icf


def createITkFastTrackingPassFlags():

    icf = createITkTrackingPassFlags()

    icf.minPT                 = [1.0 * Units.GeV, 0.4 * Units.GeV, 0.4 * Units.GeV]
    icf.maxZImpact            = [150.0 * Units.mm]
    icf.minPixel              = [3]
    icf.nHolesMax             = [1]
    icf.nHolesGapMax          = [1]
    icf.minPTSeed             = 1.0 * Units.GeV
    icf.maxZImpactSeed        = 150.0 * Units.mm
    icf.useITkStripSeeding    = False

    return icf

### ITk with FTF standalone mode ####
def createITkFTFPassFlags():

    icf = createITkFastTrackingPassFlags()
    
    icf.minPT                 = [0.9 * Units.GeV, 0.4 * Units.GeV, 0.4 * Units.GeV]
    icf.minPTSeed             = 0.9 * Units.GeV

    return icf


### ITk LRT mode ####################
def createITkLargeD0TrackingPassFlags():

    icf = createITkTrackingPassFlags()
    icf.extension             = "LargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container

    icf.useEtaDepCuts      = True
    icf.minPT              = [1000 * Units.MeV]
    icf.maxEta             = 4.0
    icf.etaBins            = [-1.0, 4.0]
    icf.maxPrimaryImpact   = [300 * Units.mm]
    icf.maxZImpact         = [500 * Units.mm]
    icf.minClusters        = [8]
    icf.minSiNotShared     = [6]
    icf.maxShared          = [2]
    icf.minPixel           = [0]
    icf.maxHoles           = [1]
    icf.maxPixelHoles      = [1]
    icf.maxSctHoles        = [1]
    icf.maxDoubleHoles     = [0]

    icf.maxZImpactSeed     = 500.0 * Units.mm
    icf.maxPrimaryImpactSeed = 300.0 * Units.mm
    icf.minPTSeed          = 1000 * Units.MeV
    icf.addFlag("maxZSpacePointsPPPSeeds" , 500 * Units.mm)
    icf.addFlag("maxZSpacePointsSSSSeeds" , 2700 * Units.mm) # Off

    icf.radMax             = 1100. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles
    icf.seedFilterLevel    = 1
    icf.roadWidth          = 5

    # --- seeding
    icf.maxdImpactSSSSeeds       = [300.0 * Units.mm]

    # --- min pt cut for brem
    icf.minPTBrem                = [1000.0 * Units.mm]
    icf.phiWidthBrem             = [0.3]
    icf.etaWidthBrem             = [0.2]

    icf.Xi2max                  = [9.0]
    icf.Xi2maxNoAdd             = [25.0]
    icf.nWeightedClustersMin    = [6]

    return icf

def createITkLargeD0FastTrackingPassFlags():

    icf = createITkLargeD0TrackingPassFlags()

    icf.useITkPixelSeeding = False
    icf.useITkStripSeeding = True

    icf.maxEta             = 2.4
    icf.etaBins            = [-1.0, 2.4]
    icf.minPT              = [5.0 * Units.GeV]
    icf.minPTSeed          = 5.0 * Units.GeV
    icf.nWeightedClustersMin = [8]
    icf.maxPrimaryImpact   = [150 * Units.mm]
    icf.maxPrimaryImpactSeed = 150. * Units.mm
    icf.maxdImpactSSSSeeds = [150.0 * Units.mm]
    icf.maxZImpact         = [200 * Units.mm]
    icf.maxZImpactSeed     = 200. * Units.mm
    icf.radMax             = 400. * Units.mm

    return icf

### ITk LowPt mode ####################
def createITkLowPtTrackingPassFlags():

    icf = createITkTrackingPassFlags()
    icf.extension          = "LowPt"
    icf.minPT              = [0.4 * Units.GeV]
    icf.minPTSeed          = 0.4 * Units.GeV

    return icf

### HighPileUP mode ####################
def createHighPileupTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "HighPileup"
    icf.seedFilterLevel         = 1
    icf.minPT                   = 0.900 * Units.GeV
    icf.minClusters             = 9
    icf.maxPixelHoles           = 0

    return icf

## MinBias mode ########################
def createMinBiasTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension                 = "MinBias"
    icf.minPT = lambda pcf: (0.3 if  pcf.Tracking.doHIP300 else 0.1) * Units.GeV

    icf.minClusters               = 5
    icf.minSecondaryPt            = 0.4 * Units.GeV  # Pt cut for back tracking + segment finding for these
    icf.minTRTonlyPt              = 0.4 * Units.GeV  # Pt cut for TRT only
    icf.TRTSegFinderPtBins        = 50
    icf.maxdImpactSSSSeeds        = 20.0    # apply cut on SSS seeds
    icf.excludeUsedTRToutliers    = False   # TRT outliers are added to the exclusion list
    icf.useTRTonlyOldLogic        = True    # turn off ole overlap logic to reduce number of hits
    icf.maxSecondaryImpact        = 100.0 * Units.mm # low lumi

    return icf

## LargeD0 mode ########################
def createLargeD0TrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "LargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container
    icf.maxPT              = 1.0 * Units.TeV
    icf.minPT              = 900 * Units.MeV
    icf.maxEta             = 5
    icf.maxPrimaryImpact   = 300.0 * Units.mm
    icf.maxZImpact         = 1500.0 * Units.mm
    icf.maxSecondaryImpact = 300.0 * Units.mm
    icf.minSecondaryPt     = 500.0 * Units.MeV
    icf.minClusters        = 7
    icf.minSiNotShared     = 5
    icf.maxShared          = 2   # cut is now on number of shared modules
    icf.minPixel           = 0
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 2
    icf.maxDoubleHoles     = 1
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles # not as tight as 2*maxDoubleHoles
    icf.seedFilterLevel    = 1
    icf.maxTracksPerSharedPRD = 2

    icf.RunPixelPID             = False
    icf.RunTRTPID               = False

    return icf


## R3LargeD0 mode ########################
def createR3LargeD0TrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "R3LargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container
    icf.maxPT              = 1.0 * Units.TeV
    icf.minPT              = 1.0 * Units.GeV                                                                                    
    icf.maxEta             = 3                                                                                                        
    icf.maxPrimaryImpact   = 300.0 * Units.mm
    icf.maxEMImpact        = 300 * Units.mm
    icf.maxZImpact         = 500 * Units.mm    
    icf.maxSecondaryImpact = 300.0 * Units.mm  
    icf.minSecondaryPt     = 1000.0 * Units.MeV 
    icf.minClusters        = 8                  
    icf.minSiNotShared     = 6                 
    icf.maxShared          = 2   # cut is now on number of shared modules                                                                                  
    icf.minPixel           = 0
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 1  
    icf.maxDoubleHoles     = 0  
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = 1 
    icf.seedFilterLevel    = 1  
    icf.maxTracksPerSharedPRD   = 2
    icf.Xi2max                  = 9.0  
    icf.Xi2maxNoAdd             = 25.0 
    icf.roadWidth               = 5. 
    icf.nWeightedClustersMin    = 8   
    icf.maxdImpactSSSSeeds      = 300.0
    icf.doZBoundary             = True
    icf.keepAllConfirmedStripSeeds = True
    icf.maxSeedsPerSP_Strips = 1

    icf.RunPixelPID        = False
    icf.RunTRTPID          = False

    return icf

## LowPtLargeD0 mode ########################
def createLowPtLargeD0TrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "LowPtLargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container
    icf.maxPT              = 1.0 * Units.TeV
    icf.minPT              = 100 * Units.MeV
    icf.maxEta             = 5
    icf.maxPrimaryImpact   = 300.0 * Units.mm
    icf.maxZImpact         = 1500.0 * Units.mm
    icf.maxSecondaryImpact = 300.0 * Units.mm
    icf.minSecondaryPt     = 400.0 * Units.MeV
    icf.minClusters        = 5
    icf.minSiNotShared     = 5
    icf.maxShared          = 2   # cut is now on number of shared modules
    icf.minPixel           = 0
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 2
    icf.maxDoubleHoles     = 1
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles
    icf.seedFilterLevel    = 1
    icf.maxTracksPerSharedPRD = 2

    icf.RunPixelPID        = False
    icf.RunTRTPID          = False

    return icf

## LowPt mode ########################
def createLowPtTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "LowPt"
    icf.usePrdAssociationTool = True
    icf.isLowPt          = True
    icf.maxPT = lambda pcf: (1e6 if pcf.Tracking.doMinBias else
                             pcf.Tracking.MainPass.minPT + 0.3) * Units.GeV
    icf.minPT            = 0.050 * Units.GeV
    icf.minClusters      = 5
    icf.minSiNotShared   = 4
    icf.maxShared        = 1   # cut is now on number of shared modules
    icf.minPixel         = 2   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
    icf.maxHoles         = 2
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 2
    icf.maxDoubleHoles   = 1
    icf.radMax           = 600. * Units.mm
    icf.nHolesMax        = icf.maxHoles
    icf.nHolesGapMax     = icf.maxHoles # not as tight as 2*maxDoubleHoles
    icf.maxPrimaryImpact = lambda pcf: 100.0 * Units.mm if pcf.Tracking.doMinBias else maxPrimaryImpact_ranges( pcf )
    
    return icf

## ITkConversionFinding mode ########################
def createITkConversionFindingTrackingPassFlags(): #To be updated
    icf = createITkTrackingPassFlags()
    icf.extension               = "ConversionFinding"
    icf.usePrdAssociationTool   = True

    icf.useEtaDepCuts           = True
    icf.etaBins                 = [-1.0,4.0]
    icf.minPT                   = [0.9 * Units.GeV]
    icf.maxPrimaryImpact        = [10.0 * Units.mm]
    icf.maxZImpact              = [150.0 * Units.mm]
    icf.minClusters             = [6]
    icf.minSiNotShared          = [6]
    icf.maxShared               = [0]
    icf.minPixel                = [0]
    icf.maxHoles                = [0]
    icf.maxPixelHoles           = [1]
    icf.maxSctHoles             = [2]
    icf.maxDoubleHoles          = [1]

    icf.nHolesMax               = icf.maxHoles
    icf.nHolesGapMax            = icf.maxHoles
    icf.nWeightedClustersMin    = [6]
    icf.maxdImpactSSSSeeds      = [20.0 * Units.mm]
    icf.radMax                  = 1000. * Units.mm
    icf.doZBoundary             = False

    icf.Xi2max                  = [9.0]
    icf.Xi2maxNoAdd             = [25.0]
    icf.minPTBrem               = [1000.0 * Units.mm]
    icf.phiWidthBrem            = [0.3]
    icf.etaWidthBrem            = [0.2]

    return icf

## VeryLowPt mode ########################
def createVeryLowPtTrackingPassFlags():
    icf = createTrackingPassFlags() #TODO consider using createLowPtTrackingPassFlags as a base here
    icf.extension        = "VeryLowPt"
    icf.usePrdAssociationTool = True
    icf.isLowPt          = True
    icf.useTRTExtension  = False
    icf.maxPT            = lambda pcf : (1e6 if pcf.Tracking.doMinBias else 
                                         pcf.Tracking.MainPass.minPT + 0.3) * Units.GeV # some overlap
    icf.minPT            = 0.050 * Units.GeV
    icf.minClusters      = 3
    icf.minSiNotShared   = 3
    icf.maxShared        = 1   # cut is now on number of shared modules
    icf.minPixel         = 3   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
    icf.maxHoles         = 1
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 1
    icf.maxDoubleHoles   = 0
    icf.nHolesMax        = 1
    icf.nHolesGapMax     = 1 # not as tight as 2*maxDoubleHoles
    icf.radMax           = 600. * Units.mm # restrivt to pixels

    return icf

## ForwardTracks mode ########################
def createForwardTracksTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Forward"
    icf.usePrdAssociationTool = True
    icf.useTIDE_Ambi     = False
    icf.useTRTExtension  = False
    icf.storeSeparateContainer = True
    icf.minEta           = 2.4 # restrict to minimal eta
    icf.maxEta           = 2.7
    icf.minPT            = 2 * Units.GeV
    icf.minClusters      = 3
    icf.minSiNotShared   = 3
    icf.maxShared        = 1
    icf.minPixel         = 3
    icf.maxHoles         = 1
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 1
    icf.maxDoubleHoles   = 0
    icf.nHolesMax        = icf.maxHoles
    icf.nHolesGapMax     = icf.maxHoles
    icf.radMax           = 600. * Units.mm
    icf.useTRT           = False # no TRT for forward tracks

    icf.RunPixelPID      = False
    icf.RunTRTPID        = False

    return icf

## BeamGas mode ########################
def createBeamGasTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "BeamGas"
    icf.usePrdAssociationTool = True
    icf.minPT            = 0.500 * Units.GeV
    icf.maxPrimaryImpact = 300. * Units.mm
    icf.maxZImpact       = 2000. * Units.mm
    icf.minClusters      = 6
    icf.maxHoles         = 3
    icf.maxPixelHoles    = 3
    icf.maxSctHoles      = 3
    icf.maxDoubleHoles   = 1
    icf.nHolesMax        = 3
    icf.nHolesGapMax     = 3 # not as tight as 2*maxDoubleHoles

    return icf

## VtxLumi mode ########################
def createVtxLumiTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "VtxLumi"
    icf.seedFilterLevel         = 1
    icf.minPT                   = 0.900 * Units.GeV
    icf.minClusters             = 7
    icf.maxPixelHoles           = 1
    icf.radMax                  = 600. * Units.mm
    icf.nHolesMax               = 2
    icf.nHolesGapMax            = 1
    icf.useTRT                  = False

    return icf

## VtxBeamSpot mode ########################
def createVtxBeamSpotTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "VtxBeamSpot"
    icf.seedFilterLevel         = 1
    icf.minPT                   = 0.900 * Units.GeV
    icf.minClusters             = 9
    icf.maxPixelHoles           = 0
    icf.radMax                  = 320. * Units.mm
    icf.nHolesMax               = 2
    icf.nHolesGapMax            = 1
    icf.useTRT                  = False

    return icf

## Cosmics mode ########################
def createCosmicsTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Cosmics"
    icf.minPT            = 0.500 * Units.GeV
    icf.maxPrimaryImpact = 1000. * Units.mm
    icf.maxZImpact       = 10000. * Units.mm
    icf.minClusters      = 4
    icf.minSiNotShared   = 4
    icf.maxHoles         = 3
    icf.maxPixelHoles    = 3
    icf.maxSctHoles      = 3
    icf.maxDoubleHoles   = 1
    icf.minTRTonTrk      = 15
    icf.minTRTonly       = 15
    icf.roadWidth        = 60.
    icf.seedFilterLevel  = 3
    icf.Xi2max           = 60.0
    icf.Xi2maxNoAdd      = 100.0
    icf.nWeightedClustersMin = 8
    icf.nHolesMax        = 3
    icf.nHolesGapMax     = 3 # not as tight as 2*maxDoubleHoles

    return icf

## Heavyion mode #######################
def createHeavyIonTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "HeavyIon"
    icf.maxZImpact       = 200. * Units.mm
    icf.minClusters      = 9
    icf.minSiNotShared   = 7
    icf.maxShared        = 2 # was 1, cut is now on number of shared modules

    icf.nHolesMax        = 0
    icf.nHolesGapMax     = 0
    icf.Xi2max           = 6.
    icf.Xi2maxNoAdd      = 10.

    # CutLevel dependendent flags:
    # CutLevel 3 MinBias
    # CutLevel 4  # ==CutLevel 2 with loosened hole cuts and chi^2 cuts
    # CutLevel 5 # ==CutLevel 3 with loosened hole cuts and chi^2 cuts    
    icf.seedFilterLevel = lambda pcf: 2 if pcf.Tracking.cutLevel >= 2 else 1
    
    icf.maxdImpactSSSSeeds =  lambda pcf: \
                              20. if pcf.Tracking.cutLevel >= 2 else 1000.
    
    icf.minPT              = lambda pcf: \
                             0.3 *Units.GeV  if pcf.Tracking.cutLevel in [3, 5] else 0.5 * Units.GeV
    icf.useParameterizedTRTCuts = lambda pcf: \
                                  False if pcf.Tracking.cutLevel >= 3 else True #Make these false on all HI cut levels >=3, since standard cut levels set it true from levels >=3
    icf.useNewParameterizationTRT = lambda pcf: \
                                    False if pcf.Tracking.cutLevel >= 3 else True

    #set this to 1.7 for all HI cut levels >=4, since standard cut levels set it to 2.0 from levels >=4. Not sure it has any effect, since we don't usually run mixed seeds (also true for HI?)
    icf.maxdImpactPPSSeeds = lambda pcf: \
                             1.7 if pcf.Tracking.cutLevel >= 4 else True
    
    icf.maxHoles = lambda pcf: 2 if pcf.Tracking.cutLevel in [4, 5] else 0
    icf.maxPixelHoles = lambda pcf: 1 if pcf.Tracking.cutLevel in [4, 5] else 0
    icf.maxSctHoles = lambda pcf: 1 if pcf.Tracking.cutLevel in [4, 5] else 0
    icf.maxDoubleHoles   = 0    
    icf.Xi2max           = lambda pcf: 9. if pcf.Tracking.cutLevel in [4, 5] else 6.
    icf.Xi2maxNoAdd      = lambda pcf: 25. if pcf.Tracking.cutLevel in [4, 5] else 10.
    icf.radMax           = 600. * Units.mm # restrict to pixels + first SCT layer
    icf.useTRT           = False

    return icf

### Pixel mode ###############################################
def createPixelTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Pixel"
    icf.isLowPt          = lambda pcf : pcf.Tracking.doMinBias

    def _minPt( pcf ):
        if pcf.Beam.Type is BeamType.Cosmics:
            return 0.5 * Units.GeV
        if pcf.Reco.EnableHI:
            return 0.1 * Units.GeV
        if pcf.Tracking.doMinBias:
            if pcf.Tracking.doHIP300:
                return 0.3 * Units.GeV
            else:
                return 0.05 * Units.GeV
        return 0.1 * Units.GeV
    
    icf.minPT            = _minPt
    icf.minClusters      = 3

    def _pick( default, hion, cosmics):
        def _internal( pcf ):
            if pcf.Reco.EnableHI:
                return hion
            if pcf.Beam.Type is BeamType.Cosmics:
                return cosmics
            return default
        return _internal
    
    icf.maxHoles         = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.maxPixelHoles    = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.maxSctHoles      = 0
    icf.maxDoubleHoles   = 0
    icf.minSiNotShared   = 3
    icf.maxShared        = 0
    icf.seedFilterLevel  = _pick( default = 2, hion = 2, cosmics = 3 )
    icf.nHolesMax        = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.nHolesGapMax     = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.useSCT           = False
    icf.useTRT           = False
    icf.minSecondaryPt   = 3 * Units.GeV
    icf.maxPrimaryImpact = lambda pcf: 1000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics \
                           else 5. * Units.mm
    icf.roadWidth        = lambda pcf: 60.0 if pcf.Beam.Type is BeamType.Cosmics \
                           else 12.0
    icf.maxZImpact       = lambda pcf: 10000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics \
                           else maxZImpact_ranges( pcf )
    icf.Xi2max           = lambda pcf: 60.0  if pcf.Beam.Type is BeamType.Cosmics \
                           else Xi2max_ranges( pcf )
    icf.Xi2maxNoAdd      = lambda pcf: 100.0  if pcf.Beam.Type is BeamType.Cosmics \
                           else Xi2maxNoAdd_ranges( pcf )
    icf.nWeightedClustersMin = 6

    icf.RunPixelPID      = False
    icf.RunTRTPID        = False
    return icf

########## Disappearing mode ######################
def createDisappearingTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Disappearing"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = True
    icf.minPT            = 5 * Units.GeV
    icf.minClusters      = 4
    icf.maxHoles         = 0
    icf.maxPixelHoles    = 0
    icf.maxSctHoles      = 0
    icf.maxDoubleHoles   = 0
    icf.minSiNotShared   = 3
    icf.maxShared        = 0
    icf.seedFilterLevel  = 2
    icf.nHolesMax        = 0
    icf.nHolesGapMax     = 0
    icf.useSCT           = True
    icf.useTRT           = True
    icf.useSCTSeeding    = False
    icf.maxEta           = 2.2

    return icf

########## SCT mode ######################
def createSCTTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "SCT"
    icf.minClusters      = 7
    icf.maxDoubleHoles   = 1
    icf.minSiNotShared   = 5
    icf.usePixel         = False
    icf.useTRT           = False


    def _pick( default, cosmics, minbias, hion):
        def _internal( pcf ):
            if pcf.Beam.Type is BeamType.Cosmics:
                return cosmics
            if pcf.Tracking.doMinBias:
                if pcf.Tracking.doHIP300:
                    return hion
                else:
                    return minbias
            return default
        return _internal

    icf.minPT            = _pick( default = 0.1 * Units.GeV,
                                  minbias=0.1 * Units.GeV,
                                  hion=0.3* Units.GeV,
                                  cosmics = 0.5* Units.GeV )
    icf.maxPrimaryImpact = lambda pcf: 1000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics \
                           else maxPrimaryImpact_ranges( pcf )
    icf.maxZImpact       = lambda pcf: 10000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics \
                           else maxZImpact_ranges( pcf )
    maxHolesDefault = 2
    icf.maxHoles         = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics \
                           else maxHolesDefault
    icf.nHolesMax        = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics \
                           else maxHolesDefault
    icf.nHolesGapMax     = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics \
                           else maxHolesDefault
    icf.maxPixelHoles    = lambda pcf: 0 if pcf.Beam.Type is BeamType.Cosmics \
                           else 0
    icf.maxSctHoles      = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics \
                           else 2
    icf.maxShared        = 0
    icf.roadWidth        = lambda pcf: 60. if pcf.Beam.Type is BeamType.Cosmics \
                           else roadWidth_ranges( pcf )
    icf.seedFilterLevel  = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics \
                           else 2
    icf.Xi2max           = lambda pcf: 60.0 if pcf.Beam.Type is BeamType.Cosmics \
                           else Xi2max_ranges( pcf )
    icf.Xi2maxNoAdd      = lambda pcf: 100.0 if pcf.Beam.Type is BeamType.Cosmics \
                           else Xi2maxNoAdd_ranges( pcf )
    icf.nWeightedClustersMin = lambda pcf: 4 if pcf.Beam.Type is BeamType.Cosmics \
                               else 6
    icf.minClusters      = lambda pcf: 4 if pcf.Beam.Type is BeamType.Cosmics \
                           else minClusters_ranges( pcf )
    icf.minSiNotShared   = lambda pcf: 4 if pcf.Beam.Type is BeamType.Cosmics \
                           else 5
    
    icf.RunPixelPID      = False
    icf.RunTRTPID        = False
    return icf

########## TRT subdetector tracklet cuts  ##########
def createTRTTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "TRT"
    icf.useTIDE_Ambi            = False
    icf.usePrdAssociationTool   = True
    icf.minPT                   = 0.4 * Units.GeV
    icf.minTRTonly              = 15
    icf.maxTRTonlyShared        = 0.7

    icf.RunPixelPID             = False
    icf.RunTRTPID               = False
    return icf


########## TRT standalone tracklet cuts  ##########
def createTRTStandaloneTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension              = "TRTStandalone"
    icf.useTIDE_Ambi           = False
    icf.usePrdAssociationTool  = True

    icf.minSecondaryTRTPrecFrac = 0.15
    # Mu- and eta- dependent cuts on nTRT
    icf.TrkSel.TRTTrksEtaBins                  = [ 0.7,   0.8,   0.9,  1.2,  1.3,  1.6,  1.7,  1.8,  1.9,  999]  # eta bins (10) for eta-dep cuts on TRT conversion tracks
    icf.TrkSel.TRTTrksMinTRTHitsThresholds     = lambda pcf: [  25,    18,    18,   18,   26,   28,   26,   24,   22,    0] if pcf.GeoModel.Run is LHCPeriod.Run3 else \
                                                 [  27,    18,    18,   18,   26,   28,   26,   24,   22,    0]  # eta-dep nTRT for TRT conversion tracks (> 15 is applied elsewhere)
    icf.TrkSel.TRTTrksMinTRTHitsMuDependencies = [ 0.2,  0.05,  0.05, 0.05, 0.15, 0.15, 0.15, 0.15, 0.15,    0]  # eta-dep nTRT, mu dependence for TRT conversion tracks

    return icf

#####################################################################
#####################################################################
#####################################################################

if __name__ == "__main__":

  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  from AthenaConfiguration.TestDefaults import defaultTestFiles
  flags.Input.Files=defaultTestFiles.RAW_RUN2
  
  from AthenaCommon.Logging import logging
  l = logging.getLogger('TrackingPassFlags')
  from AthenaCommon.Constants import INFO
  l.setLevel(INFO)

  flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")

  assert flags.Tracking.cutLevel == 19 , "default cut level is wrong"
  assert flags.Tracking.ActiveConfig.minRoIClusterEt == 6000.0 * Units.MeV, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.minRoIClusterEt)
  flags.Tracking.cutLevel = 2
  assert flags.Tracking.ActiveConfig.minRoIClusterEt == 0.0, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.minRoIClusterEt)
  assert flags.Tracking.BeamGasPass.minRoIClusterEt == 0.0, "wrong cut value {}, not following cutLevel setting ".format(flags.Tracking.BeamGasPass.minRoIClusterEt)

  assert flags.Tracking.HeavyIonPass.minSiNotShared == 7, "wrong cut value, overwrite"
  assert flags.Tracking.HeavyIonPass.minRoIClusterEt == 0.0, "wrong cut value, overwrite"

  l.info("flags.Tracking.ActiveConfig.minSecondaryPt %f", flags.Tracking.ActiveConfig.minSecondaryPt * 1.0)
  l.info("type(flags.Tracking.ActiveConfig.minSecondaryPt) " + str(type(flags.Tracking.ActiveConfig.minSecondaryPt)))




