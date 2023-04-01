
if "StreamESD" in dir():
    StreamESD.ItemList += [ "xAOD::BaseContainer#SpacePoints", "xAOD::AuxContainerBase#SpacePointsAux.x.y.z.tot.csize" ]


if "StreamAOD" in dir():
    StreamAOD.ItemList += [ "xAOD::BaseContainer#SpacePoints", "xAOD::AuxContainerBase#SpacePointsAux.x.y.z.tot.csize" ]

if "StreamESD" in dir() or  ("StreamAOD" in dir() and not rec.readESD):
    from HIGlobal.HIGlobalConf import SpacePointCopier
    copier = SpacePointCopier(maxTotalSP=1000, maxTracks=20)
    topSequence += copier
