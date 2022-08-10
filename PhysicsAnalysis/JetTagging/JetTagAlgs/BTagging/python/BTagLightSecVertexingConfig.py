# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetTagTools.JetFitterVariablesFactoryConfig import JetFitterVariablesFactoryCfg
#from BTagging.MSVVariablesFactoryConfig import MSVVariablesFactoryCfg

def BTagLightSecVtxToolCfg(flags, Name, JetCollection, VxSecVertexInfoNameList, secVtxFinderxAODBaseNameList, secVtxFinderTrackNameList, PrimaryVertexCollectionName="", TimeStamp = "", **options):
    """Adds a SecVtxTool instance and registers it.

    input: name:               The tool's name.
           JetCollection       The name of the jet collections.
           ToolSvc:            The ToolSvc instance.
           options:            Python dictionary of options to be passed to the SecVtxTool.
    output: The tool.
    If outputObjs is set, then it is filled with objects written to SG."""
    acc = ComponentAccumulator()

    OutputFilesJFVxname = "JFVtx"
    OutputFilesJFVxFlipname = "JFVtxFlip"
    OutputFilesSVname = "SecVtx"
    OutputFilesSVFlipname = 'SecVtxFlip'
    if TimeStamp:
        TimeStamp = '_' + TimeStamp

    jetFitterVF = acc.popToolsAndMerge(JetFitterVariablesFactoryCfg('JFVarFactory'))

    #Keep this commented out for the moment: ATLASRECTS-5247
    #inDetVKalMultiVxInJetTool = acc.popToolsAndMerge(InDetVKalVxInJetToolCfg("IDVKalMultiVxInJet", MSV = True))
    #secVtxFinderList.append(inDetVKalMultiVxInJetTool)
    #secVtxFinderTrackNameList.append('BTagTrackToJetAssociatorBB')
    #secVtxFinderxAODBaseNameList.append('MSV')
    #varFactory = acc.popToolsAndMerge(MSVVariablesFactoryCfg("MSVVarFactory"))

    options = {}
    options.setdefault('SecVtxFinderTrackNameList', secVtxFinderTrackNameList)
    options.setdefault('SecVtxFinderxAODBaseNameList', secVtxFinderxAODBaseNameList)
    options['BTagVxSecVertexInfoNames'] = VxSecVertexInfoNameList
    options.setdefault('vxPrimaryCollectionName', PrimaryVertexCollectionName)
    options.setdefault('JetFitterVariableFactory', jetFitterVF)
    options['JetSecVtxLinkName'] = JetCollection + '.' + OutputFilesSVname
    options['JetJFVtxLinkName'] = JetCollection + '.' + OutputFilesJFVxname
    
    if flags.BTagging.RunFlipTaggers is True:
        options['JetJFFlipVtxLinkName'] = JetCollection + '.' + OutputFilesJFVxFlipname
        options['JetSecVtxFlipLinkName'] = JetCollection + '.' + OutputFilesSVFlipname

    #options.setdefault('MSVVariableFactory', varFactory)
    options['name'] = Name+TimeStamp

    tool = CompFactory.Analysis.BTagLightSecVertexing(**options)
    
    acc.setPrivateTools(tool)

    return acc
