# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack
# @author Tadej Novak

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, createService

def makeOverlapSequence (dataType) :
    algSeq = AlgSequence()

    # Skip events with no primary vertex:
    algSeq += createAlgorithm( 'CP::VertexSelectionAlg',
                               'PrimaryVertexSelectorAlg' )
    algSeq.PrimaryVertexSelectorAlg.VertexContainer = 'PrimaryVertices'
    algSeq.PrimaryVertexSelectorAlg.MinVertices = 1

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )

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
    algSeq += pileupSequence

    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import \
        makeElectronAnalysisSequence
    electronSequence = makeElectronAnalysisSequence( dataType, 'LooseLHElectron.Loose_VarRad',
                                                     recomputeLikelihood = True )
    electronSequence.configure( inputName = 'Electrons',
                                outputName = 'AnalysisElectrons_%SYS%' )
    algSeq += electronSequence

    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import \
        makePhotonAnalysisSequence
    photonSequence = makePhotonAnalysisSequence( dataType, 'Tight.FixedCutTight')
    photonSequence.configure( inputName = 'Photons',
                              outputName = 'AnalysisPhotons_%SYS%' )
    algSeq += photonSequence

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    muonSequence = makeMuonAnalysisSequence( dataType, 'Tight.Loose_VarRad' )
    muonSequence.configure( inputName = 'Muons',
                            outputName = 'AnalysisMuons_%SYS%' )
    algSeq += muonSequence

    # Include, and then set up the jet analysis algorithm sequence:
    jetContainer = 'AntiKt4EMPFlowJets'
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer )
    jetSequence.configure( inputName = jetContainer,
                           outputName = 'AnalysisJets_%SYS%' )
    algSeq += jetSequence

    # TODO: disabled for now
    # Include, and then set up the tau analysis algorithm sequence:
    # from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    # tauSequence = makeTauAnalysisSequence( dataType, 'Tight' )
    # tauSequence.configure( inputName = 'TauJets',
    #                        outputName = 'AnalysisTauJets_%SYS%' )
    # algSeq += tauSequence

    # Include, and then set up the overlap analysis algorithm sequence:
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import \
        makeOverlapAnalysisSequence
    overlapSequence = makeOverlapAnalysisSequence( dataType, doTaus=False, enableCutflow=True )
    overlapSequence.configure(
        inputName = {
            'electrons' : 'AnalysisElectrons_%SYS%',
            'photons'   : 'AnalysisPhotons_%SYS%',
            'muons'     : 'AnalysisMuons_%SYS%',
            'jets'      : 'AnalysisJets_%SYS%',
            # 'taus'      : 'AnalysisTauJets_%SYS%'
        },
        outputName = {
            'electrons' : 'AnalysisElectronsOR_%SYS%',
            'photons'   : 'AnalysisPhotonsOR_%SYS%',
            'muons'     : 'AnalysisMuonsOR_%SYS%',
            'jets'      : 'AnalysisJetsOR_%SYS%',
            # 'taus'      : 'AnalysisTauJetsOR_%SYS%'
        } )
    algSeq += overlapSequence

    # Set up an ntuple to check the job with:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'particles'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'particles'
    ntupleMaker.Branches = [
        'EventInfo.runNumber   -> runNumber',
        'EventInfo.eventNumber -> eventNumber',
        'AnalysisElectrons_%SYS%.eta -> el_%SYS%_eta',
        'AnalysisElectrons_%SYS%.phi -> el_%SYS%_phi',
        'AnalysisElectrons_%SYS%.pt  -> el_%SYS%_pt',
        'AnalysisElectronsOR_%SYS%.eta -> el_OR_%SYS%_eta',
        'AnalysisElectronsOR_%SYS%.phi -> el_OR_%SYS%_phi',
        'AnalysisElectronsOR_%SYS%.pt  -> el_OR_%SYS%_pt',
        'AnalysisPhotons_%SYS%.eta -> ph_%SYS%_eta',
        'AnalysisPhotons_%SYS%.phi -> ph_%SYS%_phi',
        'AnalysisPhotons_%SYS%.pt  -> ph_%SYS%_pt',
        'AnalysisPhotonsOR_%SYS%.eta -> ph_OR_%SYS%_eta',
        'AnalysisPhotonsOR_%SYS%.phi -> ph_OR_%SYS%_phi',
        'AnalysisPhotonsOR_%SYS%.pt  -> ph_OR_%SYS%_pt',
        'AnalysisMuons_%SYS%.eta -> mu_%SYS%_eta',
        'AnalysisMuons_%SYS%.phi -> mu_%SYS%_phi',
        'AnalysisMuons_%SYS%.pt  -> mu_%SYS%_pt',
        'AnalysisMuonsOR_%SYS%.eta -> mu_OR_%SYS%_eta',
        'AnalysisMuonsOR_%SYS%.phi -> mu_OR_%SYS%_phi',
        'AnalysisMuonsOR_%SYS%.pt  -> mu_OR_%SYS%_pt',
        'AnalysisJets_%SYS%.eta -> jet_%SYS%_eta',
        'AnalysisJets_%SYS%.phi -> jet_%SYS%_phi',
        'AnalysisJets_%SYS%.pt  -> jet_%SYS%_pt',
        'AnalysisJetsOR_%SYS%.eta -> jet_OR_%SYS%_eta',
        'AnalysisJetsOR_%SYS%.phi -> jet_OR_%SYS%_phi',
        'AnalysisJetsOR_%SYS%.pt  -> jet_OR_%SYS%_pt',
        # 'AnalysisTauJets_%SYS%.eta -> tau_%SYS%_eta',
        # 'AnalysisTauJets_%SYS%.phi -> tau_%SYS%_phi',
        # 'AnalysisTauJets_%SYS%.pt  -> tau_%SYS%_pt',
        # 'AnalysisTauJetsOR_%SYS%.eta -> tau_OR_%SYS%_eta',
        # 'AnalysisTauJetsOR_%SYS%.phi -> tau_OR_%SYS%_phi',
        # 'AnalysisTauJetsOR_%SYS%.pt  -> tau_OR_%SYS%_pt'
    ]
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'particles'
    algSeq += treeFiller

    return algSeq


