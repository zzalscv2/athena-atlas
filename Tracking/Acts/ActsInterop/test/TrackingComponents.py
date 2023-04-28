#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from TrkConfig.TrkConfigFlags import TrackingComponent
    from ActsInterop.TrackingComponentConfigurer import TrackingComponentConfigurer

    request_collection = {
        "Production Mode" : [TrackingComponent.AthenaChain, TrackingComponent.ActsChain],
        "Validation Mode - Clusters" : [TrackingComponent.ValidateActsClusters],
        "Validation Mode - Space Points" : [TrackingComponent.ValidateActsSpacePoints],
        "Validation Mode - Seeds" : [TrackingComponent.ValidateActsSeeds],
        "Validation Mode - Tracks" : [TrackingComponent.ValidateActsTracks]
    }

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    for (mode, request) in request_collection.items():
        print("=" * 40)
        print(mode)
        print("-" * 40)
        flags.Tracking.recoChain = request
        configuration = TrackingComponentConfigurer(flags)
        print(configuration)
        print("=" * 40)

