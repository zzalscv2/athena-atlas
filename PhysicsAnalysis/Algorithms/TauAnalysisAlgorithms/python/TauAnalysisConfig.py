# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class TauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self, containerName, postfix) :
        super (TauCalibrationConfig, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        if self.postfix != '' and self.postfix[0] != '_' :
            self.postfix = '_' + self.postfix
        self.rerunTruthMatching = True


    def makeAlgs (self, config) :

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisTauJets")
        else :
            config.setSourceName (self.containerName, "TauJets")

        # Set up the tau truth matching algorithm:
        if self.rerunTruthMatching and config.dataType() != 'data':
            alg = config.createAlgorithm( 'CP::TauTruthMatchingAlg',
                                          'TauTruthMatchingAlg' + self.postfix )
            config.addPrivateTool( 'matchingTool',
                                   'TauAnalysisTools::TauTruthMatchingTool' )
            alg.matchingTool.WriteTruthTaus = 1
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::TauSmearingAlg', 'TauSmearingAlg' + self.postfix )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::TauSmearingTool' )
        alg.taus = config.readName (self.containerName)
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')





class TauWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point

    This may at some point be split into multiple blocks (16 Mar 22)."""

    def __init__ (self, containerName, postfix, quality) :
        super (TauWorkingPointConfig, self).__init__ ()
        self.containerName = containerName
        self.selectionName = postfix
        self.postfix = postfix
        if self.postfix != '' and self.postfix[0] != '_' :
            self.postfix = '_' + self.postfix
        self.quality = quality
        self.legacyRecommendations = False


    def makeAlgs (self, config) :

        nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}.conf'
        if self.legacyRecommendations:
            nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}_legacy.conf'

        if self.quality not in ['Tight', 'Medium', 'Loose', 'VeryLoose', 'NoID', 'Baseline'] :
            raise ValueError ("invalid tau quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose, " +
                              "VeryLoose, NoID, Baseline")
        inputfile = nameFormat.format(self.quality.lower())

        # Setup the tau selection tool
        selectionTool = config.createPublicTool( 'TauAnalysisTools::TauSelectionTool',
                                                 'TauSelectionTool' + self.postfix)
        selectionTool.ConfigPath = inputfile

        # Set up the algorithm selecting taus:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'TauSelectionAlg' + self.postfix )
        alg.selectionTool = '%s/%s' % \
            ( selectionTool.getType(), selectionTool.getName() )
        alg.selectionDecoration = 'selected_tau' + self.postfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration, bits=6, preselection=True)

        # Set up an algorithm used for decorating baseline tau selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'TauSelectionSummary' + self.postfix )
        alg.selectionDecoration = 'baselineSelection' + self.postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)


        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        if config.dataType() != 'data':
            alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlg' + self.postfix )
            config.addPrivateTool( 'efficiencyCorrectionsTool',
                            'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
            alg.efficiencyCorrectionsTool.TauSelectionTool = '%s/%s' % \
                ( selectionTool.getType(), selectionTool.getName() )
            alg.scaleFactorDecoration = 'tau_effSF' + self.postfix + '_%SYS%'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + self.postfix
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)





def makeTauCalibrationConfig( seq, containerName, postfix = '',
                              rerunTruthMatching = True):
    """Create tau calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      rerunTruthMatching -- Whether or not to rerun truth matching
    """

    config = TauCalibrationConfig (containerName, postfix)
    config.rerunTruthMatching = rerunTruthMatching
    seq.append (config)





def makeTauWorkingPointConfig( seq, containerName, workingPoint, postfix,
                               legacyRecommendations = False):
    """Create tau analysis algorithms for a single working point

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      legacyRecommendations -- use legacy tau BDT and electron veto recommendations
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
    """

    splitWP = workingPoint.split ('.')
    if len (splitWP) != 1 :
        raise ValueError ('working point should be of format "quality", not ' + workingPoint)

    config = TauWorkingPointConfig (containerName, postfix, splitWP[0])
    config.legacyRecommendations = legacyRecommendations
    seq.append (config)
