# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

# AnaAlgorithm import(s):
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool
import re

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

def makeJetAnalysisSequence( dataType, jetCollection, postfix = '',
                             deepCopyOutput = False,
                             shallowViewOutput = True,
                             runGhostMuonAssociation = True,
                             enableCutflow = False,
                             enableKinematicHistograms = False,
                             **kwargs):
    """Create a jet analysis algorithm sequence
      The jet collection is interpreted and selects the correct function to call, 
      makeSmallRJetAnalysisSequence, makeRScanJetAnalysisSequence or 
      makeLargeRJetAnalysisSequence

      Keyword arguments
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        postfix -- String to be added to the end of all public names.
        deepCopyOutput -- Whether or not to deep copy the output
        shallowViewOutput -- Whether or not to output a shallow view as the output
        enableCutflow -- Whether or not to dump the cutflow
        enableKinematicHistograms -- Whether or not to dump the kinematic histograms
        Other keyword arguments are forwarded to the other functions.
    """

    if dataType not in ["data", "mc", "afii"]:
        raise ValueError ("invalid data type: " + dataType )

    # Setup the postfix
    if postfix != '':
        postfix = "_" + postfix

    # Make sure selection options make sense
    if deepCopyOutput and shallowViewOutput:
        raise ValueError ("deepCopyOutput and shallowViewOutput can't both be true!")

    # Remove b-tagging calibration from the container name
    btIndex = jetCollection.find('_BTagging')
    if btIndex != -1:
        jetCollection = jetCollection[:btIndex]

    # interpret the jet collection
    collection_pattern = re.compile(
        r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFOCSSK)(TrimmedPtFrac5SmallR20|SoftDropBeta100Zcut10)?Jets")
    match = collection_pattern.match(jetCollection)
    if not match:
        raise ValueError(
            "Jet collection {0} does not match expected pattern!".format(jetCollection) )
    radius = int(match.group(1) )
    if radius not in [2, 4, 6, 10]:
        raise ValueError("Jet collection has an unsupported radius '{0}'!".format(radius) )
    jetInput = match.group(2)

    # Create the analysis algorithm sequence object.
    seq = AnaAlgSequence( "JetAnalysisSequence"+postfix )
    # Relink original jets in case of b-tagging calibration
    if btIndex != -1:
        alg = createAlgorithm( 'CP::AsgOriginalObjectLinkAlg',
            'JetOriginalObjectLinkAlg'+postfix )
        alg.baseContainerName = jetCollection
        seq.append( alg, inputPropName = 'particles', outputPropName = 'particlesOut', stageName = 'calibration' )

    # Set up the jet ghost muon association algorithm:
    if runGhostMuonAssociation:
        alg = createAlgorithm( 'CP::JetGhostMuonAssociationAlg', 
            'JetGhostMuonAssociationAlg'+postfix )
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )

    # IsBtag decoration for Jet Flavour Uncertainties
    # (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR)
    if dataType != 'data' and radius < 10:
        # Step 1: pt and eta selection
        alg = createAlgorithm('CP::AsgSelectionAlg', 'JetIsBtagPtEtaSelectionAlg'+postfix)
        addPrivateTool(alg, 'selectionTool', 'CP::AsgPtEtaSelectionTool')
        alg.selectionTool.minPt = 20
        alg.selectionTool.maxEta = 2.5
        alg.selectionDecoration = 'kinematicSelectionBtag,as_char'
        seq.append(alg, inputPropName='particles',
                stageName='selection')
        # Step 2: truth selection using HadronConeExclTruthLabelID
        alg = createAlgorithm('CP::AsgSelectionAlg', 'JetIsBtagTruthSelectionAlg'+postfix)
        alg.preselection = 'kinematicSelectionBtag,as_char'
        addPrivateTool(alg, 'selectionTool', 'CP::AsgIntValueSelectionTool')
        alg.selectionTool.selectionFlag = 'HadronConeExclTruthLabelID'
        alg.selectionTool.acceptedValues = [5]
        alg.selectionDecoration = 'IsBjet,as_char'
        seq.append(alg, inputPropName='particles',
                stageName='selection')

    # record all the selections each subfunction makes
    seq.addMetaConfigDefault ("selectionDecorNames", [])
    seq.addMetaConfigDefault ("selectionDecorCount", [])

    if radius == 4:
        makeSmallRJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)
    elif radius in [2, 6]:
        makeRScanJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, radius=radius, 
            postfix=postfix, **kwargs)
    else:
        trim = match.group(3)
        if trim == "":
            raise ValueError("Untrimmed large-R jets are not supported!")
        makeLargeRJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)

    # Set up an algorithm used to create jet selection cutflow:
    if enableCutflow:
        alg = createAlgorithm( 'CP::ObjectCutFlowHistAlg', 'JetCutFlowDumperAlg'+postfix )
        alg.histPattern = 'jet_cflow_%SYS%'+postfix
        seq.append( alg, inputPropName = 'input', stageName = 'selection',
                    dynConfig = {'selection' : lambda meta : meta["selectionDecorNames"][:],
                                 'selectionNCuts' : lambda meta : meta["selectionDecorCount"][:]} )

    # Set up an algorithm dumping the kinematic properties of the jets:
    if enableKinematicHistograms:
        alg = createAlgorithm( 'CP::KinematicHistAlg', 'JetKinematicDumperAlg'+postfix )
        alg.histPattern = 'jet_%VAR%_%SYS%'+postfix
        seq.append( alg, inputPropName = 'input', stageName = 'selection',
                    dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNames"])} )

    if shallowViewOutput:
      # Set up an algorithm that makes a view container using the selections
      # performed previously:
      alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'JetViewFromSelectionAlg'+postfix )
      seq.append( alg, inputPropName = 'input', outputPropName = 'output',
                  stageName = 'selection',
                  dynConfig = {'selection' : lambda meta : meta["selectionDecorNames"][:]} )

    # Set up a final deep copy making algorithm if requested:
    if deepCopyOutput:
        alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'JetDeepCopyMaker'+postfix )
        alg.deepCopy = True
        seq.append( alg, inputPropName = 'input', outputPropName = 'output',
                    stageName = 'selection' )
    
    return seq

