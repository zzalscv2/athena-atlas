# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# Schedules all tools needed for jet/MET analyses
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkJob
from AthenaCommon import CfgMgr
from AthenaCommon import Logging
from AthenaCommon.Configurable import ConfigurableCABehavior
dfjetlog = Logging.logging.getLogger('JetCommon')

##################################################################
# Schedule the augmentation of a flag to label events with large
# EMEC-IW Noise based on the presence of many bad quality clusters
##################################################################

def addBadBatmanFlag(sequence=DerivationFrameworkJob):

    from RecExConfig.ObjKeyStore import objKeyStore

    if objKeyStore.isInInput( "McEventCollection", "GEN_EVENT" ):
        dfjetlog.warning('Running over EVNT files, BadBatmanAugmentation will not be scheduled')
        return

    if not objKeyStore.isInInput( "xAOD::CaloClusterContainer", "CaloCalTopoClusters" ):
        dfjetlog.warning('CaloCalTopoClusters not present. Could not schedule BadBatmanAugmentation!!!')

    if hasattr(sequence,"BadBatmanAugmentation"):
        dfjetlog.warning( "BadBatmanAugmentation: BadBatmanAugmentation already scheduled on sequence "+sequence.name )
        return

    batmanaugtool = CfgMgr.DerivationFramework__BadBatmanAugmentationTool("BadBatmanAugmentationTool")
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += batmanaugtool

    batmanaug = CfgMgr.DerivationFramework__CommonAugmentation("BadBatmanAugmentation",
                                                               AugmentationTools = [batmanaugtool])
    sequence += batmanaug

################################################################## 
# Schedule adding the BCID info
################################################################## 

def addDistanceInTrain(sequence=DerivationFrameworkJob):

    if hasattr(sequence,"DistanceInTrainAugmentation"):
        dfjetlog.warning( "DistanceInTrainAugmentation: DistanceInTrainAugmentation already scheduled on sequence"+sequence.name )
        return

    from LumiBlockComps.BunchCrossingCondAlgDefault import BunchCrossingCondAlgDefault
    BunchCrossingCondAlgDefault()

    distanceintrainaugtool = CfgMgr.DerivationFramework__DistanceInTrainAugmentationTool("DistanceInTrainAugmentationTool")
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += distanceintrainaugtool
    distanceintrainaug = CfgMgr.DerivationFramework__CommonAugmentation("DistanceInTrainAugmentation",
                                                                        AugmentationTools = [distanceintrainaugtool])
    sequence += distanceintrainaug

##################################################################
# Overlap removal tool (needed for cleaning flags)
##################################################################

def applyOverlapRemoval(sequence=DerivationFrameworkJob):

    from  DerivationFrameworkTau.TauCommon import AddTauAugmentation
    AddTauAugmentation(sequence,doLoose=True)

    from AssociationUtils.config import recommended_tools
    from AssociationUtils.AssociationUtilsConf import OverlapRemovalGenUseAlg
    outputLabel = 'DFCommonJets_passOR'
    bJetLabel = '' #default
    tauLabel = 'DFTauLoose'
    orTool = recommended_tools(outputLabel=outputLabel,bJetLabel=bJetLabel)
    algOR = OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg',
                                    OverlapLabel=outputLabel,
                                    OverlapRemovalTool=orTool,
                                    TauLabel=tauLabel,
                                    BJetLabel=bJetLabel)
    sequence += algOR

    MuonJetDrTool = CfgMgr.DerivationFramework__MuonJetDrTool("MuonJetDrTool")
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += MuonJetDrTool
    sequence += CfgMgr.DerivationFramework__CommonAugmentation("DFCommonMuonsKernel2",
                                                          AugmentationTools = [MuonJetDrTool])

##################################################################
# Jet cleaning tool
##################################################################

def getJetCleaningTool(cleaningLevel,jetdef):
    jetcleaningtoolname = 'JetCleaningTool_'+cleaningLevel
    jetcleaningtool = None
    from AthenaCommon.AppMgr import ToolSvc
    if hasattr(ToolSvc,jetcleaningtoolname):
        jetcleaningtool = getattr(ToolSvc,jetcleaningtoolname)
    else:
        jetcleaningtool = CfgMgr.JetCleaningTool(jetcleaningtoolname,CutLevel=cleaningLevel,JetContainer=jetdef)
        jetcleaningtool.UseDecorations = False
        ToolSvc += jetcleaningtool

    return jetcleaningtool

