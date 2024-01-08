# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def BTagToolCfg(flags, TaggerList, PrimaryVertexCollectionName="", scheme = '', useBTagFlagsDefaults = True):
      """Adds a new myBTagTool instance and registers it.

      input: jetcol:             The name of the jet collections.
             ToolSvc:            The ToolSvc instance.
             options:            Python dictionary of options to be passed to the BTagTool.
             (note the options storeSecondaryVerticesInJet is passed to the removal tool instead)

      The following default options exist:

      BTagLabelingTool                       default: None
      storeSecondaryVerticesInJet            default: BTaggingFlags.writeSecondaryVertices

      output: The btagtool for the desired jet collection."""

      acc=ComponentAccumulator()

      tagToolList = []

      if 'IP2D' in TaggerList:
          from JetTagTools.IP2DTagConfig import IP2DTagCfg
          ip2dtool = acc.popToolsAndMerge(IP2DTagCfg(flags, 'IP2DTag', scheme))
          tagToolList.append(ip2dtool)
 
      # Setup for Neg/Flip version of IP2(3)D taggers. 'FlipOption' default value is "STANDARD". Set this value only if want to 'flip' the tagger
      # Naming of the flip options follows ENUMS here: https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/JetTagging/FlavorTagDiscriminants/Root/FlipTagEnums.cxx
      if 'IP2DNeg' in TaggerList:
          from JetTagTools.IP2DTagConfig import IP2DTagCfg
          ip2dnegtool = acc.popToolsAndMerge(IP2DTagCfg(flags, 'IP2DNegTag' ,scheme,FlipOption='NEGATIVE_IP_ONLY'))
          tagToolList.append(ip2dnegtool)

      if 'IP2DFlip' in TaggerList:
          from JetTagTools.IP2DTagConfig import IP2DTagCfg
          ip2dfliptool = acc.popToolsAndMerge(IP2DTagCfg(flags, 'IP2DFlipTag' ,scheme,FlipOption='FLIP_SIGN'))
          tagToolList.append(ip2dfliptool)

      if 'IP3D' in TaggerList:
          from JetTagTools.IP3DTagConfig import IP3DTagCfg
          ip3dtool = acc.popToolsAndMerge(IP3DTagCfg(flags, 'IP3DTag', PrimaryVertexCollectionName, scheme))
          tagToolList.append(ip3dtool)

      if 'IP3DNeg' in TaggerList:
          from JetTagTools.IP3DTagConfig import IP3DTagCfg
          ip3dnegtool = acc.popToolsAndMerge(IP3DTagCfg(flags, 'IP3DNegTag', PrimaryVertexCollectionName, scheme,FlipOption='NEGATIVE_IP_ONLY'))
          tagToolList.append(ip3dnegtool)

      if 'IP3DFlip' in TaggerList:
          from JetTagTools.IP3DTagConfig import IP3DTagCfg
          ip3dfliptool = acc.popToolsAndMerge(IP3DTagCfg(flags, 'IP3DFlipTag', PrimaryVertexCollectionName, scheme,FlipOption='FLIP_SIGN'))
          tagToolList.append(ip3dfliptool)

      if 'SV1' in TaggerList:
          from JetTagTools.SV1TagConfig import SV1TagCfg
          sv1tool = acc.popToolsAndMerge(SV1TagCfg(flags, 'SV1Tag', scheme))
          tagToolList.append(sv1tool)
      
      if 'SV1Flip' in TaggerList:
          from JetTagTools.SV1TagConfig import SV1TagCfg
          sv1fliptool = acc.popToolsAndMerge(SV1TagCfg(flags, 'SV1FlipTag', scheme))
          tagToolList.append(sv1fliptool)

      if 'MultiSVbb1' in TaggerList:
          from JetTagTools.MultiSVTagConfig import MultiSVTagCfg
          multisvbb1tool = acc.popToolsAndMerge(MultiSVTagCfg(flags,'MultiSVbb1Tag','MultiSVbb1', scheme))
          tagToolList.append(multisvbb1tool)

      if 'MultiSVbb2' in TaggerList:
          from JetTagTools.MultiSVTagConfig import MultiSVTagCfg
          multisvbb2tool = acc.popToolsAndMerge(MultiSVTagCfg(flags, 'MultiSVbb2Tag','MultiSVbb2', scheme))
          tagToolList.append(multisvbb2tool)

      if 'JetVertexCharge' in TaggerList:
          from JetTagTools.JetVertexChargeConfig import JetVertexChargeCfg
          jvc = acc.popToolsAndMerge(JetVertexChargeCfg(flags, 'JetVertexCharge', scheme))
          tagToolList.append(jvc)

      # list of taggers that use MultivariateTagManager
      mvtm_taggers = ['MV2c00','MV2c10','MV2c20','MV2c10mu','MV2m','DL1','DL1mu']
      mvtm_active_taggers = list(set(mvtm_taggers) & set(TaggerList))
      if len(mvtm_active_taggers) > 0:
          from JetTagTools.MultivariateTagManagerConfig import MultivariateTagManagerCfg
          mvtm = acc.popToolsAndMerge(MultivariateTagManagerCfg(flags, 'mvtm', TaggerList = mvtm_active_taggers, scheme = scheme))
          tagToolList.append(mvtm)

      options = {}
      if useBTagFlagsDefaults:
        defaults = { 'vxPrimaryCollectionName'      : PrimaryVertexCollectionName,
                     'TagToolList'                  : tagToolList,
                   }
        for option in defaults:
            options.setdefault(option, defaults[option])
      options['name'] = 'btag'
      btagtool = CompFactory.Analysis.BTagTool(**options)
      acc.setPrivateTools(btagtool)

      return acc
