# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon import Logging
metlog = Logging.logging.getLogger('METConfig')

from GaudiKernel.Constants import INFO
import six

#################################################################################
# Define some default values


defaultInputKey = {
   'Ele'       :'Electrons',
   'LRTEle'    :'LRTElectrons',
   'Gamma'     :'Photons',
   'Tau'       :'TauJets',
   'LCJet'     :'AntiKt4LCTopoJets',
   'EMJet'     :'AntiKt4EMTopoJets',
   'PFlowJet'  :'AntiKt4EMPFlowJets',
   'Muon'      :'Muons',
   'MuonLRT'   :'MuonsLRT',
   'Soft'      :'',
   'Clusters'  :'CaloCalTopoClusters',
   'Tracks'    :'InDetTrackParticles',
   'PFlowObj'  :'CHSGParticleFlowObjects',
   'PrimVxColl':'PrimaryVertices',
   'Truth'     :'TruthEvents',
   }

prefix = 'METAssocConfig:   '

#################################################################################
# Configuration of builders

class AssocConfig:
    def __init__(self,objType='',inputKey=''):
        self.objType = objType
        self.inputKey = inputKey

def getAssociator(configFlags, config,suffix,doPFlow=False,
                  trkseltool=None,
                  trkisotool=None,caloisotool=None,
                  useFELinks=False,
                  modConstKey="",
                  modClusColls={}):
    tool = None
    doModClus = (modConstKey!="" and not doPFlow)
    if doModClus:
        modLCClus = modClusColls['LC{0}Clusters'.format(modConstKey)]
        modEMClus = modClusColls['EM{0}Clusters'.format(modConstKey)]

    # Construct tool and set defaults for case-specific configuration
    if config.objType == 'Ele':
        tool = CompFactory.getComp("met::METElectronAssociator")('MET_ElectronAssociator_'+suffix,TCMatchMethod=1)
    if config.objType == 'LRTEle':
        tool = CompFactory.getComp("met::METElectronAssociator")('MET_LRTElectronAssociator_'+suffix,TCMatchMethod=1)
    if config.objType == 'Gamma':
        tool = CompFactory.getComp("met::METPhotonAssociator")('MET_PhotonAssociator_'+suffix,TCMatchMethod=1)
    if config.objType == 'Tau':
        tool = CompFactory.getComp("met::METTauAssociator")('MET_TauAssociator_'+suffix)
    if config.objType == 'LCJet':
        tool = CompFactory.getComp("met::METJetAssocTool")('MET_LCJetAssocTool_'+suffix)
    if config.objType == 'EMJet':
        tool = CompFactory.getComp("met::METJetAssocTool")('MET_EMJetAssocTool_'+suffix)
    if config.objType == 'PFlowJet':
        tool = CompFactory.getComp("met::METJetAssocTool")('MET_PFlowJetAssocTool_'+suffix)
    if config.objType == 'CustomJet':
        tool = CompFactory.getComp("met::METJetAssocTool")('MET_CustomJetAssocTool_'+suffix)
    if config.objType == 'Muon':
        tool = CompFactory.getComp("met::METMuonAssociator")('MET_MuonAssociator_'+suffix)
    if config.objType == 'MuonLRT':
        tool = CompFactory.getComp("met::METMuonAssociator")('MET_MuonLRTAssociator_'+suffix)
    if config.objType == 'Soft':
        tool = CompFactory.getComp("met::METSoftAssociator")('MET_SoftAssociator_'+suffix)
        tool.DecorateSoftConst = True
        if doModClus:
            tool.LCModClusterKey = modLCClus
            tool.EMModClusterKey = modEMClus
    if config.objType == 'Truth':
        tool = CompFactory.getComp("met::METTruthAssociator")('MET_TruthAssociator_'+suffix)
        tool.RecoJetKey = config.inputKey
    if doPFlow:
        tool.PFlow = True
        tool.FlowElementCollection = modConstKey if modConstKey!="" else defaultInputKey["PFlowObj"]
    else:
        tool.UseModifiedClus = doModClus
    tool.UseFELinks = False if config.objType == 'MuonLRT' or config.objType == 'LRTEle' else  useFELinks
    # set input/output key names
    if config.inputKey == '' and defaultInputKey[config.objType] != '':
        tool.InputCollection = defaultInputKey[config.objType]
        config.inputKey = tool.InputCollection
    elif hasattr(tool, 'InputCollection'):
        tool.InputCollection = config.inputKey
    if doModClus:
        tool.ClusColl = modLCClus
        if 'EMTopo' in suffix: tool.ClusColl = modEMClus
    tool.TrkColl = defaultInputKey['Tracks']
    tool.UseTracks = configFlags.MET.UseTracks
    tool.TrackSelectorTool = trkseltool
    tool.TrackIsolationTool = trkisotool
    tool.CaloIsolationTool = caloisotool

    return tool

#################################################################################
# Top level MET configuration

