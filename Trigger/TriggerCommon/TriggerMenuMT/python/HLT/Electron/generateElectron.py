# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Electron.ElectronRecoSequences import l2CaloRecoCfg
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, \
    ChainStep, Chain, EmptyMenuSequence, InViewRecoCA, SelectionCA

from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
from TrigEDMConfig.TriggerEDMRun3 import recordable
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

@AccumulatorCache
def _fastCaloSeq(flags):
    selAcc=SelectionCA('FastCaloElectron')
    selAcc.mergeReco(l2CaloRecoCfg(flags))

    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoAlgCfg
    l2CaloHypo = TrigEgammaFastCaloHypoAlgCfg(flags,
                                              name='ElectronEgammaFastCaloHypo',
                                              CaloClusters=recordable('HLT_FastCaloEMClusters'))
    
    selAcc.mergeHypo(l2CaloHypo)

    fastCaloSequence = MenuSequenceCA(flags, selAcc,
                                      HypoToolGen=TrigEgammaFastCaloHypoToolFromDict)
    return (selAcc , fastCaloSequence)

def _fastCalo(flags, chainDict):

    selAcc , fastCaloSequence = _fastCaloSeq(flags)

     # this cannot work for asymmetric combined chains....FP
    return ChainStep(name=selAcc.name, Sequences=[fastCaloSequence], chainDicts=[chainDict])

@AccumulatorCache
def _ftfSeq(flags):
    selAcc=SelectionCA('ElectronFTF')
    # # # fast ID (need to be customised because require secialised configuration of the views maker - i.e. parent has to be linked)
    fastInDetReco = InViewRecoCA('FastElectron', RequireParentView = True)
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    idTracking = trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName='electron')
    fastInDetReco.mergeReco(idTracking)
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDVElectronFastCalo',
                                DataObjects=[('xAOD::TrigEMClusterContainer', 'StoreGateSvc+HLT_FastCaloEMClusters')]) )

    from TrigEgammaRec.TrigEgammaFastElectronConfig import fastElectronFexAlgCfg
    fastInDetReco.mergeReco(fastElectronFexAlgCfg(flags, rois=fastInDetReco.inputMaker().InViewRoIs))
    selAcc.mergeReco(fastInDetReco)
    fastElectronHypoAlg = CompFactory.TrigEgammaFastElectronHypoAlg()
    fastElectronHypoAlg.Electrons = 'HLT_FastElectrons'
    fastElectronHypoAlg.RunInView = True
    selAcc.addHypoAlgo(fastElectronHypoAlg)
    from TrigEgammaHypo.TrigEgammaFastElectronHypoTool import TrigEgammaFastElectronHypoToolFromDict
    fastInDetSequence = MenuSequenceCA(flags, selAcc,
                                       HypoToolGen=TrigEgammaFastElectronHypoToolFromDict)
    return (selAcc , fastInDetSequence)

def _ftf(flags, chainDict):
    selAcc , fastInDetSequence = _ftfSeq(flags)
    return ChainStep( name=selAcc.name, Sequences=[fastInDetSequence], chainDicts=[chainDict])

@AccumulatorCache
def _precisonCaloSeq(flags):
    recoAcc = InViewRecoCA('ElectronRoITopoClusterReco', RequireParentView = True)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDV'+recoAcc.name,
                                                              DataObjects=[('TrigRoiDescriptorCollection', recoAcc.inputMaker().InViewRoIs.Path),
                                                                           ('CaloBCIDAverage', 'StoreGateSvc+CaloBCIDAverage')]))

    from TrigCaloRec.TrigCaloRecConfig import hltCaloTopoClusteringCfg
    recoAcc.mergeReco(hltCaloTopoClusteringCfg(flags,
                                               namePrefix='Electron',
                                               roisKey=recoAcc.inputMaker().InViewRoIs) ) # RoI

    copier = CompFactory.egammaTopoClusterCopier('TrigEgammaTopoClusterCopierPrecisionCaloRoIs',
                                                 InputTopoCollection='HLT_TopoCaloClustersRoI',
                                                 OutputTopoCollection='HLT_CaloEMClusters_Electron')
    recoAcc.addRecoAlgo(copier)

    selAcc = SelectionCA('PrecisionCalo')
    selAcc.mergeReco(recoAcc)
    hypoAlg = CompFactory.TrigEgammaPrecisionCaloHypoAlg(name='ElectronPrecisionCaloHypo',
                                                           CaloClusters=recordable('HLT_CaloEMClusters_Electron'))
    selAcc.addHypoAlgo(hypoAlg)
    from TrigEgammaHypo.TrigEgammaPrecisionCaloHypoTool import TrigEgammaPrecisionCaloHypoToolFromDict
    menuSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen=TrigEgammaPrecisionCaloHypoToolFromDict)
    return (selAcc , menuSequence)

def _precisonCalo(flags, chainDict):
    selAcc , menuSequence = _precisonCaloSeq(flags)
    return ChainStep(name=selAcc.name, Sequences=[menuSequence], chainDicts=[chainDict])

