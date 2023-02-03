# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ImprovedJetFitterInitializationHelperCfg(flags, name, useBTagFlagsDefaults = True, **options):
    """Sets up a ImprovedJetFitterInitializationHelper tool and returns it.

    The following options have BTaggingFlags defaults:

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        from TrkConfig.TrkVertexFitterUtilsConfig import AtlasFullLinearizedTrackFactoryCfg
        jetFitterFullLinearizedTrackFactory = acc.popToolsAndMerge(AtlasFullLinearizedTrackFactoryCfg(flags, 'JFFullLinearizedTrackFactory'))
        defaults = {'LinearizedTrackFactory' : jetFitterFullLinearizedTrackFactory}
        for option in defaults:
            options.setdefault(option, defaults[option])

    options['name'] = name
    acc.setPrivateTools(CompFactory.Trk.JetFitterInitializationHelper(**options))

    return acc
