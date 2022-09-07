#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Configurable import ConfigurableRun3Behavior
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, appendCAtoAthena, conf2toConfigurable
from TrigEDMConfig.DataScoutingInfo import getFullHLTResultID
from libpyeformat_helper import SourceIdentifier, SubDetector
from RegionSelector import RegSelToolConfig

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


def StaticPEBInfoWriterToolCfg(name='StaticPEBInfoWriterTool'):
    def addROBs(self, robs):
        self.ROBList.extend(robs)

    def addSubDets(self, dets):
        self.SubDetList.extend([int(detid) for detid in dets])

    def addHLTResultToROBList(self, moduleId=getFullHLTResultID()):
        hltResultSID = SourceIdentifier(SubDetector.TDAQ_HLT, moduleId)
        self.addROBs([hltResultSID.code()])

    def addCTPResultToROBList(self, moduleId=0):
        ctpResultSID = SourceIdentifier(SubDetector.TDAQ_CTP, moduleId)
        self.addROBs([ctpResultSID.code()])

    CompFactory.StaticPEBInfoWriterTool.addROBs = addROBs
    CompFactory.StaticPEBInfoWriterTool.addSubDets = addSubDets
    CompFactory.StaticPEBInfoWriterTool.addHLTResultToROBList = addHLTResultToROBList
    CompFactory.StaticPEBInfoWriterTool.addCTPResultToROBList = addCTPResultToROBList

    tool = CompFactory.StaticPEBInfoWriterTool(name)

    return tool


def getRegSelTools(flags, detNames):
    '''
    Get a list of RegionSelector tools for given detector look-up tables to build list of ROBs
    in these detectors that intersect with the RoI. Special value 'All' can be also given
    in the detNames list to include all detectors available in RegionSelector.
    '''
    # To check Detector flags before adding RegSel tool configs, we need to map RegSel det names to Detector flag names
    _regSelToDetFlagMap = {
        # Calo
        'TTEM': 'Calo',
        'TTHEC': 'Calo',
        'FCALEM': 'LAr',
        'FCALHAD': 'LAr',
        'TILE': 'Tile',
    }
    # ID
    _regSelToDetFlagMap |= dict([(d,d) for d in ['Pixel', 'SCT', 'TRT']])
    # Muon
    _regSelToDetFlagMap |= dict([(d,d) for d in ['MDT', 'RPC', 'TGC', 'CSC', 'MM']])
    _regSelToDetFlagMap['STGC'] = 'sTGC'  # inconsistent capitalisation, regSelTool_STGC_Cfg should be regSelTool_sTGC_Cfg
    if 'All' in detNames:
        detNames = _regSelToDetFlagMap.keys()

    acc = ComponentAccumulator()
    regSelTools = []
    for det in detNames:
        if det=='sTGC':
            det='STGC'  # inconsistent capitalisation, regSelTool_STGC_Cfg should be regSelTool_sTGC_Cfg
        if det not in _regSelToDetFlagMap:
            raise RuntimeError('Cannot add detector "' + det + '" because it is not in _regSelToDetFlagMap')
        detFlag = 'Enable'+_regSelToDetFlagMap[det]
        detFlagCont = getattr(flags, 'Detector')
        detEnabled = getattr(detFlagCont, detFlag)
        if not detEnabled:
            _log.debug('addRegSelDets: skip adding detector "%s" because the flag Detector.%s is False', det, detFlag)
            continue
        funcName = f'regSelTool_{det}_Cfg'
        if not hasattr(RegSelToolConfig, funcName):
            raise RuntimeError('Cannot add detector "' + det + '", RegSelToolConfig does not have a function ' + funcName)
        func = getattr(RegSelToolConfig, funcName)
        if not callable(func):
            raise RuntimeError('Cannot add detector "' + det + '", RegSelToolConfig.' + funcName + ' is not callable')
        with ConfigurableRun3Behavior():
            regSelTools += [acc.popToolsAndMerge(func(flags))]
    acc.setPrivateTools(regSelTools)
    return acc


def RoIPEBInfoWriterToolCfg(name='RoIPEBInfoWriterTool'):
    def addRegSelDets(self, flags, detNames):
        '''Add RegionSelector tools for given detector look-up tables for RoI-based PEB'''
        acc = getRegSelTools(flags, detNames)
        self.RegionSelectorTools += [conf2toConfigurable(tool) for tool in acc.popPrivateTools()]
        # Hack: remove empty AthAlgSeq to avoid AthAlgSeq->TopAlg renaming throwing "AttributeError: 'AthSequencer' object attribute 'name' is read-only"
        acc._allSequences = [s for s in acc._allSequences if len(s.Members)>0]
        appendCAtoAthena(acc)

    def addROBs(self, robs):
        '''Add extra fixed list of ROBs independent of RoI'''
        self.ExtraROBs.extend(robs)

    def addSubDets(self, dets):
        '''Add extra fixed list of SubDets independent of RoI'''
        self.ExtraSubDets.extend([int(detid) for detid in dets])

    def addHLTResultToROBList(self, moduleId=getFullHLTResultID()):
        hltResultSID = SourceIdentifier(SubDetector.TDAQ_HLT, moduleId)
        self.addROBs([hltResultSID.code()])

    def addCTPResultToROBList(self, moduleId=0):
        ctpResultSID = SourceIdentifier(SubDetector.TDAQ_CTP, moduleId)
        self.addROBs([ctpResultSID.code()])

    CompFactory.RoIPEBInfoWriterTool.addRegSelDets = addRegSelDets
    CompFactory.RoIPEBInfoWriterTool.addROBs = addROBs
    CompFactory.RoIPEBInfoWriterTool.addSubDets = addSubDets
    CompFactory.RoIPEBInfoWriterTool.addHLTResultToROBList = addHLTResultToROBList
    CompFactory.RoIPEBInfoWriterTool.addCTPResultToROBList = addCTPResultToROBList

    tool = CompFactory.RoIPEBInfoWriterTool(name)

    return tool