##################################################################
# Cleaning flags
##################################################################

def addEventCleanFlags(sequence, workingPoints = ['Loose', 'Tight', 'LooseLLP']):

    # Prereqs
    addPassJvtForCleaning(sequence)
    applyOverlapRemoval(sequence)

    from JetSelectorTools.JetSelectorToolsConf import ECUtils__EventCleaningTool as EventCleaningTool
    from JetSelectorTools.JetSelectorToolsConf import EventCleaningTestAlg
    supportedWPs = ['Loose', 'Tight', 'LooseLLP', 'VeryLooseLLP', 'SuperLooseLLP']
    prefix = "DFCommonJets_"

    for wp in workingPoints:
        if wp not in supportedWPs:
            dfjetlog.warning('*** Unsupported event cleaning WP {} requested! Skipping it.***'.format(wp))
            continue
        algName = 'EventCleaningTestAlg_' + wp
        if hasattr(sequence, algName):
            continue

        cleaningLevel = wp + 'Bad'
        # LLP WPs have a slightly different name format
        if 'LLP' in wp:
            cleaningLevel = wp.replace('LLP', 'BadLLP')

        ecTool = EventCleaningTool('EventCleaningTool_' + wp, CleaningLevel=cleaningLevel)
        ecTool.JetCleanPrefix = prefix
        ecTool.JetCleaningTool = getJetCleaningTool(cleaningLevel,"AntiKt4EMTopoJets")
        ecTool.JetContainer = "AntiKt4EMTopoJets"
        algClean = EventCleaningTestAlg(algName,
                                        EventCleaningTool=ecTool,
                                        JetCollectionName="AntiKt4EMTopoJets",
                                        EventCleanPrefix=prefix,
                                        CleaningLevel=cleaningLevel,
                                        doEvent = ('Loose' in wp)) # Only store event-level flags for Loose and LooseLLP
        sequence += algClean


###################################################################
# Function to add jet collections via new config
################################################################### 

def addDAODJets(jetlist,sequence):

    from JetRecConfig.JetRecConfig import getJetAlgs, reOrderAlgs
    from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    from JetRecConfig.JetConfigFlags import jetInternalFlags
    # This setting implies that jet components failing job condition (ex: truth-related calculation in data job) are automatically removed
    jetInternalFlags.isRecoJob = True 

    for jd in jetlist:
        algs, jetdef_i = getJetAlgs(ConfigFlags, jd, True)
        with ConfigurableCABehavior():
            algs, ca = reOrderAlgs( [a for a in algs if a is not None])
        # ignore dangling CA instance in legacy config
        ca.wasMerged()
        for a in algs:
            if hasattr(sequence,a.getName()):
                continue
            sequence += conf2toConfigurable(a)


def swapAlgsInSequence(sequence, name1, name2):
    """Used to swap specific algs with an AthSequence. 

    the sequence is modified such that alg name2 always comes before alg name1.

    This is mainly usefull when running in Run-II style config and when mix with CA without automatic scheduler 
    result in wrong ordering in UFO building algs.
    Typically : "jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK" and "UFOInfoAlgCSSK"
    """
    from AthenaCommon.CFElements import findAlgorithm
    alg1 = findAlgorithm(sequence, name1)
    alg2 = findAlgorithm(sequence, name2)

    if alg1 is None or alg2 is None:
        return
    alg1I = sequence.getChildren().index(alg1)
    alg2I = sequence.getChildren().index(alg2)
    if alg1I>alg2I:
        sequence.remove(alg1)
        sequence.insert(alg2I, alg1)
    
    
##################################################################  

