# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Testing job options for magnetic field changes. Can be used e.g. in the
# "magFieldToggle" test instead of the full menu. Only runs one algorithm
# that prints the field status.

from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg


def run(flags):
   """Setup magnetic field and reader algorithm"""

   # Default flags are good enough:
   flags.lock()

   cfg = ComponentAccumulator()
   cfg.merge( AtlasFieldCacheCondAlgCfg(flags) )
   cfg.addEventAlgo( CompFactory.getComp("MagField::CondReader")("MagFieldCondReader") )

   cfg.foreach_component("*AtlasField.*CondAlg").OutputLevel = DEBUG

   return cfg
