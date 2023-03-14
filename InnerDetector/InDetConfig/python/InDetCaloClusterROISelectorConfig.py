# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetCaloClusterROISelector package
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

def CaloClusterROIPhiRZContainerMakerCfg(flags, name="CaloClusterROIPhiRZContainerMaker", **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    result = egammaTopoClusterCopierCfg(flags)

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    kwargs.setdefault("InputClusterContainerName",
                      flags.Egamma.Keys.Internal.EgammaTopoClusters)
    kwargs.setdefault("EMEnergyOnly", True)

    OutputROIContainerName=[]
    minPt=[]
    phiWidth=[]

    if flags.InDet.Tracking.ActiveConfig.RoISeededBackTracking :
        # TRT_TrackSegmentsFinder
        # TRT_SeededTrackFinder
        pt_cut = flags.InDet.Tracking.ActiveConfig.minRoIClusterEt
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ%.0fGeVUnordered' % (pt_cut/Units.GeV))
        minPt.append(pt_cut)
        phiWidth.append(0.)  # no phi ordering, no Roi duplication close to +- pi

    if flags.InDet.Tracking.doCaloSeededBrem:
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ0GeV')
        minPt.append(0)
        phiWidth.append(flags.InDet.Tracking.ActiveConfig.phiWidthBrem) # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidt)

    if flags.InDet.Tracking.doCaloSeededAmbi and \
       flags.Detector.EnableCalo:
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ10GeV')
        minPt.append(10000)
        phiWidth.append(0.05) # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool

    if flags.InDet.Tracking.doBremRecovery and \
       flags.InDet.Tracking.doCaloSeededBrem and \
       flags.Detector.EnableCalo:
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ5GeV')
        minPt.append(5000)
        phiWidth.append(0.075) # must be equal or larger than phiWidth of its clients: InDetNNScoringTool (phiWidthEM)

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(name, **kwargs), primary=True)
    return result

def ITkCaloClusterROIPhiRZContainerMakerCfg(flags, name="CaloClusterROIPhiRZContainerMaker", **kwargs):

    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    result = egammaTopoClusterCopierCfg(flags)

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    kwargs.setdefault("InputClusterContainerName",
                      flags.Egamma.Keys.Internal.EgammaTopoClusters)
    kwargs.setdefault("EMEnergyOnly", True)

    OutputROIContainerName=[]
    minPt=[]
    phiWidth=[]

    if flags.ITk.Tracking.doCaloSeededBrem:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ0GeV')
        minPt.append(0)
        phiWidth.append(flags.ITk.Tracking.ActiveConfig.phiWidthBrem[0]) # value from central eta bin
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)

    if flags.ITk.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ10GeV')
        minPt.append(10000)
        phiWidth.append(0.05) # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool

    if flags.ITk.Tracking.doBremRecovery and \
       flags.ITk.Tracking.doCaloSeededBrem and \
       flags.Detector.EnableCalo:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ5GeV')
        minPt.append(5000)
        phiWidth.append(0.075) # must be equal or larger than phiWidth of its clients: InDetNNScoringTool (phiWidthEM)

    if flags.ITk.Tracking.doConversionFinding:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ15GeVUnordered')
        minPt.append(15000)
        phiWidth.append(0.) # no phi ordering, no Roi duplication close to +- pi

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(name, **kwargs), primary=True)
    return result

def HadCaloClusterROIPhiRZContainerMakerCfg(flags, name="HadCaloClusterROIPhiRZContainerMaker", **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    result = egammaTopoClusterCopierCfg(flags)

    kwargs.setdefault("InputClusterContainerName",  "CaloCalTopoClusters")

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    OutputROIContainerName=[]
    minPt=[]
    phiWidth=[]

    if flags.InDet.Tracking.doHadCaloSeededSSS and \
       flags.Detector.EnableCalo:
        OutputROIContainerName.append("InDetHadCaloClusterROIPhiRZ")
        minPt.append(0)
        phiWidth.append(flags.InDet.Tracking.ActiveConfig.phiWidthBrem) # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)

    if flags.InDet.Tracking.doCaloSeededAmbi and \
       flags.Detector.EnableCalo :
        OutputROIContainerName.append("InDetHadCaloClusterROIPhiRZBjet")
        minPt.append(0)
        phiWidth.append(0.05) # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaHadCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaHadCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(name, **kwargs), primary=True)
    return result

def ITkHadCaloClusterROIPhiRZContainerMakerCfg(flags, name="HadCaloClusterROIPhiRZContainerMaker", **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    result = egammaTopoClusterCopierCfg(flags)

    kwargs.setdefault("InputClusterContainerName",  "CaloCalTopoClusters")

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    OutputROIContainerName=[]
    minPt=[]
    phiWidth=[]

    if flags.ITk.Tracking.doHadCaloSeededSSS:
        OutputROIContainerName.append("ITkHadCaloClusterROIPhiRZ")
        minPt.append(0)
        phiWidth.append(flags.ITk.Tracking.ActiveConfig.phiWidthBrem[0]) # value from central eta bin
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)

    if flags.ITk.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append("ITkHadCaloClusterROIPhiRZBjet")
        minPt.append(0)
        phiWidth.append(0.05) # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaHadCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaHadCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(name, **kwargs), primary=True)
    return result
