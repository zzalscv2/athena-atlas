# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from OutputStreamAthenaPool.OutputStreamConfig import addToMetaData

def InfileMetaDataCfg(flags, streamName="", AcceptAlgs=[]):
    """
    The main configuration for setting up Metadata for the stream named streamName

    Currently this is a single method that is used in all workflows
    However, it would be more maintainable/preferable to split it into custom ones
    For example, one for simulation/digitization/overlay, another for reco etc.
    The additional argument, AcceptAlgs, is needed for workflows with custom kernels

    Returns CA to be merged
    """

    # Bare CA
    result = ComponentAccumulator()
    if not isinstance(streamName, str) or not streamName:
        return result

    # Internal caches of tools
    helperTools = []
    mdTools = []
    mdToolNames = []

    # FileMetaData items
    MetadataItemList = ["xAOD::FileMetaData#FileMetaData"
                       ,"xAOD::FileMetaDataAuxInfo#FileMetaDataAux."]

    # FileMetaDataCreatorTool and FileMetaDataTool work together
    # Therefore, we want both of them configured
    helperTools.append(
        CompFactory.xAODMaker.FileMetaDataCreatorTool(
            f"Stream{streamName}_FileMetaDataCreatorTool",
            OutputKey="FileMetaData",
            StreamName=f"Stream{streamName}",
        )
    )

    if "FileMetaData" in flags.Input.MetadataItems:
        mdToolNames.append("xAODMaker::FileMetaDataTool")

    # Add MetaData that are relevant to downstream formats: ESD, AOD, and DAOD
    if not flags.Output.doWriteHITS and not flags.Output.doWriteRDO:

        # Trigger MetaData items
        mdTools.append(CompFactory.xAODMaker.TriggerMenuMetaDataTool("TriggerMenuMetaDataTool"))
        MetadataItemList += ["xAOD::TriggerMenuContainer#*"
                            ,"xAOD::TriggerMenuAuxContainer#*"
                            ,"xAOD::TriggerMenuJsonContainer#*"
                            ,"xAOD::TriggerMenuJsonAuxContainer#*"]

        # Collision data specific MetaData items
        if not flags.Input.isMC:
            MetadataItemList += ["ByteStreamMetadataContainer#*"
                                ,"xAOD::LumiBlockRangeContainer#*"
                                ,"xAOD::LumiBlockRangeAuxContainer#*"]
        # MC specific MetaData items
        else:
            MetadataItemList += ["xAOD::TruthMetaDataContainer#TruthMetaData"
                                ,"xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux."]

        # CutFlow related MetaData items
        # Now, I think this needs the CutFlowSvc to be set up
        # For the time being this is done in the relevant skeleton, e.g. Derivations
        # These should be reconciled, i.e. move that here and configure via flags
        from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowOutputList
        MetadataItemList += CutFlowOutputList(flags)

        # Data specific MetaData components
        if not flags.Input.isMC:
            # Here we're running RAWtoALL so we're in creation mode
            if flags.Input.Format == Format.BS and not flags.Common.isOnline:
                # Configure ByteStream related components
                from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
                result.merge(ByteStreamReadCfg(flags))
                # Configure LumiBlock Creation Algorithm
                from LumiBlockComps.CreateLumiBlockCollectionFromFileConfig import CreateLumiBlockCollectionFromFileCfg
                result.merge(CreateLumiBlockCollectionFromFileCfg(flags))
            elif "LumiBlock" in flags.Input.MetadataItems:
                # Propagate LumiBlock data if it exists in the input
                # Do we always want this tool instead?
                mdToolNames.append("LumiBlockMetaDataTool")
        # MC specific MetaData components
        else:
            # Configure TruthMetaDataTool
            mdTools.append(CompFactory.xAODMaker.TruthMetaDataTool("TruthMetaDataTool"))

        # Propagate CutBookkeepers if it exists in the input
        if "CutBookkeepers" in flags.Input.MetadataItems:
           mdToolNames.append("BookkeeperTool")

    # Configure the relevant output stream
    result.merge(
        addToMetaData(
                      flags,
                      streamName=streamName,
                      itemOrList=MetadataItemList,
                      AcceptAlgs=AcceptAlgs,
                      HelperTools=helperTools,
                     )
    )

    # Configure the MetaDataSvc and pass the relevant tools
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    result.merge(MetaDataSvcCfg(flags, tools = mdTools, toolNames = mdToolNames))

    # Finally return the CA
    return result
