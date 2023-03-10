#!/usr/bin/env python
"""ComponentAccumulator PileUp (PU) Digitization configuration test

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from Digitization.DigitizationSteering import DigitizationMainCfg, DigitizationMessageSvcCfg, DigitizationTestingPostInclude
from RunDependentSimComps.PileUpUtils import generateBackgroundInputCollections, setupPileUpFlags

# Set up logging
log.setLevel(DEBUG)

flags = initConfigFlags()
flags.Exec.MaxEvents = 4

flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.Output.RDOFileName = "mc16d_ttbar.CA.RDO.pool.root"
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-25-02"

flags.GeoModel.Align.Dynamic = False
flags.GeoModel.AtlasVersion = "ATLAS-R2-2016-01-00-01"

flags.Beam.BunchSpacing = 25
flags.Beam.NumberOfCollisions = 20.

flags.LAr.ROD.NumberOfCollisions = 20
flags.LAr.ROD.nSamples = 4
flags.LAr.ROD.FirstSample = 0
flags.LAr.ROD.UseHighestGainAutoCorr = True

flags.Digitization.HighGainEMECIW = False

flags.Tile.BestPhaseFromCOOL = False
flags.Tile.correctTime = False

flags.Digitization.PileUp = True
flags.Digitization.EnableCaloHSTruthRecoInputs = False
flags.Digitization.RandomSeedOffset = 170

flags.Digitization.DigiSteeringConf = 'StandardSignalOnlyTruthPileUpToolsAlg'
flags.Digitization.DoXingByXingPileUp = True

flags.Digitization.PU.BunchSpacing = 25
flags.Digitization.PU.CavernIgnoresBeamInt = True
flags.Digitization.PU.NumberOfCavern = 0.0
flags.Digitization.PU.NumberOfHighPtMinBias = 0.2099789464
flags.Digitization.PU.NumberOfLowPtMinBias = 80.290021063135

cols = generateBackgroundInputCollections(flags, defaultTestFiles.HITS_RUN2_MINBIAS_HIGH,
                                          flags.Digitization.PU.NumberOfHighPtMinBias, True)
flags.Digitization.PU.HighPtMinBiasInputCols = cols

cols = generateBackgroundInputCollections(flags, defaultTestFiles.HITS_RUN2_MINBIAS_LOW,
                                          flags.Digitization.PU.NumberOfLowPtMinBias, True)
flags.Digitization.PU.LowPtMinBiasInputCols = cols

setupPileUpFlags(flags, 'RunDependentSimData.BunchTrains_MC20_2017', 'RunDependentSimData.PileUpProfile_run300000_MC20d')

flags.lock()

# Construct our accumulator to run
acc = DigitizationMainCfg(flags)
acc.merge(DigitizationMessageSvcCfg(flags))

DigitizationTestingPostInclude(flags, acc)

# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True, summariseProps=True)
flags.dump()
# print services
from AthenaConfiguration.ComponentAccumulator import filterComponents
for s, _ in filterComponents(acc._services):
    acc._msg.info(s)
# print conditions
for s in acc._conditionsAlgs:
    acc._msg.info(s)

# Execute and finish
sc = acc.run(maxEvents=flags.Exec.MaxEvents)
# Success should be 0
sys.exit(not sc.isSuccess())
