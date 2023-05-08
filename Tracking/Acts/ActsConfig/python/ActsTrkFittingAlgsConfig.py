#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsReFitterAlgCfg(flags, name="ActsReFitterAlg", **kwargs):
    result = ComponentAccumulator()

    from ActsConfig.ActsTrkFittingToolsConfig import ActsFitterCfg
    actsFitter = result.popToolsAndMerge(ActsFitterCfg(flags))

    kwargs.setdefault("ActsFitter", actsFitter)
    kwargs.setdefault("TrackName", "ResolvedTracks")

    result.addEventAlgo(
        CompFactory.ActsReFitterAlg(
            name=name,
            **kwargs,
        )
    )

    if flags.Acts.writeTrackCollection:
        result.merge(writeAdditionalTracks(flags))

    return result



def writeAdditionalTracks(flags, trackName='ResolvedTracks', newTrackName='ReFitted_Tracks'):
    result = ComponentAccumulator()
    
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(flags,
              Tracks = trackName,
              DetailedTruth = f"{trackName}DetailedTruth",
              TracksTruth = f"{trackName}TruthCollection"))
        result.merge(ITkTrackTruthCfg(flags,
              Tracks = newTrackName,
              DetailedTruth = f"{newTrackName}DetailedTruth",
              TracksTruth = f"{newTrackName}TruthCollection"))

    result.merge(ITkTrackParticleCnvAlgCfg(flags,
           name = f"{trackName}TrackParticleCnvAlg",
           TrackContainerName = trackName,
           xAODTrackParticlesFromTracksContainerName = f"{trackName}TrackParticles",
           TrackTruthContainerName = f"{trackName}TruthCollection")) 
    result.merge(ITkTrackParticleCnvAlgCfg(flags,
           name = f"{newTrackName}TrackParticleCnvAlg",
           TrackContainerName = newTrackName,
           xAODTrackParticlesFromTracksContainerName = f"{newTrackName}TrackParticles",
           TrackTruthContainerName = f"{newTrackName}TruthCollection")) 

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    itemList = [f"xAOD::TrackParticleContainer#{trackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{trackName}TrackParticlesAux.",
                f"xAOD::TrackParticleContainer#{newTrackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{newTrackName}TrackParticlesAux."]

    result.merge(addToESD(flags, itemList))
    result.merge(addToAOD(flags, itemList))
    
    return result

