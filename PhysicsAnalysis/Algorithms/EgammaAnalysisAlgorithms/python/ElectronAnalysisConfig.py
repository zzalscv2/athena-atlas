# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

# E/gamma import(s).
from xAODEgamma.xAODEgammaParameters import xAOD

import PATCore.ParticleDataType


class ElectronCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the electron four-momentum correction"""

    def __init__ (self, containerName) :
        super (ElectronCalibrationConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('crackVeto', False, type=bool)
        self.addOption ('ptSelectionOutput', False, type=bool)
        self.addOption ('isolationCorrection', False, type=bool)
        self.addOption ('trackSelection', True, type=bool)



    def makeAlgs (self, config) :

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisElectrons")
        else :
            config.setSourceName (self.containerName, "Electrons")

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'ElectronShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)


        # Set up the eta-cut on all electrons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronEtaCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.47
        if self.crackVeto:
            alg.selectionTool.etaGapLow = 1.37
            alg.selectionTool.etaGapHigh = 1.52
        alg.selectionTool.useClusterEta = True
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=(5 if self.crackVeto else 4))

        # Set up the track selection algorithm:
        if self.trackSelection :
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                                        'ElectronTrackSelectionAlg' + self.postfix )
            alg.selectionDecoration = 'trackSelection' + self.postfix + ',as_bits'
            alg.maxD0Significance = 5
            alg.maxDeltaZ0SinTheta = 0.5
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=3)

        # Select electrons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronObjectQualityAlg' + self.postfix )
        alg.selectionDecoration = 'goodOQ' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = xAOD.EgammaParameters.BADCLUSELECTRON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=1)

        # Set up the calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg',
                                      'ElectronCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                               'CP::EgammaCalibrationAndSmearingTool' )
        alg.calibrationAndSmearingTool.ESModel = 'es2022_R22_PRE'
        alg.calibrationAndSmearingTool.decorrelationModel = '1NP_v1'
        if config.dataType() == 'afii':
            alg.calibrationAndSmearingTool.useAFII = 1
        else:
            alg.calibrationAndSmearingTool.useAFII = 0
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the the pt selection
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronPtCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.minPt = 4.5e3
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=2, preselection=self.ptSelectionOutput)

        # Set up the isolation correction algorithm:
        if self.isolationCorrection:
            alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                          'ElectronIsolationCorrectionAlg' + self.postfix )
            config.addPrivateTool( 'isolationCorrectionTool',
                                   'CP::IsolationCorrectionTool' )
            if config.dataType() == 'data':
                alg.isolationCorrectionTool.IsMC = 0
            else:
                alg.isolationCorrectionTool.IsMC = 1
            alg.egammas = config.readName (self.containerName)
            alg.egammasOut = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)



class ElectronWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the electron working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self, containerName, postfix) :
        super (ElectronWorkingPointConfig, self).__init__ (containerName + '.' + postfix)
        self.containerName = containerName
        self.selectionName = postfix
        self.addOption ('postfix', None, type=str)
        self.addOption ('likelihoodWP', None, type=str)
        self.addOption ('isolationWP', None, type=str)
        self.addOption ('recomputeLikelihood', False, type=bool)
        self.addOption ('chargeIDSelection', False, type=bool)

    def makeAlgs (self, config) :

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if 'LH' in self.likelihoodWP:
            # Set up the likelihood ID selection algorithm
            # It is safe to do this before calibration, as the cluster E is used
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' + postfix )
            alg.selectionDecoration = 'selectLikelihood' + selectionPostfix + ',as_bits'
            if self.recomputeLikelihood:
                # Rerun the likelihood ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronLikelihoodTool' )
                alg.selectionTool.primaryVertexContainer = 'PrimaryVertices'
                alg.selectionTool.WorkingPoint = self.likelihoodWP
                algDecorCount = 7
            else:
                # Select from Derivation Framework flags
                config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
                dfFlag = "DFCommonElectronsLH" + self.likelihoodWP.split('LH')[0]
                alg.selectionTool.selectionFlags = [dfFlag]
                algDecorCount = 1
        else:
            # Set up the DNN ID selection algorithm
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronDNNAlg' + postfix )
            alg.selectionDecoration = 'selectDNN' + selectionPostfix + ',as_bits'
            if self.recomputeLikelihood:
                # Rerun the DNN ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronSelectorTool' )
                alg.selectionTool.WorkingPoint = self.likelihoodWP
                algDecorCount = 6
            else:
                # Select from Derivation Framework flags
                raise ValueError ( "DNN working points are not available in derivations yet.")
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             bits=algDecorCount)

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'ElectronIsolationSelectionAlg' + postfix )
            alg.selectionDecoration = 'isolated' + selectionPostfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.ElectronWP = self.isolationWP
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 bits=1)

        # Select electrons only if they don't appear to have flipped their charge.
        if self.chargeIDSelection:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                          'ElectronChargeIDSelectionAlg' + postfix )
            alg.selectionDecoration = 'chargeID' + selectionPostfix + ',as_bits'
            config.addPrivateTool( 'selectionTool',
                                   'AsgElectronChargeIDSelectorTool' )
            alg.selectionTool.TrainingFile = \
                'ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root'
            alg.selectionTool.WorkingPoint = 'Loose'
            alg.selectionTool.CutOnBDT = -0.337671 # Loose 97%
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 bits=1)

        # Set up an algorithm used for decorating baseline electron selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'ElectronSelectionSummary' + postfix )
        alg.selectionDecoration = 'baselineSelection' + selectionPostfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)
        config.addOutputVar (self.containerName, 'baselineSelection' + postfix, 'select' + postfix)

        # Set up the electron efficiency correction algorithm:
        if config.dataType() != 'data':
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlg' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.RecoKey = "Reconstruction"
            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            if config.dataType() == 'afii':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Fast
            elif config.dataType() == 'mc':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'effSF' + postfix)




def makeElectronCalibrationConfig( seq, containerName, postfix = None,
                                   crackVeto = None,
                                   ptSelectionOutput = None,
                                   isolationCorrection = None):
    """Create electron calibration configuration blocks

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      isolationCorrection -- Whether or not to perform isolation correction
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
    """

    config = ElectronCalibrationConfig (containerName)
    config.setOptionValue ('crackVeto', crackVeto, noneAction='ignore')
    config.setOptionValue ('ptSelectionOutput', ptSelectionOutput, noneAction='ignore')
    config.setOptionValue ('isolationCorrection', isolationCorrection, noneAction='ignore')
    seq.append (config)





def makeElectronWorkingPointConfig( seq, containerName, workingPoint,
                                    postfix,
                                    recomputeLikelihood = None,
                                    chargeIDSelection = None ):
    """Create electron analysis configuration blocks

    Keyword arguments:
      workingPoint -- The working point to use
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      recomputeLikelihood -- Whether to rerun the LH. If not, use derivation flags
      chargeIDSelection -- Whether or not to perform charge ID/flip selection
    """


    config = ElectronWorkingPointConfig (containerName, postfix)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "likelihood.isolation", not ' + workingPoint)
        config.setOptionValue ('likelihoodWP', splitWP[0])
        config.setOptionValue ('isolationWP', splitWP[1])
    config.setOptionValue ('recomputeLikelihood', recomputeLikelihood, noneAction='ignore')
    config.setOptionValue ('chargeIDSelection', chargeIDSelection, noneAction='ignore')
    seq.append (config)
