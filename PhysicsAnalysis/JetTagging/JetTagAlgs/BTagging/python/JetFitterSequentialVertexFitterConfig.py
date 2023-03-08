# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetFitterSequentialVertexFitterCfg(flags, name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterSequentialVertexFitter tool and returns it.

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                             Note however that this tool has no BTaggingFlags defaults; the option is
                             here only for consistency.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        from TrkConfig.TrkVertexFitterUtilsConfig import AtlasFullLinearizedTrackFactoryCfg
        jetFitterFullLinearizedTrackFactory = acc.popToolsAndMerge(AtlasFullLinearizedTrackFactoryCfg(flags, 'JFFullLinearizedTrackFactory'))

        from TrkConfig.TrkVertexFittersConfig import SequentialVertexSmootherCfg
        jetFitterSequentialVertexSmoother = acc.popToolsAndMerge(SequentialVertexSmootherCfg(flags, 'JFSequentialVertexSmoother'))

        defaults = { 'LinearizedTrackFactory' : jetFitterFullLinearizedTrackFactory,
                     'VertexSmoother'         : jetFitterSequentialVertexSmoother, }
        for option in defaults:
            options.setdefault(option, defaults[option])

    options['name'] = name
    acc.setPrivateTools(CompFactory.Trk.SequentialVertexFitter(**options))

    return acc
