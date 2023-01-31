#!/usr/bin/env python
"""Run tests on BeamPipeGeoModel configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.HITS_RUN2
    flags.GeoModel.Align.Dynamic = False
    flags.lock()

    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    acc = BeamPipeGeometryCfg(flags)
    with open('BeamPipeGeometryCfg.pkl', 'wb') as f:
        acc.store(f)
