# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, ProductionStep
from AthenaCommon.Logging import logging


def OutputStreamCfg(flags, streamName, ItemList=[], MetadataItemList=[],
                    disableEventTag=False, trigNavThinningSvc=None, AcceptAlgs=[]):
   eventInfoKey = "EventInfo"
   if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
      eventInfoKey = f"{flags.Overlay.BkgPrefix}EventInfo"

   msg = logging.getLogger("OutputStreamCfg")
   outputStreamName = f"Stream{streamName}"
   flagName = f"Output.{streamName}FileName"
   if flags.hasFlag(flagName):
      fileName = flags._get(flagName)
   else:
      fileName = f"my{streamName}.pool.root"
      msg.info("No file name predefined for stream %s. Using %s", streamName, fileName)

   if fileName in flags.Input.Files:
      raise ConfigurationError("Same name for input and output file %s" % fileName)

   result = ComponentAccumulator(sequence = CompFactory.AthSequencer("AthOutSeq", StopOverride=True))

   # Set up AthenaPoolCnvSvc through PoolWriteCfg
   from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
   result.merge(PoolWriteCfg(flags))

   # define athena output stream
   writingTool = CompFactory.AthenaOutputStreamTool(f"Stream{streamName}Tool",
                                                    DataHeaderKey=outputStreamName)

   # If we're running in augmentation mode, configure the writing tool accordingly
   parentStream = f"Output.{streamName}ParentStream"
   childStream = f"Output.{streamName}ChildStream"
   if flags.hasFlag(childStream):
      writingTool.SaveDecisions = True
   elif flags.hasFlag(parentStream):
      disableEventTag = True
      writingTool.OutputCollection = f"POOLContainer_{streamName}"
      writingTool.PoolContainerPrefix = f"CollectionTree_{streamName}"
      writingTool.MetaDataOutputCollection = f"MetaDataHdr_{streamName}"
      writingTool.MetaDataPoolContainerPrefix = f"MetaData_{streamName}"
      msg.info("Stream %s running in augmentation mode with %s as parent", streamName, flags._get(parentStream))

   # In DAOD production the EventInfo is prepared specially by the SlimmingHelper to ensure it is written in AuxDyn form
   # So for derivations the ItemList from the SlimmingHelper alone is used without the extra EventInfo items
   finalItemList = []
   if any(name in streamName for name in {"DAOD_", "D2AOD_"}):
      finalItemList = ItemList
   else:
      finalItemList = [f"xAOD::EventInfo#{eventInfoKey}", f"xAOD::EventAuxInfo#{eventInfoKey}Aux."] + ItemList 

   outputStream = CompFactory.AthenaOutputStream(
      f"OutputStream{streamName}",
      StreamName=outputStreamName,
      WritingTool=writingTool,
      ItemList=finalItemList,
      MetadataItemList=MetadataItemList,
      OutputFile=fileName,
   )
   outputStream.AcceptAlgs += AcceptAlgs
   outputStream.ExtraOutputs += [("DataHeader", f"StoreGateSvc+{outputStreamName}")]
   if flags.Scheduler.CheckOutputUsage and flags.Concurrency.NumThreads > 0:
      outputStream.ExtraInputs = [tuple(l.split('#')) for l in finalItemList if '*' not in l and 'Aux' not in l]
      # Ignore dependencies
      from AthenaConfiguration.MainServicesConfig import OutputUsageIgnoreCfg
      result.merge(OutputUsageIgnoreCfg(flags, outputStream.name))

   result.addService(CompFactory.StoreGateSvc("MetaDataStore"))
   outputStream.MetadataStore = result.getService("MetaDataStore")
   outputStream.MetadataItemList += [
      f"EventStreamInfo#{outputStreamName}",
      "IOVMetaDataContainer#*",
   ]

   streamInfoTool = CompFactory.MakeEventStreamInfo(f"Stream{streamName}_MakeEventStreamInfo")
   streamInfoTool.Key = outputStreamName
   streamInfoTool.DataHeaderKey = outputStreamName
   streamInfoTool.EventInfoKey = eventInfoKey
   outputStream.HelperTools.append(streamInfoTool)

   # Make EventFormat object
   from xAODEventFormatCnv.EventFormatConfig import EventFormatCfg
   result.merge(EventFormatCfg(flags,
                               stream=outputStream,
                               streamName=outputStreamName))

   # Setup FileMetaData
   from xAODMetaDataCnv.FileMetaDataConfig import FileMetaDataCfg
   result.merge(FileMetaDataCfg(flags,
                                stream=outputStream,
                                streamName=outputStreamName))

   # Setup additional MetaData

   # ======================================================
   # TODO:
   # ======================================================
   # For the time being we're adding common MetaData items
   # and configure the necessary tools/services en masse.
   # Ideally, we should introduce a self-sufficienct config
   # for each item and merge them here.
   # ======================================================
   if any([ x in streamName for x in ['AOD','ESD'] ]):
      # LumiBlockMetaDataTool seems to cause crashes in MP derivation jobs
      # As done in RecExCommon_topOptions.py use the algorithm in MP jobs
      # This needs to be checked and confirmed...
      mdToolNames = []
      # Propagate cutbookkeepers
      if 'CutBookkeepers' in flags.Input.MetadataItems:
         mdToolNames.append('BookkeeperTool')

      if flags.Input.Format == Format.BS and not flags.Common.isOnline:
         from LumiBlockComps.CreateLumiBlockCollectionFromFileConfig import CreateLumiBlockCollectionFromFileCfg
         result.merge(CreateLumiBlockCollectionFromFileCfg(flags))
      elif "LumiBlock" in flags.Input.MetadataItems:
         mdToolNames.append('LumiBlockMetaDataTool')

      outputStream.MetadataItemList += ['xAOD::TriggerMenuContainer#*'
                                       ,'xAOD::TriggerMenuAuxContainer#*'
                                       ,'xAOD::TriggerMenuJsonContainer#*'
                                       ,'xAOD::TriggerMenuJsonAuxContainer#*'
                                       ,'xAOD::LumiBlockRangeContainer#*'
                                       ,'xAOD::LumiBlockRangeAuxContainer#*'
                                       ,'ByteStreamMetadataContainer#*'
                                       ,'xAOD::TruthMetaDataContainer#TruthMetaData'
                                       ,'xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux.']
      # TODO: temporary   
      from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowOutputList
      outputStream.MetadataItemList += CutFlowOutputList(flags)

      from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
      result.merge(MetaDataSvcCfg(flags,
                                  tools = [CompFactory.xAODMaker.TriggerMenuMetaDataTool('TriggerMenuMetaDataTool')],
                                  toolNames = mdToolNames))

   # Support for MT thinning.
   thinningCacheTool = CompFactory.Athena.ThinningCacheTool(f"ThinningCacheTool_Stream{streamName}",
                                                            StreamName=outputStreamName)
   if trigNavThinningSvc is not None:
      thinningCacheTool.TrigNavigationThinningSvc = trigNavThinningSvc
   outputStream.HelperTools.append(thinningCacheTool)

   # Event Tag
   if not disableEventTag:
      key = "SimpleTag"
      outputStream.WritingTool.AttributeListKey=key

      propagateInputAttributeList = False
      if "AthenaAttributeList#Input" in flags.Input.TypedCollections:
         from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
         result.merge(SGInputLoaderCfg(flags, ["AthenaAttributeList#Input"]))
         propagateInputAttributeList = True

      # build eventinfo attribute list
      tagBuilder = CompFactory.EventInfoTagBuilder(AttributeList=key,
                                                   Tool=CompFactory.EventInfoAttListTool(),
                                                   EventInfoKey=eventInfoKey,
                                                   PropagateInput=propagateInputAttributeList)
      result.addEventAlgo(tagBuilder)

   # For xAOD output
   if "AOD" in streamName:
      outputStream.WritingTool.SubLevelBranchName = "<key>"

   result.addEventAlgo(outputStream, domain='IO')
   return result


def addToESD(flags, itemOrList, **kwargs):
   """
   Adds items to ESD stream

   The argument can be either list of items or just one item
   if further customisations are needed for output stream they can be passed via kwargs

   returns CA to be merged i.e.: result.merge(addToESD(flags, "xAOD::CoolObject"))
   """
   if not flags.Output.doWriteESD:
      return ComponentAccumulator()
   items = [itemOrList] if isinstance(itemOrList, str) else itemOrList
   return OutputStreamCfg(flags, "ESD", ItemList=items, **kwargs)


def addToAOD(flags, itemOrList, **kwargs):
   """
   Adds items to AOD stream

   @see add addToESD
   """
   if not flags.Output.doWriteAOD:
      return ComponentAccumulator()
   items = [itemOrList] if isinstance(itemOrList, str) else itemOrList
   return OutputStreamCfg(flags, "AOD", ItemList=items, **kwargs)
