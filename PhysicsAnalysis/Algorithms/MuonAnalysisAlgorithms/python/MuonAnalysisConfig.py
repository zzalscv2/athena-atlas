# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import ROOT


class MuonCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the muon four-momentum correction"""

    def __init__ (self, containerName) :
        super (MuonCalibrationConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', "", type=str)
        self.addOption ('ptSelectionOutput', False, type=bool)

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisMuons")
        else :
            config.setSourceName (self.containerName, "Muons")

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'MuonShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all muons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                               'MuonEtaCutAlg' + self.postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.7
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
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

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)


class MuonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the muon working point

    This may at some point be split into multiple blocks (10 Mar 22)."""

    def __init__ (self, containerName, postfix) :
        super (MuonWorkingPointConfig, self).__init__ (containerName + '.' + postfix)
        self.containerName = containerName
        self.selectionName = postfix
        self.addOption ('postfix', postfix, type=str)
        self.addOption ('quality', None, type=str)
        self.addOption ('isolation', None, type=str)
        self.addOption ('isRun3Geo', False, type=bool,
                        info='use run 3 geometry for the muon selection tool')
        self.addOption ('qualitySelectionOutput', True, type=bool)

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
            if self.isRun3Geo:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '220817_Preliminary_r22run3'
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'effSF' + postfix)

        # Set up an algorithm used for decorating baseline muon selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'MuonSelectionSummary' + postfix )
        alg.selectionDecoration = 'baselineSelection' + postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)
        config.addOutputVar (self.containerName, 'baselineSelection' + postfix, 'select' + postfix)



def makeMuonCalibrationConfig( seq, containerName,
                               postfix = None, ptSelectionOutput = None):
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

    config = MuonCalibrationConfig (containerName)
    config.setOptionValue ('postfix', postfix, noneAction='ignore')
    config.setOptionValue ('ptSelectionOutput', ptSelectionOutput, noneAction='ignore')
    seq.append (config)





def makeMuonWorkingPointConfig( seq, containerName, workingPoint, postfix,
                                qualitySelectionOutput = None,
                                isRun3Geo = None):
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


    config = MuonWorkingPointConfig (containerName, postfix)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)
        config.setOptionValue ('quality', splitWP[0])
        config.setOptionValue ('isolation', splitWP[1])
    config.setOptionValue ('isRun3Geo', isRun3Geo, noneAction='ignore')
    config.setOptionValue ('qualitySelectionOutput', qualitySelectionOutput, noneAction='ignore')
    seq.append (config)
