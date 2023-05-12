# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


from __future__ import print_function

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import re
import ROOT


class PreJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the common preprocessing of jet sequences"""

    def __init__ (self, containerName, jetCollection) :
        super (PreJetAnalysisConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.jetCollection = jetCollection
        self.addOption ('postfix', '', type=str)
        self.addOption ('runOriginalObjectLink', False, type=bool)
        self.addOption ('runGhostMuonAssociation', None, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if config.isPhyslite() and self.jetCollection == 'AntiKt4EMPFlowJets' :
            config.setSourceName (self.containerName, "AnalysisJets", originalName = self.jetCollection)
        elif config.isPhyslite() and self.jetCollection == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' :
            config.setSourceName (self.containerName, "AnalysisLargeRJets", originalName = self.jetCollection)
        else :
            config.setSourceName (self.containerName, self.jetCollection, originalName = self.jetCollection)

        # Relink original jets in case of b-tagging calibration
        if self.runOriginalObjectLink :
            alg = config.createAlgorithm( 'CP::AsgOriginalObjectLinkAlg',
                                          'JetOriginalObjectLinkAlg'+postfix )
            alg.baseContainerName = self.jetCollection
            alg.particles = config.readName (self.containerName)
            if config.wantCopy (self.containerName) :
                alg.particlesOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the jet ghost muon association algorithm:
        if (self.runGhostMuonAssociation is None and not config.isPhyslite()) or \
           (self.runGhostMuonAssociation is True):
            alg = config.createAlgorithm( 'CP::JetGhostMuonAssociationAlg',
                                          'JetGhostMuonAssociationAlg'+postfix )
            alg.jets = config.readName (self.containerName)
            if config.wantCopy (self.containerName) :
                alg.jetsOut = config.copyName (self.containerName)

        # Set up shallow copy if needed and not yet done
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'JetShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)
 
        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'm', 'm')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True, enabled=False)



class SmallRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the small-r jet sequence"""

    def __init__ (self, containerName, jetCollection, jetInput) :
        super (SmallRJetAnalysisConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.jetCollection = jetCollection
        self.jetInput = jetInput
        self.addOption ('postfix', '', type=str)
        self.addOption ('runJvtUpdate', False, type=bool)
        self.addOption ('runNNJvtUpdate', False, type=bool)
        self.addOption ('runFJvtUpdate', False, type=bool)
        self.addOption ('runJvtSelection', True, type=bool)
        self.addOption ('runFJvtSelection', False, type=bool)
        self.addOption ('runJvtEfficiency', True, type=bool)
        self.addOption ('runFJvtEfficiency', False, type=bool)
        self.addOption ('reduction', "Global", type=str)
        self.addOption ('JEROption', "Simple", type=str)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

        if self.jetInput not in ["EMTopo", "EMPFlow"]:
            raise ValueError(
                "Unsupported input type '{0}' for R=0.4 jets!".format(self.jetInput) )

        # Prepare the jet calibration algorithm
        alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
        config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )
        alg.calibrationTool.JetCollection = jetCollectionName[:-4]
        # Get the correct string to use in the config file name
        if self.jetInput == "EMPFlow":
            configFile = "PreRec_R22_PFlow_ResPU_EtaJES_GSC_February23_230215.config"
        else:
            if config.dataType == 'afii':
                configFile = "JES_MC16Recommendation_AFII_{0}_Apr2019_Rel21.config"
            else:
                configFile = "JES_MC16Recommendation_Consolidated_{0}_Apr2019_Rel21.config"
            configFile = configFile.format(self.jetInput)
        alg.calibrationTool.ConfigFile = configFile
        if config.dataType() == 'data':
            alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Insitu'
        else:
            if self.jetInput == "EMPFlow":
                alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC'
            else:
                alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Smear'
        alg.calibrationTool.IsData = (config.dataType() == 'data')
        alg.jets = config.readName (self.containerName)
        alg.jetsOut = config.copyName (self.containerName)

        # Jet uncertainties
        # Prepare the config file
        if self.reduction == "All" and self.JEROption == "All":
            alg.uncertaintiesTool.ConfigFile = "R4_AllNuisanceParameters_AllJERNP.config"
        elif "Scenario" in self.reduction:
            if self.JEROption != "Simple":
                raise ValueError(
                    "Invalid uncertainty configuration - Scenario* reductions can "
                    "only be used together with the Simple JEROption")
            configFile = "R4_{0}_SimpleJER.config".format(self.reduction)
        elif self.reduction in ["Global", "Category"] and self.JEROption in ["Simple", "Full"]:
            configFile = "R4_{0}Reduction_{1}JER.config".format(self.reduction, self.JEROption)
        else:
            raise ValueError(
                "Invalid combination of reduction and JEROption settings: "
                "reduction: {0}, JEROption: {1}".format(self.reduction, self.JEROption) )

        alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
        config.addPrivateTool( 'uncertaintiesTool', 'JetUncertaintiesTool' )
        alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
        # Add the correct directory on the front
        alg.uncertaintiesTool.ConfigFile = "rel21/Fall2018/"+configFile
        alg.uncertaintiesTool.MCType = "AFII" if config.dataType() == "afii" else "MC16"
        alg.uncertaintiesTool.IsData = (config.dataType() == 'data')
        alg.jets = config.readName (self.containerName)
        alg.jetsOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the JVT update algorithm:
        if self.runJvtUpdate :
            alg = config.createAlgorithm( 'CP::JvtUpdateAlg', 'JvtUpdateAlg'+postfix )
            config.addPrivateTool( 'jvtTool', 'JetVertexTaggerTool' )
            alg.jvtTool.JetContainer = self.jetCollection
            alg.jvtTool.SuppressInputDependence=True
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.runNNJvtUpdate:
            assert self.jetInput=="EMPFlow", "NN JVT only defined for PFlow jets"
            alg = config.createAlgorithm( 'CP::JetDecoratorAlg', 'NNJvtUpdateAlg'+postfix )
            config.addPrivateTool( 'decorator', 'JetPileupTag::JetVertexNNTagger' )
            # Set this actually to the *output* collection
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.decorator.JetContainer = alg.jetsOut.replace ('%SYS%', 'NOSYS')
            alg.decorator.SuppressInputDependence=True
            alg.decorator.SuppressOutputDependence=True

        if self.runFJvtUpdate :
            alg = config.createAlgorithm( 'CP::JetModifierAlg', 'JetModifierAlg'+postfix )
            config.addPrivateTool( 'modifierTool', 'JetForwardJvtTool')
            alg.modifierTool.OutputDec = "passFJVT" #Output decoration
            # fJVT WPs depend on the MET WP
            # see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EtmissRecommendationsRel21p2#fJVT_and_MET
            alg.modifierTool.UseTightOP = 1 # 1 = Tight, 0 = Loose
            alg.modifierTool.EtaThresh = 2.5 # Eta dividing central from forward jets
            alg.modifierTool.ForwardMaxPt = 120.0e3 #Max Pt to define fwdJets for JVT
            alg.RenounceOutputs = True
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the jet efficiency scale factor calculation algorithm
        # Change the truthJetCollection property to AntiKt4TruthWZJets if preferred
        if self.runJvtSelection :
            alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg'+postfix )
            config.addPrivateTool( 'efficiencyTool', 'CP::JetJvtEfficiency' )
            if self.jetInput == 'EMPFlow':
                alg.efficiencyTool.SFFile = 'JetJvtEfficiency/Moriond2018/JvtSFFile_EMPFlowJets.root'
                alg.efficiencyTool.MaxPtForJvt = 60e3
            else:
                alg.efficiencyTool.SFFile = 'JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root'
                alg.efficiencyTool.MaxPtForJvt = 120e3
            alg.efficiencyTool.TaggingAlg = ROOT.CP.JvtTagger.NNJvt
            alg.efficiencyTool.WorkingPoint = 'FixedEffPt'
            alg.truthJetCollection = 'AntiKt4TruthDressedWZJets'
            alg.selection = 'jvt_selection'
            alg.scaleFactorDecoration = 'jvt_effSF_%SYS%'
            # Disable scale factor decorations if running on data
            # We still want to run the JVT selection
            if not self.runJvtEfficiency or config.dataType() == 'data':
                alg.scaleFactorDecoration = ''
                alg.truthJetCollection = ''
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_jvt'
            alg.skipBadEfficiency = 0
            alg.jets = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            if alg.scaleFactorDecoration != '' :
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'jvtEfficiency')

        if self.runFJvtSelection :
            alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'ForwardJvtEfficiencyAlg' )
            config.addPrivateTool( 'efficiencyTool', 'CP::JetJvtEfficiency' )
            if self.jetInput == 'EMPFlow':
                alg.efficiencyTool.SFFile = 'JetJvtEfficiency/May2020/fJvtSFFile.EMPFlow.root'
            else:
                alg.efficiencyTool.SFFile = 'JetJvtEfficiency/May2020/fJvtSFFile.EMtopo.root'
            alg.efficiencyTool.TaggingAlg = ROOT.CP.JvtTagger.fJvt
            alg.efficiencyTool.WorkingPoint = 'Loose'
            alg.truthJetCollection = 'AntiKt4TruthDressedWZJets'
            alg.dofJVT = True
            alg.fJVTStatus = 'passFJVT,as_char'
            alg.selection = 'fjvt_selection'
            alg.scaleFactorDecoration = 'fjvt_effSF_%SYS%'
            # Disable scale factor decorations if running on data
            # We still want to run the JVT selection
            if not self.runFJvtEfficiency or config.dataType() == 'data':
                alg.scaleFactorDecoration = ''
                alg.truthJetCollection = ''
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_fjvt'
            alg.skipBadEfficiency = 0
            alg.jets = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')





class RScanJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the r-scan jet sequence"""

    def __init__ (self, containerName, jetCollection, jetInput, radius) :
        super (RScanJetAnalysisConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.jetCollection = jetCollection
        self.jetInput = jetInput
        self.radius = radius
        self.addOption ('postfix', '', type=str)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"

        if self.jetInput != "LCTopo":
            raise ValueError(
                "Unsupported input type '{0}' for R-scan jets!".format(self.jetInput) )
        # Prepare the jet calibration algorithm
        alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
        config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )
        alg.calibrationTool.JetCollection = jetCollectionName[:-4]
        alg.calibrationTool.ConfigFile = \
            "JES_MC16Recommendation_Rscan{0}LC_Feb2022_R21.config".format(self.radius)
        if config.dataType() == 'data':
            alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
        else:
            alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
        alg.calibrationTool.IsData = (config.dataType() == 'data')
        alg.jets = config.readName (self.containerName)
        # Logging would be good
        print("WARNING: uncertainties for R-Scan jets are not yet released!")





class LargeRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the large-r jet sequence"""

    def __init__ (self, containerName, jetCollection, jetInput) :
        super (LargeRJetAnalysisConfig, self).__init__ ()
        self.containerName = containerName
        self.jetCollection = jetCollection
        self.jetInput = jetInput
        self.addOption ('postfix', '', type=str)
        self.addOption ('largeRMass', "Comb", type=str)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

        if self.largeRMass not in ["Comb", "Calo", "TA"]:
            raise ValueError ("Invalid large-R mass defintion {0}!".format(self.largeRMass) )

        if self.jetInput not in ["LCTopo", "TrackCaloCluster", "UFO"]:
            raise ValueError (
                "Unsupported input type '{0}' for large-R jets!".format(self.jetInput) )

        if self.jetInput == "TrackCaloCluster":
            # Only one mass defintion supported
            if self.largeRMass != "Calo":
                raise ValueError(
                    "Unsupported large-R TCC jet mass '{0}'!".format(self.largeRMass) )
            configFile = "JES_MC16recommendation_FatJet_TCC_JMS_calo_30Oct2018.config"

        if self.jetInput == "LCTopo":
            if self.largeRMass == "Comb":
                if config.dataType() == "data":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
                if config.dataType() == "mc":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_17Oct2018.config"
            elif self.largeRMass == "Calo":
                if config.dataType() == "data":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
                if config.dataType() == "mc":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_calo_12Oct2018.config "
            elif self.largeRMass == "TA":
                if config.dataType() == "data":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
                if config.dataType() == "mc":
                    configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_TA_12Oct2018.config"

        if self.jetInput == "UFO":
            configFile = "JES_MC20PreRecommendation_R10_UFO_CSSK_SoftDrop_JMS_R21Insitu_10Mar2023.config"

        # Prepare the jet calibration algorithm
        alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
        config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )
        alg.calibrationTool.JetCollection = jetCollectionName[:-4]
        alg.calibrationTool.ConfigFile = configFile
        if self.jetInput == "TrackCaloCluster" or self.jetInput == "UFO" or config.dataType() == "mc":
            alg.calibrationTool.CalibSequence = "EtaJES_JMS"
        elif config.dataType() == "data":
            alg.calibrationTool.CalibSequence = "EtaJES_JMS_Insitu_InsituCombinedMass"
        alg.calibrationTool.IsData = (config.dataType() == "data")
        alg.jets = config.readName (self.containerName)

        # Jet uncertainties

        if self.jetInput == "UFO":
            print("WARNING: uncertainties for UFO jets are not yet released!")

        if self.jetInput != "UFO":
            alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
            # R=1.0 jets have a validity range
            alg.outOfValidity = 2 # SILENT
            alg.outOfValidityDeco = 'outOfValidity'
            config.addPrivateTool( 'uncertaintiesTool', 'JetUncertaintiesTool' )

            alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
            alg.uncertaintiesTool.ConfigFile = \
                "rel21/Moriond2018/R10_{0}Mass_all.config".format(self.largeRMass)
            alg.uncertaintiesTool.MCType = "MC16a"
            alg.uncertaintiesTool.IsData = (config.dataType() == "data")

            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', 'outOfValidity',
                                 bits=1)





