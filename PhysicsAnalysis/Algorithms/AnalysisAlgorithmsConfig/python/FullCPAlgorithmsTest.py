# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, createService, addPrivateTool
from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

# Config:
triggerChains = [
    'HLT_2mu14',
    'HLT_mu20_mu8noL1',
    'HLT_2e17_lhvloose_nod0'
]

electronMinPt = 10e3
electronMaxEta = None
photonMinPt = 10e3
photonMaxEta = None
muonMinPt = None
muonMaxEta = None
tauMinPt = None
tauMaxEta = None
jetMinPt = None
jetMaxEta = None

def addOutputCopyAlgorithms (algSeq, postfix, inputContainer, outputContainer, selection) :
    """add a uniformly filtered set of deep copies based on the
    systematics dependent selection"""

    if postfix[0] != '_' :
        postfix = '_' + postfix

    if selection != '' :
        unionalg = createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlg' + postfix)
        unionalg.preselection = selection
        unionalg.particles = inputContainer
        unionalg.selectionDecoration = 'outputSelect'
        algSeq += unionalg

    copyalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'DeepCopyAlg' + postfix )
    copyalg.input = inputContainer
    copyalg.output = outputContainer
    if selection != '' :
        copyalg.selection = ['outputSelect']
    else :
        copyalg.selection = []
    copyalg.deepCopy = False
    algSeq += copyalg


