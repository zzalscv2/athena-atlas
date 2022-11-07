# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import ROOT


class PhotonCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the photon four-momentum correction"""

    def __init__ (self, containerName, postfix,
                  crackVeto = False,
                  enableCleaning = True,
                  cleaningAllowLate = False,
                  recomputeIsEM = False,
                  ptSelectionOutput = False) :
        super (PhotonCalibrationConfig, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        if self.postfix != '' and self.postfix[0] != '_' :
            self.postfix = '_' + self.postfix
        self.crackVeto = crackVeto
        self.enableCleaning = enableCleaning
        self.cleaningAllowLate = cleaningAllowLate
        self.recomputeIsEM = recomputeIsEM
        self.ptSelectionOutput = ptSelectionOutput


    def makeAlgs (self, config) :

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisPhotons")
        else :
            config.setSourceName (self.containerName, "Photons")

        cleaningWP = 'NoTime' if self.cleaningAllowLate else ''

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'PhotonShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all photons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonEtaCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
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
                                          'PhotonShowerShapeFudgeAlg' + self.postfix )
            config.addPrivateTool( 'showerShapeFudgeTool',
                                   'ElectronPhotonShowerShapeFudgeTool' )
            alg.showerShapeFudgeTool.Preselection = 22 # Rel 21
            alg.showerShapeFudgeTool.FFCalibFile = \
                'ElectronPhotonShowerShapeFudgeTool/v2/PhotonFudgeFactors.root' # only for rel21
            alg.photons = config.readName (self.containerName)
            alg.photonsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Select photons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonObjectQualityAlg' + self.postfix )
        alg.selectionDecoration = 'goodOQ,as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = ROOT.xAOD.EgammaParameters.BADCLUSPHOTON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=1)

        # Select clean photons
        if self.enableCleaning:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonCleaningAlg' + self.postfix)
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            alg.selectionDecoration = 'isClean,as_bits'
            alg.selectionTool.selectionFlags = ['DFCommonPhotonsCleaning' + cleaningWP]
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 bits=1)

        # Do calibration
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg',
                                      'PhotonCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                               'CP::EgammaCalibrationAndSmearingTool' )
        alg.calibrationAndSmearingTool.ESModel = 'es2018_R21_v0'
        alg.calibrationAndSmearingTool.decorrelationModel = '1NP_v1'
        if config.dataType() == 'afii':
            alg.calibrationAndSmearingTool.useAFII = 1
        else :
            alg.calibrationAndSmearingTool.useAFII = 0
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the the pt selection
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonPtCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.minPt = 10e3
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=2, preselection=self.ptSelectionOutput)

        # Set up the isolation correction algorithm.
        alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                      'PhotonIsolationCorrectionAlg' + self.postfix )
        config.addPrivateTool( 'isolationCorrectionTool',
                               'CP::IsolationCorrectionTool' )
        if config.dataType() == 'data':
            alg.isolationCorrectionTool.IsMC = 0
        else:
            alg.isolationCorrectionTool.IsMC = 1
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')



class PhotonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the photon working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self, containerName, postfix, qualityWP, isolationWP) :
        super (PhotonWorkingPointConfig, self).__init__ ()
        self.containerName = containerName
        self.selectionName = postfix
        self.postfix = postfix
        if self.postfix != '' and self.postfix[0] != '_' :
            self.postfix = '_' + self.postfix
        self.qualityWP = qualityWP
        self.isolationWP = isolationWP
        self.recomputeIsEM = False

    def makeAlgs (self, config) :

        if self.qualityWP == 'Tight' :
            quality = ROOT.egammaPID.PhotonTight
        elif self.qualityWP == 'Loose' :
            quality = ROOT.egammaPID.PhotonLoose
        else :
            raise Exception ('unknown photon quality working point "' + self.qualityWP + '" should be Tight or Loose')

        # Set up the photon selection algorithm:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonIsEMSelectorAlg' + self.postfix )
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
                             bits=(32 if self.recomputeIsEM else 1), preselection=True)

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'PhotonIsolationSelectionAlg' + self.postfix )
            alg.selectionDecoration = 'isolated' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.PhotonWP = self.isolationWP
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 bits=1, preselection=True)

        # Set up an algorithm used for decorating baseline photon selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'PhotonSelectionSummary' + self.postfix )
        alg.selectionDecoration = 'baselineSelection' + self.postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)

        if config.dataType() != 'data':
            # Set up the photon efficiency correction algorithm.
            alg = config.createAlgorithm( 'CP::PhotonEfficiencyCorrectionAlg',
                                          'PhotonEfficiencyCorrectionAlg' + self.postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgPhotonEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'ph_effSF' + self.postfix + '_%SYS%'
            if config.dataType() == 'afii':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    ROOT.PATCore.ParticleDataType.Fast
            elif config.dataType() == 'mc':
                alg.efficiencyCorrectionTool.ForceDataType = \
                    ROOT.PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + self.postfix
            alg.photons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)





def makePhotonCalibrationConfig( seq, containerName,
                                 postfix = '',
                                 crackVeto = False,
                                 enableCleaning = True,
                                 cleaningAllowLate = False,
                                 recomputeIsEM = False,
                                 ptSelectionOutput = False ):
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

    config = PhotonCalibrationConfig (containerName, postfix)
    config.crackVeto = crackVeto
    config.enableCleaning = enableCleaning
    config.cleaningAllowLate = cleaningAllowLate
    config.recomputeIsEM = recomputeIsEM
    config.ptSelectionOutput = ptSelectionOutput
    seq.append (config)



def makePhotonWorkingPointConfig( seq, containerName, workingPoint, postfix = '',
                                  recomputeIsEM = False ):
    """Create photon analysis algorithms for a single working point

    Keywrod arguments:
      workingPoint -- The working point to use
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      recomputeIsEM -- Whether to rerun the cut-based selection. If not, use derivation flags
    """

    splitWP = workingPoint.split ('.')
    if len (splitWP) != 2 :
        raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)

    config = PhotonWorkingPointConfig (containerName, postfix, splitWP[0], splitWP[1])
    config.recomputeIsEM = recomputeIsEM
    seq.append (config)
