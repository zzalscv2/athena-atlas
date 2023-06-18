# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.SystemOfUnits import GeV
def FPGATrackSimSGInputCfg(flags):
    """
    Configure FPGATrackSim wrappers generation, outFile will be taken from flags in the future
    """

    acc = ComponentAccumulator()
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    acc.addPublicTool(extrapolator)


    FPGATrackSimSGInputTool = CompFactory.FPGATrackSimSGToRawHitsTool(maxEta=3.2, minPt=0.8 * GeV,
        Extrapolator = extrapolator )
    acc.addPublicTool(FPGATrackSimSGInputTool)

    wrapperAlg = CompFactory.TrigFPGATrackSimRawHitsWrapperAlg(
        InputTool=FPGATrackSimSGInputTool, OutFileName=flags.Trigger.FPGATrackSim.wrapperFileName
    )
    acc.addEventAlgo(wrapperAlg)

    return acc
# how to run? See README file