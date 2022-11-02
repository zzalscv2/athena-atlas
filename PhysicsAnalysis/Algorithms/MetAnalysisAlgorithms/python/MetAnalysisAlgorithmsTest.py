# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import addPrivateTool, createAlgorithm, createService

def makeSequence (dataType) :
    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetContainer = 'AntiKt4EMPFlowJets'
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer )
    jetSequence.configure( inputName = jetContainer, outputName = 'AnalysisJets_%SYS%' )

    # Add all algorithms to the job:
    algSeq += jetSequence

    # Set up a selection alg for demonstration purposes
    # Also to avoid warnings from building MET with very soft electrons

    # We need to copy here, because w/o an output container, it's assumed
    # that the input container is non-const
    eleCopyAlg = createAlgorithm( 'CP::AsgShallowCopyAlg', 'MetEleCopyAlg' )
    eleCopyAlg.input = 'Electrons'
    eleCopyAlg.output = 'METElectrons_%SYS%'
    algSeq += eleCopyAlg


    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'METEleSelAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    selalg.selectionTool.minPt = 10e3
    selalg.selectionTool.maxEta = 2.47
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'METElectrons_%SYS%'
    algSeq += selalg

    # Include, and then set up the met analysis algorithm sequence:
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    metSequence = makeMetAnalysisSequence(
            dataType,
            metSuffix = jetContainer[:-4],
            electronsSelection = 'selectPtEta'
    )
    metSequence.configure( inputName = { 'jets'      : 'AnalysisJets_%SYS%',
                                         'muons'     : 'Muons',
                                         'electrons' : 'METElectrons_%SYS%' },
                           outputName = 'AnalysisMET_%SYS%' )

    # Add the sequence to the job:
    algSeq += metSequence

    # Write the freshly produced MET object(s) to an output file:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'met'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'met'
    ntupleMaker.Branches = [ 'EventInfo.runNumber     -> runNumber',
                             'EventInfo.eventNumber   -> eventNumber',
                             'AnalysisMET_%SYS%.mpx   -> met_%SYS%_mpx',
                             'AnalysisMET_%SYS%.mpy   -> met_%SYS%_mpy',
                             'AnalysisMET_%SYS%.sumet -> met_%SYS%_sumet',
                             'AnalysisMET_%SYS%.name  -> met_%SYS%_name', ]
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'met'
    algSeq += treeFiller

    return algSeq
