#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

##########################################################################################
# Helper functions to digest the reconstruction options dictionary
# and translate it into the python configuration objects used by
# jet reco code.

from JetRecConfig.JetDefinition import JetInputConstitSeq,JetInputConstit, xAODType, JetDefinition
from ..Menu.SignatureDicts import JetRecoKeys as recoKeys
from JetRecConfig import StandardJetContext
# this is to define trigger specific JetModifiers (ex: ConstitFourMom_copy) : 
from . import TriggerJetMods
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

import copy

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

##########################################################################################
### --- Common jet eta ranges --- 
etaRangeAbbrev = {
    "j":"0eta320", # default
    "a":"0eta490",
    "c":"0eta240",
    "f":"320eta490"
}

##########################################################################################
### --- Extracting jet chain parts --- 

def jetChainParts(chainParts):
    jChainParts = []
    for p in chainParts:
        if p['trigType'] == 'j':
            jChainParts.append(p)
    return jChainParts

##########################################################################################
### --- General reco dict handling --- 

# Translate the reco dict to a string for suffixing etc
def jetRecoDictToString(jetRecoDict):
    if jetRecoDict['ionopt']=='ion':
        # Unique settings for heavy ions
        # other values will be default
        return "a4_ion"
    strtemp = "{recoAlg}_{constitMod}{constitType}_{clusterCalib}_{jetCalib}"
    if doTracking(jetRecoDict):
        strtemp += "_{trkopt}"
    return strtemp.format(**jetRecoDict)

# Extract the jet reco dict from the chainDict
def extractRecoDict(chainParts):
    # interpret the reco configuration only
    # eventually should just be a subdict in the chainDict
    recoDict = {}
    for p in chainParts:
        for k in recoKeys:
            # Look for our key in the chain part
            if k in p.keys():
                # found the key, check for consistency with other chain parts of this chain
                if k in recoDict.keys():
                    if p[k] != recoDict[k]:
                        raise RuntimeError('Inconsistent reco setting for %s' % k)
                # copy this entry to the reco dictionary
                recoDict[k] = p[k]

    # set proper jetCalib key in default case
    if recoDict['jetCalib'] == "default":
        recoDict['jetCalib'] = getJetCalibDefaultString(recoDict)

    recoDict['jetDefStr'] = jetRecoDictToString(recoDict)

    return recoDict

# Inverse of the above, essentially only for CF tests
def jetRecoDictFromString(jet_def_string):

    # Translate the definition string into an approximation
    # of the "recoParts" in the jet chainParts.
    jetRecoDict = {}
    from TriggerMenuMT.HLT.Menu.SignatureDicts import JetChainParts,JetChainParts_Default
    for key in recoKeys:
        keyFound = False
        tmp_key =  key
        for part in jet_def_string.split('_'):
            if part in JetChainParts[tmp_key]:
                jetRecoDict[key] = part
                keyFound         = True
        if not keyFound:
            jetRecoDict[key] = JetChainParts_Default[key]

    # set proper jetCalib key in default case
    if jetRecoDict['jetCalib'] == "default":
        jetRecoDict['jetCalib'] = getJetCalibDefaultString(jetRecoDict)

    jetRecoDict['jetDefStr'] = jetRecoDictToString(jetRecoDict)

    return jetRecoDict

# Assist with grooming and reclustering workflows, we need to
# ensure that the name is updated consistently
def cloneAndUpdateJetRecoDict(jetRecoDict,**kwargs):
    newJetRecoDict = copy.copy(jetRecoDict)
    newJetRecoDict.update(dict(**kwargs))
    newJetRecoDict["jetDefStr"] = jetRecoDictToString(newJetRecoDict)
    return newJetRecoDict

##########################################################################################
### --- General helpers ---

# decodes SignatureDict field recoAlg
# returns 3 values:
#   - jet reco alg: 'a' (antikT)
#   - jet radius: 4, 10,..
#   - extra reco details: e.g. 'r' (reclustered), 't' (trimmed), 'sd' (softdrop)
def interpretRecoAlg(recoAlg):
    import re
    jetalg, jetradius, jetextra = re.split(r'(\d+)',recoAlg)    
    return jetalg, int(jetradius), jetextra

