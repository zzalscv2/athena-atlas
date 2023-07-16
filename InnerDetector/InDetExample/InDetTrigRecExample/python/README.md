# Overview of Modules
## Flags and cuts:
* [InDetTrigFlags](InDetTrigFlags.py)
  * Trigger-specific version of the offline InDetFlags extended with the addition of InDetTrigJobProperties 
* [InDetTrigJobProperties](InDetTrigJobProperties.py)
  * ID trigger-specific job properties 
* [ConfiguredNewTracking](TrigCutsConfiguredNewTrackingTrigCuts.py)
  * Configured sets of tracking cuts for the modes: Offline, Cosmics, BeamGas, LowPt, TRT, HeavyIon, LRT   
* [InDetTrigTrackingCuts](InDetTrigTrackingCuts.py)
  * Cuts applied in tracking algorithms and tools for each mode. Also depends on the cutLevel set in InDetTrigFlags.  
## Creation & configuration of tools used by ID tracking: 
* [InDetTrigConfigRecLoadTools](InDetTrigConfigRecLoadTools.py)
  * Creates and configures tracking tools
* [InDetTrigConfigRecLoadToolsPost](InDetTrigConfigRecLoadToolsPost.py)     
  * Creates and configures postprocessing tools (particle creation, vertexing)
* [InDetTrigConfigRecLoadToolsBack](BackInDetTrigConfigRecLoadToolsBack.py)
  * **removed** configures tools for TRT backtracking
* [InDetTrigConfigRecLoadToolsLowPt](InDetTrigConfigRecLoadToolsLowPt.py)
  * configures lowPt versions of tracking tools
* [InDetTrigConfigRecLoadToolsBeamGas](InDetTrigConfigRecLoadToolsBeamGas.py)  
  * **removed** configures BeamGas versions of tracking tools
## Create & configure ID Conditions
* [InDetTrigConditionsAccess](InDetTrigConditionsAccess.py)      
  * uses InDetTrigConfigConditions to configure & create conditions tools, services & algorithms
* [InDetTrigConfigConditions](InDetTrigConfigConditions.py)      
  * configures & creates conditions tools, services & algorithms

