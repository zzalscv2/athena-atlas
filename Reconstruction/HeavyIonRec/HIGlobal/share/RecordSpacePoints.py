
if "StreamESD" in dir():
    StreamESD.ItemList += [ "xAOD::BaseContainer#SpacePoints", "xAOD::AuxContainerBase#SpacePointsAux.x.y.z" ]

    from HIGlobal.HIGlobalConf import SpacePointCopier
    copier = SpacePointCopier(maxTotalSP=1000, maxTracks=20)
    topSequence += copier

if "StreamAOD" in dir():
    StreamAOD.ItemList += [ "xAOD::BaseContainer#SpacePoints", "xAOD::AuxContainerBase#SpacePointsAux.x.y.z" ]