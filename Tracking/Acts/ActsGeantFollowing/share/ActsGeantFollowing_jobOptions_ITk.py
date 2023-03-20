# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#==============================================================
#
#
#		This job option runs the G4 simulation
#		of the ATLAS detector and the GeantFollower in ID (and MS)
#		It can be run using athena.py
#
#==============================================================


import sys
from argparse import ArgumentParser

from AthenaCommon.Constants import INFO
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from ActsGeometry.ActsGeometryConfig import ActsExtrapolationToolCfg


def defaultTestFlags(flags, args):


    ## Just enable ID for the moment.
    flags.Input.isMC            = True
    flags.ITk.Geometry.AllLocal = True
    detectors = [
      "ITkPixel",
      "ITkStrip",
      "Bpipe"
    ]
 
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-01-01-00"
    flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
    flags.GeoModel.Align.Dynamic = False
    # flags.Acts.TrackingGeometry.MaterialSource = "Input"
    # flags.Acts.TrackingGeometry.MaterialSource = "material-maps.json"

    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False

    # # This should run serially for the moment.
    # flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.Input.Files = [args.inputevntfile]
    
    flags.Output.HITSFileName = args.outputhitsfile

    from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground
    flags.Sim.CalibrationRun = CalibrationRun.Off
    flags.Sim.RecordStepInfo = False
    flags.Sim.CavernBackground = CavernBackground.Signal
    flags.Sim.ISFRun = False
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim

    flags.Input.RunNumber = [284500]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumber = [1]

def printAndRun(accessor, flags, args):
    """debugging and execution"""
    # Dump config
    if args.verboseAccumulators:
        accessor.printConfig(withDetails=True)
    if args.verboseStoreGate:
        accessor.getService("StoreGateSvc").Dump = True
    flags.dump()

    # Execute and finish
    sc = accessor.run(maxEvents=args.maxEvents)

    # Dump config summary
    accessor.printConfig(withDetails=False)

    # Success should be 0
    return not sc.isSuccess()


def ITkCfg(flags):
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    acc.merge(BeamEffectsAlgCfg(flags))

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    itkPixel = ITkPixelReadoutGeometryCfg(flags)
    acc.merge(itkPixel)

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    itkStrip = ITkStripReadoutGeometryCfg(flags)
    acc.merge(itkStrip)

    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    acc.merge(BeamPipeGeometryCfg(flags))

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    gmsAcc = GeoModelCfg(flags)
    acc.merge(gmsAcc)

    return acc

def ActsGeantFollowerCfg(flags, name="ActsGeantFollowerTool", **kwargs):
    
    result = ComponentAccumulator()

    from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
    result.merge(TrackingGeometrySvcCfg(flags))

    from ActsGeometry.ActsGeometryConfig import NominalAlignmentCondAlgCfg
    nomAli = NominalAlignmentCondAlgCfg(flags, OutputLevel=INFO)
    result.merge(nomAli)

    from ActsGeometry.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    tgSvc = ActsTrackingGeometrySvcCfg(flags, OutputLevel=INFO)
    result.merge(tgSvc)

    print('DEF WRITER : ')
    Actsextrapol = result.popToolsAndMerge(ActsExtrapolationToolCfg(flags,
                                                                    InteractionMultiScatering = True,
                                                                    InteractionEloss = True,
                                                                    InteractionRecord=True,
                                                                    OutputLevel=INFO))
    result.addPublicTool(Actsextrapol)

    from TrkConfig.AtlasExtrapolationEngineConfig import AtlasExtrapolationEngineCfg
    AtlasExtrapolationEngine = result.getPrimaryAndMerge(AtlasExtrapolationEngineCfg(flags))


    #Setup Helper
    followingHelper = CompFactory.ActsGeantFollowerHelper("ActsGeantFollowerHelper",
                                                          **kwargs,
                                                          ExtrapolationEngine=AtlasExtrapolationEngine,
                                                          ActsExtrapolator=result.getPublicTool(Actsextrapol.name), # PublicToolHandle
                                                          ExtrapolateDirectly=False,
                                                          ExtrapolateIncrementally=True,
                                                          OutputLevel=INFO)
    result.addPublicTool(followingHelper)

    #Setting up the CA for the ActsGeantFollower
    from ActsGeantFollowing.ActsGeantFollowingConfig import ActsGeantFollowerToolCfg
    actionAcc = ComponentAccumulator()
    actions = []
    actions += [actionAcc.popToolsAndMerge(ActsGeantFollowerToolCfg(flags))
    actionAcc.setPrivateTools(actions)
    ActsGeantFollowerAction = result.popToolsAndMerge(actionAcc)
    
    #Retrieving the default action list
    from G4AtlasServices.G4AtlasUserActionConfig import getDefaultActions
    defaultActions = result.popToolsAndMerge(getDefaultActions(flags))

    #Adding LengthIntegrator to defaults
    actionList = (defaultActions + ActsGeantFollowerAction)

    #Setting up UserActionsService
    kwargs.setdefault("UserActionTools",actionList)
    result.addService(CompFactory.G4UA.UserActionSvc(name, **kwargs))

    return result


# Argument parsing
parser = ArgumentParser("ActsGeantFollowing_jobOption_ITk.py")
parser.add_argument("--simulate", default=True, action="store_true",
                    help="Run Simulation")
parser.add_argument("-V", "--verboseAccumulators", default=False, 
                    action="store_true",
                    help="Print full details of the AlgSequence")
parser.add_argument("-S", "--verboseStoreGate", default=False, 
                    action="store_true",
                    help="Dump the StoreGate(s) each event iteration")
parser.add_argument("--maxEvents",default=-1, type=int,
                    help="The number of events to run. 0 skips execution")
parser.add_argument("--inputevntfile",
                    # default="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/e_E50_eta0-25.evgen.pool.root",
                    default="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetSLHC_Example/inputs/pgun_2M_10GeV_geantinos_Eta6_v2_EVNT.root",
                    # default="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetSLHC_Example/inputs/EVNT.09244569._000001.pool.root.1",
                    help="The input EVNT file to use")
parser.add_argument("--outputhitsfile",default="myHITS.pool.root", type=str,
                    help="The output HITS filename")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
defaultTestFlags(flags, args)
flags.lock()

# Construct our accumulator to run
acc = ITkCfg(flags)
kwargs = {}

svcName = "ActsGeantFollowerTool"
acc.merge(ActsGeantFollowerCfg(flags,svcName,**kwargs))
kwargs.update(UserActionSvc=svcName)

from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
acc.merge(G4AtlasAlgCfg(flags, "ITkG4AtlasAlg", **kwargs))

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
                acc.merge( OutputStreamCfg(flags,"HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True, AcceptAlgs=['ITkG4AtlasAlg']) )

# dump pickle
with open("ITkTest.pkl", "wb") as f:
    acc.store(f)

# Print and run
sys.exit(printAndRun(acc, flags, args))
