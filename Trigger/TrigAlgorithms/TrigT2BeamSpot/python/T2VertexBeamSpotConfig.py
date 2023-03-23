# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

from TrigT2BeamSpot.T2VertexBeamSpotMonitoring import (T2VertexBeamSpotMonitoring,
                                                       T2VertexBeamSpotToolMonitoring,
                                                       T2BSTrackFilterToolMonitoring,
                                                       T2TrackBeamSpotToolMonitoring)

# track filter tool used by vertex tool
def trackFilterForVertex(flags):
    return CompFactory.getComp("PESA::T2BSTrackFilterTool")(
        name = "TrackFilterVtx",
        MonTool = T2BSTrackFilterToolMonitoring(flags).monTool,
        TrackMinPt          = 0.5,      # Minimum track pT to be considered for vertexing
        TrackMaxEta         = 2.5,      # Maximum absolute value of eta
        TrackMaxZ0          = 200.0,    # Maximum track Z0 to be considered for vertexing
        TrackMaxD0          = 10.0,     # Maximum track d0 to be considered for vertexing
        TrackMaxZ0err       = 5.0,      # Maximum track Z0 error to be considered for vertexing
        TrackMaxD0err       = 5.0,      # Maximum track d0 error to be considered for vertexing
        TrackMinNDF         = 2.0,      # Minimum track NDF to be considered for vertexing
        TrackMinQual        = 0.0,      # Minimum track chi^2/NDF to be considered for vertexing
        TrackMaxQual        = 10.0,     # Maximum track chi^2/NDF to be considered for vertexing
        TrackMinChi2Prob    = 0.05,     # Minimum track cumulative chi2 probability
        TrackMinSiHits      = 7,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinPIXHits     = 0,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinSCTHits     = 0,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinTRTHits     = -10,      # Minimum # track TRT hits to be considered for vertexing
        GoalSeedTracks      = 500,      # Number of tracks for local beamspot estimate
        D0Chi2Cutoff        = 25.,      # Cutoff on D0 Chi^2 for BS-based filtering
        BeamSizeLS          = 0.01,     # Approximate beam size, mm
    )

# track filter tool used by track tool
def trackFilterForTrack(flags):
    return CompFactory.getComp("PESA::T2BSTrackFilterTool")(
        name = "TrackFilterTrk",
        MonTool = T2BSTrackFilterToolMonitoring(flags).monTool,
        TrackMinPt          = 0.5,      # Minimum track pT to be considered for vertexing
        TrackMaxEta         = 2.5,      # Maximum absolute value of eta
        TrackMaxZ0          = 200.0,    # Maximum track Z0 to be considered for vertexing
        TrackMaxD0          = 10.0,     # Maximum track d0 to be considered for vertexing
        TrackMaxZ0err       = 5.0,      # Maximum track Z0 error to be considered for vertexing
        TrackMaxD0err       = 5.0,      # Maximum track d0 error to be considered for vertexing
        TrackMinNDF         = 2.0,      # Minimum track NDF to be considered for vertexing
        TrackMinQual        = 0.0,      # Minimum track chi^2/NDF to be considered for vertexing
        TrackMaxQual        = 10.0,     # Maximum track chi^2/NDF to be considered for vertexing
        TrackMinChi2Prob    = 0.05,     # Minimum track cumulative chi2 probability
        TrackMinSiHits      = 7,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinPIXHits     = 0,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinSCTHits     = 0,        # Minimum # track silicon (PIX + SCT) hits to be considered for vertexing
        TrackMinTRTHits     = -10,      # Minimum # track TRT hits to be considered for vertexing
        GoalSeedTracks      = 500,      # Number of tracks for local beamspot estimate
        D0Chi2Cutoff        = 10.,      # Cutoff on D0 Chi^2 for BS-based filtering
        BeamSizeLS          = 0.01,     # Approximate beam size, mm
    )


def InDetTrigMTBeamSpotTool(flags):
    return CompFactory.getComp("PESA::T2VertexBeamSpotTool")(
        name = "T2VertexBeamSpotTool",
        MonTool = T2VertexBeamSpotToolMonitoring(flags).monTool,
        TrackFilter         = trackFilterForVertex(flags),
        PrimaryVertexFitter = CompFactory.TrigPrimaryVertexFitter(zVariance=3.0, CreateTrackLists=True),

        WeightClusterZ      = True,     # Use the track Z0 weighted cluster Z position as seed
        ReclusterSplit      = False,    # Recluster split track collections before vertex fitting
        ClusterPerigee      = "beamspot",
        nSplitVertices      = 2,        # Turn on (>1) or off vertex splitting
        TotalNTrackMin      = 4,        # Minimum number of tracks required in an event
        TrackSeedPt         = 0.7,      # Minimum track pT to be considered for seeding a vertex fit
        TrackClusterDZ      = 0.35,      # Maximum distance between tracks considered as a cluster

        VertexMinNTrk       = 2,        # Minimum # tracks in a cluster to be considered for vertexing
        VertexMaxNTrk       = 100,      # Maximum # tracks in a cluster to be considered for vertexing (saves on time!)
        VertexMaxXerr       = 1.,       # Maximum resulting X error on vertex fit for "good" vertices
        VertexMaxYerr       = 1.,       # Maximum resulting Y error on vertex fit for "good" vertices
        VertexMaxZerr       = 10.,      # Maximum resulting Z error on vertex fit for "good" vertices
        VertexMinQual       = 0.0,      # Minimum resulting chi^2/NDF on vertex fit for "good" vertices
        VertexMaxQual       = 100.0,    # Maximum resulting chi^2/NDF on vertex fit for "good" vertices
        VertexMinChi2Prob   = -10.0,    # Minimum cumulative chi2 probability, from CLHEP/GenericFunctions/CumulativeChiSquare.hh
        VertexBCIDMinNTrk   = 10,       # Minimum # tracks in a vertex to be used for per-BCID monitoring

        filterBS            = True,     # filter tracks w.r.t. beamspot
    )


def InDetTrigMTTrackBeamSpotTool(flags):
    return CompFactory.getComp("PESA::T2TrackBeamSpotTool")(
        name                = "T2TrackBeamSpotTool",
        TrackFilter         = trackFilterForTrack(flags),
        MonTool             = T2TrackBeamSpotToolMonitoring(flags).monTool,
        doLeastSquares      = True,
        doLogLikelihood     = True,
        beamSizeLS          = 0.01,      # Approximate beam size, mm
    )


# Setup for writing out all events seen by the BeamSpot algorithm
def T2VertexBeamSpot_activeAllTE(flags, name="T2VertexBeamSpot_activeAllTE"):
    return CompFactory.getComp("PESA::T2VertexBeamSpot")(
        name,
        doTrackBeamSpot = True,    # run track-based calibration tool
        TrackBeamSpotTool = InDetTrigMTTrackBeamSpotTool(flags),
        BeamSpotTool = InDetTrigMTBeamSpotTool(flags),
        MonTool = T2VertexBeamSpotMonitoring(flags).monTool
    )


if __name__=="__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.addEventAlgo( T2VertexBeamSpot_activeAllTE(flags) )
    cfg.wasMerged()
