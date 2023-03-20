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


def makeSequenceOld (dataType, algSeq, forCompare, isPhyslite, noPhysliteBroken, autoconfigFromFlags=None) :

    vars = []
    metVars = []

    # Include, and then set up the pileup analysis sequence:
    if not isPhyslite :
        campaign, files, prwfiles, lumicalcfiles = None, None, None, None
        useDefaultConfig = False
        if autoconfigFromFlags is not None:
            campaign = autoconfigFromFlags.Input.MCCampaign
            files = autoconfigFromFlags.Input.Files
            useDefaultConfig = True 
        else:
            prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

        from AsgAnalysisAlgorithms.PileupAnalysisSequence import \
            makePileupAnalysisSequence
        pileupSequence = makePileupAnalysisSequence(
            dataType,
            campaign=campaign,
            files=files,
            useDefaultConfig=useDefaultConfig,
            userPileupConfigs=prwfiles,
            userLumicalcFiles=lumicalcfiles,
        )
        pileupSequence.configure( inputName = {}, outputName = {} )

        # Add the pileup sequence to the job:
        algSeq += pileupSequence

    # Add the pileup sequence to the job:
    vars += [ 'EventInfo.runNumber     -> runNumber',
             'EventInfo.eventNumber   -> eventNumber', ]


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
                                           runJvtUpdate = False, runNNJvtUpdate = True,
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False,
                                           runGhostMuonAssociation = not isPhyslite)

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
    btagger = "DL1dv01"
    btagWP = "FixedCutBEff_60"
    makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = False,
                              enableCutflow=True, btagger = btagger, btagWP = btagWP, kinematicSelection = True )
    vars += [
        'OutJets_%SYS%.ftag_select_' + btagger + '_' + btagWP + ' -> jet_ftag_select_%SYS%',
    ]
    if dataType != 'data' :
        vars += [
            'OutJets_%SYS%.ftag_effSF_' + btagger + '_' + btagWP + '_%SYS% -> jet_ftag_eff_%SYS%'
        ]
    if not noPhysliteBroken :
        from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
        btagger = "DL1r"
        btagWP = "FixedCutBEff_77"
        makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = False, legacyRecommendations = True,
                                  enableCutflow=True, btagger = btagger, btagWP = btagWP, kinematicSelection = True, postfix='legacy' )
        vars += [
            'OutJets_%SYS%.ftag_select_' + btagger + '_' + btagWP + ' -> jet_ftag_legacy_select_%SYS%',
        ]
        if dataType != 'data' :
            vars += [
                'OutJets_%SYS%.ftag_effSF_' + btagger + '_' + btagWP + '_%SYS% -> jet_ftag_legacy_eff_%SYS%'
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
                                                   workingPoint = 'Medium.Loose_VarRad', postfix = 'medium',
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
                                                  workingPoint = 'Tight.Loose_VarRad', postfix = 'tight',
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
    metVars += [
        'AnaMET_%SYS%.met   -> met_met_%SYS%',
        'AnaMET_%SYS%.phi   -> met_phi_%SYS%',
        'AnaMET_%SYS%.sumet -> met_sumet_%SYS%',
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
    overlapSequence = makeOverlapAnalysisSequence( dataType, doTaus=True, enableCutflow=True, shallowViewOutput = False, inputLabel = 'preselectOR', outputLabel = 'passesOR' )
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
    if len (metVars) > 0:
        ntupleMaker = createAlgorithm( 'CP::AsgxAODMetNTupleMakerAlg', 'MetNTupleMaker' )
        ntupleMaker.TreeName = 'analysis'
        ntupleMaker.Branches = metVars
        #ntupleMaker.OutputLevel = 2  # For output validation
        algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'analysis'
    algSeq += treeFiller

    pass



def makeSequenceBlocks (dataType, algSeq, forCompare, isPhyslite, noPhysliteBroken, autoconfigFromFlags=None) :

    vars = []
    metVars = []

    from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

    # it seems the right containers are in the test input files so far
    largeRJets = False
    # there are no track jets in PHYSLITE, or in the sequence configuration
    trackJets = not isPhyslite and not forCompare

    configSeq = ConfigSequence ()

    outputContainers = {'mu_': 'OutMuons',
                        'el_': 'OutElectrons',
                        'ph_': 'OutPhotons',
                        'tau_': 'OutTauJets',
                        'jet_': 'OutJets',
                        'met_': 'AnaMET'}

    if not isPhyslite :
        campaign, files, prwfiles, lumicalcfiles = None, None, None, None
        useDefaultConfig = False
        if autoconfigFromFlags is not None:
            campaign = autoconfigFromFlags.Input.MCCampaign
            files = autoconfigFromFlags.Input.Files
            useDefaultConfig = True 
        else:
            prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

        configSeq += makeConfig ('Event.PileupReweighting', None)
        configSeq.setOptionValue ('.campaign', campaign, noneAction='ignore')
        configSeq.setOptionValue ('.files', files, noneAction='ignore')
        configSeq.setOptionValue ('.useDefaultConfig', useDefaultConfig)
        configSeq.setOptionValue ('.userPileupConfigs', prwfiles, noneAction='ignore')
        configSeq.setOptionValue ('.userLumicalcFiles', lumicalcfiles, noneAction='ignore')

    vars += [ 'EventInfo.runNumber     -> runNumber',
              'EventInfo.eventNumber   -> eventNumber', ]


    # Skip events with no primary vertex:
    configSeq += makeConfig ('Event.Cleaning', None)


    # Include, and then set up the jet analysis algorithm sequence:
    configSeq += makeConfig( 'Jets', 'AnaJets', jetCollection='AntiKt4EMPFlowJets')
    configSeq.setOptionValue ('.runJvtUpdate', False )
    configSeq.setOptionValue ('.runNNJvtUpdate', True )

    btagger = "DL1dv01"
    btagWP = "FixedCutBEff_60"
    configSeq += makeConfig( 'FlavourTagging', 'AnaJets.ftag' )
    configSeq.setOptionValue ('.noEfficiency', False)
    configSeq.setOptionValue ('.btagger', btagger)
    configSeq.setOptionValue ('.btagWP', btagWP)
    configSeq.setOptionValue ('.kinematicSelection', True )
    if not noPhysliteBroken :
        btagger = "DL1r"
        btagWP = "FixedCutBEff_77"
        configSeq += makeConfig( 'FlavourTagging', 'AnaJets.ftag_legacy' )
        configSeq.setOptionValue ('.noEfficiency', False)
        configSeq.setOptionValue ('.legacyRecommendations', True)
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)
        configSeq.setOptionValue ('.kinematicSelection', True )

    configSeq += makeConfig( 'Jets.Jvt', 'AnaJets' )

    if largeRJets :
        configSeq += makeConfig( 'Jets', 'AnaLargeRJets', jetCollection='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets')
        configSeq.setOptionValue ('.postfix', 'largeR_jets' )
        outputContainers['larger_jet_'] = 'OutLargeRJets'

    if trackJets :
        configSeq += makeConfig( 'Jets', 'AnaTrackJets', jetCollection='AntiKtVR30Rmax4Rmin02PV0TrackJets')
        configSeq.setOptionValue ('.postfix', 'track_jets' )
        outputContainers['track_jet_'] = 'OutTrackJets'


    # Include, and then set up the electron analysis algorithm sequence:

    likelihood = True
    recomputeLikelihood=False
    configSeq += makeConfig ('Electrons', 'AnaElectrons')
    configSeq += makeConfig ('Electrons.Selection', 'AnaElectrons.loose')
    if likelihood:
        configSeq.setOptionValue ('.likelihoodWP', 'LooseLHElectron')
    else:
        configSeq.setOptionValue ('.likelihoodWP', 'LooseDNNElectron')
    # FIXME: fails for PHYSLITE with missing data item
    # ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000
    if not noPhysliteBroken :
        configSeq.setOptionValue ('.isolationWP', 'Loose_VarRad')
    else :
        configSeq.setOptionValue ('.isolationWP', 'NonIso')
    configSeq.setOptionValue ('.recomputeLikelihood', recomputeLikelihood)


    # Include, and then set up the photon analysis algorithm sequence:
    configSeq += makeConfig ('Photons', 'AnaPhotons')
    configSeq.setOptionValue ('.recomputeIsEM', False)
    configSeq += makeConfig ('Photons.Selection', 'AnaPhotons.tight')
    configSeq.setOptionValue ('.qualityWP', 'Tight')
    configSeq.setOptionValue ('.isolationWP', 'FixedCutTight')
    configSeq.setOptionValue ('.recomputeIsEM', False)


    # set up the muon analysis algorithm sequence:
    from AthenaConfiguration.Enums import LHCPeriod
    run3Muons = autoconfigFromFlags is not None and autoconfigFromFlags.GeoModel.Run >= LHCPeriod.Run3

    configSeq += makeConfig ('Muons', 'AnaMuons')
    configSeq += makeConfig ('Muons.Selection', 'AnaMuons.medium')
    configSeq.setOptionValue ('.quality', 'Medium')
    configSeq.setOptionValue ('.isolation', 'Loose_VarRad')
    configSeq.setOptionValue ('.isRun3Geo', run3Muons)
    configSeq += makeConfig ('Muons.Selection', 'AnaMuons.tight')
    configSeq.setOptionValue ('.quality', 'Tight')
    configSeq.setOptionValue ('.isolation', 'Loose_VarRad')
    configSeq.setOptionValue ('.isRun3Geo', run3Muons)


    # Include, and then set up the tau analysis algorithm sequence:
    configSeq += makeConfig ('TauJets', 'AnaTauJets')
    configSeq += makeConfig ('TauJets.Selection', 'AnaTauJets.tight')
    configSeq.setOptionValue ('.quality', 'Tight')


    if dataType != 'data' :
        # Include, and then set up the generator analysis sequence:
        configSeq += makeConfig( 'Event.Generator', None)
        configSeq.setOptionValue ('.saveCutBookkeepers', True)
        configSeq.setOptionValue ('.runNumber', 284500)
        configSeq.setOptionValue ('.cutBookkeepersSystematics', True)
        vars += [ 'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%', ]


    # Include, and then set up the trigger analysis sequence:
    configSeq += makeConfig( 'Trigger.Chains', None )
    configSeq.setOptionValue ('.triggerChains', triggerChains )
    configSeq.setOptionValue ('.noFilter', True )


    configSeq += makeConfig ('Selection.PtEta', 'AnaElectrons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', electronMinPt, noneAction='ignore')
    configSeq.setOptionValue ('.maxEta', electronMaxEta, noneAction='ignore')
    configSeq += makeConfig ('Selection.PtEta', 'AnaPhotons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', photonMinPt, noneAction='ignore')
    configSeq.setOptionValue ('.maxEta', photonMaxEta, noneAction='ignore')
    configSeq += makeConfig ('Selection.PtEta', 'AnaMuons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', muonMinPt, noneAction='ignore')
    configSeq.setOptionValue ('.maxEta', muonMaxEta, noneAction='ignore')
    configSeq += makeConfig ('Selection.PtEta', 'AnaTauJets')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', tauMinPt, noneAction='ignore')
    configSeq.setOptionValue ('.maxEta', tauMaxEta, noneAction='ignore')
    configSeq += makeConfig ('Selection.PtEta', 'AnaJets')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', jetMinPt, noneAction='ignore')
    configSeq.setOptionValue ('.maxEta', jetMaxEta, noneAction='ignore')
    if largeRJets :
        configSeq += makeConfig ('Selection.PtEta', 'AnaLargeRJets')
        configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
        configSeq.setOptionValue ('.minPt', jetMinPt, noneAction='ignore')
        configSeq.setOptionValue ('.maxEta', jetMaxEta, noneAction='ignore')
    if trackJets :
        configSeq += makeConfig ('Selection.PtEta', 'AnaTrackJets')
        configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
        configSeq.setOptionValue ('.minPt', jetMinPt, noneAction='ignore')
        configSeq.setOptionValue ('.maxEta', jetMaxEta, noneAction='ignore')


    # Include, and then set up the met analysis algorithm config:
    configSeq += makeConfig ('MissingET', 'AnaMET')
    configSeq.setOptionValue ('.jets', 'AnaJets')
    configSeq.setOptionValue ('.taus', 'AnaTauJets.tight')
    configSeq.setOptionValue ('.electrons', 'AnaElectrons.loose')
    configSeq.setOptionValue ('.photons', 'AnaPhotons.tight')
    # Note that the configuration for the muons is not what you'd
    # normally do.  This is specifically here because this is a unit
    # test and I wanted to make sure that selection expressions work.
    # For an actual analysis that would just be `AnaMuons.medium`, but
    # since `tight` is a strict subset of `medium` it doesn't matter
    # if we do an "or" of the two.
    configSeq.setOptionValue ('.muons', 'AnaMuons.medium||tight')


    # Include, and then set up the overlap analysis algorithm config:
    configSeq += makeConfig( 'OverlapRemoval', None )
    configSeq.setOptionValue ('.electrons',   'AnaElectrons.loose')
    configSeq.setOptionValue ('.photons',     'AnaPhotons.tight')
    configSeq.setOptionValue ('.muons',       'AnaMuons.medium||tight')
    configSeq.setOptionValue ('.jets',        'AnaJets')
    configSeq.setOptionValue ('.taus',        'AnaTauJets.tight')
    configSeq.setOptionValue ('.inputLabel',  'preselectOR')
    configSeq.setOptionValue ('.outputLabel', 'passesOR' )


    configSeq += makeConfig ('Output.Thinning', 'AnaElectrons.Thinning')
    configSeq.setOptionValue ('.selectionName', 'loose')
    configSeq.setOptionValue ('.outputName', 'OutElectrons')
    configSeq += makeConfig ('Output.Thinning', 'AnaPhotons.Thinning')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutPhotons')
    configSeq += makeConfig ('Output.Thinning', 'AnaMuons.Thinning')
    configSeq.setOptionValue ('.selectionName', 'medium')
    configSeq.setOptionValue ('.outputName', 'OutMuons')
    configSeq += makeConfig ('Output.Thinning', 'AnaTauJets.Thinning')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutTauJets')
    configSeq += makeConfig ('Output.Thinning', 'AnaJets.Thinning')
    configSeq.setOptionValue ('.outputName', 'OutJets')
    if largeRJets :
        configSeq += makeConfig ('Output.Thinning', 'AnaLargeRJets.Thinning')
        configSeq.setOptionValue ('.outputName', 'OutLargeRJets')
    if trackJets :
        configSeq += makeConfig ('Output.Thinning', 'AnaTrackJets.Thinning')
        configSeq.setOptionValue ('.outputName', 'OutTrackJets')

    configSeq += makeConfig ('Output.Simple', 'Output')
    configSeq.setOptionValue ('.treeName', 'analysis')
    configSeq.setOptionValue ('.vars', vars)
    configSeq.setOptionValue ('.metVars', metVars)
    configSeq.setOptionValue ('.containers', outputContainers)

    configAccumulator = ConfigAccumulator (dataType, algSeq, isPhyslite=isPhyslite)
    configSeq.fullConfigure (configAccumulator)


def printSequenceAlgs (sequence) :
    """print the algorithms in the sequence without the sequence structure

    This is mostly meant for easy comparison of different sequences
    during configuration, particularly the sequences resulting from
    the old sequence configuration and the new block configuration.
    Those have different sequence structures in the output, but the
    algorithms should essentially be configured the same way."""
    if isinstance (sequence, AlgSequence) :
        for alg in sequence :
            printSequenceAlgs (alg)
    else :
        # assume this is an algorithm then
        print (sequence)


def makeSequence (dataType, useBlocks, forCompare, noSystematics, hardCuts = False, isPhyslite = False, noPhysliteBroken = False, autoconfigFromFlags = None) :

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

    # Need to explicitly instantiate the CutFlowSvc in Athena to allow
    # the event filters to run.  In AnalysisBase that is (currently)
    # not necessary, but we may need a CutFlowSvc instance to report
    # cutflows to the user.
    try :
        from EventBookkeeperTools.CutFlowHelpers import CreateCutFlowSvc
        CreateCutFlowSvc( seq=algSeq )
    except ModuleNotFoundError :
        # must be in AnalysisBase
        pass

    if not useBlocks :
        makeSequenceOld (dataType, algSeq, forCompare=forCompare,
                         isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken,
                         autoconfigFromFlags=autoconfigFromFlags)
    else :
        makeSequenceBlocks (dataType, algSeq, forCompare=forCompare,
                            isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken,
                            autoconfigFromFlags=autoconfigFromFlags)

    return algSeq
