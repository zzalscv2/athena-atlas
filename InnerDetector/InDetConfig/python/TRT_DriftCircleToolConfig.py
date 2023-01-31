# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_DriftCircleTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def TRTDriftTimes(flags):
    from collections import namedtuple
    from AthenaCommon.SystemOfUnits import ns
    driftTimes = namedtuple("driftTimes", ("LowGate", "HighGate", "LowGateArgon", "HighGateArgon"))
    if flags.Beam.Type is BeamType.Cosmics:
        return driftTimes(LowGate         = 19.0*ns,
                          HighGate        = 44.0*ns,
                          LowGateArgon    = 19.0*ns,
                          HighGateArgon   = 44.0*ns)
    if not flags.Input.isMC:
        return driftTimes(LowGate         = 17.1875*ns,
                          HighGate        = 45.3125*ns,
                          LowGateArgon    = 18.75*ns,
                          HighGateArgon   = 43.75*ns)
    # MC
    return driftTimes(LowGate         = 14.0625*ns, # 4.5*3.125 ns
                      HighGate        = 42.1875*ns, # LowGate + 9*3.125 ns
                      LowGateArgon    = 14.0625*ns,
                      HighGateArgon   = 42.1875*ns)


def TRT_DriftCircleToolCfg(flags, name = "InDetTRT_DriftCircleTool", usePhase = False, **kwargs):
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTAlignCondAlgCfg
    acc = TRTAlignCondAlgCfg(flags)

    if "TRTDriftFunctionTool" not in kwargs:
        from InDetConfig.TRT_DriftFunctionToolConfig import TRT_DriftFunctionToolCfg
        TRT_DriftFunctionTool = acc.popToolsAndMerge(TRT_DriftFunctionToolCfg(flags, **kwargs))
        acc.addPublicTool(TRT_DriftFunctionTool)
        kwargs.setdefault("TRTDriftFunctionTool", TRT_DriftFunctionTool)

    if "ConditionsSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        kwargs.setdefault("ConditionsSummaryTool", acc.popToolsAndMerge(
            TRT_StrawStatusSummaryToolCfg(flags)))

    kwargs.setdefault("UseConditionsStatus", True)
    kwargs.setdefault("UseConditionsHTStatus", True)
    kwargs.setdefault("useDriftTimeHTCorrection", True)
    kwargs.setdefault("useDriftTimeToTCorrection", True)

    kwargs.setdefault("SimpleOutOfTimePileupSupression",       flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("SimpleOutOfTimePileupSupressionArgon" , flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("RejectIfFirstBit",      False)
    kwargs.setdefault("RejectIfFirstBitArgon", False)
    kwargs.setdefault("ValidityGateSuppression",       flags.Beam.Type is not BeamType.Cosmics)
    kwargs.setdefault("ValidityGateSuppressionArgon" , flags.Beam.Type is not BeamType.Cosmics)

    gains = TRTDriftTimes(flags)
    kwargs.setdefault("LowGate",      gains.LowGate)
    kwargs.setdefault("LowGateArgon", gains.LowGate)  # see discussion in MR !45402 why these are not Argon specific settings
    kwargs.setdefault("HighGate",      gains.HighGate)
    kwargs.setdefault("HighGateArgon", gains.HighGate)

    from AthenaCommon.SystemOfUnits import ns
    MinTrailingEdge = 11.0*ns
    MaxDriftTime = 60.0*ns
    kwargs.setdefault("MinTrailingEdge",      MinTrailingEdge)
    kwargs.setdefault("MinTrailingEdgeArgon", MinTrailingEdge)
    kwargs.setdefault("MaxDriftTime",      MaxDriftTime)
    kwargs.setdefault("MaxDriftTimeArgon", MaxDriftTime)

    if flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions:
        kwargs.setdefault("ValidityGateSuppression", True)
        kwargs.setdefault("SimpleOutOfTimePileupSupression", False)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("SimpleOutOfTimePileupSupression", False)

    TRT_DriftCircleTool = CompFactory.InDet.TRT_DriftCircleToolCosmics(name, **kwargs) if usePhase \
                          else CompFactory.InDet.TRT_DriftCircleTool(name, **kwargs)
    acc.setPrivateTools(TRT_DriftCircleTool)
    return acc

def TRT_NoTime_DriftCircleToolCfg(flags, name = "InDetTRT_NoTime_DriftCircleTool", **kwargs):
    acc = ComponentAccumulator()

    if "TRTDriftFunctionTool" not in kwargs:
        from InDetConfig.TRT_DriftFunctionToolConfig import TRT_NoTime_DriftFunctionToolCfg
        TRT_DriftFunctionTool = acc.popToolsAndMerge(TRT_NoTime_DriftFunctionToolCfg(flags, **kwargs))
        acc.addPublicTool(TRT_DriftFunctionTool)
        kwargs.setdefault("TRTDriftFunctionTool", TRT_DriftFunctionTool)

    acc.setPrivateTools(acc.popToolsAndMerge(
        TRT_DriftCircleToolCfg(flags, name, **kwargs)))
    return acc

def TRT_Phase_DriftCircleToolCfg(flags, name = "InDetTRT_Phase_DriftCircleTool", **kwargs):
    acc = ComponentAccumulator()

    if "TRTDriftFunctionTool" not in kwargs:
        from InDetConfig.TRT_DriftFunctionToolConfig import TRT_Phase_DriftFunctionToolCfg
        TRT_DriftFunctionTool = acc.popToolsAndMerge(TRT_Phase_DriftFunctionToolCfg(flags, **kwargs))
        acc.addPublicTool(TRT_DriftFunctionTool)
        kwargs.setdefault("TRTDriftFunctionTool", TRT_DriftFunctionTool)

    acc.setPrivateTools(acc.popToolsAndMerge(
        TRT_DriftCircleToolCfg(flags, name, usePhase=True, **kwargs)))
    return acc