@AccumulatorCache
def _precisionTrackingSeq(flags,chainDict):
    name='ElectronPrecisionTracking'
    selAcc=SelectionCA('ElectronPrecisionTracking')

    roisKey = flags.Trigger.InDetTracking.electron.roi
    precisionInDetReco = InViewRecoCA(name, 
                                        RoITool=CompFactory.ViewCreatorPreviousROITool(), # view maker args
                                        RequireParentView=True, 
                                        InViewRoIs=roisKey)

    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    idTracking = trigInDetPrecisionTrackingCfg(flags, rois= roisKey, signatureName='electron')
    precisionInDetReco.mergeReco(idTracking)
    selAcc.mergeReco(precisionInDetReco)
    hypoAlg = CompFactory.TrigStreamerHypoAlg('ElectronprecisionTrackingHypo')
    selAcc.addHypoAlgo(hypoAlg)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    menuSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen=acceptAllHypoToolGen)
    return (selAcc , menuSequence)

def _precisionTracking(flags, chainDict):
    selAcc , menuSequence = _precisionTrackingSeq(flags,chainDict)
    return ChainStep(name=selAcc.name, Sequences=[menuSequence], chainDicts=[chainDict])

@AccumulatorCache
def _precisionElectronSeq(flags):
    name='PrecionElectron'
    selAcc=SelectionCA(name)
    recoAcc = InViewRecoCA(name, RequireParentView=True)
    #TODO this should likely go elsewhere (egamma experts to decide)
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name='VDV'+recoAcc.name,
                                                              DataObjects=[( 'CaloCellContainer' , 'StoreGateSvc+CaloCells' ),
                                                                           ( 'xAOD::CaloClusterContainer' , 'StoreGateSvc+%s' % TrigEgammaKeys.precisionElectronCaloClusterContainer),
                                                                           ( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % TrigEgammaKeys.precisionTrackingContainer)] ) )


    def TrigEMTrackMatchBuilderToolCfg(flags, name='TrigEMTrackMatchBuilder_noGSF'):
        acc = ComponentAccumulator()

        from egammaTrackTools.egammaTrackToolsConfig import EMExtrapolationToolsCfg
        emExtrapolatorTools = recoAcc.popToolsAndMerge(EMExtrapolationToolsCfg(flags))
        builderTool = CompFactory.EMTrackMatchBuilder(  name, #TODO, this is provate tool, it does not need to be specifically named
                                                        TrackParticlesName = TrigEgammaKeys.precisionTrackingContainer,
                                                        ExtrapolationTool  = emExtrapolatorTools,
                                                        broadDeltaEta      = 0.1, #candidate match is done in 2 times this  so +- 0.2
                                                        broadDeltaPhi      = 0.15,  #candidate match is done in 2 times this  so +- 0.3
                                                        useCandidateMatch  = True,
                                                        useScoring         = True,
                                                        SecondPassRescale  = True,
                                                        UseRescaleMetric   = True,
                                                        isCosmics          = flags.Beam.Type is BeamType.Cosmics)
        acc.setPrivateTools(builderTool)
        return acc

    def TrigEgammaRecElectronCfg(flags, name='TrigEgammaRecElectron_noGSF'):
        acc = ComponentAccumulator()
        electronRec = CompFactory.egammaRecBuilder( name,
                                                    InputClusterContainerName= 'HLT_CaloEMClusters_Electron',
                                                    egammaRecContainer= TrigEgammaKeys.precisionCaloEgammaRecCollection,
                                                    doConversions = False,
                                                    TrackMatchBuilderTool = recoAcc.popToolsAndMerge(TrigEMTrackMatchBuilderToolCfg(flags)) )
        acc.addEventAlgo(electronRec)
        return acc

    recoAcc.mergeReco( TrigEgammaRecElectronCfg(flags) )
    

    def TrigElectronSuperClusterBuilderCfg(flags):
        acc = ComponentAccumulator()
        from egammaTools.egammaSwToolConfig import egammaSwToolCfg
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        superClusterBuilder = CompFactory.electronSuperClusterBuilder( 'TrigElectronSuperClusterBuilder_noGSF',
                                                                        InputEgammaRecContainerName = TrigEgammaKeys.precisionCaloEgammaRecCollection,
                                                                        SuperElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterCollection,
                                                                        ClusterCorrectionTool = recoAcc.popToolsAndMerge(egammaSwToolCfg(flags)),
                                                                        MVACalibSvc = recoAcc.getPrimaryAndMerge(egammaMVASvcCfg(flags)),
                                                                        EtThresholdCut = 1000,
                                                                        TrackMatchBuilderTool = recoAcc.popToolsAndMerge(TrigEMTrackMatchBuilderToolCfg(flags)) )
        acc.addEventAlgo(superClusterBuilder)
        return acc

    recoAcc.mergeReco(TrigElectronSuperClusterBuilderCfg(flags))

    def TrigEMClusterToolCfg(flags):
        acc = ComponentAccumulator()
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        tool = CompFactory.EMClusterTool('TrigEMClusterTool_electron',
                                            OutputClusterContainerName = TrigEgammaKeys.precisionElectronEMClusterContainer,
                                            MVACalibSvc = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags))
        )        
        acc.setPrivateTools(tool)
        return acc

    def TrigTopoEgammaElectronCfg(flags, name='topoEgammaBuilder_TrigElectrons'):
        acc = ComponentAccumulator()
        from egammaTools.EMShowerBuilderConfig import EMShowerBuilderCfg
        builder = CompFactory.xAODEgammaBuilder(name,
                                                InputElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterCollection,
                                                InputPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterCollection,
                                                ElectronOutputName = TrigEgammaKeys.precisionElectronContainer,
                                                PhotonOutputName = TrigEgammaKeys.precisionPhotonContainer,  
                                                AmbiguityTool = CompFactory.EGammaAmbiguityTool(),
                                                EMClusterTool = acc.popToolsAndMerge(TrigEMClusterToolCfg(flags)),
                                                EMShowerTool = acc.popToolsAndMerge(EMShowerBuilderCfg(flags, CellsName="CaloCells")),
                                                egammaTools = [CompFactory.EMFourMomBuilder()], # TODO use list defined elsewhere
                                                doPhotons = False,
                                                doElectrons = True)
        acc.addEventAlgo(builder)
        return acc

    recoAcc.mergeReco(TrigTopoEgammaElectronCfg(flags))

    def TrigTrackIsolationToolCfg(flags, name='TrigTrackIsolationTool'):
        acc = ComponentAccumulator()

        tpicTool = CompFactory.xAOD.TrackParticlesInConeTool(TrackParticleLocation = TrigEgammaKeys.precisionTrackingContainer)

        tiTool = CompFactory.xAOD.TrackIsolationTool(name,
                                                    TrackParticleLocation = TrigEgammaKeys.precisionTrackingContainer,
                                                    VertexLocation = '',
                                                    TracksInConeTool  = tpicTool)
        # configure default TrackSelectionTool
        tiTool.TrackSelectionTool.maxZ0SinTheta = 3
        tiTool.TrackSelectionTool.minPt         = 1000
        tiTool.TrackSelectionTool.CutLevel      = "Loose"

        acc.setPrivateTools(tiTool)
        return acc

    def TrigElectronIsoBuilderCfg(flags, name='TrigElectronIsolationBuilder'):
        acc = ComponentAccumulator()
        from xAODPrimitives.xAODIso import xAODIso as isoPar
        builder = CompFactory.IsolationBuilder(
                                        name,
                                        ElectronCollectionContainerName = 'HLT_egamma_Electrons',
                                        CaloCellIsolationTool = None,
                                        CaloTopoIsolationTool = None,
                                        PFlowIsolationTool    = None,
                                        TrackIsolationTool    = acc.popToolsAndMerge(TrigTrackIsolationToolCfg(flags)),
                                        ElIsoTypes            = [[isoPar.ptcone20]],
                                        ElCorTypes            = [[isoPar.coreTrackPtr]],
                                        ElCorTypesExtra       = [[]])
        acc.addEventAlgo(builder)
        return acc

    recoAcc.mergeReco(TrigElectronIsoBuilderCfg(flags))

    selAcc.mergeReco(recoAcc)
    from TrigEgammaHypo.TrigEgammaPrecisionElectronHypoTool import TrigEgammaPrecisionElectronHypoToolFromDict, TrigEgammaPrecisionElectronHypoAlgCfg
    selAcc.mergeHypo( TrigEgammaPrecisionElectronHypoAlgCfg(flags, 'TrigEgammaHypoAlg_noGSF', TrigEgammaKeys.precisionElectronContainer ) )
    menuSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen=TrigEgammaPrecisionElectronHypoToolFromDict)
    return (selAcc, menuSequence)

def _precisionElectron(flags, chainDict):
    selAcc , menuSequence = _precisionElectronSeq(flags)
    return ChainStep(name=selAcc.name, Sequences=[menuSequence], chainDicts=[chainDict])


def generateChains(flags, chainDict):

    l1Thresholds=[]
    for part in chainDict['chainParts']:
        l1Thresholds.append(part['L1threshold'])

    emptyStep = ChainStep(name="EmptyElStep", Sequences=[EmptyMenuSequence("EmptyElStep")], chainDicts=[chainDict])
    # # # offline egamma
    steps = []
    if 'etcut' in chainDict['chainName']:
        steps = [ _fastCalo(flags, chainDict), 
                  _ftf(flags, chainDict),
                  _precisonCalo(flags, chainDict),
                  _precisionTracking(flags, chainDict), 
                  emptyStep]
    else:
        steps = [ _fastCalo(flags, chainDict),
                  _ftf(flags, chainDict),
                  _precisonCalo(flags, chainDict),
                  _precisionTracking(flags, chainDict), 
                  _precisionElectron(flags, chainDict)]

    chain = Chain(chainDict['chainName'], L1Thresholds=l1Thresholds,
                            ChainSteps=steps)

    return chain