def makeSequenceOld (dataType, algSeq, vars, forCompare, isPhyslite, noPhysliteBroken) :

    # Include, and then set up the pileup analysis sequence:
    prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

    if not isPhyslite :
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

    # Add the pileup sequence to the job:
    vars += [ 'EventInfo.runNumber     -> runNumber',
             'EventInfo.eventNumber   -> eventNumber', ]


    # FIX ME: this algorithm should be run on both data and MC, but in MC
    # it produces strange errors in CutFlowSvc in Athena
    if not forCompare or dataType == 'data' :
        # Skip events with no primary vertex:
        algSeq += createAlgorithm( 'CP::VertexSelectionAlg',
                                   'PrimaryVertexSelectorAlg' )
        algSeq.PrimaryVertexSelectorAlg.VertexContainer = 'PrimaryVertices'
        algSeq.PrimaryVertexSelectorAlg.MinVertices = 1


    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetContainer = 'AntiKt4EMPFlowJets'
    if isPhyslite :
        input = 'AnalysisJets'
    else :
        input = jetContainer
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer,
                                           runJvtUpdate = True, runNNJvtUpdate = True,
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False,
                                           runGhostMuonAssociation = not isPhyslite)

    if not noPhysliteBroken :
        from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
        btagger = "DL1r"
        btagWP = "FixedCutBEff_77"
        makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = False, legacyRecommendations = True,
                                  enableCutflow=True, btagger = btagger, btagWP = btagWP, kinematicSelection = True )
        vars += [
            'OutJets_%SYS%.ftag_select_' + btagger + '_' + btagWP + ' -> jet_ftag_select_%SYS%',
        ]
        if dataType != 'data' :
            vars += [
                'OutJets_%SYS%.ftag_effSF_' + btagger + '_' + btagWP + '_%SYS% -> jet_ftag_eff_%SYS%'
            ]

    jetSequence.configure( inputName = input, outputName = 'AnaJets_%SYS%' )

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetJvtAnalysisSequence import makeJetJvtAnalysisSequence
    jvtSequence = makeJetJvtAnalysisSequence( dataType, jetContainer, enableCutflow=True, shallowViewOutput = False )
    jvtSequence.configure( inputName = { 'jets'      : 'AnaJets_%SYS%' },
                           outputName = {  } )

    # Add the sequences to the job:
    algSeq += jetSequence
    algSeq += jvtSequence
    vars += ['OutJets_%SYS%.pt  -> jet_pt_%SYS%',
             'OutJets_NOSYS.phi -> jet_phi',
             'OutJets_NOSYS.eta -> jet_eta', ]
    if dataType != 'data' :
        vars += [ 'OutJets_%SYS%.jvt_effSF_%SYS% -> jet_jvtEfficiency_%SYS%', ]
        vars += [
            # 'EventInfo.jvt_effSF_%SYS% -> jvtSF_%SYS%',
            # 'EventInfo.fjvt_effSF_%SYS% -> fjvtSF_%SYS%',
            # 'OutJets_%SYS%.fjvt_effSF_NOSYS -> jet_fjvtEfficiency_%SYS%',
        ]


    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    if isPhyslite :
        input = 'AnalysisMuons'
    else :
        input = 'Muons'

    muonSequenceMedium = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                   workingPoint = 'Medium.Iso', postfix = 'medium',
                                                   enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    # FIX ME: the current version of the `MuonSelectionTool` doesn't work
    # on the current version of PHYSLITE, and needs a new PHYSLITE production
    # campaign
    if noPhysliteBroken :
        muonSequenceMedium.__delattr__ ('MuonSelectionAlg_medium')
    muonSequenceMedium.configure( inputName = input,
                                  outputName = 'AnaMuonsMedium_%SYS%' )
    algSeq += muonSequenceMedium

    muonSequenceTight = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                  workingPoint = 'Tight.Iso', postfix = 'tight',
                                                  enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    muonSequenceTight.removeStage ("calibration")
    # FIX ME: the current version of the `MuonSelectionTool` doesn't work
    # on the current version of PHYSLITE, and needs a new PHYSLITE production
    # campaign
    if noPhysliteBroken :
        muonSequenceTight.__delattr__ ('MuonSelectionAlg_tight')
    muonSequenceTight.configure( inputName = 'AnaMuonsMedium_%SYS%',
                                 outputName = 'AnaMuons_%SYS%')
    algSeq += muonSequenceTight
    vars += [ 'OutMuons_NOSYS.eta -> mu_eta',
              'OutMuons_NOSYS.phi -> mu_phi',
              'OutMuons_%SYS%.pt  -> mu_pt_%SYS%',
              'OutMuons_NOSYS.charge -> mu_charge',
              'OutMuons_%SYS%.baselineSelection_medium -> mu_select_medium_%SYS%',
              'OutMuons_%SYS%.baselineSelection_tight  -> mu_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutMuons_%SYS%.muon_effSF_medium_%SYS% -> mu_effSF_medium_%SYS%',
                  'OutMuons_%SYS%.muon_effSF_tight_%SYS% -> mu_effSF_tight_%SYS%', ]


    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import \
        makeElectronAnalysisSequence
    if isPhyslite :
        input = 'AnalysisElectrons'
    else :
        input = 'Electrons'
    likelihood = True
    recomputeLikelihood=False
    if likelihood:
        workingpoint = 'LooseLHElectron.Loose_VarRad'
    else:
        workingpoint = 'LooseDNNElectron.Loose_VarRad'
    # FIXME: fails for PHYSLITE with missing data item
    # ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000
    if noPhysliteBroken :
        workingpoint = workingpoint.split('.')[0] + '.NonIso'
    electronSequence = makeElectronAnalysisSequence( dataType, workingpoint, postfix = 'loose',
                                                     recomputeLikelihood=recomputeLikelihood, enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    electronSequence.configure( inputName = input,
                                outputName = 'AnaElectrons_%SYS%' )
    algSeq += electronSequence
    vars += [ 'OutElectrons_%SYS%.pt  -> el_pt_%SYS%',
              'OutElectrons_NOSYS.phi -> el_phi',
              'OutElectrons_NOSYS.eta -> el_eta',
              'OutElectrons_NOSYS.charge -> el_charge',
              'OutElectrons_%SYS%.baselineSelection_loose -> el_select_loose_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutElectrons_%SYS%.effSF_loose_%SYS% -> el_effSF_loose_%SYS%', ]


    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import \
        makePhotonAnalysisSequence
    if isPhyslite :
        input = 'AnalysisPhotons'
    else :
        input = 'Photons'
    photonSequence = makePhotonAnalysisSequence( dataType, 'Tight.FixedCutTight', postfix = 'tight',
                                                 recomputeIsEM=False, enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    photonSequence.configure( inputName = input,
                              outputName = 'AnaPhotons_%SYS%' )
    algSeq += photonSequence
    vars += [ 'OutPhotons_%SYS%.pt  -> ph_pt_%SYS%',
              'OutPhotons_NOSYS.phi -> ph_phi',
              'OutPhotons_NOSYS.eta -> ph_eta',
              'OutPhotons_%SYS%.baselineSelection_tight -> ph_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutPhotons_%SYS%.ph_effSF_tight_%SYS% -> ph_effSF_tight_%SYS%', ]


    # Include, and then set up the tau analysis algorithm sequence:
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    if isPhyslite :
        input = 'AnalysisTauJets'
    else :
        input = 'TauJets'
    tauSequence = makeTauAnalysisSequence( dataType, 'Tight', postfix = 'tight',
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    tauSequence.configure( inputName = input, outputName = 'AnaTauJets_%SYS%' )

    # Add the sequence to the job:
    algSeq += tauSequence
    vars += [ 'OutTauJets_%SYS%.pt  -> tau_pt_%SYS%',
              'OutTauJets_NOSYS.phi -> tau_phi',
              'OutTauJets_NOSYS.eta -> tau_eta',
              'OutTauJets_NOSYS.charge -> tau_charge',
              'OutTauJets_%SYS%.baselineSelection_tight -> tau_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutTauJets_%SYS%.tau_effSF_tight_%SYS% -> tau_effSF_tight_%SYS%', ]



    # FIX ME: temporarily disabled until di-taus are supported in R22
    # # Include, and then set up the tau analysis algorithm sequence:
    # from TauAnalysisAlgorithms.DiTauAnalysisSequence import makeDiTauAnalysisSequence
    # diTauSequence = makeDiTauAnalysisSequence( dataType, 'Tight', postfix = 'tight' )
    # diTauSequence.configure( inputName = 'DiTauJets', outputName = 'AnaDiTauJets_%SYS%' )

    # Add the sequence to the job:
    # disabling this, the standard test files don't have DiTauJets
    # algSeq += diTauSequence


    # set up pt-eta selection for all the object types
    # currently disabling most cuts, but leaving these as placeholders
    # the cuts I have are mostly to silence MET building warnings

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserElectronsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if electronMinPt is not None :
        selalg.selectionTool.minPt = electronMinPt
    if electronMaxEta is not None :
        selalg.selectionTool.maxEta = electronMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaElectrons_%SYS%'
    algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserPhotonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if photonMinPt is not None :
        selalg.selectionTool.minPt = photonMinPt
    if photonMaxEta is not None :
        selalg.selectionTool.maxEta = photonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaPhotons_%SYS%'
    algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserMuonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if muonMinPt is not None :
        selalg.selectionTool.minPt = muonMinPt
    if muonMaxEta is not None :
        selalg.selectionTool.maxEta = muonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaMuons_%SYS%'
    algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserTauJetsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if tauMinPt is not None :
        selalg.selectionTool.minPt = tauMinPt
    if tauMaxEta is not None :
        selalg.selectionTool.maxEta = tauMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaTauJets_%SYS%'
    algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserJetsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if jetMinPt is not None :
        selalg.selectionTool.minPt = jetMinPt
    if jetMaxEta is not None :
        selalg.selectionTool.maxEta = jetMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaJets_%SYS%'
    algSeq += selalg



    # Now make view containers for the inputs to the met calculation
    metInputs = { 'jets'      : 'AnaJets_%SYS%',
                  'taus'      : 'AnaTauJets_%SYS%',
                  'muons'     : 'AnaMuons_%SYS%',
                  'electrons' : 'AnaElectrons_%SYS%',
                  'photons'   : 'AnaPhotons_%SYS%' }
    # Include, and then set up the met analysis algorithm sequence:
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    if isPhyslite :
        metSuffix = 'AnalysisMET'
    else :
        metSuffix = jetContainer[:-4]
    metSequence = makeMetAnalysisSequence(
            dataType,
            metSuffix=metSuffix,
            electronsSelection = "selectPtEta && baselineSelection_loose,as_char",
            photonsSelection = "selectPtEta && baselineSelection_tight,as_char",
            muonsSelection = "selectPtEta && baselineSelection_medium,as_char",
            tausSelection = "selectPtEta && baselineSelection_tight,as_char" )
    metSequence.configure( inputName = metInputs,
                           outputName = 'AnaMET_%SYS%' )

    # Add the sequence to the job:
    algSeq += metSequence
    if not forCompare :
        vars += [
            'AnaMET_%SYS%.mpx   -> met_mpx_%SYS%',
            'AnaMET_%SYS%.mpy   -> met_mpy_%SYS%',
            'AnaMET_%SYS%.sumet -> met_sumet_%SYS%',
            'AnaMET_%SYS%.name  -> met_name_%SYS%',
        ]


    # Make view containers holding as inputs for OR
    orInputs = {
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            'taus'      : 'AnaTauJets_%SYS%'
        }

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORElectronsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_loose,as_char'
    selectalg.particles = 'AnaElectrons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg
    orInputs['electrons'] = 'AnaElectrons_%SYS%'

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORPhotonsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_tight,as_char'
    selectalg.particles = 'AnaPhotons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORMuonsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_medium,as_char'
    selectalg.particles = 'AnaMuons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORTauJetsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_tight,as_char'
    selectalg.particles = 'AnaTauJets_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORJetsSelectAlg' )
    selectalg.preselection = 'selectPtEta'
    selectalg.particles = 'AnaJets_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg


    # Include, and then set up the overlap analysis algorithm sequence:
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import \
        makeOverlapAnalysisSequence
    overlapSequence = makeOverlapAnalysisSequence( dataType, doTaus=False, enableCutflow=True, shallowViewOutput = False, inputLabel = 'preselectOR', outputLabel = 'passesOR' )
    overlapSequence.configure(
        inputName = {
            'electrons' : 'AnaElectrons_%SYS%',
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            'taus'      : 'AnaTauJets_%SYS%'
        },
        outputName = { } )

    algSeq += overlapSequence
    if not forCompare :
        vars += [
            'OutJets_%SYS%.passesOR_%SYS% -> jet_select_or_%SYS%',
            'OutElectrons_%SYS%.passesOR_%SYS% -> el_select_or_%SYS%',
            'OutPhotons_%SYS%.passesOR_%SYS% -> ph_select_or_%SYS%',
            'OutMuons_%SYS%.passesOR_%SYS% -> mu_select_or_%SYS%',
            'OutTauJets_%SYS%.passesOR_%SYS% -> tau_select_or_%SYS%',
        ]

    if dataType != 'data' :
        # Include, and then set up the generator analysis sequence:
        from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import \
            makeGeneratorAnalysisSequence
        generatorSequence = makeGeneratorAnalysisSequence( dataType, saveCutBookkeepers=True, runNumber=284500, cutBookkeepersSystematics=True )
        algSeq += generatorSequence
        vars += [ 'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%', ]


    # FIX ME: this algorithm should be run on both data and MC, but in MC
    # it produces strange errors in CutFlowSvc in Athena
    if not forCompare or dataType == 'data' :
        # Include, and then set up the trigger analysis sequence:
        from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import \
            makeTriggerAnalysisSequence
        triggerSequence = makeTriggerAnalysisSequence( dataType, triggerChains=triggerChains, noFilter=True )
        algSeq += triggerSequence
        vars += ['EventInfo.trigPassed_' + t + ' -> trigPassed_' + t for t in triggerChains]



    # make filtered output containers

    addOutputCopyAlgorithms (algSeq, 'Electrons', 'AnaElectrons_%SYS%', 'OutElectrons_%SYS%',
                             'selectPtEta&&baselineSelection_loose,as_char')
    addOutputCopyAlgorithms (algSeq, 'Photons', 'AnaPhotons_%SYS%', 'OutPhotons_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char')
    addOutputCopyAlgorithms (algSeq, 'Muons', 'AnaMuons_%SYS%', 'OutMuons_%SYS%',
                             'selectPtEta&&baselineSelection_medium,as_char')
    addOutputCopyAlgorithms (algSeq, 'TauJets', 'AnaTauJets_%SYS%', 'OutTauJets_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char')
    addOutputCopyAlgorithms (algSeq, 'Jets', 'AnaJets_%SYS%', 'OutJets_%SYS%',
                             'selectPtEta')

    pass



def makeSequenceBlocks (dataType, algSeq, vars, forCompare, isPhyslite, noPhysliteBroken) :

    configSeq = ConfigSequence ()


    if not isPhyslite :
        # Include, and then set up the pileup analysis sequence:
        prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

        from AsgAnalysisAlgorithms.AsgAnalysisConfig import \
            makePileupReweightingConfig
        makePileupReweightingConfig (configSeq,
                                     userPileupConfigs=prwfiles,
                                     userLumicalcFiles=lumicalcfiles)

    vars += [ 'EventInfo.runNumber     -> runNumber',
              'EventInfo.eventNumber   -> eventNumber', ]


    # FIX ME: this algorithm should be run on both data and MC, but in MC
    # it produces strange errors in CutFlowSvc in Athena
    if not forCompare or dataType == 'data' :
        # Skip events with no primary vertex:
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import \
            makePrimaryVertexConfig
        makePrimaryVertexConfig (configSeq)


    # Include, and then set up the electron analysis algorithm sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import makeElectronCalibrationConfig, makeElectronWorkingPointConfig

    likelihood = True
    recomputeLikelihood=False
    if likelihood:
        workingpoint = 'LooseLHElectron.Loose_VarRad'
    else:
        workingpoint = 'LooseDNNElectron.Loose_VarRad'
    # FIXME: fails for PHYSLITE with missing data item
    # ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000
    if noPhysliteBroken :
        workingpoint = workingpoint.split('.')[0] + '.NonIso'
    makeElectronCalibrationConfig (configSeq, 'AnaElectrons')
    makeElectronWorkingPointConfig (configSeq, 'AnaElectrons', workingpoint, postfix = 'loose',
                                    recomputeLikelihood=recomputeLikelihood)
    vars += [ 'OutElectrons_NOSYS.eta -> el_eta',
              'OutElectrons_NOSYS.phi -> el_phi',
              'OutElectrons_%SYS%.pt  -> el_pt_%SYS%',
              'OutElectrons_NOSYS.charge -> el_charge',
              'OutElectrons_%SYS%.baselineSelection_loose -> el_select_loose_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutElectrons_%SYS%.effSF_loose_%SYS% -> el_effSF_loose_%SYS%', ]


    # Include, and then set up the photon analysis algorithm sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import makePhotonCalibrationConfig, makePhotonWorkingPointConfig

    makePhotonCalibrationConfig (configSeq, 'AnaPhotons', recomputeIsEM=False)
    makePhotonWorkingPointConfig (configSeq, 'AnaPhotons', 'Tight.FixedCutTight', postfix = 'tight', recomputeIsEM=False)
    vars += [ 'OutPhotons_NOSYS.eta -> ph_eta',
              'OutPhotons_NOSYS.phi -> ph_phi',
              'OutPhotons_%SYS%.pt  -> ph_pt_%SYS%',
              'OutPhotons_%SYS%.baselineSelection_tight -> ph_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutPhotons_%SYS%.ph_effSF_tight_%SYS% -> ph_effSF_tight_%SYS%', ]


    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig, makeMuonWorkingPointConfig

    makeMuonCalibrationConfig (configSeq, 'AnaMuons')
    makeMuonWorkingPointConfig (configSeq, 'AnaMuons', workingPoint='Medium.Iso', postfix='medium')
    makeMuonWorkingPointConfig (configSeq, 'AnaMuons', workingPoint='Tight.Iso', postfix='tight')
    vars += [ 'OutMuons_NOSYS.eta -> mu_eta',
              'OutMuons_NOSYS.phi -> mu_phi',
              'OutMuons_%SYS%.pt  -> mu_pt_%SYS%',
              'OutMuons_NOSYS.charge -> mu_charge',
              'OutMuons_%SYS%.baselineSelection_medium -> mu_select_medium_%SYS%',
              'OutMuons_%SYS%.baselineSelection_tight  -> mu_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutMuons_%SYS%.muon_effSF_medium_%SYS% -> mu_effSF_medium_%SYS%', ]
        vars += [ 'OutMuons_%SYS%.muon_effSF_tight_%SYS% -> mu_effSF_tight_%SYS%', ]


    # Include, and then set up the tau analysis algorithm sequence:
    from TauAnalysisAlgorithms.TauAnalysisConfig import makeTauCalibrationConfig, makeTauWorkingPointConfig

    makeTauCalibrationConfig (configSeq, 'AnaTauJets')
    makeTauWorkingPointConfig (configSeq, 'AnaTauJets', workingPoint='Tight', postfix='tight')
    vars += [ 'OutTauJets_NOSYS.eta -> tau_eta',
              'OutTauJets_NOSYS.phi -> tau_phi',
              'OutTauJets_NOSYS.charge -> tau_charge',
              'OutTauJets_%SYS%.pt  -> tau_pt_%SYS%',
              'OutTauJets_%SYS%.baselineSelection_tight  -> tau_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutTauJets_%SYS%.tau_effSF_tight_%SYS% -> tau_effSF_tight_%SYS%', ]

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
    jetContainer = 'AntiKt4EMPFlowJets'
    makeJetAnalysisConfig( configSeq, 'AnaJets', jetContainer, runJvtUpdate = True, runNNJvtUpdate = True )
    vars += ['OutJets_%SYS%.pt  -> jet_pt_%SYS%',
             'OutJets_NOSYS.phi -> jet_phi',
             'OutJets_NOSYS.eta -> jet_eta', ]

    from JetAnalysisAlgorithms.JetJvtAnalysisConfig import makeJetJvtAnalysisConfig
    makeJetJvtAnalysisConfig( configSeq, 'AnaJets', jetContainer )
    if dataType != 'data':
        vars += [
            'OutJets_%SYS%.jvt_effSF_%SYS% -> jet_jvtEfficiency_%SYS%',
        ]

    if not noPhysliteBroken :
        from FTagAnalysisAlgorithms.FTagAnalysisConfig import makeFTagAnalysisConfig
        btagger = "DL1r"
        btagWP = "FixedCutBEff_77"
        makeFTagAnalysisConfig( configSeq, 'AnaJets', noEfficiency = False, legacyRecommendations = True,
                                btagger = btagger, btagWP = btagWP, kinematicSelection = True )
        vars += [
            'OutJets_%SYS%.ftag_select_' + btagger + '_' + btagWP + ' -> jet_ftag_select_%SYS%',
        ]
        if dataType != 'data' :
            vars += [
                'OutJets_%SYS%.ftag_effSF_' + btagger + '_' + btagWP + '_%SYS% -> jet_ftag_eff_%SYS%'
            ]


    if dataType != 'data' :
        # Include, and then set up the generator analysis sequence:
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import \
            makeGeneratorAnalysisConfig
        makeGeneratorAnalysisConfig( configSeq, saveCutBookkeepers=True, runNumber=284500, cutBookkeepersSystematics=True )
        vars += [ 'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%', ]


    # FIX ME: this algorithm should be run on both data and MC, but in MC
    # it produces strange errors in CutFlowSvc in Athena
    if not forCompare or dataType == 'data' :
        # Include, and then set up the trigger analysis sequence:
        from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import \
            makeTriggerAnalysisConfig
        makeTriggerAnalysisConfig( configSeq, triggerChains=triggerChains, noFilter=True )
        vars += ['EventInfo.trigPassed_' + t + ' -> trigPassed_' + t for t in triggerChains]


    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makePtEtaSelectionConfig

    makePtEtaSelectionConfig (configSeq, 'AnaElectrons',
                              selectionDecoration='selectPtEta',
                              minPt=electronMinPt, maxEta=electronMaxEta)
    makePtEtaSelectionConfig (configSeq, 'AnaPhotons',
                              selectionDecoration='selectPtEta',
                              minPt=photonMinPt, maxEta=photonMaxEta)
    makePtEtaSelectionConfig (configSeq, 'AnaMuons',
                              selectionDecoration='selectPtEta',
                              minPt=muonMinPt, maxEta=muonMaxEta)
    makePtEtaSelectionConfig (configSeq, 'AnaTauJets',
                              selectionDecoration='selectPtEta',
                              minPt=tauMinPt, maxEta=tauMaxEta)
    makePtEtaSelectionConfig (configSeq, 'AnaJets',
                              selectionDecoration='selectPtEta',
                              minPt=jetMinPt, maxEta=jetMaxEta)


    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeOutputThinningConfig

    makeOutputThinningConfig (configSeq, 'AnaElectrons',
                              selection='selectPtEta&&baselineSelection_loose,as_char',
                              outputName='OutElectrons')
    makeOutputThinningConfig (configSeq, 'AnaPhotons',
                              selection='selectPtEta&&baselineSelection_tight,as_char',
                              outputName='OutPhotons')
    makeOutputThinningConfig (configSeq, 'AnaMuons',
                              selection='selectPtEta&&baselineSelection_medium,as_char',
                              outputName='OutMuons')
    makeOutputThinningConfig (configSeq, 'AnaTauJets',
                              selection='selectPtEta&&baselineSelection_tight,as_char',
                              outputName='OutTauJets')
    makeOutputThinningConfig (configSeq, 'AnaJets',
                              selection='selectPtEta',
                              outputName='OutJets')



    configAccumulator = ConfigAccumulator (dataType, algSeq, isPhyslite=isPhyslite)
    configSeq.fullConfigure (configAccumulator)



def makeSequence (dataType, useBlocks, forCompare, noSystematics, hardCuts = False, isPhyslite = False, noPhysliteBroken = False) :

    # do some harder cuts on all object types, this is mostly used for
    # benchmarking
    if hardCuts :
        global electronMinPt
        electronMinPt = 27e3
        global photonMinPt
        photonMinPt = 27e3
        global muonMinPt
        muonMinPt = 27e3
        global tauMinPt
        tauMinPt = 27e3
        global jetMinPt
        jetMinPt = 45e3

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    if not noSystematics :
        sysService.sigmaRecommended = 1

    vars = []
    if not useBlocks :
        makeSequenceOld (dataType, algSeq, vars=vars, forCompare=forCompare,
                         isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken)
    else :
        makeSequenceBlocks (dataType, algSeq, vars=vars, forCompare=forCompare,
                            isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken)


    # Add an ntuple dumper algorithm:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'analysis'
    # the auto-flush setting still needs to be figured out
    #treeMaker.TreeAutoFlush = 0
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'analysis'
    ntupleMaker.Branches = vars
    # ntupleMaker.OutputLevel = 2  # For output validation
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'analysis'
    algSeq += treeFiller

    return algSeq
