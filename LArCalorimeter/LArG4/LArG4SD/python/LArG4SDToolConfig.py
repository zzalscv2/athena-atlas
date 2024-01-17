# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SimulationConfig.SimEnums import CalibrationRun, LArParameterization
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

#to be migrated: getCalibrationDefaultCalculator, getDeadMaterialCalibrationHitMerger

def LArActiveSensitiveDetectorToolCfg(flags, name="LArActiveSensitiveDetector", **kwargs):
    result = ComponentAccumulator()

    ## Main configuration
    if flags.GeoModel.AtlasVersion not in ["tb_LArH6_2003","tb_LArH6_2002"]:
        kwargs.setdefault("StacVolumes",["LArMgr::LAr::EMB::STAC"])
        kwargs.setdefault("PresamplerVolumes",["LArMgr::LAr::Barrel::Presampler::Module"])
        kwargs.setdefault("NegIWVolumes",["LArMgr::LAr::EMEC::Neg::InnerWheel"])
        kwargs.setdefault("NegOWVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel"])
        kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
    if flags.GeoModel.AtlasVersion!="tb_LArH6_2003":
        kwargs.setdefault("PosIWVolumes",["LArMgr::LAr::EMEC::Pos::InnerWheel"])
        kwargs.setdefault("PosOWVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel"])
        kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
        kwargs.setdefault("PresVolumes", ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
        kwargs.setdefault("SliceVolumes",["LArMgr::LAr::HEC::Module::Depth::Slice"])
    if flags.GeoModel.AtlasVersion not in ["tb_LArH6_2002"]:
        kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::Gap"])
        kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::Gap"])
        kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::Gap"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID",flags.Sim.ParticleID)
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", ["LArCalibrationHitActive"])

    from LArG4Barrel.LArG4BarrelConfig import BarrelCalibrationCalculatorCfg, BarrelPresamplerCalibrationCalculatorCfg
    kwargs.setdefault("EMBPSCalibrationCalculator",
                      result.getPrimaryAndMerge(BarrelPresamplerCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCalibrationCalculator",
                      result.getPrimaryAndMerge(BarrelCalibrationCalculatorCfg(flags)).name)

    from LArG4EC.LArG4ECConfig import EMECPosInnerWheelCalibrationCalculatorCfg, EMECNegInnerWheelCalibrationCalculatorCfg, EMECPosOuterWheelCalibrationCalculatorCfg, EMECNegOuterWheelCalibrationCalculatorCfg, EMECPosBackOuterBarretteCalibrationCalculatorCfg, EMECNegBackOuterBarretteCalibrationCalculatorCfg, EMECPresamplerCalibrationCalculatorCfg
    kwargs.setdefault("EMECPosIWCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECPosInnerWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegIWCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECNegInnerWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPosOWCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECPosOuterWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegOWCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECNegOuterWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPSCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECPresamplerCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPosBOBCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECPosBackOuterBarretteCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegBOBCalibrationCalculator",
                      result.getPrimaryAndMerge(EMECNegBackOuterBarretteCalibrationCalculatorCfg(flags)).name)

    from LArG4HEC.LArG4HECConfig import HECCalibrationWheelActiveCalculatorCfg
    kwargs.setdefault("HECWActiveCalculator",
                      result.getPrimaryAndMerge(HECCalibrationWheelActiveCalculatorCfg(flags)).name)

    from LArG4FCAL.LArG4FCALConfig import FCAL1CalibCalculatorCfg, FCAL2CalibCalculatorCfg, FCAL3CalibCalculatorCfg
    kwargs.setdefault("FCAL1CalibCalculator",
                      result.getPrimaryAndMerge(FCAL1CalibCalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL2CalibCalculator",
                      result.getPrimaryAndMerge(FCAL2CalibCalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL3CalibCalculator",
                      result.getPrimaryAndMerge(FCAL3CalibCalculatorCfg(flags)).name)

   
    result.setPrivateTools( CompFactory.LArG4.ActiveSDTool(name, **kwargs))
    return result


def LArDeadSensitiveDetectorToolCfg(flags, name="LArDeadSensitiveDetector", **kwargs):
    ## Main configuration
    kwargs.setdefault("BarrelCryVolumes",   ["LArMgr::LAr::Barrel::Cryostat::InnerWall::Vis",
                                             "LArMgr::LAr::Barrel::Cryostat::Sector::*",
                                             "LArMgr::LAr::Barrel::Cryostat::InnerWall",
                                             "LArMgr::LAr::Barrel::Cryostat::Cylinder::*"])
    kwargs.setdefault("BarrelCryLArVolumes",["LArMgr::LAr::Barrel::Cryostat::ExtraMat*",
                                             "LArMgr::LAr::Barrel::Cryostat::HalfLAr*",
                                             "LArMgr::LAr::Barrel::Cryostat::TotalLAr",
                                             "LArMgr::LAr::Barrel::Cryostat::MotherVolume"])
    kwargs.setdefault("BarrelCryMixVolumes",["LArMgr::LAr::Barrel::Cryostat::InnerEndWall",
                                             "LArMgr::LAr::Barrel::Cryostat::OuterWall",
                                             "LArMgr::LAr::Barrel::Cryostat::Mixed::Cylinder::*"])
    kwargs.setdefault("DeadMaterialVolumes",["LArMgr::LAr::DM::*"])
    kwargs.setdefault("BarrelPresVolumes",  ["LArMgr::LAr::Barrel::Presampler",
                                             "LArMgr::LAr::Barrel::Presampler::Sector",
                                             "LArMgr::LAr::Barrel::Presampler::ProtectionShell",
                                             "LArMgr::LAr::Barrel::Presampler::MotherBoard",
                                             "LArMgr::LAr::Barrel::Presampler::Connectics",
                                             "LArMgr::LAr::Barrel::Presampler::Rail",
                                             "LArMgr::LAr::Barrel::Presampler::ProtectionPlate"])
    kwargs.setdefault("BarrelVolumes",      ["LArMgr::LAr::EMB::ExtraMat*",
                                             "LArMgr::LAr::EMB::FrontBack::Absorber",
                                             "LArMgr::LAr::EMB::FrontBack::Absorber2",
                                             "LArMgr::LAr::EMB::FrontBack::Steel",
                                             "LArMgr::LAr::EMB::FrontBack::G10",
                                             "LArMgr::LAr::EMB::FrontBack::Electrode",
                                             "LArMgr::LAr::EMB::GTENB",
                                             "LArMgr::LAr::EMB::GTENF",
                                             "LArMgr::LAr::EMB::SUMB",
                                             "LArMgr::LAr::EMB::CAAC",
                                             "LArMgr::LAr::EMB::MOAC",
                                             "LArMgr::LAr::EMB::TELB",
                                             "LArMgr::LAr::EMB::TELF",
                                             "LArMgr::LAr::EMB::ECAM"])
    kwargs.setdefault("ECCryVolumes",       ["LArMgr::LAr::Endcap::Cryostat::FcalLAr::Cylinder",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Sector",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Cylinder",
                                             "LArMgr::LAr::Endcap::Cryostat::Sector",
                                             "LArMgr::LAr::Endcap::Cryostat::Cone",
                                             "LArMgr::LAr::Endcap::Cryostat::Cylinder"])
    kwargs.setdefault("ECCryLArVolumes",    ["LArMgr::Moderator*",
                                             "LArMgr::LAr::Endcap::Cryostat::FcalLAr",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr",
                                             "LArMgr::LAr::Endcap::Cryostat::MotherVolume"])
    kwargs.setdefault("ECCryMixVolumes",    ["LArMgr::LAr::FCAL::LiquidArgonC",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Sector::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::Sector::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::Cone::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::ExtraCyl_beforePS",
                                             "LArMgr::LAr::Endcap::Cryostat::Cylinder::Mixed"])
    kwargs.setdefault("ECSupportVolumes",   ["LArMgr::LAr::EMEC::ExtraCyl_afterPS",
                                             "LArMgr::LAr::EMEC::InnerTransversalBars",
                                             "LArMgr::LAr::EMEC::InnerAluCone::*",
                                             "LArMgr::LAr::EMEC::OuterTransversalBars",
                                             "LArMgr::LAr::EMEC::OuterSupportMother",
                                             "LArMgr::LAr::EMEC::*Stretchers",
                                             "LArMgr::LAr::EMEC::Top*",
                                             "LArMgr::LAr::EMEC::Back*GTen",
                                             "LArMgr::LAr::EMEC::Back*Hole",
                                             "LArMgr::LAr::EMEC::Back*Bar",
                                             "LArMgr::LAr::EMEC::Back*Ring",
                                             "LArMgr::LAr::EMEC::Back*Ele",
                                             "LArMgr::LAr::EMEC::Back*Abs",
                                             "LArMgr::LAr::EMEC::BackInnerBarrette::Module::Phidiv",
                                             "LArMgr::LAr::EMEC::Back*Barrette::Module",
                                             "LArMgr::LAr::EMEC::Back*Barrettes",
                                             "LArMgr::LAr::EMEC::BackSupport*",
                                             "LArMgr::LAr::EMEC::Front*",
                                             "LArMgr::LAr::EMEC::Mother*"])
    kwargs.setdefault("HECWheelVolumes",    ["LArMgr::LAr::HEC::Mother",
                                             "LArMgr::LAr::HEC::LiquidArgon",
                                             "LArMgr::LAr::HEC::Clamp",
                                             "LArMgr::LAr::HEC::Clamp::LiquidArgon",
                                             "LArMgr::LAr::HEC::Module",
                                             "LArMgr::LAr::HEC::FrontModule",
                                             "LArMgr::LAr::HEC::RearModule",
                                             "LArMgr::LAr::HEC::Module::Depth",
                                             "LArMgr::LAr::HEC::Module::Depth::FirstAbsorber",
                                             "LArMgr::LAr::HEC::Module::Depth::FirstAbsorber::TieRod"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID", flags.Sim.ParticleID)
    kwargs.setdefault("doEscapedEnergy", flags.Sim.CalibrationRun is not CalibrationRun.DeadLAr)
    # No effect currently
    outputCollectionName = "LArCalibrationHitDeadMaterial"
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        outputCollectionName = "LArCalibrationHitDeadMaterial_DEAD"
    kwargs.setdefault("HitCollectionName", outputCollectionName)

    result = ComponentAccumulator()

    from LArG4Barrel.LArG4BarrelConfig import BarrelCryostatCalibrationCalculatorCfg, BarrelCryostatCalibrationLArCalculatorCfg, BarrelCryostatCalibrationMixedCalculatorCfg, DMCalibrationCalculatorCfg,   BarrelCalibrationCalculatorCfg, BarrelPresamplerCalibrationCalculatorCfg
    kwargs.setdefault("EMBCryoCalibrationCalculator", result.getPrimaryAndMerge(BarrelCryostatCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCryoLArCalibrationCalculator", result.getPrimaryAndMerge(BarrelCryostatCalibrationLArCalculatorCfg(flags)).name)
    kwargs.setdefault("DefaultCalibrationCalculator", result.getPrimaryAndMerge(CalibrationDefaultCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCryoMixCalibrationCalculator", result.getPrimaryAndMerge(BarrelCryostatCalibrationMixedCalculatorCfg(flags)).name)
    kwargs.setdefault("DMCalibrationCalculator", result.getPrimaryAndMerge(DMCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBPSCalibrationCalculator", result.getPrimaryAndMerge(BarrelPresamplerCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCalibrationCalculator", result.getPrimaryAndMerge(BarrelCalibrationCalculatorCfg(flags)).name)

    from LArG4EC.LArG4ECConfig import EndcapCryostatCalibrationCalculatorCfg, EndcapCryostatCalibrationLArCalculatorCfg, EndcapCryostatCalibrationMixedCalculatorCfg, EMECSupportCalibrationCalculatorCfg
    kwargs.setdefault("ECCryoCalibrationCalculator", result.getPrimaryAndMerge(EndcapCryostatCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("ECCryoLArCalibrationCalculator", result.getPrimaryAndMerge(EndcapCryostatCalibrationLArCalculatorCfg(flags)).name)
    kwargs.setdefault("ECCryoMixCalibrationCalculator", result.getPrimaryAndMerge(EndcapCryostatCalibrationMixedCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECSuppCalibrationCalculator", result.getPrimaryAndMerge(EMECSupportCalibrationCalculatorCfg(flags)).name)

    from LArG4HEC.LArG4HECConfig import HECCalibrationWheelDeadCalculatorCfg
    kwargs.setdefault("HECWheelDeadCalculator", result.getPrimaryAndMerge(HECCalibrationWheelDeadCalculatorCfg(flags)).name)

    result.setPrivateTools(CompFactory.LArG4.DeadSDTool(name, **kwargs))
    return result


def LArEMBSensitiveDetectorCfg(flags,name="LArEMBSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitEMB"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArEMBHits"
    region = "CALO"
    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)

    result.merge(acc)
    ## Main configuration
    kwargs.setdefault("StacVolumes",["LArMgr::LAr::EMB::STAC"])
    kwargs.setdefault("PresamplerVolumes",["LArMgr::LAr::Barrel::Presampler::Module"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    kwargs.setdefault("UseFrozenShowers", flags.Sim.LArParameterization is LArParameterization.FrozenShowers)

    from LArG4Barrel.LArG4BarrelConfig import EMBPresamplerCalculatorCfg, EMBCalculatorCfg
    kwargs.setdefault("EMBPSCalculator", result.getPrimaryAndMerge(EMBPresamplerCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCalculator", result.getPrimaryAndMerge(EMBCalculatorCfg(flags)).name)

    result.setPrivateTools( CompFactory.LArG4.EMBSDTool(name, **kwargs) )
    return result


def LArEMECSensitiveDetectorCfg(flags, name="LArEMECSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitEMEC"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArEMECHits"
    region = "CALO"
    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)
    result.merge(acc)

    if flags.GeoModel.AtlasVersion not in ["tb_LArH6_2002","tb_LArH6EC_2002"]:
        kwargs.setdefault("NegIWVolumes",["LArMgr::LAr::EMEC::Neg::InnerWheel"])
        kwargs.setdefault("NegOWVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel"])
        kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
    if flags.GeoModel.AtlasVersion !="tb_LArH6EC_2002":
        kwargs.setdefault("PosIWVolumes",["LArMgr::LAr::EMEC::Pos::InnerWheel"])
        kwargs.setdefault("PosOWVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel"])
        kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PresVolumes", ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    kwargs.setdefault("UseFrozenShowers", flags.Sim.LArParameterization is LArParameterization.FrozenShowers)

    from LArG4EC.LArG4ECConfig import EMECPosInnerWheelCalculatorCfg, EMECNegInnerWheelCalculatorCfg, EMECPosOuterWheelCalculatorCfg, EMECNegOuterWheelCalculatorCfg, EMECPresamplerCalculatorCfg, EMECPosBackOuterBarretteCalculatorCfg, EMECNegBackOuterBarretteCalculatorCfg
    kwargs.setdefault("EMECPosIWCalculator", result.getPrimaryAndMerge(EMECPosInnerWheelCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegIWCalculator", result.getPrimaryAndMerge(EMECNegInnerWheelCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPosOWCalculator", result.getPrimaryAndMerge(EMECPosOuterWheelCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegOWCalculator", result.getPrimaryAndMerge(EMECNegOuterWheelCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPSCalculator", result.getPrimaryAndMerge(EMECPresamplerCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPosBOBCalculator", result.getPrimaryAndMerge(EMECPosBackOuterBarretteCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegBOBCalculator", result.getPrimaryAndMerge(EMECNegBackOuterBarretteCalculatorCfg(flags)).name)

    result.setPrivateTools( CompFactory.LArG4.EMECSDTool(name, **kwargs) )
    return result


def LArFCALSensitiveDetectorCfg(flags, name="LArFCALSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitFCAL"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArFCALHits"
    region = "CALO"
    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)
    result.merge(acc)

    kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::Gap"])
    kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::Gap"])
    kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::Gap"])
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    kwargs.setdefault("UseFrozenShowers", flags.Sim.LArParameterization is not LArParameterization.NoFrozenShowers)

    from LArG4FCAL.LArG4FCALConfig import FCAL1CalculatorCfg, FCAL2CalculatorCfg, FCAL3CalculatorCfg
    kwargs.setdefault("FCAL1Calculator", result.getPrimaryAndMerge(FCAL1CalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL2Calculator", result.getPrimaryAndMerge(FCAL2CalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL3Calculator", result.getPrimaryAndMerge(FCAL3CalculatorCfg(flags)).name)

    result.setPrivateTools( CompFactory.LArG4.FCALSDTool(name, **kwargs) )
    return result


def LArHECSensitiveDetectorCfg(flags, name="LArHECSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitHEC"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArHECHits"
    region = "CALO"
    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)
    result.merge(acc)

    kwargs.setdefault("WheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Slice"])
    #kwargs.setdefault("SliceVolumes",["LAr::HEC::Module::Depth::Slice"])
    #kwargs.setdefault("LocalVolumes",["LAr::HEC::Module::Depth::Slice::Local"])
    #  You might think this should go here, but we don't think so!  LAr::HEC::Module::Depth::Slice::Wheel"])
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    from LArG4HEC.LArG4HECConfig import HECWheelCalculatorCfg
    kwargs.setdefault("HECWheelCalculator", result.getPrimaryAndMerge(HECWheelCalculatorCfg(flags)).name)

    result.setPrivateTools( CompFactory.LArG4.HECSDTool(name, **kwargs) )
    return result


def LArInactiveSensitiveDetectorToolCfg(flags, name="LArInactiveSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    ## Main configuration
    if flags.GeoModel.AtlasVersion not in ["tb_LArH6_2003","tb_LArH6_2002"]:
        kwargs.setdefault("BarrelPreVolumes",["LArMgr::LAr::Barrel::Presampler::Cathode*",
                                              "LArMgr::LAr::Barrel::Presampler::Anode*",
                                              "LArMgr::LAr::Barrel::Presampler::Prep*"])
        kwargs.setdefault("BarrelVolumes",["LArMgr::LAr::EMB::*::Straight",
                                           "LArMgr::LAr::EMB::*::*Fold"])
        kwargs.setdefault("ECPosInVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Lead",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Lead",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Lead"])
        kwargs.setdefault("ECPosOutVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel::Lead",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Glue",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Lead",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Glue",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Lead",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Absorber"])
        kwargs.setdefault("ECNegInVolumes", ["LArMgr::LAr::EMEC::Neg::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Lead",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Lead",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Lead"])
        kwargs.setdefault("ECNegOutVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel::Lead",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Glue",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Lead",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Glue",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Lead",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Absorber"])
        #kwargs.setdefault("HECVolumes",["LAr::HEC::Inactive"])
        #kwargs.setdefault("HECLocalVolumes",["LAr::HEC::Local::Inactive"])
        kwargs.setdefault("HECWheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Absorber::TieRod",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRodDead",
                                             "LArMgr::LAr::HEC::Module::Depth::Absorber",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRod",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode::Copper",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode"])
    if flags.GeoModel.AtlasVersion=="tb_LArH6_2002":
        kwargs.setdefault("ECPosInVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Electrode"])
        kwargs.setdefault("ECPosOutVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Absorber"])
        kwargs.setdefault("HECWheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Absorber",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode::Copper",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRod"])
    if flags.GeoModel.AtlasVersion!="tb_LArH6_2002":
        kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::CableTrough",
                                          "LArMgr::LAr::FCAL::Module1::Absorber"])
        kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::CableTrough",
                                          "LArMgr::LAr::FCAL::Module2::Absorber",
                                          "LArMgr::LAr::FCAL::Module2::Rod"])
        kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::CableTrough",
                                          "LArMgr::LAr::FCAL::Module3::Absorber",
                                          "LArMgr::LAr::FCAL::Module3::Rod"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID",flags.Sim.ParticleID)
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", ["LArCalibrationHitInactive"])

    from LArG4Barrel.LArG4BarrelConfig import BarrelCalibrationCalculatorCfg, BarrelPresamplerCalibrationCalculatorCfg
    kwargs.setdefault("EMBPSCalibrationCalculator", result.getPrimaryAndMerge(BarrelPresamplerCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMBCalibrationCalculator", result.getPrimaryAndMerge(BarrelCalibrationCalculatorCfg(flags)).name)

    from LArG4EC.LArG4ECConfig import EMECPosInnerWheelCalibrationCalculatorCfg, EMECNegInnerWheelCalibrationCalculatorCfg, EMECPosOuterWheelCalibrationCalculatorCfg, EMECNegOuterWheelCalibrationCalculatorCfg
    kwargs.setdefault("EMECPosIWCalibrationCalculator", result.getPrimaryAndMerge(EMECPosInnerWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegIWCalibrationCalculator", result.getPrimaryAndMerge(EMECNegInnerWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECPosOWCalibrationCalculator", result.getPrimaryAndMerge(EMECPosOuterWheelCalibrationCalculatorCfg(flags)).name)
    kwargs.setdefault("EMECNegOWCalibrationCalculator", result.getPrimaryAndMerge(EMECNegOuterWheelCalibrationCalculatorCfg(flags)).name)

    from LArG4HEC.LArG4HECConfig import HECCalibrationWheelInactiveCalculatorCfg
    kwargs.setdefault("HECWheelInactiveCalculator", result.getPrimaryAndMerge(HECCalibrationWheelInactiveCalculatorCfg(flags)).name)

    from LArG4FCAL.LArG4FCALConfig import FCAL1CalibCalculatorCfg, FCAL2CalibCalculatorCfg, FCAL3CalibCalculatorCfg
    kwargs.setdefault("FCAL1CalibCalculator", result.getPrimaryAndMerge(FCAL1CalibCalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL2CalibCalculator", result.getPrimaryAndMerge(FCAL2CalibCalculatorCfg(flags)).name)
    kwargs.setdefault("FCAL3CalibCalculator", result.getPrimaryAndMerge(FCAL3CalibCalculatorCfg(flags)).name)

   
    result.setPrivateTools( CompFactory.LArG4.InactiveSDTool(name, **kwargs) )
    return result


def CalibrationDefaultCalculatorCfg(flags, name="CalibrationDefaultCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.CalibrationDefaultCalculator(name, **kwargs), primary = True)
    return result


def DeadMaterialCalibrationHitMergerCfg(flags, name="DeadMaterialCalibrationHitMerger", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("InputHits", ["LArCalibrationHitDeadMaterial_DEAD","LArCalibrationHitActive_DEAD","LArCalibrationHitInactive_DEAD"])
    kwargs.setdefault("OutputHits", "LArCalibrationHitDeadMaterial")
    result.addEventAlgo(CompFactory.LArG4.CalibrationHitMerger(name, **kwargs))
    return result
