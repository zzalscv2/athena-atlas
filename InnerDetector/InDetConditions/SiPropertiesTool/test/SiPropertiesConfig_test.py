#!/usr/bin/env python
"""Run tests on SCT_SiPropertiesConfig.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from SiPropertiesTool.SCT_SiPropertiesConfig import SCT_SiPropertiesToolCfg
from SiPropertiesTool.PixelSiPropertiesConfig import PixelSiPropertiesToolCfg

# test setup
log.setLevel(DEBUG)
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.lock()
# test
sct_acc = SCT_SiPropertiesToolCfg(flags, name="SCT_SiPropertiesConfigTest")
sct_acc.popPrivateTools()
pix_acc = PixelSiPropertiesToolCfg(flags, name="PixelSiPropertiesConfigTest")
pix_acc.popPrivateTools()

sct_acc.wasMerged()
pix_acc.wasMerged()