# Check if jet definition needs tracks or if it should be agnostic of the tracking choice
def jetDefNeedsTracks(jetRecoDict):
  # For tc_a10, tc_a10t and tc_a10sd, we will be agnostic of tracking (no suffix will be added)
  # For everything else (constitType=pf or dependence on small-R jets) we need to be aware of what tracking was used
  return jetRecoDict["trkopt"]!="notrk" and (jetRecoDict["constitType"]!="tc" or jetRecoDict["recoAlg"] in ['a4','a10'])

# Check if track reconstruction is enabled
def doTracking(jetRecoDict):
    return jetRecoDict["trkopt"]!="notrk"
# Check if full scan track reconstruction is enabled
def doFSTracking(jetRecoDict):
    return jetRecoDict["trkopt"]=="ftf"

# Check if constituent type is pflow. Concurrently check that the tracking option is valid.
def isPFlow(jetRecoDict):
    isPFlow = jetRecoDict["constitType"] == "pf"
    if isPFlow and not doFSTracking(jetRecoDict):
        raise ValueError("This is a PFlow chain but an incompatible tracking option is given!")
    return isPFlow

# return the min jet pT in MeV for the configured recoAlg
def getFilterCut(recoAlg):
    return {"a4":4000, "a10":50000, "a10r": 50000, "a10t":50000, "a10sd":50000}[recoAlg]

def getJetContext(jetRecoDict):
    context = StandardJetContext.jetContextDic.get(jetRecoDict["trkopt"],None) 
    if doTracking(jetRecoDict) and not context:
        raise ValueError("Tracking option provided but no jet context returned!")
    return context


##########################################################################################
### --- String getters ---

def getHLTPrefix():
    prefix = "HLT_"
    return prefix

def getClustersKey(recoDict):
        clusterCalib = recoDict["clusterCalib"]
        if clusterCalib == "em":
            from ..CommonSequences.FullScanDefs import em_clusters
            return em_clusters
        elif clusterCalib == "lcw":
            from ..CommonSequences.FullScanDefs import lc_clusters
            return lc_clusters
        else:
            raise ValueError("Invalid value for calib: '{}'".format(clusterCalib))

def getJetCalibDefaultString(recoDict):
    if recoDict['recoAlg'] == 'a4':
        if recoDict['constitType'] == 'tc':
            return 'subresjesgscIS' if recoDict['trkopt'] == 'ftf' else 'subjesIS'
        elif recoDict['constitType'] == 'pf':
            return 'subresjesgscIS'
    elif recoDict['recoAlg'] == 'a10':
        return 'subjes'
    elif recoDict['recoAlg'] == 'a10t':
        return 'jes'
    elif recoDict['recoAlg'] == 'a10sd':
        return 'jes'
    elif recoDict['recoAlg'] == 'a10r':
        return 'subjesIS' # calibration for the small-R jets used to reconstruct the reclustered jets
    else:
        raise RuntimeError('No default calibration is defined for %s' % recoDict['recoAlg'])

cleaningDict = {
    'CLEANlb':  'LooseBad',
    'CLEANllp': 'LooseBadLLP',
}
# returns cleaning string based on prefilter list
def getPrefilterCleaningString(prefilters_list):
    found_cleanings= [ci for ck, ci in cleaningDict.items() if ck in prefilters_list]
    if len(found_cleanings) <= 1:  # Only one supported cleaning decoration at the moment
        return 'noCleaning' if len(found_cleanings) == 0 else found_cleanings[0]
    else:
        raise RuntimeError(
            'Multijet jet cleanings found in jet trigger reco dictionary {}. Multiple jet cleanings are currently unsupported'.format(found_cleanings))


##########################################################################################
### --- Modifier and decoration list getters ---

