#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
  Run ACTS geometry construction for ITk
"""
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.Enums import ProductionStep
flags.Common.ProductionStep = ProductionStep.Simulation
flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-00"
flags.IOVDb.GlobalTag = "OFLCOND-MC15c-SDR-14-05"
flags.GeoModel.Align.Dynamic = False
flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1']

flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = False

flags.Concurrency.NumThreads = 64
flags.Concurrency.NumConcurrentEvents = 64

flags.Exec.MaxEvents = 200

flags.lock()
flags.dump()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

from ActsConfig.ActsGeometryConfig import ActsExtrapolationAlgCfg, ActsTrackingGeometrySvcCfg

from AthenaCommon.Constants import INFO
tgSvc = ActsTrackingGeometrySvcCfg(flags,
                                   OutputLevel=INFO,
                                   RunConsistencyChecks=True,
                                   #  ConsistencyCheckOutput="trk_geo_check.csv", # enable debug output writing
                                   ObjDebugOutput=True)
acc.merge(tgSvc)

alg = ActsExtrapolationAlgCfg(flags,
                              OutputLevel=INFO,
                              NParticlesPerEvent = int(100),
                              WritePropStep = True,
                              EtaRange = [-5, 5],
                              PtRange = [20, 100])

acc.merge(alg)
acc.printConfig()
acc.run()
