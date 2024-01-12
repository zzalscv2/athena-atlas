# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


@AccumulatorCache
def FastSimulationToolListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.GeometryBpipe:
        if  not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
            # equivalent of simFlags.ForwardDetectors() == 2:
            from ForwardTransport.ForwardTransportConfig import ForwardTransportModelCfg
            tools += [ result.popToolsAndMerge(ForwardTransportModelCfg(flags)) ]
        if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal:
            from G4FastSimulation.G4FastSimulationConfig import SimpleFastKillerCfg
            tools += [ result.popToolsAndMerge(SimpleFastKillerCfg(flags)) ]
    if flags.Detector.GeometryLAr:
        if flags.Sim.LArParameterization is LArParameterization.NoFrozenShowers:
            from AthenaCommon.Logging import logging
            msg = logging.getLogger("FastSimulationToolListCfg")
            msg.info( "Not using Frozen Showers" )
        else:
            from LArG4FastSimulation.LArG4FastSimulationConfig import EMBFastShowerCfg, EMECFastShowerCfg, FCALFastShowerCfg, FCAL2FastShowerCfg
            # We run production with LArParameterization.FrozenShowersFCalOnly, so the EMB and EMEC tools are not required
            if flags.Sim.LArParameterization is LArParameterization.FrozenShowers:
                tools += [ result.popToolsAndMerge(EMBFastShowerCfg(flags)) ]
                tools += [ result.popToolsAndMerge(EMECFastShowerCfg(flags)) ]
            tools += [ result.popToolsAndMerge(FCALFastShowerCfg(flags)) ]
            tools += [ result.popToolsAndMerge(FCAL2FastShowerCfg(flags)) ]
            if flags.Sim.LArParameterization in [LArParameterization.DeadMaterialFrozenShowers, LArParameterization.FrozenShowersFCalOnly]:
                from G4FastSimulation.G4FastSimulationConfig import DeadMaterialShowerCfg
                tools += [ result.popToolsAndMerge(DeadMaterialShowerCfg(flags)) ]
    if flags.Detector.GeometryMuon:
        if flags.Sim.CavernBackground not in [CavernBackground.Off, CavernBackground.Read] and not flags.Sim.RecordFlux:
            from TrackWriteFastSim.TrackWriteFastSimConfig import NeutronFastSimCfg
            tools += [ result.popToolsAndMerge(NeutronFastSimCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def FastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FastSimulations", result.popToolsAndMerge(FastSimulationToolListCfg(flags)))
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    result.setPrivateTools(FastSimulationMasterTool(name="FastSimulationMasterTool", **kwargs))
    return result


def EmptyFastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    tool = result.popToolsAndMerge(FastSimulationMasterTool(name="EmptyFastSimulationMasterTool", **kwargs))
    result.setPrivateTools(tool)
    return result


def FwdSensitiveDetectorListCfg(flags):
    # TODO: migrate to CA
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.EnableLucid:
        from LUCID_G4_SD.LUCID_G4_SDConfig import LUCID_SensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LUCID_SensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableForward:
        if flags.Detector.EnableZDC:
            from ZDC_SD.ZDC_SDConfig import ZDC_PixelSDCfg, ZDC_StripSDCfg
            tools += [ result.popToolsAndMerge(ZDC_PixelSDCfg(flags)) ]
            tools += [ result.popToolsAndMerge(ZDC_StripSDCfg(flags)) ]
        if flags.Detector.EnableALFA:
            from ALFA_G4_SD.ALFA_G4_SDConfig import ALFA_SensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(ALFA_SensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableAFP:
            from AFP_G4_SD.AFP_G4_SDConfig import AFP_SensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(AFP_SensitiveDetectorCfg(flags)) ]
            # Alternative implementations
            # from AFP_G4_SD.AFP_G4_SDConfig import AFP_SiDSensitiveDetectorCfg, AFP_TDSensitiveDetectorCfg
            # tools += [ result.popToolsAndMerge(AFP_SiDSensitiveDetectorCfg(flags)) ]
            # tools += [ result.popToolsAndMerge(AFP_TDSensitiveDetectorCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def TrackFastSimSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if (flags.Detector.EnableMuon and flags.Sim.CavernBackground in [CavernBackground.Write, CavernBackground.WriteWorld]) or flags.Sim.StoppedParticleFile:
        from TrackWriteFastSim.TrackWriteFastSimConfig import TrackFastSimSDCfg
        tools += [ result.popToolsAndMerge(TrackFastSimSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def ITkSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableITkPixel:
        from PixelG4_SD.PixelG4_SDToolConfig import ITkPixelSensorSDCfg
        tools += [ result.popToolsAndMerge(ITkPixelSensorSDCfg(flags)) ]
        pass
    if flags.Detector.EnableITkStrip:
        from SCT_G4_SD.SCT_G4_SDToolConfig import ITkStripSensorSDCfg
        tools += [ result.popToolsAndMerge(ITkStripSensorSDCfg(flags)) ]
    if flags.Detector.EnablePLR:
        from PixelG4_SD.PixelG4_SDToolConfig import PLRSensorSDCfg
        tools += [ result.popToolsAndMerge(PLRSensorSDCfg(flags)) ]
    
    result.setPrivateTools(tools)
    return result


def HGTDSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableHGTD:
        from HGTD_G4_SD.HGTD_G4_SDToolConfig import HgtdSensorSDCfg
        tools += [ result.popToolsAndMerge(HgtdSensorSDCfg(flags)) ]
        pass

    result.setPrivateTools(tools)
    return result


def InDetSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnablePixel:
        from PixelG4_SD.PixelG4_SDToolConfig import PixelSensorSDCfg
        tools += [ result.popToolsAndMerge(PixelSensorSDCfg(flags)) ]
    if flags.Detector.EnableSCT:
        from SCT_G4_SD.SCT_G4_SDToolConfig import SctSensorSDCfg
        tools += [ result.popToolsAndMerge(SctSensorSDCfg(flags)) ]
    if flags.Detector.EnableTRT:
        from TRT_G4_SD.TRT_G4_SDToolConfig import TRTSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(TRTSensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableBCM:
        from BCM_G4_SD.BCM_G4_SDToolConfig import BCMSensorSDCfg
        tools += [ result.popToolsAndMerge(BCMSensorSDCfg(flags)) ]
        from BLM_G4_SD.BLM_G4_SDToolConfig import BLMSensorSDCfg
        tools += [ result.popToolsAndMerge(BLMSensorSDCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def CaloSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableLAr:
        from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg, LArEMECSensitiveDetectorCfg, LArFCALSensitiveDetectorCfg, LArHECSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LArEMBSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArEMECSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArFCALSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArHECSensitiveDetectorCfg(flags)) ]
        
        if flags.Detector.EnableMBTS:
            from MinBiasScintillator.MinBiasScintillatorToolConfig import MinBiasScintillatorSDCfg
            tools += [ result.popToolsAndMerge(MinBiasScintillatorSDCfg(flags)) ]

        if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg, LArActiveSensitiveDetectorToolCfg, LArInactiveSensitiveDetectorToolCfg
            tools += [ result.popToolsAndMerge(LArDeadSensitiveDetectorToolCfg(flags)) ]
            tools += [ result.popToolsAndMerge(LArInactiveSensitiveDetectorToolCfg(flags)) ]
            tools += [ result.popToolsAndMerge(LArActiveSensitiveDetectorToolCfg(flags)) ]
        elif flags.Sim.CalibrationRun is CalibrationRun.DeadLAr:
            from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg
            tools += [ result.popToolsAndMerge(LArDeadSensitiveDetectorToolCfg(flags)) ]

    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileGeoG4CalibSDCfg(flags)) ]  # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileGeoG4SDCfg(flags)) ]       # mode 0 : No CaloCalibrationHits
    if flags.Sim.RecordStepInfo:
        from ISF_FastCaloSimSD.ISF_FastCaloSimSDToolConfig import FCS_StepInfoSDToolCfg
        tools += [ result.popToolsAndMerge(FCS_StepInfoSDToolCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def MuonSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Beam.Type is BeamType.Cosmics:
        if flags.Detector.EnableMDT:
            from MuonG4SD.MuonG4SDToolConfig import MDTSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(MDTSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableRPC:
            from MuonG4SD.MuonG4SDToolConfig import RPCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(RPCSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableTGC:
            from MuonG4SD.MuonG4SDToolConfig import TGCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(TGCSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableCSC:
            from MuonG4SD.MuonG4SDToolConfig import CSCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(CSCSensitiveDetectorCosmicsCfg(flags)) ]
    else:
        if flags.Detector.EnableMDT:
            from MuonG4SD.MuonG4SDToolConfig import MDTSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(MDTSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableRPC:
            from MuonG4SD.MuonG4SDToolConfig import RPCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(RPCSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableTGC:
            from MuonG4SD.MuonG4SDToolConfig import TGCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(TGCSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableCSC:
            from MuonG4SD.MuonG4SDToolConfig import CSCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(CSCSensitiveDetectorCfg(flags)) ]

    if flags.Detector.EnablesTGC :
        from MuonG4SD.MuonG4SDToolConfig import sTGCSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(sTGCSensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableMM :
        from MuonG4SD.MuonG4SDToolConfig import MicromegasSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(MicromegasSensitiveDetectorCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def EnvelopeSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Beam.Type is BeamType.Cosmics and not flags.Sim.ReadTR:
        from TrackWriteFastSim.TrackWriteFastSimConfig import CosmicTRSDCfg
        tools += [ result.popToolsAndMerge(CosmicTRSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def SensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    tools += result.popToolsAndMerge(EnvelopeSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(InDetSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(ITkSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(HGTDSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(CaloSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(MuonSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(TrackFastSimSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(FwdSensitiveDetectorListCfg(flags))

    result.setPrivateTools(tools)
    return result


def TileTestBeamSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileCTBGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4CalibSDCfg(flags)) ] # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileCTBGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4SDCfg(flags)) ]      # mode 0 : No CaloCalibrationHits
            from MuonWall.MuonWallConfig import MuonWallSDCfg
            tools += [ result.popToolsAndMerge(MuonWallSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def CombinedTestBeamSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.EnablePixel:
        from PixelG4_SD.PixelG4_SDToolConfig import PixelSensor_CTBCfg
        tools += [ result.popToolsAndMerge(PixelSensor_CTBCfg(flags)) ]
    if flags.Detector.EnableSCT:
        from SCT_G4_SD.SCT_G4_SDToolConfig import SctSensor_CTBCfg
        tools += [ result.popToolsAndMerge(SctSensor_CTBCfg(flags)) ]
    if flags.Detector.EnableTRT:
        from TRT_G4_SD.TRT_G4_SDToolConfig import TRTSensitiveDetector_CTBCfg
        tools += [ result.popToolsAndMerge(TRTSensitiveDetector_CTBCfg(flags)) ]
    if flags.Detector.EnableLAr:
        from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LArEMBSensitiveDetectorCfg(flags)) ]
        if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC, CalibrationRun.DeadLAr]:
            tools += [ 'LArH8CalibSensitiveDetector' ] # mode 1 : With CaloCalibrationHits
    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileCTBGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4CalibSDCfg(flags)) ] # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileCTBGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4SDCfg(flags)) ]      # mode 0 : No CaloCalibrationHits
            tools += [ 'MuonWallSD' ]
    if flags.Detector.EnableMuon:
        tools += [ 'MuonEntryRecord' ]
    tools += result.popToolsAndMerge(MuonSensitiveDetectorListCfg(flags))

    result.setPrivateTools(tools)
    return result


def SensitiveDetectorMasterToolCfg(flags, name="SensitiveDetectorMasterTool", **kwargs):
    result = ComponentAccumulator()
    # NB Currently only supporting the standard ATLAS dector and the Tile Test Beam
    if flags.Beam.Type is BeamType.TestBeam:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(TileTestBeamSensitiveDetectorListCfg(flags)))
    elif "ATLAS" in flags.GeoModel.AtlasVersion:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(SensitiveDetectorListCfg(flags)))
    elif "tb_LArH6" in flags.GeoModel.AtlasVersion:
        pass
    elif "ctbh8" in flags.GeoModel.AtlasVersion:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(CombinedTestBeamSensitiveDetectorListCfg(flags)))

    result.setPrivateTools(CompFactory.SensitiveDetectorMasterTool(name, **kwargs))
    return result


def EmptySensitiveDetectorMasterToolCfg(name="EmptySensitiveDetectorMasterTool", **kwargs):
    result = ComponentAccumulator()
    tool = result.popToolsAndMerge(CompFactory.SensitiveDetectorMasterTool(name, **kwargs))
    result.setPrivateTools(tool)
    return result
