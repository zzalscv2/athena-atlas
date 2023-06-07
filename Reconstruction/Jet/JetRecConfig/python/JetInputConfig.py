# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
# JetInputConfig: A helper module providing function to setup algorithms
# in charge of preparing input sources to jets (ex: EventDensity algo, track
# or truth selection,...)
#
# Author: P-A Delsart                                              #
"""
from AthenaConfiguration.ComponentFactory import CompFactory

# we can't add the imports here, because some modules may not be available
# in all releases (Ex: AthGeneration, AnalysisBase...) so we delay the imports
# inside the functions

def _buildJetAlgForInput(suffix, tools ):
    jetalg = CompFactory.JetAlgorithm("jetalg_"+suffix,
                                      Tools = tools,
    )
    return jetalg

def buildJetTrackUsedInFitDeco( parentjetdef, inputspec ):
    from InDetUsedInVertexFitTrackDecorator.UsedInVertexFitTrackDecoratorCfg import getUsedInVertexFitTrackDecoratorAlg
    from JetRecConfig.StandardJetContext import jetContextDic
    trkProperties = jetContextDic[parentjetdef.context]

    return getUsedInVertexFitTrackDecoratorAlg(trackCont=trkProperties["Tracks"] , vtxCont= trkProperties["Vertices"])

    
def buildJetInputTruth(parentjetdef, truthmod):
    truthmod = truthmod or ""
    from ParticleJetTools.ParticleJetToolsConfig import getCopyTruthJetParticles
    return _buildJetAlgForInput("truthpartcopy_"+truthmod,
                                tools = [ getCopyTruthJetParticles(truthmod, parentjetdef._cflags) ]
    )

def buildJetInputTruthGEN(parentjetdef, truthmod):
    """  Build truth constituents as in EVTGEN jobs in the r21 config. 
    IMPORTANT : this is expected to be temporary, only to reproduce the EVTGEN r21 config with the new config. The definitions should be harmonized with reco-level at some point and this function removed.
    The source for r21 EVTGEN config was in GeneratorFilters/share/common/GenerateTruthJets.py
    """
    truthmod = truthmod or ""

    # recopy config from GeneratorFilters/share/common/GenerateTruthJets.py
    truthClassifier = CompFactory.MCTruthClassifier("JetMCTruthClassifier") 

    if truthmod == "":
        truthpartcopy = CompFactory.CopyTruthJetParticles("truthpartcopy",
                                                                   OutputName="JetInputTruthParticlesGEN",
                                                                   MCTruthClassifier=truthClassifier)
    elif truthmod=="NoWZ":
 
        truthpartcopy = CompFactory.CopyTruthJetParticles("truthpartcopywz",
                                                                     OutputName="JetInputTruthParticlesGENNoWZ",
                                                                     MCTruthClassifier=truthClassifier,
                                                                     IncludePromptLeptons=False)
        
    return _buildJetAlgForInput("truthpartcopy_"+truthmod,
                                tools = [ truthpartcopy ]
    )

def buildLabelledTruth(parentjetdef, truthmod):
    from ParticleJetTools.ParticleJetToolsConfig import getCopyTruthLabelParticles
    tool = getCopyTruthLabelParticles(truthmod)
    return _buildJetAlgForInput("truthlabelcopy_"+truthmod,
                                tools = [ tool ]
    )

def buildPV0TrackSel(parentjetdef, spec):
    from JetRecConfig.StandardJetContext import jetContextDic
    from TrackVertexAssociationTool.getTTVAToolForReco import getTTVAToolForReco
    
    trkOptions = jetContextDic[parentjetdef.context]
    tvaTool = getTTVAToolForReco("trackjetTVAtool", 
                                 returnCompFactory = True,
                                 addDecoAlg = False ,# not needed : UsedInFit decorations are part of other prerequisites  
                                 WorkingPoint = "Nonprompt_All_MaxWeight",
                                 TrackContName = trkOptions['JetTracksQualityCuts'],
                                 VertexContName = trkOptions['Vertices'],
                                 )
    alg = CompFactory.PV0TrackSelectionAlg("pv0tracksel_trackjet", 
                                           InputTrackContainer = trkOptions['JetTracksQualityCuts'],
                                           VertexContainer = trkOptions['Vertices'],
                                           OutputTrackContainer = "PV0"+trkOptions['JetTracks'],
                                           TVATool = tvaTool,
                                           )
    return alg


def buildPFlowSel(parentjetdef, spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg",
                                              electronID = "LHMedium",
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects"
                                             )

########################################################################

def buildEventShapeAlg(jetOrConstitdef, inputspec, voronoiRf = 0.9, radius = 0.4, suffix = None ):
    """Function producing an EventShapeAlg to calculate
     median energy density for pileup correction"""

    from .JetRecConfig import getPJContName
    from EventShapeTools.EventDensityConfig import configEventDensityTool, getEventShapeName

    
    pjContName = getPJContName(jetOrConstitdef,suffix)
    nameprefix = inputspec or ""
    
    rhotool = configEventDensityTool(
        f"EventDensity_{nameprefix}Kt4{pjContName}",
        jetOrConstitdef,
        InputContainer = pjContName, 
        OutputContainer = getEventShapeName(jetOrConstitdef, nameprefix=inputspec, suffix=suffix, radius=radius),
        JetRadius = radius,
        VoronoiRfact = voronoiRf,
        )
    
    eventshapealg = CompFactory.EventDensityAthAlg(
        f"EventDensity_{nameprefix}Kt4{pjContName}Alg",
        EventDensityTool = rhotool )

    return eventshapealg

