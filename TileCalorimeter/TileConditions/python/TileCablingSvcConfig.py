# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile cabling service"""

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


@AccumulatorCache
def TileCablingSvcCfg(flags):
    """Return component accumulator with configured Tile cabling service

    Arguments:
        flags  -- Athena configuration flags
    """

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('TileCablingSvc')

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg(flags) )

    tileCablingSvc = CompFactory.TileCablingSvc()

    geometry = flags.GeoModel.AtlasVersion

    if not flags.Common.isOnline:
      runNumber = flags.Input.RunNumbers[0]
      if flags.Input.OverrideRunNumber > 0:
          from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers
          runNumber = getMinMaxRunNumbers(flags)[0]
      if flags.GeoModel.Run is LHCPeriod.Run1:
          if runNumber > 219651:
              # Choose RUN2 cabling for old geometry tags starting from 26-MAR-2013
              tileCablingSvc.CablingType = 4
              msg.warning("Forcing RUN2 cabling for run %s with geometry %s", runNumber, geometry)

      elif flags.GeoModel.Run is LHCPeriod.Run2:
          if (flags.Input.isMC and runNumber >= 310000) or runNumber >= 343000 or runNumber < 1:
              # Choose RUN2a cabling for R2 geometry tags starting from 31-Jan-2018
              tileCablingSvc.CablingType = 5
              msg.info("Forcing RUN2a (2018) cabling for run %s with geometry %s", runNumber, geometry)

          else:
              tileCablingSvc.CablingType = 4
              msg.info("Forcing RUN2 (2014-2017) cabling for run %s with geometry %s", runNumber, geometry)
      elif flags.GeoModel.Run is LHCPeriod.Run3:
          tileCablingSvc.CablingType = 6
          msg.info("Forcing RUN3 cabling for run %s with geometry %s", flags.GeoModel.Run.value, geometry)
    else: #Running online or simulating running online: either way, do not access run number
      if flags.GeoModel.Run is LHCPeriod.Run1:
          tileCablingSvc.CablingType = 4
          msg.warning("Forcing RUN2 (2014-2017) cabling for unknown run number and geometry %s", geometry)
      if flags.GeoModel.Run is LHCPeriod.Run2:
          tileCablingSvc.CablingType = 5
          msg.info("Forcing RUN2a (2018) cabling for online run with geometry %s", geometry)
      elif flags.GeoModel.Run is LHCPeriod.Run3:
          tileCablingSvc.CablingType = 6
          msg.info("Forcing RUN3 cabling for online run with geometry %s", geometry)
      elif flags.GeoModel.Run is LHCPeriod.Run4:
          tileCablingSvc.CablingType = 6
          msg.warning("Forcing RUN3 cabling beyond Run3 for online run with geometry %s", geometry)
      else:
          msg.error("Tile Cabling version not set for %s", geometry)

    acc.addService(tileCablingSvc, primary = True)

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    
    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = ComponentAccumulator()
    acc.merge( TileCablingSvcCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('TileCablingSvc'))
    acc.store( open('TileCablingSvc.pkl','wb') )

    print('All OK')
