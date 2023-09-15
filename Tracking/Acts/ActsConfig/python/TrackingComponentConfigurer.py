# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrkConfig.TrkConfigFlags import TrackingComponent
 
class TrackingComponentConfigurer:
    __configuration_setters = {
        TrackingComponent.AthenaChain : "AthenaChain",
        TrackingComponent.ActsChain : "ActsChain",
        TrackingComponent.ValidateActsClusters : "ValidateActsClusters",
        TrackingComponent.ValidateActsSpacePoints : "ValidateActsSpacePoints",
        TrackingComponent.ValidateActsSeeds : "ValidateActsSeeds",
        TrackingComponent.ValidateActsTracks : "ValidateActsTracks",
        TrackingComponent.BenchmarkSpot : "BenchmarkSpot"
    }
    
    def __init__(self,
                 flags):
        configuration_requests = flags.Tracking.recoChain
        assert isinstance(configuration_requests, list)
    
        # Athena Algorithms
        self.AthenaCluster = False
        self.AthenaSpacePoint = False
        self.AthenaSeed = False
        self.AthenaTrack = False
        self.AthenaAmbiguityResolution = False

        # Acts Algorithms
        self.ActsCluster = False
        self.ActsSpacePoint = False
        self.ActsSeed = False
        self.ActsTrack = False
        self.ActsAmbiguityResolution = False

        # Athena -> Acts EDM Converters
        self.AthenaToActsClusterConverter = False
        self.AthenaToActsSpacePointConverter = False
        
        # Acts -> Athena EDM Converters
        self.ActsToAthenaClusterConverter = False
        self.ActsToAthenaSpacePointConverter = False
        self.ActsToAthenaSeedConverter = False
        self.ActsToAthenaTrackConverter = False
        self.ActsToAthenaResolvedTrackConverter = False

        # Enable the requested configurations
        for req in configuration_requests:
            eval(f"self.{self.__configuration_setters[req]}(flags)")

        # To-Do
        # Possibly check the resulting configuration and print useful statements
        # on possible mis-configurations
        # The scuduler will complain about conflicts, but we may want to do this check
        # already here

    def __str__(self) -> str:
        message = "Configuration of tracking components:\n"
        message += " - Athena Algorithms:\n"
        message += f"    * Clustering: {'ENABLED' if self.doAthenaCluster else 'DISABLED'}\n"
        message += f"    * Space Point Formation: {'ENABLED' if self.doAthenaSpacePoint else 'DISABLED'}\n"
        message += f"    * Seeding: {'ENABLED' if self.doAthenaSeed else 'DISABLED'}\n"
        message += f"    * Track Finding: {'ENABLED' if self.doAthenaTrack else 'DISABLED'}\n"
        message += f"    * Ambiguity Resolution: {'ENABLED' if self.doAthenaAmbiguityResolution else 'DISABLED'}\n"
        message += " - Acts Algorithms:\n"
        message += f"    * Clustering: {'ENABLED' if self.doActsCluster else 'DISABLED'}\n"
        message += f"    * Space Point Formation: {'ENABLED' if self.doActsSpacePoint else 'DISABLED'}\n"
        message += f"    * Seeding: {'ENABLED' if self.doActsSeed else 'DISABLED'}\n"
        message += f"    * Track Finding: {'ENABLED' if self.doActsTrack else 'DISABLED'}\n"
        message += f"    * Ambiguity Resolution: {'ENABLED' if self.doActsAmbiguityResolution else 'DISABLED'}\n"
        message += " - Athena -> Acts EDM Convertions:\n"
        message += f"    * Cluster: {'ENABLED' if self.doAthenaToActsCluster else 'DISABLED'}\n"
        message += f"    * Space Point: {'ENABLED' if self.doAthenaToActsSpacePoint else 'DISABLED'}\n"
        message += " - Acts -> Athena EDM Convertions:\n"
        message += f"    * Cluster: {'ENABLED' if self.doActsToAthenaCluster else 'DISABLED'}\n"
        message += f"    * Space Point: {'ENABLED' if self.doActsToAthenaSpacePoint else 'DISABLED'}\n"
        message += f"    * Seed: {'ENABLED' if self.doActsToAthenaSeed else 'DISABLED'}\n"
        message += f"    * Tracks: {'ENABLED' if self.doActsToAthenaTrack else 'DISABLED'}\n"
        message += f"    * Resolved Tracks: {'ENABLED' if self.doActsToAthenaResolvedTrack else 'DISABLED'}"
        return message
        
    @property
    def doAthenaCluster(self) -> bool:
        return self.AthenaCluster

    @property
    def doAthenaSpacePoint(self) -> bool:
        return self.AthenaSpacePoint

    @property
    def doAthenaSeed(self) -> bool:
        return self.AthenaSeed

    @property
    def doAthenaTrack(self) -> bool:
        return self.AthenaTrack

    @property
    def doActsCluster(self) -> bool:
        return self.ActsCluster

    @property
    def doActsSpacePoint(self) -> bool:
        return self.ActsSpacePoint

    @property
    def doActsSeed(self) -> bool:
        return self.ActsSeed
    
    @property
    def doActsTrack(self) -> bool:
        return self.ActsTrack

    @property
    def doAthenaToActsCluster(self) -> bool:
        return self.AthenaToActsClusterConverter

    @property
    def doAthenaToActsSpacePoint(self) -> bool:
        return self.AthenaToActsSpacePointConverter

    @property
    def doActsToAthenaCluster(self) -> bool:
        return self.ActsToAthenaClusterConverter

    @property
    def doActsToAthenaSpacePoint(self) -> bool:
        return self.ActsToAthenaSpacePointConverter

    @property
    def doActsToAthenaSeed(self) -> bool:
        return self.ActsToAthenaSeedConverter

    @property
    def doActsToAthenaTrack(self) -> bool:
        return self.ActsToAthenaTrackConverter

    @property
    def doAthenaAmbiguityResolution(self) -> bool:
        return self.AthenaAmbiguityResolution

    @property
    def doActsAmbiguityResolution(self) -> bool:
        return self.ActsAmbiguityResolution

    @property
    def doActsToAthenaResolvedTrack(self) -> bool:
        return self.ActsToAthenaResolvedTrackConverter

    def producesActsClusters(self) -> bool:
        return self.ActsCluster or self.AthenaToActsClusterConverter
    
    def producesActsSpacePoints(self) -> bool:
        return self.ActsSpacePoint or self.AthenaToActsSpacePointConverter

    def AthenaChain(self, flags):
        self.AthenaCluster = True
        self.AthenaSpacePoint = True
        self.AthenaSeed = True
        self.AthenaTrack = True
        self.AthenaAmbiguityResolution = True

    def ActsChain(self, flags):
        self.ActsCluster = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        self.ActsTrack = True
        # Ambiguity resolution can follow if ActsTrack is 
        # enabled. Ambi. can be activated/deactivated with 
        # the flag: Acts.doAmbiguityResolution
        self.ActsAmbiguityResolution = flags.Acts.doAmbiguityResolution

    def ValidateActsClusters(self, flags):
        self.ActsCluster = True
        self.ActsToAthenaClusterConverter = True
        self.AthenaSpacePoint = True
        self.AthenaSeed = True
        self.AthenaTrack = True
        self.AthenaAmbiguityResolution = True
        
    def ValidateActsSpacePoints(self, flags):
        self.AthenaCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        # we should schedule here the Acts -> Athena SP converter, but that is not available yet
        # so we go for the seeding convertion (i.e. ActsTrk::SiSpacePointSeedMaker)
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True
        self.AthenaAmbiguityResolution = True

    def ValidateActsSeeds(self, flags):
        self.AthenaCluster = True
        self.AthenaSpacePoint = True
        self.AthenaToActsSpacePointConverter = True
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True
        self.AthenaAmbiguityResolution = True

    def ValidateActsTracks(self, flags):
        # sequence is still a work in progress
        # Requires Athena cluster and cluster EDM converter 
        # for adding decoration to cluster objects
        # It produces Athena TrackCollection EDM
        self.AthenaCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        self.ActsTrack = True
        # If we do not want acts ambi resolution, first do the track convertion
        # and then the Athena ambi
        # If we want acts ambi, first do the ambi and then convert the tracks
        # without Athena ambi
        if flags.Acts.doAmbiguityResolution:
            self.ActsAmbiguityResolution = True
            self.ActsToAthenaResolvedTrackConverter = True
        else:
            self.ActsToAthenaTrackConverter = True
            self.AthenaAmbiguityResolution = True

    def BenchmarkSpot(self, flags):
        # Very not-standard configuration
        self.AthenaCluster = True
        self.ActsCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True
        self.AthenaAmbiguityResolution = True
