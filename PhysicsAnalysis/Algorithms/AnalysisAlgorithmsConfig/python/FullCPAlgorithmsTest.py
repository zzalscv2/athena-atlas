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
    copyalg.deepCopy = True
    algSeq += copyalg


def makeSequenceOld (dataType, algSeq, vars, forCompare) :

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
    vars += [ 'EventInfo.runNumber     -> runNumber',
             'EventInfo.eventNumber   -> eventNumber', ]


    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetContainer = 'AntiKt4EMPFlowJets'
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer, enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
    makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = True, legacyRecommendations = True,
                              enableCutflow=True )

    jetSequence.configure( inputName = jetContainer, outputName = 'AnaJets_%SYS%' )


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
    if dataType != 'data':
        vars += [ 'OutJets_%SYS%.jvt_effSF_%SYS% -> jet_jvtEfficiency_%SYS%', ]
        if not forCompare :
            vars += [
                # 'EventInfo.jvt_effSF_%SYS% -> jvtSF_%SYS%',
                # 'EventInfo.fjvt_effSF_%SYS% -> fjvtSF_%SYS%',
                # 'OutJets_%SYS%.fjvt_effSF_NOSYS -> jet_fjvtEfficiency_%SYS%',
            ]


    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    muonSequenceMedium = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                   workingPoint = 'Medium.NonIso', postfix = 'medium',
                                                   enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    muonSequenceMedium.configure( inputName = 'Muons',
                                  outputName = 'AnaMuonsMedium_%SYS%' )

    # Add the sequence to the job:
    algSeq += muonSequenceMedium

    muonSequenceTight = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                  workingPoint = 'Tight.NonIso', postfix = 'tight',
                                                  enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    muonSequenceTight.removeStage ("calibration")
    muonSequenceTight.configure( inputName = 'AnaMuonsMedium_%SYS%',
                                 outputName = 'AnaMuons_%SYS%')

    # Add the sequence to the job:
    algSeq += muonSequenceTight
    vars += [ 'OutMuons_NOSYS.eta -> mu_eta',
              'OutMuons_NOSYS.phi -> mu_phi',
              'OutMuons_%SYS%.pt  -> mu_pt_%SYS%',
              'OutMuons_%SYS%.baselineSelection_medium -> mu_select_medium_%SYS%',
              'OutMuons_%SYS%.baselineSelection_tight  -> mu_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutMuons_%SYS%.muon_effSF_medium_%SYS% -> mu_effSF_medium_%SYS%',
                  'OutMuons_%SYS%.muon_effSF_tight_%SYS% -> mu_effSF_tight_%SYS%', ]


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
                                                     recomputeLikelihood=recomputeLikelihood, enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    electronSequence.configure( inputName = 'Electrons',
                                outputName = 'AnaElectrons_%SYS%' )
    algSeq += electronSequence
    vars += [ 'OutElectrons_%SYS%.pt  -> el_pt_%SYS%',
              'OutElectrons_NOSYS.phi -> el_phi',
              'OutElectrons_NOSYS.eta -> el_eta',
              'OutElectrons_%SYS%.baselineSelection_loose -> el_select_loose_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutElectrons_%SYS%.effSF_loose_%SYS% -> el_effSF_loose_%SYS%', ]


    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import \
        makePhotonAnalysisSequence
    photonSequence = makePhotonAnalysisSequence( dataType, 'Tight.FixedCutTight', postfix = 'tight',
                                                 recomputeIsEM=False, enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    photonSequence.configure( inputName = 'Photons',
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
    tauSequence = makeTauAnalysisSequence( dataType, 'Tight', postfix = 'tight',
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    tauSequence.configure( inputName = 'TauJets', outputName = 'AnaTauJets_%SYS%' )

    # Add the sequence to the job:
    algSeq += tauSequence
    vars += [ 'OutTauJets_%SYS%.pt  -> tau_pt_%SYS%',
              'OutTauJets_NOSYS.phi -> tau_phi',
              'OutTauJets_NOSYS.eta -> tau_eta',
              'OutTauJets_%SYS%.baselineSelection_tight -> tau_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutTauJets_%SYS%.tau_effSF_tight_%SYS% -> tau_effSF_tight_%SYS%', ]


    # temporarily disabled until di-taus are supported in R22
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
    if electronMinPt :
        selalg.selectionTool.minPt = electronMinPt
    if electronMaxEta :
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
    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METElectronsViewAlg' )
    viewalg.selection = [ 'selectPtEta', 'baselineSelection_loose,as_char' ]
    viewalg.input = 'AnaElectrons_%SYS%'
    viewalg.output = 'METElectrons_%SYS%'
    algSeq += viewalg

    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METPhotonsViewAlg' )
    viewalg.selection = [ 'selectPtEta', 'baselineSelection_tight,as_char' ]
    viewalg.input = 'AnaPhotons_%SYS%'
    viewalg.output = 'METPhotons_%SYS%'
    algSeq += viewalg

    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METMuonsViewAlg' )
    viewalg.selection = [ 'selectPtEta', 'baselineSelection_medium,as_char' ]
    viewalg.input = 'AnaMuons_%SYS%'
    viewalg.output = 'METMuons_%SYS%'
    algSeq += viewalg

    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METTauJetsViewAlg' )
    viewalg.selection = [ 'selectPtEta', 'baselineSelection_tight,as_char' ]
    viewalg.input = 'AnaTauJets_%SYS%'
    viewalg.output = 'METTauJets_%SYS%'
    algSeq += viewalg

    viewalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg','METJetsViewAlg' )
    viewalg.selection = [ 'selectPtEta' ]
    viewalg.input = 'AnaJets_%SYS%'
    viewalg.output = 'METJets_%SYS%'
    algSeq += viewalg

    # Include, and then set up the met analysis algorithm sequence:
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    metSequence = makeMetAnalysisSequence( dataType, metSuffix = jetContainer[:-4] )
    metSequence.configure( inputName = { 'jets'      : 'METJets_%SYS%',
                                         'taus'      : 'METTauJets_%SYS%',
                                         'muons'     : 'METMuons_%SYS%',
                                         'electrons' : 'METElectrons_%SYS%',
                                         'photons'   : 'METPhotons_%SYS%' },
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
    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORElectronsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_loose,as_char'
    selectalg.particles = 'AnaElectrons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    algSeq += selectalg

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
    overlapSequence = makeOverlapAnalysisSequence( dataType, doMuPFJetOR=True, doTaus=False, enableCutflow=True, shallowViewOutput = False, inputLabel = 'preselectOR', outputLabel = 'passesOR' )
    overlapSequence.configure(
        inputName = {
            'electrons' : 'AnaElectrons_%SYS%',
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            'taus'      : 'AnaTauJets_%SYS%'
        },
        outputName = { } )

    # FIX ME: temporarily disabling this for data, as there are some
    # errors with missing primary vertices
    if dataType != 'data' :
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
        if not forCompare :
            vars += [ 'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%', ]


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



def makeSequenceBlocks (dataType, algSeq, vars, forCompare) :

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
    vars += [ 'EventInfo.runNumber     -> runNumber',
              'EventInfo.eventNumber   -> eventNumber', ]


    configSeq = ConfigSequence ()


    # Include, and then set up the electron analysis algorithm sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import makeElectronCalibrationConfig, makeElectronWorkingPointConfig

    likelihood = True
    recomputeLikelihood=False
    if likelihood:
        workingpoint = 'LooseLHElectron.Loose_VarRad'
    else:
        workingpoint = 'LooseDNNElectron.Loose_VarRad'
    makeElectronCalibrationConfig (configSeq, 'AnaElectrons')
    makeElectronWorkingPointConfig (configSeq, 'AnaElectrons', workingpoint, postfix = 'loose',
                                    recomputeLikelihood=recomputeLikelihood)
    vars += [ 'OutElectrons_NOSYS.eta -> el_eta',
              'OutElectrons_NOSYS.phi -> el_phi',
              'OutElectrons_%SYS%.pt  -> el_pt_%SYS%',
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
              'OutTauJets_%SYS%.pt  -> tau_pt_%SYS%',
              'OutTauJets_%SYS%.baselineSelection_tight  -> tau_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutTauJets_%SYS%.tau_effSF_tight_%SYS% -> tau_effSF_tight_%SYS%', ]

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
    jetContainer = 'AntiKt4EMPFlowJets'
    makeJetAnalysisConfig( configSeq, 'AnaJets', jetContainer )
    vars += ['OutJets_%SYS%.pt  -> jet_pt_%SYS%',
             'OutJets_NOSYS.phi -> jet_phi',
             'OutJets_NOSYS.eta -> jet_eta', ]

    from JetAnalysisAlgorithms.JetJvtAnalysisConfig import makeJetJvtAnalysisConfig
    makeJetJvtAnalysisConfig( configSeq, 'AnaJets', jetContainer )
    if dataType != 'data':
        vars += [
            'OutJets_%SYS%.jvt_effSF_%SYS% -> jet_jvtEfficiency_%SYS%',
        ]


    configAccumulator = ConfigAccumulator (dataType, algSeq)
    configSeq.fullConfigure (configAccumulator)

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserElectronsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if electronMinPt is not None :
        selalg.selectionTool.minPt = electronMinPt
    if electronMaxEta is not None :
        selalg.selectionTool.maxEta = electronMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaElectrons_%SYS%'
    algSeq += selalg
    addOutputCopyAlgorithms (algSeq, 'Electrons', 'AnaElectrons_%SYS%', 'OutElectrons_%SYS%',
                             'selectPtEta&&baselineSelection_loose,as_char')

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserPhotonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if photonMinPt is not None :
        selalg.selectionTool.minPt = photonMinPt
    if photonMaxEta is not None :
        selalg.selectionTool.maxEta = photonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaPhotons_%SYS%'
    algSeq += selalg
    addOutputCopyAlgorithms (algSeq, 'Photons', 'AnaPhotons_%SYS%', 'OutPhotons_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char')

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserMuonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if muonMinPt is not None :
        selalg.selectionTool.minPt = muonMinPt
    if muonMaxEta is not None :
        selalg.selectionTool.maxEta = muonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaMuons_%SYS%'
    algSeq += selalg
    addOutputCopyAlgorithms (algSeq, 'Muons', 'AnaMuons_%SYS%', 'OutMuons_%SYS%',
                             'selectPtEta&&baselineSelection_medium,as_char')

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserTausSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if tauMinPt is not None :
        selalg.selectionTool.minPt = tauMinPt
    if tauMaxEta is not None :
        selalg.selectionTool.maxEta = tauMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaTauJets_%SYS%'
    algSeq += selalg
    addOutputCopyAlgorithms (algSeq, 'TauJets', 'AnaTauJets_%SYS%', 'OutTauJets_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char')

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserJetsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if jetMinPt is not None :
        selalg.selectionTool.minPt = jetMinPt
    if jetMaxEta is not None :
        selalg.selectionTool.maxEta = jetMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaJets_%SYS%'
    algSeq += selalg
    addOutputCopyAlgorithms (algSeq, 'Jets', 'AnaJets_%SYS%', 'OutJets_%SYS%',
                             'selectPtEta')



def makeSequence (dataType, useBlocks, forCompare) :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1

    vars = []
    if not useBlocks :
        makeSequenceOld (dataType, algSeq, vars=vars, forCompare=forCompare)
    else :
        makeSequenceBlocks (dataType, algSeq, vars=vars, forCompare=forCompare)


    # Add an ntuple dumper algorithm:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'analysis'
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
