# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, createService
from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

# Config:
triggerChains = [
    'HLT_2mu14',
    'HLT_mu20_mu8noL1',
    'HLT_2e17_lhvloose_nod0'
]


def makeSequenceOld (dataType, algSeq, forCompare) :

    # Include, and then set up the pileup analysis sequence:
    prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

    from AsgAnalysisAlgorithms.PileupAnalysisSequence import \
        makePileupAnalysisSequence
    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwfiles,
        userLumicalcFiles=lumicalcfiles,
    )
    pileupSequence.configure( inputName = {}, outputName = {} )

    # Add the pileup sequence to the job:
    algSeq += pileupSequence


    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetContainer = 'AntiKt4EMPFlowJets'
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer, enableCutflow=True, enableKinematicHistograms=True )

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
    makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = True, legacyRecommendations = True,
                              enableCutflow=True )

    jetSequence.configure( inputName = jetContainer, outputName = 'AnaJetsBase_%SYS%' )

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetJvtAnalysisSequence import makeJetJvtAnalysisSequence
    jvtSequence = makeJetJvtAnalysisSequence( dataType, jetContainer, enableCutflow=True )
    jvtSequence.configure( inputName = { 'jets'      : 'AnaJetsBase_%SYS%' },
                           outputName = { 'jets'      : 'AnaJets_%SYS%' } )

    # Add the sequences to the job:
    algSeq += jetSequence
    algSeq += jvtSequence


    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    muonSequenceMedium = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                   workingPoint = 'Medium.NonIso', postfix = 'medium',
                                                   enableCutflow=True, enableKinematicHistograms=True )
    muonSequenceMedium.configure( inputName = 'Muons',
                                  outputName = 'AnaMuonsMedium_%SYS%' )

    # Add the sequence to the job:
    algSeq += muonSequenceMedium

    muonSequenceTight = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                  workingPoint = 'Tight.NonIso', postfix = 'tight',
                                                  enableCutflow=True, enableKinematicHistograms=True )
    muonSequenceTight.removeStage ("calibration")
    muonSequenceTight.configure( inputName = 'AnaMuonsMedium_%SYS%',
                                 outputName = 'AnaMuons_%SYS%')

    # Add the sequence to the job:
    algSeq += muonSequenceTight


    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import \
        makeElectronAnalysisSequence
    likelihood = True
    recomputeLikelihood=False
    if likelihood:
        workingpoint = 'LooseLHElectron.Loose_VarRad'
    else:
        workingpoint = 'LooseDNNElectron.Loose_VarRad'
    electronSequence = makeElectronAnalysisSequence( dataType, workingpoint, postfix = 'loose',
                                                     recomputeLikelihood=recomputeLikelihood, enableCutflow=True, enableKinematicHistograms=True )
    electronSequence.configure( inputName = 'Electrons',
                                outputName = 'AnaElectrons_%SYS%' )
    algSeq += electronSequence


    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import \
        makePhotonAnalysisSequence
    photonSequence = makePhotonAnalysisSequence( dataType, 'Tight.FixedCutTight', postfix = 'tight',
                                                 recomputeIsEM=False, enableCutflow=True, enableKinematicHistograms=True )
    photonSequence.configure( inputName = 'Photons',
                              outputName = 'AnaPhotons_%SYS%' )
    algSeq += photonSequence


    # Include, and then set up the tau analysis algorithm sequence:
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    tauSequence = makeTauAnalysisSequence( dataType, 'Tight', postfix = 'tight',
                                           enableCutflow=True, enableKinematicHistograms=True )
    tauSequence.configure( inputName = 'TauJets', outputName = 'AnaTauJets_%SYS%' )

    # Add the sequence to the job:
    algSeq += tauSequence

    # temporarily disabled until di-taus are supported in R22
    # # Include, and then set up the tau analysis algorithm sequence:
    # from TauAnalysisAlgorithms.DiTauAnalysisSequence import makeDiTauAnalysisSequence
    # diTauSequence = makeDiTauAnalysisSequence( dataType, 'Tight', postfix = 'tight' )
    # diTauSequence.configure( inputName = 'DiTauJets', outputName = 'AnaDiTauJets_%SYS%' )

    # Add the sequence to the job:
    # disabling this, the standard test files don't have DiTauJets
    # algSeq += diTauSequence


    from AnaAlgorithm.DualUseConfig import addPrivateTool, createAlgorithm
    # Set up a selection alg for demonstration purposes
    # Also to avoid warnings from building MET with very soft electrons

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'METEleSelAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    selalg.selectionTool.minPt = 10e3
    selalg.selectionTool.maxEta = 2.47
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaElectrons_%SYS%'
    algSeq += selalg

    # Now make a view container holding only the electrons for the MET calculation
    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METEleViewAlg' )
    viewalg.selection = [ 'selectPtEta' ]
    viewalg.input = 'AnaElectrons_%SYS%'
    viewalg.output = 'METElectrons_%SYS%'
    algSeq += viewalg

    # Include, and then set up the met analysis algorithm sequence:
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    metSequence = makeMetAnalysisSequence( dataType, metSuffix = jetContainer[:-4] )
    metSequence.configure( inputName = { 'jets'      : 'AnaJets_%SYS%',
                                         'muons'     : 'AnaMuons_%SYS%',
                                         'electrons' : 'METElectrons_%SYS%' },
                           outputName = 'AnaMET_%SYS%' )

    # Add the sequence to the job:
    algSeq += metSequence


    # Include, and then set up the overlap analysis algorithm sequence:
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import \
        makeOverlapAnalysisSequence
    overlapSequence = makeOverlapAnalysisSequence( dataType, doMuPFJetOR=True, doTaus=False, enableCutflow=True )
    overlapSequence.configure(
        inputName = {
            'electrons' : 'AnaElectrons_%SYS%',
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            # 'taus'      : 'AnaTauJets_%SYS%'
        },
        outputName = {
            'electrons' : 'AnaElectronsOR_%SYS%',
            'photons'   : 'AnaPhotonsOR_%SYS%',
            'muons'     : 'AnaMuonsOR_%SYS%',
            'jets'      : 'AnaJetsOR_%SYS%',
            # 'taus'      : 'AnaTauJetsOR_%SYS%'
        } )

    # FIX ME: temporarily disabling this for data, as there are some
    # errors with missing primary vertices
    if dataType != 'data' :
        algSeq += overlapSequence


    if dataType != 'data' :
        # Include, and then set up the generator analysis sequence:
        from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import \
            makeGeneratorAnalysisSequence
        generatorSequence = makeGeneratorAnalysisSequence( dataType, saveCutBookkeepers=True, runNumber=284500, cutBookkeepersSystematics=True )
        algSeq += generatorSequence


    # Include, and then set up the trigger analysis sequence:
    from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import \
        makeTriggerAnalysisSequence
    triggerSequence = makeTriggerAnalysisSequence( dataType, triggerChains=triggerChains )
    # FIXME: temporarily disabling this for comparisons, as there is no
    # corresponding configuration block.  also, maybe it should be possible
    # to disable filtering in the algorithm, i.e. just store the trigger
    # decision without throwing away events.
    if not forCompare :
        algSeq += triggerSequence



