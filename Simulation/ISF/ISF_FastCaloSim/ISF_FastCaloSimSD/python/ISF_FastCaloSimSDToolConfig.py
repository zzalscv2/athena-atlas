# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FCS_StepInfoSDToolCfg(flags, name="FCS_StepInfoSensitiveDetector", **kwargs):
    kwargs.setdefault("StacVolumes", ["LArMgr::LAr::EMB::STAC"])
    kwargs.setdefault("PresamplerVolumes", ["LArMgr::LAr::Barrel::Presampler::Module"])
    kwargs.setdefault("NegIWVolumes", ["LArMgr::LAr::EMEC::Neg::InnerWheel"])
    kwargs.setdefault("NegOWVolumes", ["LArMgr::LAr::EMEC::Neg::OuterWheel"])
    kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PosIWVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel"])
    kwargs.setdefault("PosOWVolumes", ["LArMgr::LAr::EMEC::Pos::OuterWheel"])
    kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PresVolumes",  ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
    kwargs.setdefault("SliceVolumes", ["LArMgr::LAr::HEC::Module::Depth::Slice"])
    kwargs.setdefault("FCAL1Volumes", ["LArMgr::LAr::FCAL::Module1::Gap"])
    kwargs.setdefault("FCAL2Volumes", ["LArMgr::LAr::FCAL::Module2::Gap"])
    kwargs.setdefault("FCAL3Volumes", ["LArMgr::LAr::FCAL::Module3::Gap"])
    kwargs.setdefault("TileVolumes",  ["Tile::Scintillator"])
    kwargs.setdefault("OutputCollectionNames", ["EventSteps"])

    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.FCS_Param.FCS_StepInfoSDTool(name, **kwargs))
    return result


def PostIncludeParametrizationInputSim_1mm(flags, cfg):
    stepInfoSDTool = cfg.getPublicTool("SensitiveDetectorMasterTool").SensitiveDetectors['FCS_StepInfoSensitiveDetector']
    stepInfoSDTool.shift_lar_subhit=True #default
    stepInfoSDTool.shorten_lar_step=True
    stepInfoSDTool.maxRadiusFine=1. #default (for EMB1 and EME1)
    stepInfoSDTool.maxRadius=25. #default
    stepInfoSDTool.maxRadiusTile=25. #default
    stepInfoSDTool.maxTime=25. #default
    stepInfoSDTool.maxTimeTile=100. #default
