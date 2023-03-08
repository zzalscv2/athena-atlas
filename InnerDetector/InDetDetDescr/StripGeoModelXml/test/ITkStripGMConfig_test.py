#!/usr/bin/env python
"""Run tests on StripGeoModelXml configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import Project
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ttbar.HITS.pool.root"]
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
    flags.GeoModel.Align.Dynamic = False
    flags.lock()

    if flags.Common.Project is Project.AthSimulation:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripSimulationGeometryCfg
        acc = ITkStripSimulationGeometryCfg(flags)
        f=open('ITkStripSimulationGeometryCfg.pkl','wb')
    else:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc = ITkStripReadoutGeometryCfg(flags)
        f=open('ITkStripReadoutGeometryCfg.pkl','wb')
    acc.store(f)
    f.close()
