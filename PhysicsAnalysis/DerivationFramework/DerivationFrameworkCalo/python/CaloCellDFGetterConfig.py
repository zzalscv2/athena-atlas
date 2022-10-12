# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#=========================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def thinCaloCellsForDFCfg (flags, 
                           inputClusterKeys,
                           streamName,
                           inputCellKey = 'AllCalo',
                           outputCellKey = None):

    acc = ComponentAccumulator()

    from CaloRec.CaloThinCellsByClusterAlgConfig import (
        CaloThinCellsByClusterAlgCfg )

    samplings = ['TileGap1', 'TileGap2', 'TileGap3', 
                 'TileBar0', 'TileExt0', 'HEC0']

    for clkey in inputClusterKeys:
        acc.merge(CaloThinCellsByClusterAlgCfg( flags,
                                                streamName,
                                                clkey,
                                                samplings,
                                                inputCellKey) )
        

    from CaloRec.CaloThinCellsBySamplingAlgConfig import (
        CaloThinCellsBySamplingAlgCfg )
    acc.merge(CaloThinCellsBySamplingAlgCfg( flags,
                                             streamName,
                                             ['TileGap3'],
                                             inputCellKey) )

    # Originally, calo cell thinning worked by writing out a new cell container
    # and mangling links to point to it.  Now we use standard MT thinning,
    # so we just write out the default cell container with thinning.
    # Make an alias to the name we used to use for the cells in case
    # downstream code is using it.
    if outputCellKey is not None:
        from CaloRec.CaloCellContainerAliasAlgConfig import (
            CaloCellContainerAliasAlgCfg )
        acc.merge(CaloCellContainerAliasAlgCfg( flags,
                                                outputCellKey,
                                                inputCellKey) )

    return acc
