
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
 StandardJetConstits: A module containing standard definitions for jet inputs : external container and 
 constituents.
 These can be copied and modified by users who want something a bit   
 different from what is provided.                                     
                                                                      
 Author: TJ Khoo, P-A Delsart                                                      
                                                                      

"""

########################################################################
from .JetDefinition import xAODType,  JetInputConstitSeq, JetInputExternal, JetConstitModifier, JetInputConstit
from .StandardJetContext import inputsFromContext, propFromContext
from .JetRecConfig import isAnalysisRelease 
from AthenaConfiguration.Enums import BeamType

# Prepare dictionnaries to hold all of our standard definitions.
# They will be filled from the lists below
from .Utilities import ldict
stdInputExtDic = ldict()
stdConstitDic = ldict()
stdContitModifDic = ldict()


# This module contains the helper functions needed to instantiate the input container external
# to Jet domain
import JetRecConfig.JetInputConfig as inputcfg
try:
    import JetRecTools.JetRecToolsConfig as jrtcfg
except ModuleNotFoundError:
    # In some releases JetRecTools is not existing
    pass
    
try:
    import TrackCaloClusterRecTools.TrackCaloClusterConfig as tcccfg
except ModuleNotFoundError:
    # In some releases TrackCaloClusterRecTools is not existing
    pass

def isMC(flags):
    """A simple filter function for  testing if we're running in MC
    returns (bool, str) where the str contains an explanation of why the bool is False.
    (probably worth re-allocating somehere else)"""
    return flags.Input.isMC, "Input file is not MC"

def standardReco(input):
    """Returns a helper function which invokes the standard reco configuration for the container 'input' 
    (where input is external to the jet domain).
    
    We group the definition of functions here rather than separately, so that we can change them 
    automatically to a void function in case we're in an Analysis release and we can not import upstream packages.
    
    """

    doNothingFunc = lambda *l:None # noqa: E731
    if isAnalysisRelease():
        return doNothingFunc

    
    if input=='CaloClusters':
        def f(jetdef,spec):
            from CaloRec.CaloRecoConfig import CaloRecoCfg
            flags = jetdef._cflags
            return CaloRecoCfg(flags) if flags.Jet.doUpstreamDependencies else None
    elif input=='Tracks':
        def f(jetdef,spec):
            from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
            flags = jetdef._cflags
            return InDetTrackRecoCfg(flags) if flags.Jet.doUpstreamDependencies else None
    elif input=="Muons":
        def f(jetdef,spec):
            from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
            flags = jetdef._cflags
            return MuonReconstructionCfg(flags) if flags.Jet.doUpstreamDependencies else None     
    elif input=="PFlow":
        def f(jetdef,spec):
            if not jetdef._cflags.Jet.doUpstreamDependencies:
                return None            
            from eflowRec.PFRun3Config import PFCfg
            return PFCfg(jetdef._cflags)
        
    else:
        f = doNothingFunc
        
    return f



########################################################################
## List of standard input sources for jets.

_stdInputList = [
    # Format is :
    # JetInputExternal( containername , containerType, ...optionnal parameters... )
    #  When defined, algoBuilder is a function returning the actual alg building the input.
    #  it will be called as : algoBuilder(jetdef, spec) where jetdef is the parent JetDefinition 
    
    # *****************************
    JetInputExternal("CaloCalTopoClusters", xAODType.CaloCluster, algoBuilder= standardReco("CaloClusters") ),

    # *****************************
    JetInputExternal("JetETMissParticleFlowObjects", xAODType.FlowElement, algoBuilder = standardReco("PFlow"),
                     prereqs = [inputsFromContext("Tracks"), "input:CaloCalTopoClusters"],
                     ),

    JetInputExternal("GlobalParticleFlowObjects", xAODType.FlowElement,
                     algoBuilder = inputcfg.buildPFlowSel,
                     prereqs = ["input:JetETMissParticleFlowObjects", ],
                     ),
    
    # *****************************
    JetInputExternal("InDetTrackParticles",   xAODType.TrackParticle,
                     algoBuilder = standardReco("Tracks"),
                     filterfn = lambda flags : (flags.Reco.EnableTracking or "InDetTrackParticles" in flags.Input.Collections, "Tracking is disabled and no InDetTrackParticles in input")
                     ),
    # alternative ID tracks for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    JetInputExternal("InDetTrackParticles_EleRM",   xAODType.TrackParticle),

    JetInputExternal("PrimaryVertices",   xAODType.Vertex,
                     prereqs = [inputsFromContext("Tracks")],
                     filterfn = lambda flags : (flags.Beam.Type == BeamType.Collisions, f"No vertexing with {flags.Beam.Type}"), # should be changed when a reliable "EnableVertexing" flag exists
                     ),
    # No quality criteria are applied to the tracks, used for ghosts for example
    JetInputExternal("JetSelectedTracks",     xAODType.TrackParticle,
                     prereqs= [ inputsFromContext("Tracks") ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                     algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=False )
                     ),
    # alternative JetSelected tracks for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    JetInputExternal("JetSelectedTracks_EleRM",     xAODType.TrackParticle,
                     prereqs= [ inputsFromContext("Tracks") ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                     algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=False )
                     ),
    # alternative ID tracks for ftf
    JetInputExternal("JetSelectedTracks_ftf",     xAODType.TrackParticle,
                     prereqs= [ inputsFromContext("Tracks") ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                     algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=False )
                     ),
    
    # Apply quality criteria defined via trackSelOptions in jdef.context (used e.g. for track-jets)
    JetInputExternal("JetSelectedTracks_trackSelOpt",     xAODType.TrackParticle,
                     prereqs= [ inputsFromContext("Tracks") ], # in std context, this is InDetTrackParticles (see StandardJetContext)  
                     algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=True )
                     ),
    JetInputExternal("JetTrackUsedInFitDeco", xAODType.TrackParticle,
                     prereqs= [ inputsFromContext("Tracks") , # in std context, this is InDetTrackParticles (see StandardJetContext)
                                inputsFromContext("Vertices")],
                     algoBuilder = inputcfg.buildJetTrackUsedInFitDeco
                     ),
    JetInputExternal("JetTrackVtxAssoc", xAODType.TrackParticle,
        algoBuilder = 
        lambda jdef, _ : jrtcfg.getJetTrackVtxAlg(
            jdef.context, 
            algname="jetTVA" if jdef.context in ["HL_LHC", "default", "notrk", ""] else f"jetTVA_{jdef.context}", 
            WorkingPoint="Nonprompt_All_MaxWeight"
        ),
        # previous default for ttva : WorkingPoint="Custom", d0_cut= 2.0, dzSinTheta_cut= 2.0 
        prereqs = ["input:JetTrackUsedInFitDeco", inputsFromContext("Vertices")],
    ),
    # alternative JetTrackVtxAssoc for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    JetInputExternal("JetTrackVtxAssoc_EleRM",      xAODType.TrackParticle,
                     algoBuilder = lambda jdef,_ : jrtcfg.getJetTrackVtxAlg(jdef.context, algname="jetTVA_" + jdef.context, WorkingPoint="Nonprompt_All_MaxWeight"),
                     # previous default for ttva : WorkingPoint="Custom", d0_cut= 2.0, dzSinTheta_cut= 2.0 
                     prereqs = ["input:JetTrackUsedInFitDeco", inputsFromContext("Vertices") ]
                     ),
    JetInputExternal("JetTrackVtxAssoc_ftf",      xAODType.TrackParticle,
                     algoBuilder = lambda jdef,_ : jrtcfg.getJetTrackVtxAlg(jdef.context, algname="jetTVA_" + jdef.context, WorkingPoint="Nonprompt_All_MaxWeight"),
                     # previous default for ttva : WorkingPoint="Custom", d0_cut= 2.0, dzSinTheta_cut= 2.0 
                     prereqs = ["input:JetTrackUsedInFitDeco", inputsFromContext("Vertices") ]
                     ),
    # *****************************
    JetInputExternal("EventDensity", "EventShape", algoBuilder = inputcfg.buildEventShapeAlg,
                     containername = lambda jetdef, specs : (specs or "")+"Kt4"+jetdef.inputdef.label+"EventShape",
                     prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name] # this will force the input to be build *before* the EventDensity alg.
    ),
    # alternative EventDensity for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    JetInputExternal("EleRM_EventDensity", "EventShape", algoBuilder = inputcfg.buildEventShapeAlg,
                    containername = lambda jetdef, specs : (specs or "")+"Kt4"+jetdef.inputdef.label+"EventShape",
                    prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name],
                    specs = "EleRM_"
    ),
    JetInputExternal("HLT_EventDensity", "EventShape", algoBuilder = inputcfg.buildEventShapeAlg,
                     containername = lambda jetdef, specs : (specs or "")+"Kt4"+jetdef.inputdef.label+"EventShape",
                     prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name], # this will force the input to be build *before* the EventDensity alg.
                     specs = 'HLT_'
                     ),

    # *****************************
    JetInputExternal("MuonSegments", "MuonSegment", algoBuilder=standardReco("Muons"),
                     prereqs = [inputsFromContext("Tracks")], # most likely wrong : what exactly do we need to build muon segments ?? (and not necessarily full muons ...)
                     filterfn = lambda flags : (flags.Reco.EnableCombinedMuon or "MuonSegments" in flags.Input.Collections, "Muon reco is disabled"),
                     ),


    # *****************************
    # Truth particles from the hard scatter vertex prior to Geant4 simulation.
    # Neutrinos and muons are omitted; all other stable particles are included.
    JetInputExternal("JetInputTruthParticles",  xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruth, filterfn=isMC ),

    # Truth particles from the hard scatter vertex prior to Geant4 simulation.
    # Prompt electrons, muons and neutrinos are excluded, all other stable particles
    # are included, in particular leptons and neutrinos from hadron decays.
    JetInputExternal("JetInputTruthParticlesNoWZ",  xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruth, filterfn=isMC,specs="NoWZ"),

    # Truth particles from the hard scatter vertex prior to Geant4 simulation.
    # Similar configuration as for JetInputTruthParticlesNoWZ but with slightly
    # different photon dressing option
    JetInputExternal("JetInputTruthParticlesDressedWZ", xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruth, filterfn=isMC,specs="DressedWZ"),

    # Truth particles from the hard scatter vertex prior to Geant4 simulation.
    # Only charged truth particles are used
    JetInputExternal("JetInputTruthParticlesCharged", xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruth, filterfn=isMC,specs="Charged"),


    #**************
    # TEMPORARY : special inputs for EVTGEN jobs (as long as gen-level and reco-level definitions are not harmonized)
    JetInputExternal("JetInputTruthParticlesGEN",  xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruthGEN, filterfn=isMC ),

    JetInputExternal("JetInputTruthParticlesGENNoWZ",  xAODType.TruthParticle,
                     algoBuilder = inputcfg.buildJetInputTruthGEN, filterfn=isMC,specs="NoWZ"),
    #**************
    
    
    JetInputExternal("PV0JetSelectedTracks", xAODType.TrackParticle,
                     prereqs=["input:JetSelectedTracks_trackSelOpt", "input:JetTrackUsedInFitDeco"],
                     algoBuilder = inputcfg.buildPV0TrackSel ),


    JetInputExternal("UFOCSSK", xAODType.FlowElement,
                     # in analysis releases, or if we have UFOCSSK in inputs don't  declare unneeded dependencies which could fail the config.
                     prereqs =lambda parentjdef :  [] if (isAnalysisRelease() or 'UFOCSSK' in parentjdef._cflags.Input.Collections ) else ['input:GPFlowCSSK'], 
                     filterfn =  lambda flag : ( (not isAnalysisRelease() or 'UFOCSSK' in flag.Input.Collections),  "Can't build UFO in Analysis projects and not UFOCSSK in input") ,
                     algoBuilder = lambda jdef,_ : tcccfg.runUFOReconstruction(jdef._cflags, stdConstitDic['GPFlowCSSK'])
                     ),

    JetInputExternal("UFO", xAODType.FlowElement,
                     prereqs = ['input:GPFlow'],
                     algoBuilder = lambda jdef,_ : tcccfg.runUFOReconstruction(jdef._cflags, stdConstitDic['GPFlow'])
                     ),
    
]


_truthFlavours = ["BHadronsInitial", "BHadronsFinal", "BQuarksFinal",
                  "CHadronsInitial", "CHadronsFinal", "CQuarksFinal",
                  "TausFinal",
                  "WBosons", "ZBosons", "HBosons", "TQuarksFinal",
                  "Partons",]
for label in  _truthFlavours:    
    # re-use the main truth input definition : 
    _stdInputList.append( JetInputExternal("TruthLabel"+label, xAODType.TruthParticle,
                                      algoBuilder = inputcfg.buildLabelledTruth,
                                      filterfn=isMC, specs = label ) )



# Fill the stdInputExtDic from the above list 
for ji in _stdInputList:
    ji._locked = True # lock the definitions so we have unmutable references !
    stdInputExtDic[ji.name] = ji




    



## ***************************************
## List of standard constituent sequences
##  This sequences uses the constit modifiers below     
_stdSeqList = [
    # Format is typically :
    # JetInputConstitSeq( name , input_cont_type, list_of_modifiers, inputcontainer, outputcontainer )
    # or
    # JetInputConstit( name, input_cont_type, containername)
    # see JetDefinition.py for details.

    # *****************************
    # Cluster constituents : the first one is a relic used for isolation, and might disappear soon
    JetInputConstitSeq("EMTopo", xAODType.CaloCluster, ["EM"],
                       "CaloCalTopoClusters", "EMTopoClusters", jetinputtype="EMTopo",
                       ),
    JetInputConstitSeq("LCTopo", xAODType.CaloCluster, ["LC"],
                       "CaloCalTopoClusters", "LCTopoClusters", jetinputtype="LCTopo",
                       ),
    JetInputConstitSeq("EMTopoOrigin", xAODType.CaloCluster, ["EM","Origin"],
                       "CaloCalTopoClusters", "EMOriginTopoClusters", jetinputtype="EMTopo",
                       ),
    JetInputConstitSeq("LCTopoOrigin",xAODType.CaloCluster, ["LC","Origin"],
                       "CaloCalTopoClusters", "LCOriginTopoClusters", jetinputtype="LCTopo",
                       ),
    # alternative LCTopoOrigin for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    JetInputConstitSeq("LCTopoOrigin_EleRM",xAODType.CaloCluster, ["LC","Origin"],
                       "CaloCalTopoClusters_EleRM", "LCOriginTopoClusters_EleRM", jetinputtype="LCTopo",
                       ),
    JetInputConstitSeq("LCTopoCSSK",  xAODType.CaloCluster, ["LC","Origin","CS","SK"],
                       "CaloCalTopoClusters", "LCOriginTopoCSSK", jetinputtype="LCTopo",
                       ),
    


    
    # *****************************
    # EM-scale particle flow objects with charged hadron subtraction
    # For now we don't specify a scale, as only one works well, but
    # this could be incorporated into the naming scheme and config
    JetInputConstitSeq("EMPFlow", xAODType.FlowElement,["CorrectPFO", "CHS"] , 'JetETMissParticleFlowObjects', 'CHSParticleFlowObjects'),

    # GPFlow are the same than EMPFlow except they have pflow linked to elec or muons filtered out.
    JetInputConstitSeq("GPFlow", xAODType.FlowElement,["CorrectPFO", "CHS"] , 'GlobalParticleFlowObjects', 'CHSGParticleFlowObjects',
                       label='EMPFlow'),
    
    # Particle Flow Objects with several neutral PFO copies for by-vertex reconstruction
    JetInputConstitSeq("GPFlowByVtx", xAODType.FlowElement, ["CorrectPFO", "CHS"] , 'GlobalParticleFlowObjects', 'CHSByVtxGParticleFlowObjects',
                       label='EMPFlowByVertex', byVertex=True),
    
    # Particle Flow Objects with Constituent Subtraction + SoftKiller
    JetInputConstitSeq("EMPFlowCSSK", xAODType.FlowElement,["CorrectPFO",  "CS","SK", "CHS"] ,
                       'JetETMissParticleFlowObjects', 'CSSKParticleFlowObjects', jetinputtype="EMPFlow"),

    JetInputConstitSeq("GPFlowCSSK", xAODType.FlowElement,["CorrectPFO",  "CS","SK", "CHS"] ,
                       'GlobalParticleFlowObjects', 'CSSKGParticleFlowObjects', jetinputtype="EMPFlow", label='EMPFlowCSSK'),



    JetInputConstit("UFOCSSK", xAODType.FlowElement, "UFOCSSK" ),

    JetInputConstit("UFO", xAODType.FlowElement, "UFO" ),
    
    # *****************************
    # Tower (used only as ghosts atm)
    JetInputConstit("Tower", xAODType.CaloCluster, "CaloCalFwdTopoTowers",
                    filterfn = lambda flags : ("CaloCalFwdTopoTowers" in flags.Input.Collections, "Towers as ghosts disabled as CaloCalFwdTopoTowers are not in the input")),

    # *****************************
    # Track constituents (e.g. ghosts, no quality criteria, no TTVA)
    JetInputConstit("Track", xAODType.TrackParticle, inputsFromContext("JetTracks")),
    # Track constituents (e.g. track-jets, trackSelOptions quality criteria, TTVA)
    JetInputConstit("PV0Track", xAODType.TrackParticle, inputsFromContext("JetTracks", prefix="PV0")),

    # LRT. Only used as ghosts
    JetInputConstit("TrackLRT", xAODType.TrackParticle, "InDetLargeD0TrackParticles", 
                    filterfn = lambda flags : (flags.Tracking.doLargeD0 and "InDetLargeD0TrackParticles" in flags.Input.Collections, "Large radius tracking did not run")),

    # *****************************
    # Muon segments. Only used as ghosts
    JetInputConstit("MuonSegment", "MuonSegment", "MuonSegments",                    ),

    # *****************************
    # VR track jets as ghosts for large-R jets
    #  this could work : 
    #JetInputConstit("AntiKtVR30Rmax4Rmin02PV0TrackJet", xAODType.Jet, "AntiKtVR30Rmax4Rmin02PV0TrackJets"),
    # BUT a better solution is to call
    # registerAsInputConstit(AntiKtVR30Rmax4Rmin02PV0Track) 
    #  at the place where the jetdef 'AntiKtVR30Rmax4Rmin02PV0Track' is defined : see StandardSmallRJets.py 
    
    # *****************************
    # Truth particles (see JetInputExternal declarations above for more details)
    JetInputConstit("Truth", xAODType.TruthParticle, "JetInputTruthParticles" ),
    
    JetInputConstit("TruthWZ", xAODType.TruthParticle, "JetInputTruthParticlesNoWZ", jetinputtype="TruthWZ"),

    JetInputConstit("TruthDressedWZ", xAODType.TruthParticle, "JetInputTruthParticlesDressedWZ", jetinputtype="TruthDressedWZ"),

    JetInputConstit("TruthCharged", xAODType.TruthParticle, "JetInputTruthParticlesCharged", jetinputtype="TruthCharged"),

    #**************
    # TEMPORARY : special inputs for EVTGEN jobs (as long as gen-level and reco-level definitions are not harmonized)
    JetInputConstit("TruthGEN", xAODType.TruthParticle, "JetInputTruthParticlesGEN" , label="Truth"),
    
    JetInputConstit("TruthGENWZ", xAODType.TruthParticle, "JetInputTruthParticlesGENNoWZ", jetinputtype="TruthWZ", label="TruthWZ"),
        
]

# define JetInputConstit for each flavour type :
for label in  _truthFlavours:    
    _stdSeqList.append( JetInputConstit(label, xAODType.TruthParticle, "TruthLabel"+label ) )

# Fill the stdConstitDic from the above list 
for jc in _stdSeqList:
    jc._locked = True
    stdConstitDic[jc.name] = jc



########################################################################
## List of standard constituent modifiers 

def _getPFOTool(*l):
    """One Property of the CorrectPFO constit modifier is a tool. 
    we use this function as a placeholder, allowing to delay the intantiation of this property tool
    to the time the modifier itself is instantiated.
    """
    from AthenaConfiguration.ComponentFactory import CompFactory
    return CompFactory.getComp("CP::WeightPFOTool")("weightPFO")
    


vtxKey = "PrimaryVertices"
tvaKey = "JetTrackVtxAssoc"
_stdModList = [
    # Format is :
    # JetConstitModifier( name , toolType, dictionnary_of_tool_properties )
    # (see JetDefinition.py for more details)
    
    JetConstitModifier("Origin", "CaloClusterConstituentsOrigin", prereqs=[inputsFromContext("Vertices")]),
    JetConstitModifier("EM",     "ClusterAtEMScaleTool", ),
    JetConstitModifier("LC",     "", ),
    # Particle flow
    JetConstitModifier("CorrectPFO", "CorrectPFOTool",
                       # get the track properties from the context with wich jet will be configured with propFromContext
                       # See StandardJetContext.py for the default values.
                       prereqs=[inputsFromContext("Vertices")],
                       properties=dict(VertexContainerKey=propFromContext("Vertices"),
                                       WeightPFOTool= _getPFOTool,
                                       DoByVertex = lambda jdef, _: jdef.byVertex) ), 
              
    JetConstitModifier("CHS",    "ChargedHadronSubtractionTool",
                       # get the track properties from the context with wich jet will be configured with propFromContext
                       # See StandardJetContext.py for the default values.
                       # Note : Jet trigger still needs an older CHS config, hence the cheks to jetdef.context below...
                       #        When jet trigger migrate and follow the offline settings all this can be simplified.
                       prereqs= lambda parentjdef : [inputsFromContext("Vertices"),] + ( [inputsFromContext("TVA")] if parentjdef.context=='default' else []) ,
                       properties=dict(VertexContainerKey=propFromContext("Vertices"),                                       
                                       TrackVertexAssociation=propFromContext("TVA"),
                                       UseTrackToVertexTool= lambda jdef,_: jdef.context in ['default', 'HL_LHC'], 
                                       DoByVertex = lambda jdef, _: jdef.byVertex
                                       )),
    
    # Pileup suppression
    JetConstitModifier("Vor",    "VoronoiWeightTool", properties=dict(doSpread=False, nSigma=0) ),
    JetConstitModifier("CS",     "ConstituentSubtractorTool", properties=dict(MaxEta=4.5 ) ),
    JetConstitModifier("SK",     "SoftKillerWeightTool",),
                           
]

# Fill the stdContitModifDic from the above list 
for ji in _stdModList:
    ji._locked = True
    stdContitModifDic[ji.name] = ji
