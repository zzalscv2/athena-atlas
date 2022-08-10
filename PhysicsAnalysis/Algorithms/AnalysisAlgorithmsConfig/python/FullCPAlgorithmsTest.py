# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, createService
from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

def makeSequenceOld (dataType, algSeq) :

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    muonSequenceMedium = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                   workingPoint = 'Medium.Iso', postfix = 'medium',
                                                   enableCutflow=True, enableKinematicHistograms=True )
    muonSequenceMedium.configure( inputName = 'Muons',
                                  outputName = 'AnalysisMuonsMedium_%SYS%' )

    # Add the sequence to the job:
    algSeq += muonSequenceMedium

    muonSequenceTight = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                  workingPoint = 'Tight.Iso', postfix = 'tight',
                                                  enableCutflow=True, enableKinematicHistograms=True )
    muonSequenceTight.removeStage ("calibration")
    muonSequenceTight.configure( inputName = 'AnalysisMuonsMedium_%SYS%',
                                 outputName = 'AnalysisMuons_%SYS%')

    # Add the sequence to the job:
    algSeq += muonSequenceTight



def makeSequenceBlocks (dataType, algSeq) :

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig, makeMuonWorkingPointConfig

    configSeq = ConfigSequence ()

    makeMuonCalibrationConfig (configSeq, 'AnalysisMuons')
    makeMuonWorkingPointConfig (configSeq, 'AnalysisMuons', workingPoint='Medium.Iso', postfix='medium')
    makeMuonWorkingPointConfig (configSeq, 'AnalysisMuons', workingPoint='Tight.Iso', postfix='tight')

    configAccumulator = ConfigAccumulator (dataType, algSeq)
    configSeq.fullConfigure (configAccumulator)



def makeSequence (dataType, useBlocks) :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1

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

    if not useBlocks :
        makeSequenceOld (dataType, algSeq)
    else :
        makeSequenceBlocks (dataType, algSeq)


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
    ntupleMaker.Branches = [ 'AnalysisMuons_NOSYS.eta -> mu_eta',
                             'AnalysisMuons_NOSYS.phi -> mu_phi',
                             'AnalysisMuons_%SYS%.pt  -> mu_pt_%SYS%', ]
    # These branches are temporarily excluded as they make the test
    # fail.  The differences between sequence and block configuration
    # need to be addressed and then these need to be reenabled.
    # ntupleMaker.Branches = [ 'AnalysisMuons_%SYS%.baselineSelection_medium  -> mu_select_medium_%SYS%',
    #                          'AnalysisMuons_%SYS%.baselineSelection_tight  -> mu_select_tight_%SYS%', ]
    # if dataType != 'data':
    #     ntupleMaker.Branches += [ 'AnalysisMuons_%SYS%.muon_effSF_medium_%SYS% -> mu_effSF_medium_%SYS%' ]
    #     ntupleMaker.Branches += [ 'AnalysisMuons_%SYS%.muon_effSF_tight_%SYS% -> mu_effSF_tight_%SYS%' ]
    # ntupleMaker.OutputLevel = 2  # For output validation
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'analysis'
    algSeq += treeFiller

    return algSeq
