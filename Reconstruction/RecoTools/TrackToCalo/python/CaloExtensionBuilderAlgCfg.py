# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def CaloExtensionBuilderAlgCfg(flags, name="CaloExtensionBuilderAlg", **kwargs):
    result = ComponentAccumulator()

    if "LastCaloExtentionTool" not in kwargs:
        from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
        kwargs.setdefault("LastCaloExtentionTool", result.popToolsAndMerge(
            ParticleCaloExtensionToolCfg(flags)))

    # P->T conversion extra dependencies
    if flags.Detector.GeometryITk:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+ITkPixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+ITkStripDetectorElementCollection"),
        ])
    else:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+PixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+SCT_DetectorElementCollection"),
        ])

    result.addEventAlgo(CompFactory.Trk.CaloExtensionBuilderAlg(name, **kwargs))
    return result


def CaloExtensionBuilderAlgLRTCfg(flags, name="CaloExtensionBuilderAlg_LRT", **kwargs):
    kwargs.setdefault("TrkPartContainerName", "InDetLargeD0TrackParticles")
    kwargs.setdefault("ParticleCache", "ParticleCaloExtension_LRT")
    return CaloExtensionBuilderAlgCfg(flags, name, **kwargs)


def CaloExtensionBuilderCfg(flags):
    result = CaloExtensionBuilderAlgCfg(flags)
    if flags.Tracking.doLargeD0 and flags.Tracking.storeSeparateLargeD0Container:
        result.merge(CaloExtensionBuilderAlgLRTCfg(flags))
    return result
