# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import ROOT

# E/gamma import(s).
from xAODEgamma.xAODEgammaParameters import xAOD

import PATCore.ParticleDataType

class PhotonCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the photon four-momentum correction"""

    def __init__ (self, containerName) :
        super (PhotonCalibrationConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('crackVeto', False, type=bool)
        self.addOption ('enableCleaning', True, type=bool)
        self.addOption ('cleaningAllowLate', False, type=bool)
        self.addOption ('recomputeIsEM', False, type=bool)
        self.addOption ('ptSelectionOutput', False, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisPhotons")
        else :
            config.setSourceName (self.containerName, "Photons")

        cleaningWP = 'NoTime' if self.cleaningAllowLate else ''

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'PhotonShallowCopyAlg' + postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all photons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonEtaCutAlg' + postfix )
        alg.selectionDecoration = 'selectEta' + postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.37
        if self.crackVeto:
            alg.selectionTool.etaGapLow = 1.37
            alg.selectionTool.etaGapHigh = 1.52
        alg.selectionTool.useClusterEta = True
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=(5 if self.crackVeto else 4))

        # Setup shower shape fudge
        if self.recomputeIsEM and config.dataType() == 'mc':
            alg = config.createAlgorithm( 'CP::PhotonShowerShapeFudgeAlg',
                                          'PhotonShowerShapeFudgeAlg' + postfix )
            config.addPrivateTool( 'showerShapeFudgeTool',
                                    'ElectronPhotonVariableCorrectionTool' )
            alg.showerShapeFudgeTool.ConfigFile = \
              'EGammaVariableCorrection/TUNE23/ElPhVariableNominalCorrection.conf'
            alg.photons = config.readName (self.containerName)
            alg.photonsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Select photons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonObjectQualityAlg' + postfix )
        alg.selectionDecoration = 'goodOQ,as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = xAOD.EgammaParameters.BADCLUSPHOTON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=1)

        # Select clean photons
        if self.enableCleaning:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonCleaningAlg' + postfix)
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            alg.selectionDecoration = 'isClean,as_bits'
            alg.selectionTool.selectionFlags = ['DFCommonPhotonsCleaning' + cleaningWP]
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 bits=1)

        # Do calibration
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg',
                                      'PhotonCalibrationAndSmearingAlg' + postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                               'CP::EgammaCalibrationAndSmearingTool' )
        alg.calibrationAndSmearingTool.ESModel = 'es2022_R22_PRE'
        alg.calibrationAndSmearingTool.decorrelationModel = '1NP_v1'
        if config.dataType() == 'afii':
            alg.calibrationAndSmearingTool.useAFII = 1
        else :
            alg.calibrationAndSmearingTool.useAFII = 0
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the the pt selection
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonPtCutAlg' + postfix )
        alg.selectionDecoration = 'selectPt' + postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.minPt = 10e3
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=2, preselection=self.ptSelectionOutput)

        # Set up the isolation correction algorithm.
        alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                      'PhotonIsolationCorrectionAlg' + postfix )
        config.addPrivateTool( 'isolationCorrectionTool',
                               'CP::IsolationCorrectionTool' )
        if config.dataType() == 'data':
            alg.isolationCorrectionTool.IsMC = 0
        else:
            alg.isolationCorrectionTool.IsMC = 1
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)



class PhotonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the photon working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self, containerName, postfix) :
        super (PhotonWorkingPointConfig, self).__init__ (containerName + '.' + postfix)
        self.containerName = containerName
        self.selectionName = postfix
        self.addOption ('postfix', postfix, type=str)
        self.addOption ('qualityWP', None, type=str)
        self.addOption ('isolationWP', None, type=str)
        self.addOption ('recomputeIsEM', False, type=bool)

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.qualityWP == 'Tight' :
            quality = ROOT.egammaPID.PhotonTight
        elif self.qualityWP == 'Loose' :
            quality = ROOT.egammaPID.PhotonLoose
        else :
            raise Exception ('unknown photon quality working point "' + self.qualityWP + '" should be Tight or Loose')

        # Set up the photon selection algorithm:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonIsEMSelectorAlg' + postfix )
        alg.selectionDecoration = 'selectEM,as_bits'
        if self.recomputeIsEM:
            # Rerun the cut-based ID
            config.addPrivateTool( 'selectionTool', 'AsgPhotonIsEMSelector' )
            alg.selectionTool.isEMMask = quality
            alg.selectionTool.ConfigFile = \
                'ElectronPhotonSelectorTools/offline/20180116/PhotonIsEMTightSelectorCutDefs.conf'
        else:
            # Select from Derivation Framework flags
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            dfFlag = 'DFCommonPhotonsIsEM' + self.qualityWP
            alg.selectionTool.selectionFlags = [ dfFlag ]
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             bits=(32 if self.recomputeIsEM else 1))

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'PhotonIsolationSelectionAlg' + postfix )
            alg.selectionDecoration = 'isolated' + postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.PhotonWP = self.isolationWP
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 bits=1)

        # Set up an algorithm used for decorating baseline photon selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'PhotonSelectionSummary' + postfix )
        alg.selectionDecoration = 'baselineSelection' + postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)
        config.addOutputVar (self.containerName, 'baselineSelection' + postfix, 'select' + postfix)

        if config.dataType() != 'data':
            # Set up the photon efficiency correction algorithm.
            alg = config.createAlgorithm( 'CP::PhotonEfficiencyCorrectionAlg',
                                          'PhotonEfficiencyCorrectionAlg' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgPhotonEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'ph_effSF' + postfix + '_%SYS%'
            if config.dataType() == 'afii':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full  # no AFII ID SFs for now
            elif config.dataType() == 'mc':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + postfix
            alg.photons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'effSF' + postfix)





def makePhotonCalibrationConfig( seq, containerName,
                                 postfix = None,
                                 crackVeto = None,
                                 enableCleaning = None,
                                 cleaningAllowLate = None,
                                 recomputeIsEM = None,
                                 ptSelectionOutput = None ):
    """Create photon calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keywrod arguments:
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      crackVeto -- Whether or not to perform eta crack veto
      enableCleaning -- Enable photon cleaning
      cleaningAllowLate -- Whether to ignore timing information in cleaning.
      recomputeIsEM -- Whether to rerun the cut-based selection. If not, use derivation flags
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
    """

    config = PhotonCalibrationConfig (containerName)
    config.setOptionValue ('postfix', postfix, noneAction='ignore')
    config.setOptionValue ('crackVeto', crackVeto, noneAction='ignore')
    config.setOptionValue ('enableCleaning', enableCleaning, noneAction='ignore')
    config.setOptionValue ('cleaningAllowLate', cleaningAllowLate, noneAction='ignore')
    config.setOptionValue ('recomputeIsEM', recomputeIsEM, noneAction='ignore')
    config.setOptionValue ('ptSelectionOutput', ptSelectionOutput, noneAction='ignore')
    seq.append (config)



def makePhotonWorkingPointConfig( seq, containerName, workingPoint, postfix,
                                  recomputeIsEM = None ):
    """Create photon analysis algorithms for a single working point

    Keywrod arguments:
      workingPoint -- The working point to use
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      recomputeIsEM -- Whether to rerun the cut-based selection. If not, use derivation flags
    """

    config = PhotonWorkingPointConfig (containerName, postfix)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)
        config.setOptionValue ('qualityWP',     splitWP[0])
        config.setOptionValue ('isolationWP',   splitWP[1])
    config.setOptionValue ('recomputeIsEM', recomputeIsEM, noneAction='ignore')
    seq.append (config)