# Translate calib specification into something understood by
# the calibration config helper
def getCalibMods(flags,jetRecoDict,dataSource,rhoKey="auto"):

    config = getInDetTrigConfig( 'fullScan' )

    # Minimum modifier set for calibration w/o track GSC
    # Should eventually build in more mods, depend on track info etc
    jetalg = jetRecoDict["recoAlg"]
    if jetRecoDict["jetCalib"] == "nojcalib" or jetalg=="a10r":
        calibMods = []
    else:
        dotracking = doTracking(jetRecoDict)
        if not dotracking and "gsc" in jetRecoDict["jetCalib"]:
            raise ValueError("Track GSC requested but no track source provided!")

        if not dotracking and "subres" in jetRecoDict["jetCalib"]:
            raise ValueError("Pileup residual calibration requested but no track source provided!")

        if jetRecoDict["constitType"] == "tc":
            calibKey = flags.Trigger.Jet.emtopoCalibKey
            calibContext,calibSeq = {
                ("a4","subjes"):         (calibKey,"JetArea_EtaJES_GSC"),          # Calo GSC only ( + insitu in data)
                ("a4","subjesIS"):       (calibKey,"JetArea_EtaJES_GSC"),          # Calo GSC only (no insitu)
                ("a4","subjesgscIS"):    (calibKey,"JetArea_EtaJES_GSC"),          # Calo+Trk GSC ( + insitu in data)
                ("a4","subresjesgscIS"): (calibKey,"JetArea_Residual_EtaJES_GSC"), # pu residual + calo+trk GSC ( + insitu in data)
                ("a4","subjesgsc"):      (calibKey,"JetArea_EtaJES_GSC"),          # Calo+Trk GSC (no insitu)
                ("a4","subresjesgsc"):   (calibKey,"JetArea_Residual_EtaJES_GSC"), # pu residual + calo+trk GSC (no insitu)
                ("a10","subjes"):        ("TrigUngroomed","JetArea_EtaJES"),
                ("a10t","jes"):          ("TrigTrimmed","EtaJES_JMS"),
                }[(jetRecoDict["recoAlg"],jetRecoDict["jetCalib"])]

            pvname = ""
            gscDepth = "EM3"
            if "gsc" in jetRecoDict["jetCalib"]:
                gscDepth = "trackWIDTH"
                pvname = config.vertex_jet

        elif jetRecoDict["constitType"] == "pf":
            gscDepth = "auto"
            if 'sd' in jetRecoDict["recoAlg"]:
                calibContext = "TrigSoftDrop"
                calibSeq = "EtaJES_JMS"
            else:
                calibKey = flags.Trigger.Jet.pflowCalibKey # small-R pflow
                gscDepth = "trackWIDTH"
                if "gsc" not in jetRecoDict["jetCalib"]:
                    gscDepth = "EM3" # calo-only GSC
                calibContext,calibSeq = {
                  ("a4","jes"):    (calibKey,"EtaJES_GSC"),                          # w/o jet area sub, w/o pu residual + calo GSC only (no insitu)
                  ("a4","subjesgsc"):    (calibKey,"JetArea_EtaJES_GSC"),            # w/o pu residual  + calo+trk GSC
                  ("a4","subresjesgsc"): (calibKey,"JetArea_Residual_EtaJES_GSC"),   # pu residual + calo+trk GSC
                  ("a4","subjesgscIS"): (calibKey,"JetArea_EtaJES_GSC"),             # w/o pu residual  + calo+trk GSC
                  ("a4","subresjesgscIS"): (calibKey,"JetArea_Residual_EtaJES_GSC"), # pu residual + calo+trk GSC
                  }[(jetRecoDict["recoAlg"],jetRecoDict["jetCalib"])]
            pvname = config.vertex_jet

        if jetRecoDict["jetCalib"].endswith("IS") and (dataSource=="data"):
            calibSeq += "_Insitu"

        calibSpec = ":".join( [calibContext, dataSource, calibSeq, rhoKey, pvname, gscDepth] )

        if jetalg=="a4":
            calibMods = ["EMScaleMom",
                         "ConstitFourMom_copy",
                         "CaloEnergies", # Needed for GSC
                         "Calib:"+calibSpec]
        else:
            calibMods = ["ConstitFourMom_copy",
                         "Calib:"+calibSpec]

    return calibMods

# Make generating the list a bit more comprehensible
# TODO document where and how this is used.
def getModSpec(modname,modspec=''):
    return (TriggerJetMods.stdJetModifiers[modname],str(modspec))