def makeSequenceBlocks (dataType, algSeq, forCompare) :

    # Include, and then set up the pileup analysis sequence:
    prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

    from AsgAnalysisAlgorithms.PileupAnalysisSequence import \
        makePileupAnalysisSequence
    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwfiles,
        userLumicalcFiles=lumicalcfiles,
    )
    pileupSequence.configure( inputName = {}, outputName = {} )

    # Add the pileup sequence to the job:
    algSeq += pileupSequence


    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig, makeMuonWorkingPointConfig

    configSeq = ConfigSequence ()

    makeMuonCalibrationConfig (configSeq, 'AnaMuons')
    makeMuonWorkingPointConfig (configSeq, 'AnaMuons', workingPoint='Medium.Iso', postfix='medium')
    makeMuonWorkingPointConfig (configSeq, 'AnaMuons', workingPoint='Tight.Iso', postfix='tight')

    configAccumulator = ConfigAccumulator (dataType, algSeq)
    configSeq.fullConfigure (configAccumulator)



def makeSequence (dataType, useBlocks, forCompare) :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1

    if not useBlocks :
        makeSequenceOld (dataType, algSeq, forCompare=forCompare)
    else :
        makeSequenceBlocks (dataType, algSeq, forCompare=forCompare)


    # Add an ntuple dumper algorithm:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'analysis'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMakerEventInfo' )
    ntupleMaker.TreeName = 'analysis'
    ntupleMaker.Branches = [ 'EventInfo.runNumber     -> runNumber',
                             'EventInfo.eventNumber   -> eventNumber', ]
    algSeq += ntupleMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMakerMuons' )
    ntupleMaker.TreeName = 'analysis'
    ntupleMaker.Branches = [ 'EventInfo.runNumber   -> runNumber',
                             'EventInfo.eventNumber -> eventNumber',
                             'AnaMuons_NOSYS.eta -> muon_eta',
                             'AnaMuons_NOSYS.phi -> muon_phi',
                             'AnaMuons_%SYS%.pt  -> muon_pt_%SYS%', ]

    # These branches are temporarily excluded for the comparison
    # between sequences and blocks as they are either not yet
    # implemented or make the test fail.  The differences between
    # sequence and block configuration need to be addressed and then
    # these need to be reenabled.
    if not forCompare and not useBlocks :
        ntupleMaker.Branches += [
            'AnaJets_%SYS%.pt  -> jet_%SYS%_pt',
            'AnaJets_NOSYS.phi -> jet_phi',
            'AnaJets_NOSYS.eta -> jet_eta',
            'AnaMuons_%SYS%.baselineSelection_medium -> muon_select_medium_%SYS%',
            'AnaMuons_%SYS%.baselineSelection_tight  -> muon_select_tight_%SYS%',
            'AnaElectrons_%SYS%.pt  -> electron_%SYS%_pt',
            'AnaElectrons_NOSYS.phi -> electron_phi',
            'AnaElectrons_NOSYS.eta -> electron_eta',
            'AnaElectrons_%SYS%.baselineSelection_loose -> electron_select_loose_%SYS%',
            'AnaPhotons_%SYS%.pt  -> photon_%SYS%_pt',
            'AnaPhotons_NOSYS.phi -> photon_phi',
            'AnaPhotons_NOSYS.eta -> photon_eta',
            'AnaPhotons_%SYS%.baselineSelection_tight -> photon_select_tight_%SYS%',
            'AnaTauJets_%SYS%.pt  -> tau_jet_%SYS%_pt',
            'AnaTauJets_NOSYS.phi -> tau_jet_phi',
            'AnaTauJets_NOSYS.eta -> tau_jet_eta',
            'AnaTauJets_%SYS%.baselineSelection_tight -> tau_select_tight_%SYS%',
        ]
        if dataType != 'data':
            ntupleMaker.Branches += [
                # 'EventInfo.jvt_effSF_%SYS% -> jvtSF_%SYS%',
                # 'EventInfo.fjvt_effSF_%SYS% -> fjvtSF_%SYS%',
                'AnaJets_%SYS%.jvt_effSF_%SYS% -> jet_%SYS%_jvtEfficiency',
                # 'AnaJets_%SYS%.fjvt_effSF_NOSYS -> jet_%SYS%_fjvtEfficiency',
                'AnaMET_%SYS%.mpx   -> met_%SYS%_mpx',
                'AnaMET_%SYS%.mpy   -> met_%SYS%_mpy',
                'AnaMET_%SYS%.sumet -> met_%SYS%_sumet',
                'AnaMET_%SYS%.name  -> met_%SYS%_name',
            ]

        ntupleMaker.Branches += ['EventInfo.trigPassed_' + t + ' -> trigPassed_' + t for t in triggerChains]
        if dataType != 'data':
            ntupleMaker.Branches += [ 'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%', ]
            ntupleMaker.Branches += [ 'AnaMuons_%SYS%.muon_effSF_medium_%SYS% -> muon_effSF_medium_%SYS%' ]
            ntupleMaker.Branches += [ 'AnaMuons_%SYS%.muon_effSF_tight_%SYS% -> muon_effSF_tight_%SYS%' ]
    # ntupleMaker.OutputLevel = 2  # For output validation
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'analysis'
    algSeq += treeFiller

    return algSeq
