# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the EGamma reconstruction.
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaReconstructionCfg(flags, name="egammaReconstruction"):

    mlog = logging.getLogger(name)
    mlog.info('Starting EGamma reconstruction configuration')

    acc = ComponentAccumulator()

    if flags.HeavyIon.Egamma.doSubtractedClusters:
        from HIJetRec.HIEgammaRecConfigCA import (
            HIEgammaRecCfg)
        acc.merge(HIEgammaRecCfg(flags))

    # Add e/gamma tracking algorithms
    if flags.Egamma.doTracking:
        from egammaAlgs.egammaSelectedTrackCopyConfig import (
            egammaSelectedTrackCopyCfg)
        acc.merge(egammaSelectedTrackCopyCfg(flags))

        from egammaAlgs.EMBremCollectionBuilderConfig import (
            EMBremCollectionBuilderCfg)
        acc.merge(EMBremCollectionBuilderCfg(flags))

    # Add e/gamma conversion finding
    if flags.Egamma.doConversionBuilding:
        from egammaAlgs.EMVertexBuilderConfig import (
            EMVertexBuilderCfg)
        acc.merge(EMVertexBuilderCfg(flags))

    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    acc.merge(egammaTopoClusterCopierCfg(flags))

    # Add algorithms to produce
    # xAOD Electrons and Photons
    if flags.Egamma.doCentral:
        from egammaAlgs.egammaRecBuilderConfig import (
            egammaRecBuilderCfg)
        if flags.HeavyIon.Egamma.doSubtractedClusters:
            acc.merge(egammaRecBuilderCfg(
                flags, InputClusterContainerName=flags.HeavyIon.Egamma.CaloTopoCluster))
        else:
            acc.merge(egammaRecBuilderCfg(flags))

        from egammaAlgs.egammaSuperClusterBuilderConfig import (
            electronSuperClusterBuilderCfg, photonSuperClusterBuilderCfg)
        acc.merge(electronSuperClusterBuilderCfg(flags))
        acc.merge(photonSuperClusterBuilderCfg(flags))

        from egammaAlgs.topoEgammaBuilderConfig import (
            topoEgammaBuilderCfg)
        acc.merge(topoEgammaBuilderCfg(flags))

        from egammaAlgs.egammaLargeClusterMakerAlgConfig import (
            egammaLargeClusterMakerAlgCfg)
        acc.merge(egammaLargeClusterMakerAlgCfg(flags))

    # Add calo seeded forward algorithms to produce
    # xAOD Forward Electrons
    if flags.Egamma.doForward:
        from egammaAlgs.egammaForwardBuilderConfig import (
            egammaForwardBuilderCfg)
        acc.merge(egammaForwardBuilderCfg(flags))

        from egammaAlgs.egammaLargeFWDClusterMakerAlgConfig import (
            egammaLargeFWDClusterMakerAlgCfg)
        acc.merge(egammaLargeFWDClusterMakerAlgCfg(flags))

    # Add truth association
    if flags.Egamma.doTruthAssociation:
        from egammaAlgs.egammaTruthAssociationConfig import (
            egammaTruthAssociationCfg)
        acc.merge(egammaTruthAssociationCfg(flags))

    mlog.info("EGamma reconstruction configured")

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteESD = True  # To test the ESD parts
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(egammaReconstructionCfg(flags))
    acc.printConfig(withDetails=True,
                    printDefaults=True)

    with open("egammareconstructionconfig.pkl", "wb") as f:
        acc.store(f)