def makeSmallRJetAnalysisSequence( seq, dataType, jetCollection,
                                   jetInput, postfix = '', 
                                   runJvtUpdate = True, runFJvtUpdate = True,
                                   runJvtSelection = True, runFJvtSelection = True,
                                   runJvtEfficiency = True, runFJvtEfficiency = True,
                                   reduction = "Global", JEROption = "Simple",
                                   truthJetCollection = None):
    """Add algorithms for the R=0.4 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        runJvtUpdate -- Determines whether or not to update JVT on the jets
        runFJvtUpdate -- Determines whether or not to update forward JVT on the jets
        runJvtSelection -- Determines whether or not to run JVT selection on the jets
        runFJvtSelection -- Determines whether or not to run forward JVT selection on the jets
        runJvtEfficiency -- Determines whether or not to calculate the JVT efficiency
        runFJvtEfficiency -- Determines whether or not to calculate the forward JVT efficiency
        reduction -- Which NP reduction scheme should be used (All, Global, Category, Scenario)
        JEROption -- Which variant of the reduction should be used (All, Full, Simple). Note that not all combinations of reduction and JEROption are valid!
        truthJetCollection -- a custom jet collection to use for truth jets
    """
    if jetInput not in ["EMTopo", "EMPFlow"]:
        raise ValueError(
            "Unsupported input type '{0}' for R=0.4 jets!".format(jetInput) )

    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollection[:-4]
    # Get the correct string to use in the config file name
    if dataType == 'afii':
        configFile = "JES_MC16Recommendation_AFII_{0}_Apr2019_Rel21.config"
    else:
        configFile = "JES_MC16Recommendation_Consolidated_{0}_Apr2019_Rel21.config"
    if jetInput == "EMPFlow":
        configFile = configFile.format("PFlow")
    else:
        configFile = configFile.format(jetInput)
    alg.calibrationTool.ConfigFile = configFile
    if dataType == 'data':
        alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Insitu'
    else:
        alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Smear'
    alg.calibrationTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration')

    # Jet uncertainties
    # Prepare the config file
    if reduction == "All" and JEROption == "All":
        alg.uncertaintiesTool.ConfigFile = "R4_AllNuisanceParameters_AllJERNP.config"
    elif "Scenario" in reduction:
        if JEROption != "Simple":
            raise ValueError(
                "Invalid uncertainty configuration - Scenario* reductions can "
                "only be used together with the Simple JEROption")
        configFile = "R4_{0}_SimpleJER.config".format(reduction)
    elif reduction in ["Global", "Category"] and JEROption in ["Simple", "Full"]:
        configFile = "R4_{0}Reduction_{1}JER.config".format(reduction, JEROption)
    else:
        raise ValueError(
            "Invalid combination of reduction and JEROption settings: "
            "reduction: {0}, JEROption: {1}".format(reduction, JEROption) )

    alg = createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
    addPrivateTool( alg, 'uncertaintiesTool', 'JetUncertaintiesTool' )
    alg.uncertaintiesTool.JetDefinition = jetCollection[:-4]
    # Add the correct directory on the front
    alg.uncertaintiesTool.ConfigFile = "rel21/Fall2018/"+configFile
    alg.uncertaintiesTool.MCType = "AFII" if dataType == "afii" else "MC16"
    alg.uncertaintiesTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut',
                stageName = 'calibration' )

    # Set up the JVT update algorithm:
    if runJvtUpdate :
        alg = createAlgorithm( 'CP::JvtUpdateAlg', 'JvtUpdateAlg'+postfix )
        addPrivateTool( alg, 'jvtTool', 'JetVertexTaggerTool' )
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection' )

    if runFJvtUpdate :
        alg = createAlgorithm( 'CP::JetModifierAlg', 'JetModifierAlg'+postfix )
        addPrivateTool( alg, 'modifierTool', 'JetForwardJvtTool')
        alg.modifierTool.OutputDec = "passFJVT" #Output decoration
        # fJVT WPs depend on the MET WP
        # see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EtmissRecommendationsRel21p2#fJVT_and_MET
        alg.modifierTool.UseTightOP = 1 # 1 = Tight, 0 = Loose
        alg.modifierTool.EtaThresh = 2.5 # Eta dividing central from forward jets
        alg.modifierTool.ForwardMaxPt = 120.0e3 #Max Pt to define fwdJets for JVT
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection' )
        pass

    # Set up the jet efficiency scale factor calculation algorithm
    # Change the truthJetCollection property to AntiKt4TruthWZJets if preferred
    if runJvtSelection :
        alg = createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg'+postfix )
        addPrivateTool( alg, 'efficiencyTool', 'CP::JetJvtEfficiency' )
        if jetInput == 'EMPFlow':
            alg.efficiencyTool.SFFile = 'JetJvtEfficiency/Moriond2018/JvtSFFile_EMPFlowJets.root'
            alg.efficiencyTool.MaxPtForJvt = 60e3
        else:
            alg.efficiencyTool.SFFile = 'JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root'
            alg.efficiencyTool.MaxPtForJvt = 120e3
        alg.efficiencyTool.WorkingPoint = 'Tight' if jetInput == 'EMPFlow' else 'Medium'
        alg.selection = 'jvt_selection'
        alg.scaleFactorDecoration = 'jvt_effSF_%SYS%'
        # Disable scale factor decorations if running on data
        # We still want to run the JVT selection
        if not runJvtEfficiency or dataType == 'data':
            alg.scaleFactorDecoration = ''
            alg.truthJetCollection = ''
        elif truthJetCollection is not None:
            alg.truthJetCollection = truthJetCollection
        alg.outOfValidity = 2
        alg.outOfValidityDeco = 'no_jvt'
        alg.skipBadEfficiency = 0
        seq.append( alg, inputPropName = 'jets',
                    stageName = 'selection' )

    if runFJvtSelection :
        alg = createAlgorithm( 'CP::JvtEfficiencyAlg', 'ForwardJvtEfficiencyAlg' )
        addPrivateTool( alg, 'efficiencyTool', 'CP::JetJvtEfficiency' )
        if jetInput == 'EMPFlow':
            alg.efficiencyTool.SFFile = 'JetJvtEfficiency/May2020/fJvtSFFile.EMPFlow.root'
        else:
            alg.efficiencyTool.SFFile = 'JetJvtEfficiency/May2020/fJvtSFFile.EMtopo.root'
        alg.efficiencyTool.WorkingPoint = 'Tight'
        alg.efficiencyTool.UseMuSFFormat = True
        alg.dofJVT = True
        alg.fJVTStatus = 'passFJVT,as_char'
        alg.selection = 'fjvt_selection'
        alg.scaleFactorDecoration = 'fjvt_effSF_%SYS%'
        # Disable scale factor decorations if running on data
        # We still want to run the JVT selection
        if not runFJvtEfficiency or dataType == 'data':
            alg.scaleFactorDecoration = ''
            alg.truthJetCollection = ''
        elif truthJetCollection is not None:
            alg.truthJetCollection = truthJetCollection
        alg.outOfValidity = 2
        alg.outOfValidityDeco = 'no_fjvt'
        alg.skipBadEfficiency = 0
        seq.append( alg, inputPropName = 'jets',
                    stageName = 'selection')

    # Return the sequence:
    return seq

