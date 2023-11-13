# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArPulseShapeRunCfg(flags):

   result=ComponentAccumulator()

   from LArGeoAlgsNV.LArGMConfig import LArGMCfg
   result.merge(LArGMCfg(flags))
   from TileGeoModel.TileGMConfig import TileGMCfg
   result.merge(TileGMCfg(flags))

   #Setup cabling
   from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
   result.merge(LArOnOffIdMappingCfg(flags))

   from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
   result.merge(LArRawDataReadingCfg(flags))

   from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
   result.merge(L1TriggerByteStreamDecoderCfg(flags))

   from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
   tdt=result.getPrimaryAndMerge(TrigDecisionToolCfg(flags))


   from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
   result.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True))

   from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
   result.merge(LuminosityCondAlgCfg(flags))

   from AthenaCommon.Constants import DEBUG
   result.addService(CompFactory.NTupleSvc(Output=["PULSE DATAFILE='pulse_shape.root' OPT='RECREATE'",],OutputLevel=DEBUG))
   result.setAppProperty("HistogramPersistency","ROOT")

   result.addEventAlgo(CompFactory.LArPulseShape(TrigDecisionTool=tdt))

   return result

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from LArCafJobs.LArShapeDumperFlags import addShapeDumpFlags
    addShapeDumpFlags(flags)

    flags.Input.Files=["root://eosatlas//eos/atlas/atlascerngroupdisk/det-larg/ShapeTest/00461002/data23_13p6TeV.00461002.physics_CosmicCalo.merge.RAW._lb1020._SFO-ALL._0001.1"]

    flags.Detector.GeometryID = False
    flags.Detector.GeometryITk = False
    flags.Detector.GeometryHGTD = False
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False
    flags.Detector.GeometryForward = False

    flags.Trigger.doID = False

    flags.Trigger.triggerConfig='DB'
    flags.Trigger.DecisionMakerValidation.Execute=False
    flags.Trigger.enableL1CaloPhase1=False

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(LArPulseShapeRunCfg(flags))


    cfg.run(10)

