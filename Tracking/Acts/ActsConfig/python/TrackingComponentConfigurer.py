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

        # Acts Algorithms
        self.ActsCluster = False
        self.ActsSpacePoint = False
        self.ActsSeed = False
        self.ActsTrack = False

        # Athena -> Acts EDM Converters
        self.AthenaToActsClusterConverter = False
        self.AthenaToActsSpacePointConverter = False
        
        # Acts -> Athena EDM Converters
        self.ActsToAthenaClusterConverter = False
        self.ActsToAthenaSpacePointConverter = False
        self.ActsToAthenaSeedConverter = False

        # Enable the requested configurations
        for req in configuration_requests:
            eval(f"self.{self.__configuration_setters[req]}()")

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
        message += " - Acts Algorithms:\n"
        message += f"    * Clustering: {'ENABLED' if self.doActsCluster else 'DISABLED'}\n"
        message += f"    * Space Point Formation: {'ENABLED' if self.doActsSpacePoint else 'DISABLED'}\n"
        message += f"    * Seeding: {'ENABLED' if self.doActsSeed else 'DISABLED'}\n"
        message += f"    * Track Finding: {'ENABLED' if self.doActsTrack else 'DISABLED'}\n"
        message += " - Athena -> Acts EDM Convertions:\n"
        message += f"    * Cluster: {'ENABLED' if self.doAthenaToActsCluster else 'DISABLED'}\n"
        message += f"    * Space Point: {'ENABLED' if self.doAthenaToActsSpacePoint else 'DISABLED'}\n"
        message += " - Acts -> Athena EDM Convertions:\n"
        message += f"    * Cluster: {'ENABLED' if self.doActsToAthenaCluster else 'DISABLED'}\n"
        message += f"    * Space Point: {'ENABLED' if self.doActsToAthenaSpacePoint else 'DISABLED'}\n"
        message += f"    * Seed: {'ENABLED' if self.doActsToAthenaSeed else 'DISABLED'}"
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

    def producesActsClusters(self) -> bool:
        return self.ActsCluster or self.AthenaToActsClusterConverter
    
    def producesActsSpacePoints(self) -> bool:
        return self.ActsSpacePoint or self.AthenaToActsSpacePointConverter

    def AthenaChain(self):
        self.AthenaCluster = True
        self.AthenaSpacePoint = True
        self.AthenaSeed = True
        self.AthenaTrack = True

    def ActsChain(self):
        self.ActsCluster = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        # Track finding is not yet in its final form
        # thus, it is not enabled yet

    def ValidateActsClusters(self):
        self.ActsCluster = True
        self.ActsToAthenaClusterConverter = True
        self.AthenaSpacePoint = True
        self.AthenaSeed = True
        self.AthenaTrack = True
        
    def ValidateActsSpacePoints(self):
        self.AthenaCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        # we should schedule here the Acts -> Athena SP converter, but that is not available yet
        # so we go for the seeding convertion (i.e. ActsTrk::SiSpacePointSeedMaker)
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True

    def ValidateActsSeeds(self):
        self.AthenaCluster = True
        self.AthenaSpacePoint = True
        self.AthenaToActsSpacePointConverter = True
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True

    def ValidateActsTracks(self):
        # sequence is still a work in progress
        # Requires Athena cluster and cluster EDM converter 
        # for adding decoration to cluster objects
        # It produces Athena TrackCollection EDM
        self.AthenaCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        self.ActsTrack = True

    def BenchmarkSpot(self):
        # Very not-standard configuration
        self.AthenaCluster = True
        self.ActsCluster = True
        self.AthenaToActsClusterConverter = True
        self.ActsSpacePoint = True
        self.ActsSeed = True
        self.ActsToAthenaSeedConverter = True
        self.AthenaTrack = True

