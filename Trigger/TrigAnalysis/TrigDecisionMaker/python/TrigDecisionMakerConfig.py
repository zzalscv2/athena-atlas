# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from TrigDecisionMaker.TrigDecisionMakerConf import TrigDec__TrigDecisionMaker
from TrigDecisionMaker.TrigDecisionMakerConf import TrigDec__TrigDecisionMakerMT
from AthenaCommon.Logging import logging
from AthenaCommon.AppMgr import ToolSvc

class TrigDecisionMaker( TrigDec__TrigDecisionMaker ):
    __slots__ = []
    def __init__(self, name = "TrigDecMaker"):
        super( TrigDecisionMaker, self ).__init__( name )
        log = logging.getLogger( 'TrigDecisionMaker' )
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        log.info("Setting UseNewConfig to %s (based off of ConfigFlags.Trigger.doEDMVersionConversion)", ConfigFlags.Trigger.doEDMVersionConversion)
        self.Lvl1ResultAccessTool.UseNewConfig = ConfigFlags.Trigger.doEDMVersionConversion
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        if hasattr(svcMgr,'DSConfigSvc'):
            # this case is still needed for reading Run 2 configuration from the TriggerDB
            self.Lvl1ResultAccessTool.LVL1ConfigSvc = "TrigConfigSvc"



class TrigDecisionMakerMT( TrigDec__TrigDecisionMakerMT ):
    __slots__ = []
    def __init__(self, name = "TrigDecMakerMT"):
        super( TrigDecisionMakerMT, self ).__init__( name )
        log = logging.getLogger( 'TrigDecisionMakerMT' )
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        log.info("Setting UseNewConfig to %s", ConfigFlags.Trigger.readLVL1FromJSON)
        self.UseNewConfigL1 = ConfigFlags.Trigger.readLVL1FromJSON
        self.Lvl1ResultAccessTool.UseNewConfig = ConfigFlags.Trigger.readLVL1FromJSON
        # Schedule also the prescale conditions algs
        from AthenaCommon.Configurable import Configurable
        Configurable.configurableRun3Behavior += 1
        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, appendCAtoAthena
        from TrigConfigSvc.TrigConfigSvcCfg import  L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
        acc = ComponentAccumulator()
        acc.merge( L1PrescaleCondAlgCfg( ConfigFlags ) )
        acc.merge( HLTPrescaleCondAlgCfg( ConfigFlags ) )
        appendCAtoAthena( acc )
        Configurable.configurableRun3Behavior -= 1

# Following not yet ported to the AthenaMT / Run 3 alg

class TrigDecisionStream ( object) :
    def __init__ ( self, streamName = "Stream1", fileName = "HLT.root",
                   catalog = "xmlcatalog_file:Catalog1.xml",
                   store = None) :
        import AthenaPoolCnvSvc.WriteAthenaPool  # noqa: F401
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr.PoolSvc.WriteCatalog = catalog

        # revert later from OutputStreamAthenaPool.CreateOutputStreams import createOutputStream
        # revert later self.stream = createOutputStream( streamName )

        from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
        self.stream = AthenaPoolOutputStream( streamName )

        self.stream.OutputFile = fileName

        if store is not None :
            self.stream.Store = store
        else :
            from StoreGate.StoreGateConf import StoreGateSvc
            self.stream.Store = StoreGateSvc( "StoreGateSvc" )

        TrigDecisionStream.setItemList(self.stream)

    def setItemList(stream) :
        stream.ItemList += [ "TrigDec::TrigDecision#TrigDecision" ]
    setItemList = staticmethod(setItemList)

    def stream(self) :
        return self.stream

