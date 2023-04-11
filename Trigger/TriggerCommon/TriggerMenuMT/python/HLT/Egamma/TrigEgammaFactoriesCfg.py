from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

TrigEgammaKeys = getTrigEgammaKeys()
def TrigEgammaRecCfg(flags, name= "trigEgammaRec"):
    acc = ComponentAccumulator()
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

def egammaFSCaloRecoSequenceCfg(flags,name="egammaFSRecoSequence"):
    acc = ComponentAccumulator()
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

