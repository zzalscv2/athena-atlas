# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class FTagConfig (ConfigBlock):
    """the ConfigBlock for the flavor tagging config"""

    def __init__ (self, containerName, postfix) :
        super (FTagConfig, self).__init__ (containerName + '.' + postfix)
        self.containerName = containerName
        self.postfix = postfix
        self.addOption ('btagWP', "FixedCutBEff_77", type=str)
        self.addOption ('btagger', "DL1r", type=str)
        self.addOption ('generator', "default", type=str)
        self.addOption ('kinematicSelection', True, type=bool)
        self.addOption ('noEfficiency', False, type=bool)
        self.addOption ('legacyRecommendations', False, type=bool)
        self.addOption ('minPt', None, type=float)
        self.addOption ('bTagCalibFile', None, type=str,
                        info='calibration file for CDI')

    def makeAlgs (self, config) :

        jetCollection = config.originalName (self.containerName)

        selectionName = self.postfix
        if selectionName is None or selectionName == '' :
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_' :
            postfix = '_' + postfix

        # Kinematic selection depending on validity of the calibration
        # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalibrationRecommendationsRelease21
        minPt = self.minPt
        if minPt is None:
            if "EMPFlow" in jetCollection:
                minPt = 20e3
            elif "EMTopo" in jetCollection:
                minPt = 20e3
            elif "VRTrack" in jetCollection:
                minPt = 10e3

        if self.generator not in ["default", "Pythia8", "Sherpa221", "Sherpa228", "Sherpa2210", "Herwig7", "Herwig713", "Herwig721", "amc@NLO"]:
            raise ValueError ("invalid generator type: " + self.generator)

        # MC/MC scale factors configuration
        DSID = "default"
        if self.generator == "Sherpa221":
            DSID = "410250"
        elif self.generator == "Sherpa228":
            DSID = "421152"
        elif self.generator == "Sherpa2210":
            DSID = "700122"
        elif self.generator == "Herwig7":
            DSID = "410558"
        elif self.generator == "Herwig713":
            DSID = "411233"
        elif self.generator == "Herwig721":
            DSID = "600666"
        elif self.generator == "amc@NLO":
            DSID = "410464"

        if self.legacyRecommendations:
            # Remove b-tagging calibration from the container name
            btIndex = jetCollection.find('_BTagging')
            if btIndex == -1:
                jetCollection += '_BTagging201903'

        # CDI file
        if self.bTagCalibFile is not None :
            bTagCalibFile = self.bTagCalibFile
        elif self.legacyRecommendations :
            # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalibrationRecommendationsRelease21
            bTagCalibFile = "xAODBTaggingEfficiency/13TeV/2020-21-13TeV-MC16-CDI-2021-04-16_v1.root"
        else:
            bTagCalibFile = "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"

        if self.kinematicSelection:
            # Set up the ftag kinematic selection algorithm(s):
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'FTagKinSelectionAlg'+postfix )
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = minPt
            alg.selectionTool.maxEta = 2.5
            alg.selectionDecoration = 'ftag_kin_select_' + selectionName + ',as_char'
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            config.addSelection (self.containerName, selectionName,
                                 alg.selectionDecoration,
                                 bits=1, preselection=True)
            alg.particles = config.readName (self.containerName)

        # Set up the ftag selection algorithm(s):
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'FTagSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
        alg.selectionTool.TaggerName = self.btagger
        alg.selectionTool.OperatingPoint = self.btagWP
        alg.selectionTool.JetAuthor = jetCollection
        alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        alg.selectionTool.MinPt = minPt
        alg.preselection = config.getPreselection (self.containerName, selectionName)
        alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
        alg.particles = config.readName (self.containerName)
        if self.btagWP != 'Continuous':
            config.addOutputVar (self.containerName, 'ftag_select_' + selectionName, selectionName + '_select', noSys=True)

        if self.btagWP == 'Continuous':
            alg = config.createAlgorithm( 'CP::BTaggingInformationDecoratorAlg', 'FTagInfoAlg' + postfix )
            config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
            alg.selectionTool.TaggerName = self.btagger
            alg.selectionTool.OperatingPoint = self.btagWP
            alg.selectionTool.JetAuthor = jetCollection
            alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
            alg.selectionTool.MinPt = minPt
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            alg.quantileDecoration = 'ftag_quantile_' + selectionName
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, 'ftag_quantile_' + selectionName, selectionName + '_quantile', noSys=True)

        if not self.noEfficiency and config.dataType() != 'data':
            # Set up the efficiency calculation algorithm:
            alg = config.createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                                          'FTagEfficiencyScaleFactorAlg' + postfix )
            config.addPrivateTool( 'efficiencyTool', 'BTaggingEfficiencyTool' )
            alg.efficiencyTool.TaggerName = self.btagger
            alg.efficiencyTool.OperatingPoint = self.btagWP
            alg.efficiencyTool.JetAuthor = jetCollection
            alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile
            alg.efficiencyTool.SystematicsStrategy = "Envelope"
            alg.efficiencyTool.MinPt = minPt
            if DSID != "default":
                alg.efficiencyTool.EfficiencyBCalibrations = DSID
                alg.efficiencyTool.EfficiencyTCalibrations = DSID
                alg.efficiencyTool.EfficiencyCCalibrations = DSID
                alg.efficiencyTool.EfficiencyLightCalibrations = DSID
            alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
            alg.onlyEfficiency = self.btagWP == 'Continuous'
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_ftag_' + selectionName
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, selectionName + '_eff')


def makeFTagAnalysisConfig( seq, containerName,
                            postfix,
                            btagWP = None,
                            btagger = None,
                            generator = None,
                            kinematicSelection = None,
                            noEfficiency = None,
                            legacyRecommendations = None,
                            minPt = None ):
    """Create a ftag analysis algorithm config

    Keyword arguments:
      btagWP -- Flavour tagging working point
      btagger -- Flavour tagger
      generator -- Generator for MC/MC scale factors
      kinematicSelection -- Wether to run kinematic selection
      noEfficiency -- Wether to run efficiencies calculation
      legacyRecommendations -- Use legacy recommendations without shallow copied containers
      minPt -- Kinematic selection for jet calibration validity (depending on jet collection)
    """

    config = FTagConfig (containerName, postfix)
    if btagWP is not None :
        config.setOptionValue ('btagWP', btagWP)
    if btagger is not None :
        config.setOptionValue ('btagger', btagger)
    if generator is not None :
        config.setOptionValue ('generator', generator)
    if kinematicSelection is not None :
        config.setOptionValue ('kinematicSelection', kinematicSelection)
    if noEfficiency is not None :
        config.setOptionValue ('noEfficiency', noEfficiency)
    if legacyRecommendations is not None :
        config.setOptionValue ('legacyRecommendations', legacyRecommendations)
    if minPt is not None :
        config.setOptionValue ('minPt', minPt)
    seq.append (config)