class TrigConditionStream ( object) :
    def __init__ ( self, streamName = "Stream2", fileName = "HLT.root",
                   catalog = "xmlcatalog_file:Catalog1.xml",
                   store = None ) :

        import AthenaPoolCnvSvc.WriteAthenaPool  # noqa: F401
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        from PoolSvc.PoolSvcConf import PoolSvc
        svcMgr += PoolSvc()
        svcMgr.PoolSvc.WriteCatalog = catalog

        from OutputStreamAthenaPool.CreateOutputStreams import AthenaPoolOutputConditionStream
        self.stream = AthenaPoolOutputConditionStream( streamName )

        from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
        self.stream = AthenaPoolOutputStream( streamName )

        self.stream.OutputFile = fileName

        if store is not None :
            self.stream.Store = store
        else :
            from StoreGate.StoreGateConf import StoreGateSvc
            self.stream.Store = StoreGateSvc( "DetectorStore" )

        TrigConditionStream.setItemList(self.stream)

    def setItemList(stream) :
        pass
    setItemList = staticmethod(setItemList)

    def stream(self) :
        return self.stream


class WriteTrigDecisionToFile ( object ) :
    def __init__ ( self, fileName = "TrigDec.root",
                   catalog = "xmlcatalog_file:Catalog1.xml" ) :

        from AthenaCommon.AlgSequence import AlgSequence
        TopAlg = AlgSequence()

        self.TrigDecMaker    = TrigDecisionMaker('TrigDecMaker')

        TopAlg += self.TrigDecMaker

        from StoreGate.StoreGateConf import StoreGateSvc
        sgStore = StoreGateSvc("StoreGateSvc")

        self.TrigDecStream  = TrigDecisionStream ("Stream1", fileName, catalog, sgStore)


class WriteTrigDecisionToStream ( object ) :
    def __init__ ( self, decStream, condStream ) :

        from AthenaCommon.AlgSequence import AlgSequence
        TopAlg = AlgSequence()

        self.TrigDecMaker    = TrigDecisionMaker('TrigDecMaker')

        TopAlg += self.TrigDecMaker

        decStream.setItemList(decStream)
        condStream.setItemList(condStream)

        self.TrigDecStream  = decStream
        self.TrigCondStream = condStream


class WritexAODTrigDecision ( object ) :
    def __init__(self):
        from AthenaCommon.AlgSequence import AlgSequence
        TopAlg = AlgSequence()

        from xAODTriggerCnv.xAODTriggerCnvConf import xAODMaker__TrigDecisionCnvAlg
        alg = xAODMaker__TrigDecisionCnvAlg()

        # In order for the conversion to work we need to setup the TrigDecisionTool such that it uses the old decision
        ToolSvc.TrigDecisionTool.UseAODDecision = True
        ToolSvc.TrigDecisionTool.TrigDecisionKey = "TrigDecision"


        from AthenaCommon.Logging import logging  # loads logger
        log = logging.getLogger( 'WritexAODTrigDecision' )
        log.info('TrigDecisionTool setup to use old decision')
                
        alg.xAODKey = "xTrigDecision"
        TopAlg += alg
        
        from xAODTriggerCnv.xAODTriggerCnvConf import xAODMaker__TrigNavigationCnvAlg
        navAlg = xAODMaker__TrigNavigationCnvAlg('TrigNavigationCnvAlg')
        TopAlg += navAlg

        log.info('TrigDecision writing to xAOD enabled')



class WriteTrigDecision ( object ) :
    def __init__ ( self, AODItemList = None, ESDItemList = None, doxAOD = True) :

        from AthenaCommon.AlgSequence import AlgSequence
        TopAlg = AlgSequence()

        self.TrigDecMaker    = TrigDecisionMaker('TrigDecMaker')

        TopAlg += self.TrigDecMaker

        if AODItemList is not None : self.addItemsToList(AODItemList)
        if ESDItemList is not None : self.addItemsToList(ESDItemList)

        from AthenaCommon.Logging import logging  # loads logger
        log = logging.getLogger( 'WriteTrigDecisionToAOD' )

        log.info('TrigDecision writing enabled')
        
        WritexAODTrigDecision()


    def addItemsToList(self, itemList) :
        itemList += [ "TrigDec::TrigDecision#*" ]





class ReadTrigDecisionFromFile ( object ) :
    def __init__ ( self, fileName = "TrigDec.root",
                   catalog = "xmlcatalog_file:Catalog1.xml" ) :

        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        import AthenaPoolCnvSvc.ReadAthenaPool  # noqa: F401

        svcMgr.EventSelector.InputCollections = [ fileName ]
        svcMgr.PoolSvc.ReadCatalog = [ catalog ]