def addPassJvtForCleaning(sequence=DerivationFrameworkJob):
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtEffTool
    algName = "DFJet_EventCleaning_passJvtAlg"
    if hasattr(sequence, algName):
        return

    passJvtTool = getJvtEffTool('AntiKt4EMTopo')
    passJvtTool.PassJVTKey = "AntiKt4EMTopoJets.DFCommonJets_passJvt"

    dfjetlog.info('JetCommon: Adding passJvt decoration to AntiKt4EMTopoJets for event cleaning')
    sequence += CfgMgr.JetDecorationAlg(algName, JetContainer='AntiKt4EMTopoJets', Decorators=[passJvtTool])


##################################################################
# Add specific PFlow event shape from sidebands
##################################################################

def addSidebandEventShape(sequence=DerivationFrameworkJob):

    from JetRecConfig.JetRecConfig import getInputAlgs,getConstitPJGAlg,reOrderAlgs
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetInputConfig import buildEventShapeAlg
    from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    constit_algs = getInputAlgs(cst.GPFlow, flags=ConfigFlags)
    with ConfigurableCABehavior():
        constit_algs, ca_list = reOrderAlgs( [a for a in constit_algs if a is not None])

    for a in constit_algs:
        if not hasattr(sequence,a.getName()):
            sequence += conf2toConfigurable(a)

    constitPJAlg = getConstitPJGAlg(cst.GPFlow, suffix='PUSB')
    if not hasattr(sequence,constitPJAlg.getName()):
        sequence += conf2toConfigurable(constitPJAlg)

    eventshapealg = buildEventShapeAlg(cst.GPFlow, '', suffix = 'PUSB' )
    if not hasattr(sequence, eventshapealg.getName()):
        sequence += conf2toConfigurable(eventshapealg)

    #New "sideband" definition when using CHS based on TTVA
    constitNeutralPJAlg = getConstitPJGAlg(cst.GPFlow, suffix='Neut')
    if not hasattr(sequence,constitNeutralPJAlg.getName()):
        sequence += conf2toConfigurable(constitNeutralPJAlg)

    neutraleventshapealg = buildEventShapeAlg(cst.GPFlow, '', suffix = 'Neut' )
    if not hasattr(sequence, neutraleventshapealg.getName()):
        sequence += conf2toConfigurable(neutraleventshapealg)


##################################################################
# Set up helpers for adding jets to the output streams
##################################################################
OutputJets = {}
OutputJets["SmallR"] = ["AntiKt4EMPFlowJets",
                        "AntiKt4EMTopoJets",
                        "AntiKt4TruthJets"]

OutputJets["LargeR"] = ["AntiKt10UFOCSSKJets",
                        "AntiKt10TruthJets"]

def addJetOutputs(slimhelper,contentlist,smartlist=[],vetolist=[]):
    outputlist = []
    for content in contentlist:
        if content in OutputJets.keys():
            for item in OutputJets[content]:
                if item in vetolist: continue
                outputlist.append(item)
        else:
            outputlist.append(content)

    for item in outputlist:
        if item not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary[item]='xAOD::JetContainer'
            slimhelper.AppendToDictionary[item+"Aux"]='xAOD::JetAuxContainer'
        if item in smartlist:
            dfjetlog.info( "Add smart jet collection "+item )
            slimhelper.SmartCollections.append(item)
        else:
            dfjetlog.info( "Add full jet collection "+item )
            slimhelper.AllVariables.append(item)

##################################################################
# Helper to add origin corrected clusters to output
##################################################################
def addOriginCorrectedClusters(slimhelper,writeLC=False,writeEM=False):

    slimhelper.ExtraVariables.append('CaloCalTopoClusters.calE.calEta.calPhi.calM')

    if writeLC:
        if "LCOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary["LCOriginTopoClusters"]='xAOD::CaloClusterContainer'
            slimhelper.AppendToDictionary["LCOriginTopoClustersAux"]='xAOD::ShallowAuxContainer'
            slimhelper.ExtraVariables.append('LCOriginTopoClusters.calEta.calPhi')

    if writeEM:
        if "EMOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary["EMOriginTopoClusters"]='xAOD::CaloClusterContainer'
            slimhelper.AppendToDictionary["EMOriginTopoClustersAux"]='xAOD::ShallowAuxContainer'
            slimhelper.ExtraVariables.append('EMOriginTopoClusters.calE.calEta.calPhi')
