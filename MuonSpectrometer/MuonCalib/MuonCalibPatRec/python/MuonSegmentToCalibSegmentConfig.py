# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory



def MuonSegmentToCalibSegmentCfg(flags, name = "MuonSegmentToCalibSegmentAlg", **kwargs):
    """Set up to create the MuonCalibSegment from MuonSegment

    Args:
        flags:      Job configuration flags
        
    Services:
        #Muon::MdtDriftCircleOnTrackCreator
        MdtCalibrationTool
        MuonIdHelper

    Returns:
        A component accumulator fragment containing the components required to read 
        from Muon calibration stream bytestream data. Should be merged into main job configuration.
    """
    result = ComponentAccumulator()

     #setup the tools
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg
    from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg, MdtCalibrationDbToolCfg

    result.merge(MuonIdHelperSvcCfg(configFlags))
    result.merge(MuonEDMHelperSvcCfg(configFlags))

    muonSegmentToCalibSegment = CompFactory.MuonCalib.MuonSegmentToCalibSegment(**kwargs, CalibrationTool=result.popToolsAndMerge(MdtCalibrationToolCfg(configFlags, TimeWindowSetting = 2)))
    #muonSegmentToCalibSegment.MdtCalibrationTool
    result.addEventAlgo(muonSegmentToCalibSegment)

    return result

    #Muon::MdtDriftCircleOnTrackCreator