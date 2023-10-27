#
#   @file    RegSelToolConfig.py
#
#            configuration functions for the new RegSelTools 
#
#   @author  sutt 
#
#   @date    Sun  8 Mar 2020 03:27:57 GMT
#                 
#   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration#                 
#
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)
    
def _condAlgName(detector):
    return "RegSelCondAlg_"+detector

def _createRegSelCondAlg( detector,  CondAlgConstructor, isOnline ):
    """
    Creates conditions alg that provides data to a RegSel Tool
    """
    if detector == "TRT":
        condAlg = CondAlgConstructor( name = _condAlgName( detector ),
                                      PrintTable  = False,
                                      RegSelLUT = ("RegSelLUTCondData_"+detector) )
    else:
        condAlg = CondAlgConstructor( name = _condAlgName( detector ),
                                      ManagerName = detector,
                                      PrintTable  = False,
                                      RegSelLUT = ("RegSelLUTCondData_"+detector) )

    if detector == "MDT" and isOnline:
         condAlg.Conditions = "" 
    elif detector == "Pixel":
        condAlg.DetEleCollKey = "PixelDetectorElementCollection"
        condAlg.PixelCablingCondData = "PixelCablingCondData"
    elif detector == "SCT":
        condAlg.DetEleCollKey = "SCT_DetectorElementCollection"
        condAlg.SCT_CablingData = "SCT_CablingData"
    elif detector == "ITkPixel":
        condAlg.DetEleCollKey = "ITkPixelDetectorElementCollection"
        # No cabling data for ITk
        condAlg.PixelCablingCondData = ""
    elif detector == "ITkStrip":
        condAlg.DetEleCollKey = "ITkStripDetectorElementCollection"
        # No cabling data for ITk
        condAlg.SCT_CablingData = ""
    return condAlg

def _createRegSelTool( detector, enable ):
    """
    Creates RegSelTool and corresponding cond tool that is needed for its function

    If the enable flag is set - the tool is properly configured, else it is configured NOT to provide the data.

    """

    
    tool = CompFactory.RegSelTool(name="RegSelTool_"+detector)

    # should we enable the look up table access for this subsystem ?

    if not enable:
        # detector not configured so don't enable
        # lookup table access
        tool.Initialised = False
        return tool
        
    # add the lookup table to retrieve
        
    tool.RegSelLUT = "RegSelLUTCondData_"+detector # has to match wiht appropriate RegSelCondAlg
    tool.Initialised = True
    return tool



def _makeRegSelTool( detector, enable, CondAlgConstructor ):

    from AthenaCommon.AlgSequence import AthSequencer
    condseq = AthSequencer('AthCondSeq')

    if enable and not hasattr( condseq, _condAlgName( detector ) ):
        from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
        condseq += _createRegSelCondAlg( detector, CondAlgConstructor, athenaCommonFlags.isOnline )

    return _createRegSelTool( detector, enable )



# inner detector tools

def makeRegSelTool_Pixel() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.pixel_on()
    from InDetRegionSelector.InDetRegionSelectorConf import SiRegSelCondAlg
    return _makeRegSelTool( 'Pixel', enabled, SiRegSelCondAlg )

def makeRegSelTool_SCT() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.SCT_on()
    from InDetRegionSelector.InDetRegionSelectorConf import SiRegSelCondAlg
    return _makeRegSelTool( 'SCT', enabled, SiRegSelCondAlg )

def makeRegSelTool_TRT() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.TRT_on()
    from InDetRegionSelector.InDetRegionSelectorConf import TRT_RegSelCondAlg
    return _makeRegSelTool( 'TRT', enabled, TRT_RegSelCondAlg )


# muon spectrimeter tools 

def makeRegSelTool_MDT() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.MDT_on()
    from MuonRegionSelector.MuonRegionSelectorConf import MDT_RegSelCondAlg
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    if enabled and not athenaCommonFlags.isOnline:
        from AthenaCommon.AlgSequence import AthSequencer
        from MuonCondAlg.MuonTopCondAlgConfigRUN2 import MdtCondDbAlg
        condseq = AthSequencer('AthCondSeq')
        if not hasattr(condseq, "MdtCondDbAlg"): condseq+= MdtCondDbAlg()
    return _makeRegSelTool( "MDT", enabled, MDT_RegSelCondAlg )

def makeRegSelTool_RPC() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.RPC_on()
    from MuonRegionSelector.MuonRegionSelectorConf import RPC_RegSelCondAlg
    return _makeRegSelTool( "RPC", enabled, RPC_RegSelCondAlg )

def makeRegSelTool_TGC() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.TGC_on()
    from MuonRegionSelector.MuonRegionSelectorConf import TGC_RegSelCondAlg
    return _makeRegSelTool( "TGC", enabled, TGC_RegSelCondAlg )

