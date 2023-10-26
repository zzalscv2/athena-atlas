# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, createService
from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

def makeSequence (dataType) :

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

    # Add the pileup sequence to the job:
    algSeq += pileupSequence



    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig, makeMuonWorkingPointConfig

    configSeq = ConfigSequence ()

    makeMuonCalibrationConfig (configSeq, 'AnalysisMuons')
    makeMuonWorkingPointConfig (configSeq, 'AnalysisMuons', workingPoint='Medium.Loose_VarRad', selectionName='medium')
    makeMuonWorkingPointConfig (configSeq, 'AnalysisMuons', workingPoint='Tight.Loose_VarRad', selectionName='tight')

    configAccumulator = ConfigAccumulator (dataType, algSeq)
    configSeq.fullConfigure (configAccumulator)


    # Add an ntuple dumper algorithm:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'muons'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMakerEventInfo' )
    ntupleMaker.TreeName = 'muons'
    ntupleMaker.Branches = [ 'EventInfo.runNumber     -> runNumber',
                             'EventInfo.eventNumber   -> eventNumber', ]
    algSeq += ntupleMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMakerMuons' )
    ntupleMaker.TreeName = 'muons'
    ntupleMaker.Branches = [ 'AnalysisMuons_NOSYS.eta -> mu_eta',
                             'AnalysisMuons_NOSYS.phi -> mu_phi',
                             'AnalysisMuons_%SYS%.pt  -> mu_%SYS%_pt', ]
    # need to allow for conversion of dataType from string to enum
    if {'data': DataType.Data, 'mc': DataType.FullSim, 'afii': DataType.FastSim}.get(dataType, dataType) is not DataType.Data:
        ntupleMaker.Branches += [ 'AnalysisMuons_%SYS%.muon_reco_effSF_tight_%SYS% -> mu_%SYS%_reco_effSF' ]
    ntupleMaker.OutputLevel = 2  # For output validation
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'muons'
    algSeq += treeFiller

    return algSeq