class METAssocConfig:
    def outputCollections(self):
        if self.doTruth: return 'MET_Core_'+self.suffix
        else: return 'MET_Core_'+self.suffix,'MET_Reference_'+self.suffix
    #
    def outputMap(self):
        return 'METAssoc_'+self.suffix
    #
    def setupAssociators(self, configFlags, buildconfigs):
        metlog.info("{} Setting up associators for MET config {}".format(prefix,self.suffix))
        for config in buildconfigs:
            if config.objType in self.associators:
                metlog.error("{} Config {} already contains a associator of type {}".format(prefix,self.suffix,config.objType))
                raise LookupError
            else:
                associator = getAssociator(configFlags, config=config,suffix=self.suffix,
                                           doPFlow=self.doPFlow,
                                           useFELinks=self.useFELinks,
                                           trkseltool=self.trkseltool,
                                           trkisotool=self.trkisotool,
                                           caloisotool=self.caloisotool,
                                           modConstKey=self.modConstKey,
                                           modClusColls=self.modClusColls)
                self.associators[config.objType] = associator
                self.assoclist.append(associator)
                metlog.info("{} Added {} tool named {}".format(prefix,config.objType,associator.name))
    #
    def __init__(self,suffix,inputFlags,buildconfigs=[],
                 doPFlow=False, doTruth=False,
                 usePFOLinks=False,
                 trksel=None,
                 modConstKey="",
                 modClusColls={}
                 ):
        self.accumulator = ComponentAccumulator()
        # Set some sensible defaults
        modConstKey_tmp = modConstKey
        modClusColls_tmp = modClusColls
        if doPFlow:
            # Ideally this should not be hardcoded but linked to the JetDefinition with which this MET is built
            # TODO : in new config, if possible use something like: jetdef.inputdef.containername             
            if modConstKey_tmp == "": modConstKey_tmp = "CHSGParticleFlowObjects"
        else:
            if modConstKey_tmp == "": modConstKey_tmp = "OriginCorr"
            if modClusColls_tmp == {}: modClusColls_tmp = {'LCOriginCorrClusters':'LCOriginTopoClusters',
                                                           'EMOriginCorrClusters':'EMOriginTopoClusters'}
        if doTruth:
            metlog.info("{} Creating MET TruthAssoc config {}".format(prefix,suffix))
        else:
            metlog.info("{} Creating MET Assoc config {}".format(prefix,suffix))
        self.suffix = suffix
        self.doPFlow = doPFlow
        self.useFELinks = usePFOLinks
        self.modConstKey=modConstKey_tmp
        self.modClusColls=modClusColls_tmp
        self.doTruth = doTruth
        if trksel:
            self.trkseltool = trksel
        else:
            # TODO: These Z0 and D0 cuts are left over from R21. The track vertex association can now use looser ones.
            #       To be investigated and possibly updated by the MET group.
            self.trkseltool=CompFactory.getComp("InDet::InDetTrackSelectionTool")("IDTrkSel_METAssoc",
                                                                  CutLevel="TightPrimary",
                                                                  maxZ0SinTheta=3,
                                                                  maxD0=2,
                                                                  minPt=500)

        self.trkisotool = CompFactory.getComp("xAOD::TrackIsolationTool")("TrackIsolationTool_MET")
        self.trkisotool.TrackSelectionTool = self.trkseltool # As configured above
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg  
        extrapCfg = AtlasExtrapolatorCfg(inputFlags)
        CaloExtensionTool= CompFactory.getComp("Trk::ParticleCaloExtensionTool")(Extrapolator = self.accumulator.popToolsAndMerge(extrapCfg))
        CaloCellAssocTool =  CompFactory.getComp("Rec::ParticleCaloCellAssociationTool")(ParticleCaloExtensionTool = CaloExtensionTool)
        self.caloisotool = CompFactory.getComp("xAOD::CaloIsolationTool")("CaloIsolationTool_MET",
                                                          saveOnlyRequestedCorrections=True,
                                                          ParticleCaloExtensionTool = CaloExtensionTool,
                                                          ParticleCaloCellAssociationTool = CaloCellAssocTool)
        self.associators = {}
        self.assoclist = [] # need an ordered list
        #
        self.setupAssociators(inputFlags, buildconfigs)

# Set up a top-level tool with mostly defaults
def getMETAssocTool(topconfig,msglvl=INFO):
    assocTool = None
    if topconfig.doTruth:
        assocTool = CompFactory.getComp("met::METAssociationTool")('MET_TruthAssociationTool_'+topconfig.suffix,
                                                   METAssociators = topconfig.assoclist,
                                                   METSuffix = topconfig.suffix)
    else:
        assocTool = CompFactory.getComp("met::METAssociationTool")('MET_AssociationTool_'+topconfig.suffix,
                                                   METAssociators = topconfig.assoclist,
                                                   METSuffix = topconfig.suffix,
                                                   OutputLevel=msglvl)
    return assocTool

# Convert the provided METAssocConfigs into a concrete algorithm
def getMETAssocAlg(algName='METAssociation',configs={},tools=[],msglvl=INFO):

    assocTools = []
    assocTools += tools

    if configs=={} and tools==[]:
        metlog.info("{} Empty list of MET association configs provided. None will be reconstructed.".format(prefix))
    for key,conf in six.iteritems(configs):
        metlog.info("{} Generate METAssocTool for MET_{}".format(prefix,key))
        assoctool = getMETAssocTool(conf,msglvl)
        assocTools.append(assoctool)

    for tool in assocTools:
        metlog.info("{} Added METAssocTool {} to alg {}".format(prefix,tool.name,algName))
    assocAlg = CompFactory.getComp("met::METRecoAlg")(name=algName,
                                      RecoTools=assocTools)
    return assocAlg
