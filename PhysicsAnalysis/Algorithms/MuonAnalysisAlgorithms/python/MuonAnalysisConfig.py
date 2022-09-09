# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import ROOT


class MuonCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the muon four-momentum correction"""

    def __init__ (self, containerName, postfix) :
        super (MuonCalibrationConfig, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.ptSelectionOutput = False

    def makeAlgs (self, config) :
        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName, 'Muons') :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'MuonShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName, "Muons")
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all muons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                               'MuonEtaCutAlg' + self.postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.5
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        alg.particles = config.readName (self.containerName, "Muons")
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=2)

        # Set up the track selection algorithm:
        alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                               'MuonTrackSelectionAlg' + self.postfix )
        alg.selectionDecoration = 'trackSelection' + self.postfix + ',as_bits'
        alg.maxD0Significance = 3
        alg.maxDeltaZ0SinTheta = 0.5
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=3)

        # Set up the muon calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::MuonCalibrationAndSmearingAlg',
                               'MuonCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                        'CP::MuonCalibTool' )
        alg.calibrationAndSmearingTool.calibMode = 2 # choose ID+MS with no sagitta bias
        alg.muons = config.readName (self.containerName)
        alg.muonsOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the the pt selection
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'MuonPtCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.particles = config.readName (self.containerName)
        alg.selectionTool.minPt = 3e3
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=2, preselection = self.ptSelectionOutput)



class MuonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the muon working point

    This may at some point be split into multiple blocks (10 Mar 22)."""

    def __init__ (self, containerName, postfix, quality, isolation, isRun3Geo) :
        super (MuonWorkingPointConfig, self).__init__ ()
        self.containerName = containerName
        self.selectionName = postfix
        self.postfix = postfix
        self.quality = quality
        self.isRun3Geo = isRun3Geo
        self.isolation = isolation
        self.qualitySelectionOutput = False

    def makeAlgs (self, config) :

        if self.quality == 'Tight' :
            quality = ROOT.xAOD.Muon.Tight
        elif self.quality == 'Medium' :
            quality = ROOT.xAOD.Muon.Medium
        elif self.quality == 'Loose' :
            quality = ROOT.xAOD.Muon.Loose
        elif self.quality == 'VeryLoose' :
            quality = ROOT.xAOD.Muon.VeryLoose
        elif self.quality == 'HighPt' :
            quality = 4
        elif self.quality == 'LowPtEfficiency' :
            quality = 5
        else :
            raise ValueError ("invalid muon quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose, " +
                              "VeryLoose, HighPt, LowPtEfficiency")

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.isolation not in ["Iso", "NonIso"] :
            raise ValueError ('invalid muon isolation \"' + self.isolation +
                              '\", allowed values are Iso, NonIso')

        # Setup the muon quality selection
        alg = config.createAlgorithm( 'CP::MuonSelectionAlgV2',
                               'MuonSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::MuonSelectionTool' )
        alg.selectionTool.MuQuality = quality
        alg.selectionTool.IsRun3Geo = self.isRun3Geo
        alg.selectionDecoration = 'good_muon' + postfix + ',as_bits'
        alg.badMuonVetoDecoration = 'is_bad' + postfix + ',as_char'
        alg.muons = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName,
                             alg.selectionDecoration,
                             bits=4, preselection=self.qualitySelectionOutput)

        # Set up the isolation calculation algorithm:
        if self.isolation != 'NonIso' :
            alg = config.createAlgorithm( 'CP::MuonIsolationAlg',
                                   'MuonIsolationAlg' + postfix )
            config.addPrivateTool( 'isolationTool', 'CP::IsolationSelectionTool' )
            alg.isolationDecoration = 'isolated_muon' + postfix + ',as_bits'
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName,
                                 alg.isolationDecoration,
                                 bits=1, preselection=self.qualitySelectionOutput)

        # Set up the efficiency scale factor calculation algorithm:
        if config.dataType() != 'data':
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlg' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = self.quality
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)

        # Set up an algorithm used for decorating baseline muon selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'MuonSelectionSummary' + postfix )
        alg.selectionDecoration = 'baselineSelection' + postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)



def makeMuonCalibrationConfig( seq, containerName,
                               postfix = '', ptSelectionOutput = False):
    """Create muon calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      containerName -- name of the output container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
    """

    config = MuonCalibrationConfig (containerName, postfix)
    config.ptSelectionOutput = ptSelectionOutput
    seq.append (config)





def makeMuonWorkingPointConfig( seq, containerName, workingPoint, postfix,
                                qualitySelectionOutput = True,
                                isRun3Geo = False):
    """Create muon analysis algorithms for a single working point

    Keyword arguments:
      workingPoint -- The working point to use
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      qualitySelectionOutput -- Whether or not to apply muon quality selection
                                when creating output containers.
      isRun3Geo -- switches the muon selection tool to run 3 geometry
    """

    splitWP = workingPoint.split ('.')
    if len (splitWP) != 2 :
        raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)

    config = MuonWorkingPointConfig (containerName, postfix, splitWP[0], splitWP[1], isRun3Geo)
    config.qualitySelectionOutput = qualitySelectionOutput
    seq.append (config)
