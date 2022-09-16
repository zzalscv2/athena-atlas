#!/usr/bin/env python
"""Run tests on SCT_SiPropertiesConfig.py

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from SiPropertiesTool.SCT_SiPropertiesConfig import SCT_SiPropertiesToolCfg
from SiPropertiesTool.PixelSiPropertiesConfig import PixelSiPropertiesToolCfg

# test setup
log.setLevel(DEBUG)
ConfigFlags.Input.Files = defaultTestFiles.HITS_RUN2
ConfigFlags.lock()
# test
sct_acc = SCT_SiPropertiesToolCfg(ConfigFlags, name="SCT_SiPropertiesConfigTest")
sct_acc.popPrivateTools()
pix_acc = PixelSiPropertiesToolCfg(ConfigFlags, name="PixelSiPropertiesConfigTest")
pix_acc.popPrivateTools()

sct_acc.wasMerged()
pix_acc.wasMerged()


