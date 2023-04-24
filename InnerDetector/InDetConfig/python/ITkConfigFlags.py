# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType, FlagEnum

class TrackingComponent(FlagEnum):
    AthenaChain = "AthenaChain"  # full Athena Chain (default)
    ActsChain = "ActsChain"  # full Acts Chain 
    # Validation options
    ValidateActsClusters = "ValidateActsClusters"
    ValidateActsSpacePoints = "ValidateActsSpacePoints" 
    ValidateActsSeeds = "ValidateActsSeeds"
    ValidateActsTracks = "ValidateActsTracks"

# TODO : Add some exta levels?

def createITkConfigFlags():
  itkcf = AthConfigFlags()

  # take geometry XML files from local instance rather than Detector Database, for development
  itkcf.addFlag("ITk.Geometry.AllLocal", False)
  itkcf.addFlag("ITk.Geometry.PixelLocal", lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
  itkcf.addFlag("ITk.Geometry.PixelFilename", "ITKLayouts/Pixel/ITkPixel.gmx")
  itkcf.addFlag("ITk.Geometry.PixelClobOutputName", "")
  itkcf.addFlag("ITk.Geometry.StripLocal", lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
  itkcf.addFlag("ITk.Geometry.StripFilename", "ITKLayouts/Strip/ITkStrip.gmx")
  itkcf.addFlag("ITk.Geometry.StripClobOutputName", "")
  itkcf.addFlag("ITk.Geometry.BCMPrimeLocal", lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
  itkcf.addFlag("ITk.Geometry.BCMPrimeFilename", "ITKLayouts/Pixel/BCMPrime.gmx")
  itkcf.addFlag("ITk.Geometry.BCMPrimeClobOutputName", "")
  itkcf.addFlag("ITk.Geometry.PLRLocal", lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
  itkcf.addFlag("ITk.Geometry.PLRFilename", "ITKLayouts/PLR/PLR.gmx")
  itkcf.addFlag("ITk.Geometry.PLRClobOutputName", "")
  itkcf.addFlag("ITk.Geometry.DictionaryLocal", lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
  itkcf.addFlag("ITk.Geometry.DictionaryFilename", "ITKLayouts/IdDictInnerDetector_ITK_LOCAL.xml")
  itkcf.addFlag("ITk.Geometry.isLocal", lambda prevFlags : (prevFlags.ITk.Geometry.PixelLocal
                                                         or prevFlags.ITk.Geometry.StripLocal
                                                         or prevFlags.ITk.Geometry.BCMPrimeLocal
                                                         or prevFlags.ITk.Geometry.PLRLocal))

  itkcf.addFlag("ITk.Conditions.PixelChargeCalibTag", "ChargeCalib-MC21-01")
  itkcf.addFlag("ITk.Conditions.PixelChargeCalibFile", "")
  itkcf.addFlag("ITk.Conditions.PixelOfflineCalibTag", "PixelITkError_v5")
  itkcf.addFlag("ITk.Conditions.PixelOfflineCalibFile", "")

  # Turn on running of PRD MultiTruthMaker
  itkcf.addFlag("ITk.doTruth", lambda prevFlags: prevFlags.Input.isMC)

  itkcf.addFlag("ITk.doStripModuleVeto", False) # Turn on SCT_ModuleVetoSvc, allowing it to be configured later
  itkcf.addFlag("ITk.checkDeadPixelsOnTrack", True) # Enable check for dead modules and FEs
  itkcf.addFlag("ITk.selectStripIntimeHits", lambda prevFlags: not(prevFlags.Beam.Type is BeamType.Cosmics) ) # defines if the X1X mode is used for the offline or not

  itkcf.addFlag("ITk.Tracking.doDigitalClustering", False)
  itkcf.addFlag("ITk.Tracking.doFastTracking", False) # Turn running of ITk FastTracking on and off
  itkcf.addFlag("ITk.Tracking.doConversionFinding",True) # Turn running of ConversionFinding second pass on and off
  itkcf.addFlag("ITk.Tracking.useFTF", False) # Allows TrigFastTrackFinder to be run as an offline algorithm by replacing SiSPSeededTrackFinder
  
  # config flags for tracking geometry configuration
  from InDetConfig.TrackingGeometryFlags import createITkTrackingGeometryFlags
  itkcf.addFlagsCategory ("ITk.trackingGeometry", createITkTrackingGeometryFlags, prefix=True)

  # enable reco steps 
  itkcf.addFlag("ITk.Tracking.recoChain", [TrackingComponent.AthenaChain])

  return itkcf