# Get list of jet attributes to be calculated for jet
def getDecorList(jetRecoDict):
    # Basic jet info provided by the jet builder
    decorlist = []

    # return empty list for non-calibrated jets
    if jetRecoDict['jetCalib'] == 'nojcalib': return decorlist

    decorlist += [ 'AlgorithmType', 'InputType',
                  'ActiveArea', 'ActiveArea4vec_eta', 'ActiveArea4vec_m',
                  'ActiveArea4vec_phi', 'ActiveArea4vec_pt',
                  'EMFrac','HECFrac','EnergyPerSampling','N90Constituents','constit','Tile0Frac']

    if doFSTracking(jetRecoDict):
        decorlist += ["GhostTrack_ftf",
                      "NumTrkPt500","NumTrkPt1000",
                      "SumPtTrkPt500","SumPtTrkPt1000",
                      "TrackWidthPt1000",
                      "JVFCorr", "JvtRpt", "Jvt"]
        if isPFlow(jetRecoDict):
            decorlist += ["SumPtChargedPFOPt500"]
    return decorlist


##########################################################################################
### --- Helpers for track reco ---

# Need to do this hacky extraction to get around the inability
# to hash dicts as input to RecoFragmentsPool.retrieve
def getTrkColls(jetRecoDict):
    trkcolls = {key:jetRecoDict[key] for key in StandardJetContext.jetContextDic["trackKeys"]}
    return trkcolls


##########################################################################################
### --- Jet Object getters --- 

# Define the jet constituents to be interpreted by JetRecConfig
# When actually specifying the reco, clustersKey should be
# set, but default to None to allow certain checks, in particular
# grooming configuration
def defineJetConstit(jetRecoDict,clustersKey=None,pfoPrefix=None):
    constitMods = []
    # Get the details of the constituent definition:
    # type, mods and the input container name
    
    if jetRecoDict["constitType"] == "pf":
        if pfoPrefix is None:
            raise RuntimeError("JetRecoCommon: Cannot define PF jets without pfo prefix!")

        constitMods = ["CorrectPFO"] 
        # apply constituent pileup suppression
        if "vs" in jetRecoDict["constitMod"]:
            constitMods.append("Vor")
        if "cs" in jetRecoDict["constitMod"]:
            constitMods.append("CS")
        if "sk" in jetRecoDict["constitMod"]:
            constitMods.append("SK")
        constitMods += ["CHS"]
        
        inputPFO = pfoPrefix+"ParticleFlowObjects"
        modstring = ''.join(constitMods[1:-1])
        if modstring == '':
            modstring='CHS'

        inputxAODType = xAODType.FlowElement
        if not constitMods:
            jetConstit = JetInputConstitSeq( "HLT_EMPFlow", inputxAODType, constitMods, inputname=inputPFO, outputname=pfoPrefix+"CHSParticleFlowObjects", label="EMPFlow", jetinputtype="EMPFlow")
        else:
            jetConstit = JetInputConstitSeq( "HLT_EMPFlow"+modstring, inputxAODType, constitMods, inputname=inputPFO, outputname=pfoPrefix+modstring+"ParticleFlowObjects",label='EMPFlow'+(modstring if modstring!='CHS' else ''), jetinputtype="EMPFlow" )

            
    if jetRecoDict["constitType"] == "tc":
        # apply constituent pileup suppression
        if "vs" in jetRecoDict["constitMod"]:
            constitMods.append("Vor")
        if "cs" in jetRecoDict["constitMod"]:
            constitMods.append("CS")
        if "sk" in jetRecoDict["constitMod"]:
            constitMods.append("SK")
        # build a modifier identifier :
        modstring = ''.join(constitMods)
        # prepend the cluster calib state :
        if jetRecoDict["clusterCalib"] == "em":
            constitMods = ["EM"] + constitMods
        elif jetRecoDict["clusterCalib"] == "lcw":
            constitMods = ["LC"] + constitMods
        else:
            log.error("cluster calib state not recognised : ",jetRecoDict["clusterCalib"])
        if not clustersKey:
            raise ValueError("cluster key must be provided for topocluster jets.")
            

        if not constitMods:
            jetConstit = JetInputConstitSeq( "HLT_EMTopo",xAODType.CaloCluster, constitMods, inputname=clustersKey, outputname=clustersKey+modstring,label='EMTopo'+modstring)
        else:
            jetConstit = JetInputConstitSeq( "HLT_"+constitMods[0]+"Topo",xAODType.CaloCluster, constitMods, inputname=clustersKey, outputname=clustersKey+modstring,label=constitMods[0]+'Topo'+modstring)

    # declare our new JetInputConstitSeq in the standard dictionary
    from JetRecConfig.StandardJetConstits import stdConstitDic
    stdConstitDic.setdefault(jetConstit.name, jetConstit)

    return jetConstit

    
