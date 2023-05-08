#!/usr/bin/env python
"""Run unit tests on RandomServices.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from RngComps.RandomServices import dSFMT, Ranlux64, Ranecu, RNG

# Set up logging and new style config
log.setLevel(DEBUG)
# Make each
t1 = dSFMT("TestSeed1")
t2 = Ranlux64("TestSeed2")
t3 = Ranecu("TestSeed3")
# Make RNG with all three agruments and none
t4 = RNG(name="RNGdSFMT1")
t5 = RNG("dSFMT", name="RNGdSFMT2")
t6 = RNG("Ranlux64", name="RNGRanlux64")
t7 = RNG("Ranecu", name="RNGRanecu")
# Merge
t1.merge(dSFMT("test_seed7"))
t1.merge(t2)
t1.merge(t3)
t1.merge(t4)
t1.merge(t5)
t1.merge(t6)
t1.merge(t7)
# Flag as merged to prevent error on destruction
t1.wasMerged()
