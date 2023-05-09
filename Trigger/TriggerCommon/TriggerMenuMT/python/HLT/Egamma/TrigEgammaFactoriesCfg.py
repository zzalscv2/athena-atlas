# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

def TrigEgammaRecCfg(flags, name= "trigEgammaRec"):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys()
    egammaRec = CompFactory.egammaRecBuilder( name = name,
                                                  InputClusterContainerName = TrigEgammaKeys.precisionCaloTopoCollection, # input,
                                                  egammaRecContainer        = TrigEgammaKeys.precisionCaloEgammaRecCollection, # output,
                                                  doTrackMatching           = False,
                                                  # Builder tools
                                                  TrackMatchBuilderTool     = None, # Don't want to use these for trigger....
                                                  ConversionBuilderTool     = None, 
                                                  doConversions             = False)
    acc.addEventAlgo(egammaRec)
    return acc


def TrigEgammaSuperClusterBuilderCfg(flags, name, calibrationType, superClusterCollectionName, superegammaRecCollectionName):
        acc = ComponentAccumulator()
        TrigEgammaKeys = getTrigEgammaKeys()
        from egammaTools.egammaSwToolConfig import egammaSwToolCfg
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        trigMVAfolder = flags.Trigger.egamma.Calib.precCaloMVAVersion
        TrigEgammaSuperClusterBuilder = CompFactory.egammaSuperClusterBuilder( 
                               name = name,
                               InputEgammaRecContainerName  = TrigEgammaKeys.precisionCaloEgammaRecCollection,
                               SuperClusterCollectionName   = superClusterCollectionName,
                               SuperegammaRecCollectionName = superegammaRecCollectionName, # output,
                               ClusterCorrectionTool        = acc.popToolsAndMerge(egammaSwToolCfg(flags)),   
                               MVACalibSvc                  = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags,name="trigPrecCaloEgammaMVASvc",folder = trigMVAfolder)),
                               CalibrationType              = calibrationType,
                               EtThresholdCut               = 1000,
                               LinkToConstituents           = False)
        acc.addEventAlgo(TrigEgammaSuperClusterBuilder)
        return acc

def egammaFSCaloRecoSequenceCfg(flags,name="TrigEgammaFSRecoSequence"):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys()
    from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg
    from TrigT2CaloCommon.CaloDef import _algoHLTHIEventShape
    acc.merge(
        hltCaloCellMakerCfg(flags, 'HLTCaloCellMakerEGFS', roisKey=mapThresholdToL1RoICollection('FSNOSEED'), CellsName='CaloCellsEGFS',monitorCells=False)
    )

    acc.addEventAlgo(
        _algoHLTHIEventShape(
            flags, name='HLTEventShapeMakerEG', inputEDM='CaloCellsEGFS',outputEDM=TrigEgammaKeys.egEventShape
        )
    )
    return acc

def TrigCaloClustersInConeToolCfg(flags, ion):
        acc = ComponentAccumulator()
        if ion:
            TrigEgammaKeys = getTrigEgammaKeys(ion =ion)
            name = "TrigCaloClustersInConeToolHI"
        else:
            TrigEgammaKeys = getTrigEgammaKeys()
            name = "TrigCaloClustersInConeTool"
        tool = CompFactory.xAOD.CaloClustersInConeTool(name = name,
                                                       CaloClusterLocation = TrigEgammaKeys.precisionTopoClusterContainer)
        acc.setPrivateTools(tool)
        return acc

def TrigCaloIsoCorrectionToolCfg(flags):
        acc = ComponentAccumulator()
        tool = CompFactory.CP.IsolationCorrectionTool(name = "TrigLeakageCorrTool")
        acc.setPrivateTools(tool)
        return acc

def CaloFillRectangularClusterCfg(flags, **kwargs):
        result = ComponentAccumulator()
        kwargs.setdefault("eta_size", 5)
        kwargs.setdefault("phi_size", 7)
        kwargs.setdefault("cells_name",'CaloCells')
        result.setPrivateTools(CompFactory.CaloFillRectangularCluster(**kwargs))
        return result