# Arbitrary min pt for fastjet, set to be low enough for MHT(?)
# Could/should adjust higher for large-R
def defineJets(flags,jetRecoDict,clustersKey=None,prefix='',suffix='',pfoPrefix=None):
    minpt = {
      "default": {
        4:  7000,
        10: 50000 },
      "lowpt": {    # used for HI UPC jet reco, ATR-28158
        4:  4000,
        10: 50000 }
    }

    filter_type = "lowpt" if flags.Trigger.Jet.LowPtFilter else "default"
    jetalg, jetradius, jetextra = interpretRecoAlg(jetRecoDict["recoAlg"])
    actualradius = float(jetradius)/10
    jetConstit = defineJetConstit(jetRecoDict,clustersKey,pfoPrefix)

    suffix="_"+jetRecoDict["jetCalib"]+'_'*(suffix.strip()!='')+suffix
    if jetDefNeedsTracks(jetRecoDict):
        suffix += "_"+jetRecoDict["trkopt"]

    jetDef = JetDefinition( "AntiKt", actualradius, jetConstit, ptmin=minpt[filter_type][jetradius], prefix=prefix, suffix=suffix, context=jetRecoDict["trkopt"])
    return jetDef

def defineReclusteredJets(jetRecoDict,smallRjets,inputlabel,prefix,suffix):
    rcJetConstit = JetInputConstit("RCJet", xAODType.Jet, smallRjets, label=inputlabel+'RC')
    rcJetDef = JetDefinition( "AntiKt", 1.0, rcJetConstit, prefix=prefix, suffix=suffix)
    return rcJetDef

def defineGroomedJets(jetRecoDict,ungroomedDef):#,ungroomedJetsName):
    from JetRecConfig.JetGrooming import JetTrimming, JetSoftDrop
    groomAlg = jetRecoDict["recoAlg"][3:] if 'sd' in jetRecoDict["recoAlg"] else jetRecoDict["recoAlg"][-1]
    suffix = "_"+ jetRecoDict["jetCalib"]
    if jetDefNeedsTracks(jetRecoDict):
        suffix += "_"+jetRecoDict["trkopt"]
    
    groomDef = {
        "sd":JetSoftDrop(ungroomedDef,ZCut=0.1,Beta=1.0, suffix=suffix),
        "t" :JetTrimming(ungroomedDef,RClus=0.2,PtFrac=0.04, suffix=suffix),
    }[groomAlg]
    return groomDef

#Jet Definition for VR track jets
def defineVRTrackJets(Rmax, Rmin, VRMassScale, Ptmin, prefix, suffix):
    jetconstit = JetInputConstit("PV0Track", xAODType.TrackParticle, "PV0JetSelectedTracks_ftf")
    VRTrackJetDef = JetDefinition("AntiKt", Rmax, jetconstit, ptmin=Ptmin, VRMinR=Rmin, VRMassSc=VRMassScale, prefix=prefix, suffix=suffix, lock=True)
    return VRTrackJetDef


def defineHIJets(jetRecoDict,clustersKey=None,prefix='',suffix=''):
    minpt = {2:7000, 3:7000, 4:7000, 6:7000, 10:50000}
    jetalg, jetradius, jetextra = interpretRecoAlg(jetRecoDict["recoAlg"])
    actualradius = float(jetradius)/10
    constitMods = [] # modifiers
    jetConstit = []
    jetConstit = JetInputConstitSeq( "HLT_HIConstit",xAODType.CaloCluster, constitMods, inputname=clustersKey, outputname=clustersKey,label='HI')
    from JetRecConfig.StandardJetConstits import stdConstitDic
    stdConstitDic.setdefault(jetConstit.name, jetConstit)

    jetDef = JetDefinition( "AntiKt", actualradius, jetConstit, ptmin=minpt[jetradius], prefix=prefix, suffix=suffix)
    return jetDef
