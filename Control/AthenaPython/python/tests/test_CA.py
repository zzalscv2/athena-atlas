# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPython.tests.PyTestsLib import MyAlg

flags = initConfigFlags()
flags.lock()

cfg = MainServicesCfg(flags)
cfg.addEventAlgo( MyAlg() )

cfg.store(open('test_CA.pkl','wb'))

import sys
sys.exit(cfg.run(2).isFailure())