def makeEventAlgorithmsSequence (dataType) :

    # Config:
    GRLFiles = ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml']

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )

    # Include, and then set up the pileup analysis sequence:
    from AsgAnalysisAlgorithms.EventSelectionAnalysisSequence import \
        makeEventSelectionAnalysisSequence
    eventSelectionSequence = \
        makeEventSelectionAnalysisSequence( dataType, userGRLFiles=GRLFiles )
    algSeq += eventSelectionSequence

    # Set up an ntuple to check the job with:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'events'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'events'
    ntupleMaker.Branches = [
        'EventInfo.runNumber   -> runNumber',
        'EventInfo.eventNumber -> eventNumber',
        'Electrons.eta         -> el_eta',
        'Electrons.phi         -> el_phi',
        ]
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'events'
    algSeq += treeFiller

    return algSeq


def makeGeneratorAlgorithmsSequence (dataType) :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )

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
    algSeq += pileupSequence

    # Include, and then set up the generator analysis sequence:
    from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import \
        makeGeneratorAnalysisSequence
    generatorSequence = makeGeneratorAnalysisSequence( dataType, saveCutBookkeepers=True, runNumber=284500, cutBookkeepersSystematics=True )
    algSeq += generatorSequence

    # Set up an ntuple to check the job with:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'events'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'events'
    ntupleMaker.Branches = [
        'EventInfo.runNumber   -> runNumber',
        'EventInfo.eventNumber -> eventNumber',
        'EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%',
    ]
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'events'
    algSeq += treeFiller

    return algSeq

def pileupConfigFiles(dataType):
    """Return the PRW config files and lumicalc files for tests"""
    if dataType == "data":
        prwfiles = []
        lumicalcfiles = []
    else:
        lumicalcfiles = [
            "GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",
            "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",
        ]
        if dataType == "mc":
            prwfiles = [
                "PileupReweighting/mc20_common/mc20a.284500.physlite.prw.v1.root"
            ]
        else:
            # use fast sim case to test running without PRW
            prwfiles = []
    return prwfiles, lumicalcfiles

def makePileupSequence () :
    # NB: test only run for MC

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )

    # Include, and then set up the pileup analysis sequence:
    prwfiles, lumicalcfiles = pileupConfigFiles("mc")

    from AsgAnalysisAlgorithms.PileupAnalysisSequence import \
        makePileupAnalysisSequence
    pileupSequence = makePileupAnalysisSequence(
        "mc",
        userPileupConfigs=prwfiles,
        userLumicalcFiles=lumicalcfiles,
    )
    pileupSequence.configure( inputName = {}, outputName = {} )
    algSeq += pileupSequence


    # Set up an ntuple to check the job with:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'events'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'events'
    ntupleMaker.Branches = [
        'EventInfo.runNumber   -> runNumber',
        'EventInfo.eventNumber -> eventNumber',
        # we currently don't have any pileup calib files for this test
        # 'EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%',
    ]
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'events'
    algSeq += treeFiller

    return algSeq