# These algorithms set up the jet recommendations as-of 04/02/2019.
# Jet calibration recommendations
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
# Jet uncertainties recommendations
# Small-R
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR
# Large-R
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetUncertaintiesRel21Moriond2018LargeR
# JVT recommendations
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JVTCalibrationRel21

def makeJetAnalysisConfig( seq, containerName, jetCollection, postfix = None,
                           runGhostMuonAssociation = None,
                           **kwargs):
    """Create a jet analysis algorithm sequence
      The jet collection is interpreted and selects the correct function to call,
      makeSmallRJetAnalysisConfig, makeRScanJetAnalysisConfig or
      makeLargeRJetAnalysisConfig

      Keyword arguments
        jetCollection -- The jet container to run on.
        postfix -- String to be added to the end of all public names.
        Other keyword arguments are forwarded to the other functions.
    """

    # Remove b-tagging calibration from the container name
    btIndex = jetCollection.find('_BTagging')
    if btIndex != -1:
        jetCollection = jetCollection[:btIndex]

    jetCollectionName=jetCollection
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    #AntiKt10UFO CSSKSoftDropBeta100Zcut10Jets

    if jetCollectionName == 'AntiKtVR30Rmax4Rmin02PV0TrackJets' :
        # don't to anything on track jets
        config = PreJetAnalysisConfig (containerName, jetCollection)
        if postfix is not None :
            config.setOptionValue ("postfix", postfix)
        config.setOptionValue ('runOriginalObjectLink', False)
        config.setOptionValue ('runGhostMuonAssociation', False)
        seq.append (config)
        return

    # interpret the jet collection
    collection_pattern = re.compile(
        r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFO|Track)(TrimmedPtFrac5SmallR20|CSSKSoftDropBeta100Zcut10)?Jets")
    match = collection_pattern.match(jetCollectionName)
    if not match:
        raise ValueError(
            "Jet collection {0} does not match expected pattern!".format(jetCollectionName) )
    radius = int(match.group(1) )
    if radius not in [2, 4, 6, 10]:
        raise ValueError("Jet collection has an unsupported radius '{0}'!".format(radius) )
    jetInput = match.group(2)


    config = PreJetAnalysisConfig (containerName, jetCollection)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    config.runOriginalObjectLink = (btIndex != -1)
    if runGhostMuonAssociation is not None :
        config.setOptionValue ('runGhostMuonAssociation', runGhostMuonAssociation)
    seq.append (config)

    if radius == 4:
        makeSmallRJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)
    elif radius in [2, 6]:
        makeRScanJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput, radius=radius,
            postfix=postfix, **kwargs)
    else:
        trim = match.group(3)
        if trim == "":
            raise ValueError("Untrimmed large-R jets are not supported!")
        makeLargeRJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)



