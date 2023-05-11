# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the Egamma related xAOD Thinning
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaxAODThinningCfg(flags, name="EGammaxAODThinning"):

    mlog = logging.getLogger(name)
    mlog.info('Starting EGamma xAOD Thinning configuration')

    acc = ComponentAccumulator()
    # Add e/gamma track thinning
    if flags.Egamma.doTrackThinning:
        from egammaAlgs.egammaTrackThinnerConfig import (
            egammaTrackThinnerCfg)
        acc.merge(egammaTrackThinnerCfg(flags))

    # keep cells for egamma and egammaLargeClusters
    if flags.Egamma.keepCaloCellsAOD:
        outFlags = flags.Egamma.Keys.Output
        allClusters = []
        if flags.Egamma.doCentral:
            allClusters.append(outFlags.CaloClusters)
            allClusters.append(outFlags.EgammaLargeClusters)

        if flags.Egamma.doForward:
            allClusters.append(outFlags.ForwardClusters)
            allClusters.append(outFlags.EgammaLargeFWDClusters)

        if flags.Tracking.doLargeD0:
            allClusters.append(f"LRT{outFlags.CaloClusters}")

        if flags.HeavyIon.Egamma.doSubtractedClusters:
            allClusters.append(flags.HeavyIon.Egamma.CaloTopoCluster)

        samplings = [
            "TileGap1",
            "TileGap2",
            "TileGap3",
            "TileBar0",
            "TileExt0",
            "HEC0",
        ]
        from CaloRec.CaloThinCellsByClusterAlgConfig import (
            CaloThinCellsByClusterAlgCfg)

        for clus in allClusters:
            if flags.HeavyIon.Egamma.doSubtractedClusters and (
                    clus == outFlags.CaloClusters or clus == flags.HeavyIon.Egamma.CaloTopoCluster):
                cellsName = flags.HeavyIon.Egamma.SubtractedCells
            else:
                cellsName = flags.Egamma.Keys.Input.CaloCells

            acc.merge(CaloThinCellsByClusterAlgCfg(
                flags,
                streamName="StreamAOD",
                clusters=clus,
                samplings=samplings,
                cells=cellsName
            ))

    mlog.info("EGamma xAOD Thinning configured")
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags.Input.Files = defaultTestFiles.ESD
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(egammaxAODThinningCfg(flags))
    acc.printConfig(withDetails=True,
                    printDefaults=True)

    with open("egammaxaodthinningconfig.pkl", "wb") as f:
        acc.store(f)
