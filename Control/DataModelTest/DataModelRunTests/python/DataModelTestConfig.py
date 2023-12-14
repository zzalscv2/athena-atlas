#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
#
# File: DataModelRunTests/python/DataModelTestConfig.py
# Author: snyder@bnl.gov
# Date: Nov 2023
# Purpose: Helpers for configuration tests.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython.PyAthenaComps import Alg, StatusCode
from AthenaCommon.Constants import INFO


#
# Common configuration flag settings.
# Takes an optional input file name and event count.
# Remaining keyword argument names are interpreted as stream names.  Example:
#  flags = DataModelTestFlags (infile = 'SimplePoolFile.root',
#                              Stream1 = 'SimplePoolFile2.root')
#
def DataModelTestFlags (infile = None, evtMax = 20, **kw):
    flags = initConfigFlags()
    flags.Exec.MaxEvents = evtMax
    flags.Exec.OutputLevel = INFO
    flags.Common.MsgSourceLength = 18

    # Disable FPE auditing.
    flags.Exec.FPE = -2

    # Set input/output files.
    if infile:
        flags.Input.Files = [infile]
    for stream, outfile in kw.items():
        flags.addFlag (f'Output.{stream}FileName', outfile)

    # Block input file peeking.
    from Campaigns.Utils import Campaign
    flags.Input.RunNumber = 0
    flags.Input.TimeStamp = 0
    flags.Input.ProcessingTags = []
    flags.Input.TypedCollections = []
    flags.Input.isMC = True
    flags.IOVDb.GlobalTag = ''
    flags.Input.MCCampaign = Campaign.Unknown
    return flags


#
# Common configuration for tests.
#
def DataModelTestCfg (flags, testName,
                      loadReadDicts = False,
                      loadWriteDicts = False,
                      EventsPerLB = None,
                      TimeStampInterval = None,
                      readCatalog = None):
    from AthenaConfiguration.MainServicesConfig import \
        MainServicesCfg, MessageSvcCfg
    cfg = MainServicesCfg (flags)
    cfg.merge (MessageSvcCfg (flags))
    cfg.getService("MessageSvc").debugLimit = 10000
    cfg.addService (CompFactory.ClassIDSvc (OutputLevel = INFO))
    cfg.addService (CompFactory.ChronoStatSvc (ChronoPrintOutTable = False,
                                               PrintUserTime = False,
                                               StatPrintOutTable = False))

    if flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']:
        # No input file --- configure like an event generator,
        # and make an xAODEventInfo.
        from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
        mckw = {}
        if EventsPerLB is not None:
            mckw['EventsPerLB'] = EventsPerLB
        if TimeStampInterval is not None:
            mckw['TimeStampInterval'] = TimeStampInterval
        cfg.merge (McEventSelectorCfg (flags, **mckw))

        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        cfg.merge (EventInfoCnvAlgCfg (flags, disableBeamSpot = True))
    elif not flags.Input.Files[0].endswith ('.bs'):
        # Configure reading.
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge (PoolReadCfg (flags))

    # Load dictionaries if requested.
    if loadWriteDicts:
        cfg.merge (LoadWriteDictsCfg (flags))
    if loadReadDicts:
        cfg.merge (LoadReadDictsCfg (flags))

    # Prevent races when we run tests in parallel in the same directory.
    fileCatalog = testName + '_catalog.xml'
    from AthenaPoolCnvSvc.PoolCommonConfig import PoolSvcCfg
    kw = {'WriteCatalog' : 'file:' + fileCatalog}
    if readCatalog:
        kw['ReadCatalog'] = ['file:' + readCatalog]
    cfg.merge (PoolSvcCfg (flags, **kw))
    import os
    try:
        os.remove (fileCatalog)
    except OSError:
        pass


    return cfg


#
# Configure an output stream.
#
def TestOutputCfg (flags, stream, itemList, typeNames = [], metaItemList = []):
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc = ComponentAccumulator()
    itemList = ['xAOD::EventInfo#EventInfo',
                'xAOD::EventAuxInfo#EventInfoAux.'] + itemList
    helperTools = []
    if typeNames:
        helperTools = [ CompFactory.xAODMaker.EventFormatStreamHelperTool(
            f'{stream}_EventFormatStreamHelperTool',
            Key = f'EventFormat{stream}',
            TypeNames = typeNames,
            DataHeaderKey = f'Stream{stream}') ]
        metaItemList = [ f'xAOD::EventFormat#EventFormat{stream}' ] + metaItemList
    acc.merge (OutputStreamCfg (flags, stream,
                                disableEventTag = True,
                                ItemList = itemList,
                                HelperTools = helperTools,
                                MetadataItemList = metaItemList))
    if typeNames:
        alg = acc.getEventAlgo (f'OutputStream{stream}')
        alg.WritingTool.SubLevelBranchName = '<key>'
        acc.getService ('AthenaPoolCnvSvc').PoolAttributes += ["DEFAULT_SPLITLEVEL='1'"]
    return acc
    
    


# Arrange to get dictionaries loaded for write tests.
# Do this as an algorithm so we can defer it to initialize().
# In some cases, loading DSOs during initial python processing
# can cause component loading to fail.
class LoadWriteDicts (Alg):
    def __init__ (self, name = 'LoadWriteDicts', **kw):
        return super(LoadWriteDicts, self).__init__ (name=name, **kw)
    def initialize (self):
        import ROOT
        ROOT.gROOT.SetBatch(True)
        import cppyy
        cppyy.load_library("libDataModelTestDataCommonDict")
        cppyy.load_library("libDataModelTestDataWriteDict")
        cppyy.load_library("libDataModelTestDataWriteCnvDict")
        ROOT.DMTest.B
        ROOT.DMTest.setConverterLibrary ('libDataModelTestDataWriteCnvPoolCnv.so')
        ROOT.DMTest.setTrigConverterLibrary ('libDataModelTestDataWriteSerCnv.so')
        return StatusCode.Success


def LoadWriteDictsCfg (flags):
    acc = ComponentAccumulator()
    acc.addEventAlgo (LoadWriteDicts())
    return acc


# Arrange to get dictionaries loaded for read tests.
# Do this as an algorithm so we can defer it to initialize().
# In some cases, loading DSOs during initial python processing
# can cause component loading to fail.
class LoadReadDicts (Alg):
    def __init__ (self, name = 'LoadReadDicts', **kw):
        return super(LoadReadDicts, self).__init__ (name=name, **kw)
    def initialize (self):
        import ROOT
        ROOT.gROOT.SetBatch(True)
        import cppyy
        cppyy.load_library("libDataModelTestDataCommonDict")
        cppyy.load_library("libDataModelTestDataReadDict")
        ROOT.DMTest.B
        ROOT.gROOT.GetClass('DMTest::HAuxContainer_v1')
        ROOT.gROOT.GetClass('DataVector<DMTest::H_v1>')
        ROOT.gROOT.GetClass('DMTest::HView_v1')
        ROOT.DMTest.setConverterLibrary ('libDataModelTestDataReadCnvPoolCnv.so')
        ROOT.DMTest.setTrigConverterLibrary ('libDataModelTestDataReadSerCnv.so')
        return StatusCode.Success

def LoadReadDictsCfg (flags):
    acc = ComponentAccumulator()
    acc.addEventAlgo (LoadReadDicts())
    return acc
