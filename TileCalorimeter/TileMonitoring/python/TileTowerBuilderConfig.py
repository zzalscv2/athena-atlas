#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTowerBuilderConfig.py
@brief Python configuration of Tile Tower builder algorithm for the Run III
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def TileTowerBuilderAlgCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault('name', 'TileTowerBldrAlg')
    kwargs.setdefault('TowerContainerName', 'TileTower')
    kwargs.setdefault('NumberOfPhiTowers', 64)
    kwargs.setdefault('NumberOfEtaTowers', 34)
    kwargs.setdefault('EtaMin', -1.7)
    kwargs.setdefault('EtaMax', 1.7)
    kwargs.setdefault('EnableChronoStat', flags.Concurrency.NumThreads == 0)

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    TileTowerBuilderTool=CompFactory.TileTowerBuilderTool
    tileCmbTwrBldr = TileTowerBuilderTool( name = 'TileCmbTwrBldr',
                                           CellContainerName = 'AllCalo',
                                           IncludedCalos = ['TILE'])

    kwargs['TowerBuilderTools'] = [ tileCmbTwrBldr ]

    CaloTowerAlgorithm=CompFactory.CaloTowerAlgorithm
    acc.addEventAlgo(CaloTowerAlgorithm(**kwargs), primary = True)

    return acc


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD
    flags.Output.HISTFileName = 'TileTowerMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge( TileTowerBuilderAlgCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileTowerBuilder.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
