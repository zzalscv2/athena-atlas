# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg


def trigOpMonitorCfg(flags):
   cfg = ComponentAccumulator()
   opmon = CompFactory.TrigOpMonitor(
      LuminosityCondDataKey = 'LuminosityCondData',
      AtlasFieldMapCondDataKey = 'fieldMapCondObj' )

   cfg.merge( AtlasFieldCacheCondAlgCfg(flags) )
   cfg.addEventAlgo( opmon )
   return cfg