def makeRegSelTool_CSC() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.CSC_on()
    from MuonRegionSelector.MuonRegionSelectorConf import CSC_RegSelCondAlg
    return _makeRegSelTool( "CSC", enabled, CSC_RegSelCondAlg )


# new small wheel 

def makeRegSelTool_MM() :
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    from AthenaCommon.DetFlags import DetFlags
    enabled = False
    if MuonGeometryFlags.hasMM() :
        enabled = DetFlags.detdescr.MM_on()
    from MuonRegionSelector.MuonRegionSelectorConf import MM_RegSelCondAlg
    return _makeRegSelTool( "MM", enabled, MM_RegSelCondAlg )

def makeRegSelTool_sTGC() :
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    from AthenaCommon.DetFlags import DetFlags
    enabled = False
    if MuonGeometryFlags.hasSTGC() :
        enabled = DetFlags.detdescr.sTGC_on()
    from MuonRegionSelector.MuonRegionSelectorConf import sTGC_RegSelCondAlg
    return _makeRegSelTool( "sTGC", enabled, sTGC_RegSelCondAlg )



# calorimeter 

def makeRegSelTool_TTEM() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.Calo_on()
    from LArRecUtils.LArRoIMapCondAlgDefault import LArRoIMapCondAlgDefault
    LArRoIMapCondAlgDefault()
    from LArRegionSelector.LArRegionSelectorConf import RegSelCondAlg_LAr
    return _makeRegSelTool( "TTEM", enabled, RegSelCondAlg_LAr )

def makeRegSelTool_TTHEC() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.Calo_on()
    from LArRecUtils.LArRoIMapCondAlgDefault import LArRoIMapCondAlgDefault
    LArRoIMapCondAlgDefault()
    from LArRegionSelector.LArRegionSelectorConf import RegSelCondAlg_LAr
    return _makeRegSelTool( "TTHEC", enabled, RegSelCondAlg_LAr )
            
def makeRegSelTool_FCALEM() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.Calo_on()
    from LArRecUtils.LArRoIMapCondAlgDefault import LArRoIMapCondAlgDefault
    LArRoIMapCondAlgDefault()
    from LArRegionSelector.LArRegionSelectorConf import RegSelCondAlg_LAr
    return _makeRegSelTool( "FCALEM", enabled, RegSelCondAlg_LAr )

def makeRegSelTool_FCALHAD() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.Calo_on()
    from LArRecUtils.LArRoIMapCondAlgDefault import LArRoIMapCondAlgDefault
    LArRoIMapCondAlgDefault()
    from LArRegionSelector.LArRegionSelectorConf import RegSelCondAlg_LAr
    return _makeRegSelTool( "FCALHAD", enabled, RegSelCondAlg_LAr )

# tile calorimeter

def makeRegSelTool_TILE() :
    from AthenaCommon.DetFlags import DetFlags
    enabled = DetFlags.detdescr.Calo_on()
    from TileRawUtils.TileRawUtilsConf import RegSelCondAlg_Tile
    return _makeRegSelTool( "TILE", enabled, RegSelCondAlg_Tile )


##### new JO counterparts

def regSelToolCfg(flags, detector, algorithm, readout_geometry=None, conditions=None):
    ca = ComponentAccumulator()
    if readout_geometry:
        ca.merge(readout_geometry)
    if conditions:
        ca.merge(conditions)
    ca.setPrivateTools(_createRegSelTool(detector, True))
    the_alg = _createRegSelCondAlg(detector, algorithm, flags.Common.isOnline)
    if detector == "MDT" and flags.Common.isOnline:
        the_alg.Conditions = ""
    ca.addCondAlgo(the_alg)
    return ca

# inner detector
@AccumulatorCache
def regSelTool_Pixel_Cfg(flags):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg
    return regSelToolCfg(flags, "Pixel", CompFactory.SiRegSelCondAlg,
                         readout_geometry=PixelReadoutGeometryCfg(flags), conditions=PixelCablingCondAlgCfg(flags))

@AccumulatorCache
def regSelTool_SCT_Cfg(flags):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    from SCT_Cabling.SCT_CablingConfig import SCT_CablingCondAlgCfg
    return regSelToolCfg(flags, "SCT", CompFactory.SiRegSelCondAlg,
                         readout_geometry=SCT_ReadoutGeometryCfg(flags), conditions=SCT_CablingCondAlgCfg(flags))

@AccumulatorCache
def regSelTool_TRT_Cfg(flags):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    return regSelToolCfg(flags, "TRT", CompFactory.TRT_RegSelCondAlg,
                         readout_geometry=TRT_ReadoutGeometryCfg(flags))