def TrigCaloIsolationToolCfg(flags, ion):
        acc = ComponentAccumulator()
        if ion:
            name = "TrigCaloIsolationToolHI"
        else:
            name = "TrigCaloIsolationTool"
        from CaloIdentifier import SUBCALO
        tool = CompFactory.xAOD.CaloIsolationTool(name = name,
                                                   CaloFillRectangularClusterTool  = acc.popToolsAndMerge(CaloFillRectangularClusterCfg(flags,name="trigegamma_CaloFillRectangularCluster")),
                                                   ClustersInConeTool              = acc.popToolsAndMerge(TrigCaloClustersInConeToolCfg(flags,ion=ion)),
                                                   FlowElementsInConeTool          = None,
                                                   ParticleCaloExtensionTool       = None,
                                                   IsoLeakCorrectionTool           = acc.popToolsAndMerge(TrigCaloIsoCorrectionToolCfg(flags)),
                                                   ParticleCaloCellAssociationTool = None,
                                                   saveOnlyRequestedCorrections    = True,
                                                   EMCaloNums                      = [SUBCALO.LAREM],
                                                   HadCaloNums                     = [SUBCALO.LARHEC, SUBCALO.TILE])
        acc.setPrivateTools(tool)
        return acc


def TrigPhotonIsoBuilderCfg(flags, ion = False):
        acc = ComponentAccumulator()
        if ion:
            name = 'TrigPhotonIsolationBuilderHI'
            TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
        else:
            name = 'TrigPhotonIsolationBuilder'
            TrigEgammaKeys = getTrigEgammaKeys()

        from xAODPrimitives.xAODIso import xAODIso as isoPar
        TrigPhotonIsolationBuilder = CompFactory.IsolationBuilder(name                  = name,
                                                                  PhotonCollectionContainerName = TrigEgammaKeys.precisionPhotonContainer,
                                                                  CaloCellIsolationTool = None,
                                                                  CaloTopoIsolationTool = acc.popToolsAndMerge(TrigCaloIsolationToolCfg(flags=flags,ion=ion)),
                                                                  PFlowIsolationTool    = None,
                                                                  TrackIsolationTool    = None, 
                                                                  PhIsoTypes            = [[isoPar.topoetcone20, isoPar.topoetcone30, isoPar.topoetcone40]],
                                                                  PhCorTypes            = [[isoPar.core57cells, isoPar.pileupCorrection]],
                                                                  PhCorTypesExtra       = [[]],
                                                                  )
        acc.addEventAlgo(TrigPhotonIsolationBuilder)
        return acc


def TrigEgammaPseudoJetAlgCfg(flags, name='TrigPhotonEgammaPSeudoJetBuilder'):
        acc = ComponentAccumulator()
        # This is to run pseudoJetAlgorithm to compute event density over FullScan TopoClusters
        TrigEgammaPseudoJetAlgBuilder = CompFactory.PseudoJetAlgorithm(name               = name,
                                                                       Label              = "EMTopo",
                                                                       InputContainer     = "HLT_TopoCaloClustersFS", 
                                                                       OutputContainer    = "PseudoJetTrigEMTopo",
                                                                       SkipNegativeEnergy = True)
        acc.addEventAlgo(TrigEgammaPseudoJetAlgBuilder)
        return acc


def TrigConfigEventDensityToolCfg( flags, **kwargs ):
        acc = ComponentAccumulator()
        kwargs.setdefault("JetAlgorithm", "Kt")
        kwargs.setdefault("JetRadius", 0.5)
        kwargs.setdefault("InputContainer", "PseudoJetTrigEMTopo")
        kwargs.setdefault("AbsRapidityMin",0.0)
        kwargs.setdefault("AbsRapidityMax",2.0)
        kwargs.setdefault("AreaDefinition","Voronoi")
        kwargs.setdefault("VoronoiRfact",0.9)
        kwargs.setdefault("OutputContainer",'TrigIsoEventShape')
        acc.setPrivateTools(CompFactory.EventDensityTool(**kwargs))
        return acc


def TrigIsoEventShapeAlgCfg (flags, name = 'TrigPhotonIsoEventShapeAlg'):
        acc = ComponentAccumulator()
        TrigIsoEventShapeAlg = CompFactory.EventDensityAthAlg(name = name,
                                                              EventDensityTool = acc.popToolsAndMerge(TrigConfigEventDensityToolCfg(flags=flags,name="TrigIsoTool")))
        acc.addEventAlgo(TrigIsoEventShapeAlg)
        return acc 


def TrigEgammaFSEventDensitySequenceCfg(flags, name = 'TrigEgammaFSEventDensitySequence'):
        acc = ComponentAccumulator()
        from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
        acc.merge(jetmetTopoClusteringCfg(flags,
                                          RoIs = '') )
        acc.merge(TrigEgammaPseudoJetAlgCfg(flags))
        acc.merge(TrigIsoEventShapeAlgCfg(flags))
        return acc