def makeSmallRJetAnalysisConfig( seq, containerName, jetCollection,
                                 jetInput, postfix = None,
                                 runJvtUpdate = None, runNNJvtUpdate = None, runFJvtUpdate = None,
                                 runJvtSelection = None, runFJvtSelection = None,
                                 runJvtEfficiency = None, runFJvtEfficiency = None,
                                 reduction = None, JEROption = None):
    """Add algorithms for the R=0.4 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        runJvtUpdate -- Determines whether or not to update JVT on the jets
        runNNJvtUpdate -- Determines whether or not to update NN JVT on the jets
        runFJvtUpdate -- Determines whether or not to update forward JVT on the jets
        runJvtSelection -- Determines whether or not to run JVT selection on the jets
        runFJvtSelection -- Determines whether or not to run forward JVT selection on the jets
        runJvtEfficiency -- Determines whether or not to calculate the JVT efficiency
        runFJvtEfficiency -- Determines whether or not to calculate the forward JVT efficiency
        reduction -- Which NP reduction scheme should be used (All, Global, Category, Scenario)
        JEROption -- Which variant of the reduction should be used (All, Full, Simple). Note that not all combinations of reduction and JEROption are valid!
    """

    if jetInput not in ["EMTopo", "EMPFlow"]:
        raise ValueError(
            "Unsupported input type '{0}' for R=0.4 jets!".format(jetInput) )


    config = SmallRJetAnalysisConfig (containerName, jetCollection, jetInput)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if runJvtUpdate is not None :
        config.setOptionValue ('runJvtUpdate', runJvtUpdate)
    if runNNJvtUpdate is not None :
        config.setOptionValue ('runNNJvtUpdate', runNNJvtUpdate)
    if runFJvtUpdate is not None :
        config.setOptionValue ('runFJvtUpdate', runFJvtUpdate)
    if runJvtSelection is not None :
        config.setOptionValue ('runJvtSelection', runJvtSelection)
    if runFJvtSelection is not None :
        config.setOptionValue ('runFJvtSelection', runFJvtSelection)
    if runJvtEfficiency is not None :
        config.setOptionValue ('runJvtEfficiency', runJvtEfficiency)
    if runFJvtEfficiency is not None :
        config.setOptionValue ('runFJvtEfficiency', runFJvtEfficiency)
    if reduction is not None :
        config.setOptionValue ('reduction', reduction)
    if JEROption is not None :
        config.setOptionValue ('JEROption', JEROption)
    seq.append (config)


def makeRScanJetAnalysisConfig( seq, containerName, jetCollection,
                                  jetInput, radius, postfix = None ):
    """Add algorithms for the R-scan jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        radius -- The radius of the r-scan jets.
        postfix -- String to be added to the end of all public names.
    """

    config = SmallRJetAnalysisConfig (containerName, jetCollection, jetInput, radius)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    seq.append (config)




def makeLargeRJetAnalysisConfig( seq, containerName, jetCollection,
                                 jetInput, postfix = None, largeRMass = None):
    """Add algorithms for the R=1.0 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        largeRMass -- Which large-R mass definition to use. Ignored if not running on large-R jets ("Comb", "Calo", "TA")
    """

    config = LargeRJetAnalysisConfig (containerName, jetCollection, jetInput)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if largeRMass is not None :
        config.setOptionValue ('largeRMass', largeRMass)
    seq.append (config)
