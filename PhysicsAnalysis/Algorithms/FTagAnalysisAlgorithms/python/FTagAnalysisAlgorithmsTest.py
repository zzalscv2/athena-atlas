# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createService

def makeSequence (dataType, jetContainer="AntiKt4EMPFlowJets") :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer,
                                           enableCutflow=True, enableKinematicHistograms=True )
    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
   
    btagOPs = ['FixedCutBEff_60', 'Continuous']
    for btagOP in btagOPs:
        makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = True,
                                  enableCutflow=True, btagger = 'DL1dv01', btagWP = btagOP )
    jetSequence.configure( inputName = jetContainer, outputName = 'AnalysisJets_%SYS%' )

    # Add the sequence to the job:
    algSeq += jetSequence

    return algSeq
