# Overview
The modules in this directory provide signature-specific configurations of ID algorithms to be included in trigger chains.
Inner Detector tracking consists of the algorithms, and their tools, that perform data preparation (bytestream conversions, clustering and space-point formation), Fast Tracking (seed-making, track-following and track fitting), Precision Tracking (ambiguity solving, track extension, track fitting) and primary vertex reconstruction. In most trigger chains, Precision Tracking starts from the tracks found by Fast Tracking. For specific cases, Precision Tracking can be configured to run EFIDTracking that starts from Raw Data and uses the SiSPSeededTrackFinder (rather than Fast Tracking) to find tracks.

In addition to the modules supporting the current jobOptions configuration, additional modules, [TrigInDetConfig](TrigInDetConfig.py) and [TrigTrackingCutFlags](TrigTrackingCutFlags.py), provide a parallel configuration within the ComponentAccumulator infrastucture.

# Configuration
* [ConfigSettingsBase](ConfigSettingsBase.py)
  * Base class for ConfigSettings   
* [ConfigSettings](ConfigSettings.py)
  * sets the signature-specific configuration settings including collection names, roi dimensions and algorithm properties
* [InDetTrigCollectionKeys](InDetTrigCollectionKeys.py) 
  * Defines the names of collections used internally within the Inner detector Trigger SW. It does not include the output track collection names that are set in [ConfigSettings](ConfigSettings.py).	
# Data Preparation and Fast Tracking
* [InDetTrigFastTracking](InDetTrigFastTracking.py)
  * Creates Data Preparation and Fast Tracking algorithms, configured based on the supplied config and returns the list of algorithms to be added to signature trigger chains
# Precision Tracking
* [InDetTrigPrecisionTracking](InDetTrigPrecisionTracking.py)       
  *  Creates Precision tracking algorithms that processes and refine tracks created by Fast Tracking. The algorithms are configured based on the supplied config. A list is returned of ID algorithms to be added to signature trigger chains
* [EFIDTracking](EFIDTracking.py)
  * Creates a sequence of Precision Tracking algorithms that starts from raw data using the SiSPSeededTrackFinder (rather than Fast Tracking) to find tracks.
* [InDetTrigCommon](InDetTrigCommonInDetTrigCommon.py)
  * Configures Precision Tracking algorithms and tools common to both InDetTrigPrecisionTracking and EFIDTracking 
# Vertex Reconstruction
* [InDetPriVxFinderConfig](InnerDetector/IndetConfig/python/InDetPriVxFinderConfig.py)
  * Configures the primary vertex finding algorithms, both for offline and online
# Component Accumulator
* [TrigInDetConfig](TrigInDetConfig.py)
  * ComponentAccumulator configuration of ID trigger algorithms and tools  
* [TrigTrackingCutFlags](TrigTrackingCutFlags.py)
  * Flags used by the ComponentAccumulator based configuration.  


# How to configure the tracking from the TriggerMenuMT signature code


Generally components of the ID configuration for use by the signatures should 
come from 

[Trigger/TrigTools/TrigInDetConfig](https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TrigTools/TrigInDetConfig)

Generally, there are four things that are needed:
* Tracking configuration settings - track collection names, Roi sizes etc 
* Fast tracking configuration
* Precision tracking
* Vertexing

Generally the tracking configuration parameters should really live entirely 
inside the tracking configuration

The names of Track collections however, should be set centrally and distributed
throughout the trigger configuration, rather than duplicated in different places.

Currently Roi Descriptor names are not obtained from the Tracking Configuration, 
but they should be added to the parameters configured •

# Tracking configuration

For a signature instance, eg “muon”, "tauIso" etc we fetch the tracking configuration instance for 
that signature, and obtain parameters, tack collection names etc, from the ConfigSettings 
class for that instance: 

      from TrigInDetConfig.ConfigSetings importa getInDetTrigConfig
      config = getInDetConfig( "tauIso" )
     
      algo.etaHalfWidth = config.etaHalfWidth
      algo.phiHalfWidth = config.phiHalfWidth
      
      tracks_ftf    = config.tracks_FTF()
      tracks_idtrig = config.tracks_IDTrig()

Under NO CIRCUMSTANCES should a track collection name or a vertex collection name EVER be hardcoded 
as a string string literal into the signature configuration code

For instance

      TrackParticleContainerName = "HLT_IDTrack_Electron_IDTrig"

should instead be obtained from the config

      TrackParticleContainerName = config.tracks_IDTrig()


# Fast Tracking and Data preparation

The Fast Track Finder configuration code can be found in 

[Trigger/TrigTools/TrigInDetConfig/python/InDetTrigFastTracking.py](https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TrigTools/TrigInDetConfig/python/InDetTrigFastTracking.py)

An example on how to call this 

   
      from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
      idconfig = getInDetTrigConfig( signatureNameID )

      from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking

      viewAlgs, viewVerify = makeInDetTrigFastTracking( flags, config = idconfig, rois = RoIs )

      TrackCollection = idconfig.tracks_FTF()


The actual interface is 
<pre>
  def makeInDetTrigFastTracking( flags, config = None, rois = 'EMViewRoIs', doFTF = True, viewVerifier='IDViewDataVerifier'):
</pre>
where the arguments are    
  
      config            - the ID Trigger configuration for the signature
      rois              - the view RoI name
      doFTF             - whether to run the actual FTF, or just the data preparation
      viewVerifier      - the view data verifier
					     

# Precision  Tracking
  
The code for this is in 

[Trigger/TrigTools/TrigInDetConfig/python/InDetTrigPrecisionTracking.py](https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TrigTools/TrigInDetConfig/python/InDetTrigFastTracking.py)

and should be configured using 
   
      from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
      idconfig = getInDetTrigConfig( signatureNameID )

      from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking

      viewAlgs, viewVerify = makeInDetTrigPrecisionTracking( flags, config = idconfig, rois = RoIs )

      TrackCollection = idconfig.tracks_IDTrig()


The actual interface is 
<pre>
  def makeInDetTrigPrecisionTracking( flags, config = None, verifier = False, rois = 'EMViewRoIs', prefix = 'InDetTrigMT'):
</pre>
where the arguments are    

       flags     - ConfigFlags instance
       config    - the ID Trigger configuration for the signature
       verifier  - the view data verifier
       rois      - the view RoI name 
       prefix    - Prefix for the algorithm name. This is needed to differentiate from the
		       Offline versions of th algorithm
			          

## Minbias Tracking 
 
This uses the code in 

[Trigger/TrigTools/TrigInDetConfig/EFIDTracking.py](https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TrigTools/TrigInDetConfig/python/InDetTrigFastTracking.py)

but the actual called function should be the same as for the precision tracking. *** need to check this ***
  

# Vertexing

The vertexing is configured using the code in 

[InnerDetector/IndetConfig/python/InDetPriVxFinderConfig.py](InnerDetector/IndetConfig/python/InDetPriVxFinderConfig.py)

Typically it should be used as in the following example 

    from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg

    acc = InDetTrigPriVxFinderCfg(
        flags,
        signature = "jet",
        TracksName = jetContext["Tracks"],
        VxCandidatesOutputName = jetContext["Vertices"])


The actual function is defined 
```
def InDetTrigPriVxFinderCfg(flags, name="InDetTrigPriVxFinder",
                            signature="",
                            **kwargs):
```
where the arguments are    

       signature              - the ID Trigger signature
       TracksName             - the input track collection key
       VxCandidatesOutputName - optional output vertex collection key

Last updated 2023 - 01 - 12