# ITk
@AccumulatorCache
def regSelTool_ITkPixel_Cfg(flags):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    return regSelToolCfg(flags, "ITkPixel", CompFactory.SiRegSelCondAlg,
                         readout_geometry=ITkPixelReadoutGeometryCfg(flags))

@AccumulatorCache
def regSelTool_ITkStrip_Cfg(flags):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    return regSelToolCfg(flags, "ITkStrip", CompFactory.SiRegSelCondAlg,
                         readout_geometry=ITkStripReadoutGeometryCfg(flags))


# muon spectrometer
@AccumulatorCache
def regSelTool_MDT_Cfg(flags):
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(MDTCablingConfigCfg(flags))
    if not flags.Common.isOnline:
        conditions.merge(MdtCondDbAlgCfg(flags))

    return regSelToolCfg(flags, "MDT", CompFactory.MDT_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_RPC_Cfg(flags):
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(RPCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "RPC", CompFactory.RPC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TGC_Cfg(flags):
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(TGCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "TGC", CompFactory.TGC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_CSC_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    from MuonConfig.MuonCablingConfig import CSCCablingConfigCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(CSCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "CSC", CompFactory.CSC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_STGC_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    return regSelToolCfg(flags, "sTGC", CompFactory.sTGC_RegSelCondAlg,
                         conditions=MuonGeoModelCfg(flags))

@AccumulatorCache
def regSelTool_MM_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    return regSelToolCfg(flags, "MM", CompFactory.MM_RegSelCondAlg,
                         conditions=MuonGeoModelCfg(flags))


# calo
@AccumulatorCache
def regSelTool_TTEM_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "TTEM", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TTHEC_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "TTHEC", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_FCALEM_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "FCALEM", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_FCALHAD_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "FCALHAD", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TILE_Cfg(flags):
    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    return regSelToolCfg(flags, "TILE", CompFactory.RegSelCondAlg_Tile,
                         conditions=TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True))


def regSelToolsCfg(flags, detNames):
    '''
    Get a list of RegionSelector tools for given detector look-up tables if the corresponding Detector flags are enabled
    '''
    acc = ComponentAccumulator()
    regSelTools = []
    for det in detNames:
        flagName = det
        if det in ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD']:
            flagName = 'LAr'
        elif det == 'TILE':
            flagName = 'Tile'
        elif det == 'STGC':
            flagName = 'sTGC'
        detFlag = f'Enable{flagName}'
        detFlagCont = getattr(flags, 'Detector')
        detEnabled = getattr(detFlagCont, detFlag)
        if not detEnabled:
            _log.debug('regSelToolsCfg: skip adding detector "%s" because the flag Detector.%s is False', det, detFlag)
            continue
        funcName = f'regSelTool_{det}_Cfg'
        func = globals().get(funcName, None)
        if func is None:
            raise RuntimeError('regSelToolsCfg: cannot add detector "' + det + '", RegSelToolConfig does not have a function ' + funcName)
        regSelTools += [acc.popToolsAndMerge(func(flags))]
    acc.setPrivateTools(regSelTools)
    return acc


# unit test
if __name__=='__main__':
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration import DetectorConfigFlags, TestDefaults
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    import sys

    flags = initConfigFlags()
    flags.Input.Files = TestDefaults.defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = TestDefaults.defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-02'
    flags.Exec.MaxEvents = 1
    flags.Concurrency.NumThreads = 1

    # Strict dependency checking
    flags.Input.FailOnUnknownCollections = True
    flags.Scheduler.AutoLoadUnmetDependencies = False

    detNames = sys.argv[1:]
    # Toggle detectors. Note that we are not toggling the geometry because this would
    # result in un-physical configurations that we are not supporting in reco anyway.
    DetectorConfigFlags.disableDetectors(flags, DetectorConfigFlags.allDetectors +
                                         list(DetectorConfigFlags.allGroups.keys()), toggle_geometry=False)
    DetectorConfigFlags.enableDetectors(flags, detNames, toggle_geometry=False)
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(ByteStreamReadCfg(flags))

    if 'LAr' in detNames:
        detNames.remove('LAr')
        detNames += ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD']
    if 'Tile' in detNames:
        detNames.remove('Tile')
        detNames.append('TILE')
    if 'sTGC' in detNames:
        detNames.remove('sTGC')
        detNames.append('STGC')

    toolsCfg = regSelToolsCfg(flags, detNames)
    alg = CompFactory.RegSelToolTester(
        RegionSelectorTools = acc.popToolsAndMerge(toolsCfg) )
    acc.addEventAlgo(alg, sequenceName='AthAlgSeq')

    acc.run()