def makeRScanJetAnalysisSequence( seq, dataType, jetCollection,
                                  jetInput, radius, postfix = '' ):
    """Add algorithms for the R-scan jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        radius -- The radius of the r-scan jets.
        postfix -- String to be added to the end of all public names.
    """
    if jetInput != "LCTopo":
        raise ValueError(
            "Unsupported input type '{0}' for R-scan jets!".format(jetInput) )
    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollection[:-4]
    alg.calibrationTool.ConfigFile = \
        "JES_MC16Recommendation_Rscan{0}LC_18Dec2018_R21.config".format(radius)
    if dataType == 'data':
        alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
    else:
        alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC"
    alg.calibrationTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )
    # Logging would be good
    print("WARNING: uncertainties for R-Scan jets are not yet released!")

def makeLargeRJetAnalysisSequence( seq, dataType, jetCollection,
                                   jetInput, postfix = '', largeRMass = "Comb"):
    """Add algorithms for the R=1.0 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        largeRMass -- Which large-R mass definition to use. Ignored if not running on large-R jets ("Comb", "Calo", "TCC", "TA")
    """

    if largeRMass not in ["Comb", "Calo", "TCC", "TA"]:
        raise ValueError ("Invalid large-R mass defintion {0}!".format(largeRMass) )

    if jetInput not in ["LCTopo", "TrackCaloCluster", "UFOCSSK"]:
        raise ValueError (
            "Unsupported input type '{0}' for large-R jets!".format(jetInput) )
    if jetInput == "TrackCaloCluster":
        # Only one mass defintion supported
        if largeRMass != "Calo":
            raise ValueError(
                "Unsupported large-R TCC jet mass '{0}'!".format(largeRMass) )
        configFile = "JES_MC16recommendation_FatJet_TCC_JMS_calo_30Oct2018.config"
    elif jetInput == "LCTopo":
        if largeRMass == "Comb":
            configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_17Oct2018.config"
        elif largeRMass == "Calo":
            configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_calo_12Oct2018.config"
        elif largeRMass == "TCC":
            configFile = "JES_MC16recommendation_FatJet_TCC_JMS_calo_30Oct2018.config"
        else:
            configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_TA_12Oct2018.config"
    else:
        configFile = "JES_MC16recommendation_R10_UFO_CSSK_SoftDrop_JMS_01April2020.config"

    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollection[:-4]
    alg.calibrationTool.ConfigFile = configFile
    alg.calibrationTool.CalibSequence = "EtaJES_JMS"
    alg.calibrationTool.IsData = 0
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )

    if jetInput == "LCTopo":
        # Jet uncertainties
        alg = createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
        # R=1.0 jets have a validity range
        alg.outOfValidity = 2 # SILENT
        alg.outOfValidityDeco = 'outOfValidity'
        addPrivateTool( alg, 'uncertaintiesTool', 'JetUncertaintiesTool' )
        alg.uncertaintiesTool.JetDefinition = jetCollection[:-4]
        alg.uncertaintiesTool.ConfigFile = "rel21/Moriond2018/R10_{0}Mass_all.config".format(largeRMass)
        alg.uncertaintiesTool.MCType = "MC16a"
        alg.uncertaintiesTool.IsData = (dataType == "data")
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut',
                    metaConfig = {'selectionDecorNames' : ['outOfValidity'],
                                  'selectionDecorCount' : [1]} )

    return seq
