# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, ChainStep, SelectionCA, InViewRecoCA
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType, Format

from TrigmuComb.TrigmuCombConfig import l2MuCombRecoCfg, l2MuCombHypoCfg
from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoToolFromDict, TrigMuonEFCombinerHypoToolFromDict, TrigMuonEFTrackIsolationHypoToolFromDict
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
from TrigInDetConfig.utils import getInDetFlagsForSignature

from AthenaConfiguration.ComponentFactory import CompFactory

from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonInsideOutRecoAlgCfg, MuonInDetToMuonSystemExtensionAlgCfg
from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCombinedInDetCandidateAlgCfg, MuonCombinedAlgCfg, MuonCreatorAlgCfg

from TrigMuonEF.TrigMuonEFConfig import TrigMuonEFTrackIsolationAlgCfg, MuonFilterAlgCfg, MergeEFMuonsAlgCfg
from AthenaCommon.CFElements import seqAND, parOR, seqOR

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def fakeHypoAlgCfg(flags, name="FakeHypoForMuon"):
    HLTTest__TestHypoAlg=CompFactory.HLTTest.TestHypoAlg
    return HLTTest__TestHypoAlg( name, Input="" )

@AccumulatorCache
def EFMuonCBViewDataVerifierCfg(flags, name):
    EFMuonCBViewDataVerifier =  CompFactory.AthViews.ViewDataVerifier("VDVEFCBMuon_"+name)
    EFMuonCBViewDataVerifier.DataObjects = [( 'Muon::MdtPrepDataContainer' , 'StoreGateSvc+MDT_DriftCircles' ),  
                                            ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_Measurements' ),
                                            ( 'Muon::RpcPrepDataContainer' , 'StoreGateSvc+RPC_Measurements' ),
                                            ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]

    if flags.Detector.GeometryCSC:
        EFMuonCBViewDataVerifier.DataObjects += [
                                                ( 'Muon::CscStripPrepDataContainer' , 'StoreGateSvc+CSC_Measurements' ),
                                                ( 'Muon::CscPrepDataContainer' , 'StoreGateSvc+CSC_Clusters' )]
    if 'FS' in name:
        EFMuonCBViewDataVerifier.DataObjects += [( 'MuonCandidateCollection' , 'StoreGateSvc+MuonCandidates_FS' )]
    elif flags.Beam.Type is not BeamType.Collisions:
        EFMuonCBViewDataVerifier.DataObjects +=[( 'Muon::HoughDataPerSectorVec' , 'StoreGateSvc+HoughDataPerSectorVec' )]

    else:
        if flags.Detector.GeometryITk:
            EFMuonCBViewDataVerifier.DataObjects += [( 'MuonCandidateCollection' , 'StoreGateSvc+MuonCandidates' ),
                                                    ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+flags.Trigger.ITkTracking.Muon.tracks_FTF )]
        else:
            EFMuonCBViewDataVerifier.DataObjects += [( 'MuonCandidateCollection' , 'StoreGateSvc+MuonCandidates' ),
                                                    ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+flags.Trigger.InDetTracking.Muon.tracks_FTF ),
                                                    ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_FlaggedCondData' )
                                                     ]

    if flags.Input.Format is Format.BS:
        EFMuonCBViewDataVerifier.DataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
                                                 ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' )]
    if flags.Input.isMC:
        if flags.Detector.GeometryITk:
            EFMuonCBViewDataVerifier.DataObjects += [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ), 
                                                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ) ] 
        else:
            EFMuonCBViewDataVerifier.DataObjects += [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ), 
                                                    ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ] 

    result = ComponentAccumulator()
    result.addEventAlgo(EFMuonCBViewDataVerifier)
    return result


def MuonInsideOutViewDataVerifierCfg(flags):
    MuonViewDataVerifier =  CompFactory.AthViews.ViewDataVerifier("VDVMuInsideOut")
    MuonViewDataVerifier.DataObjects = [( 'Muon::RpcPrepDataContainer' , 'StoreGateSvc+RPC_Measurements' ),
                                        ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_Measurements')]
                                     
    if flags.Detector.GeometryCSC:
        MuonViewDataVerifier.DataObjects += [( 'Muon::CscPrepDataContainer' , 'StoreGateSvc+CSC_Clusters' )]
    if flags.Beam.Type is not BeamType.Cosmics:
        MuonViewDataVerifier.DataObjects +=[( 'Muon::HoughDataPerSectorVec' , 'StoreGateSvc+HoughDataPerSectorVec' )]
    if (flags.Detector.GeometrysTGC and flags.Detector.GeometryMM): 
        MuonViewDataVerifier.DataObjects += [( 'Muon::MMPrepDataContainer'       , 'StoreGateSvc+MM_Measurements'),
                                            ( 'Muon::sTgcPrepDataContainer'     , 'StoreGateSvc+STGC_Measurements') ]

    result = ComponentAccumulator()
    result.addEventAlgo(MuonViewDataVerifier)
    return result


def MuCombViewDataVerifier():
    result = ComponentAccumulator()
    alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuComb",
                                                 DataObjects = [( 'xAOD::L2StandAloneMuonContainer' , 'StoreGateSvc+HLT_MuonL2SAInfo' )])
    result.addEventAlgo(alg)
    return result

def MuIsoViewDataVerifierCfg():
    result = ComponentAccumulator()
    alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuIso",
                                                 DataObjects = [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+EFMuIsoRecoRoIs' ),
                                                                ( 'xAOD::MuonContainer' , 'StoreGateSvc+InViewIsoMuons' )])
    result.addEventAlgo(alg)
    return result


def efMuHypoConf(flags, name="UNSPECIFIED", inputMuons="UNSPECIFIED",doSA=False):
    TrigMuonEFHypoAlg = CompFactory.TrigMuonEFHypoAlg
    efHypo = TrigMuonEFHypoAlg(name)
    efHypo.MuonDecisions = inputMuons
    efHypo.IncludeSAmuons = doSA
    return efHypo

def efMuIsoHypoConf(flags, name="UNSPECIFIED", inputMuons="UNSPECIFIED"):
    TrigMuonEFHypoAlg = CompFactory.TrigMuonEFTrackIsolationHypoAlg
    efHypo = TrigMuonEFHypoAlg(name)
    efHypo.EFMuonsName = inputMuons
    return efHypo


@AccumulatorCache
def _muCombStepSeq(flags, is_probe_leg=False):
    ### Set muon step2 - L2muComb ###
    selAccL2CB = SelectionCA("L2MuonCB", is_probe_leg)

    # L2MuonCB reco
    recoL2CB = l2MuCombRecoCfg(flags, is_probe_leg=is_probe_leg)

    #external data loading to view
    recoL2CB.mergeReco( MuCombViewDataVerifier() )

    #ID tracking
    recoL2CB.mergeReco(trigInDetFastTrackingCfg( flags, roisKey=recoL2CB.inputMaker().InViewRoIs, signatureName="Muon" ))

    selAccL2CB.mergeReco(recoL2CB)

    l2muCombHypo = l2MuCombHypoCfg(flags,
                                   name = 'TrigL2MuCombHypo',
                                   muCombInfo = 'HLT_MuonL2CBInfo' )

    selAccL2CB.addHypoAlgo(l2muCombHypo)
    return selAccL2CB
    

def muCombSequence(flags, is_probe_leg=False):
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')    
    muonflagsCBid = getInDetFlagsForSignature(muonflagsCB,'muon')
    selAcc = _muCombStepSeq(muonflagsCBid, is_probe_leg=is_probe_leg)
    return MenuSequenceCA(flags, selAcc, HypoToolGen = TrigmuCombHypoToolFromDict)
    

def muCombStep(flags, chainDict):
    """ can be removed """
    l2muCombSequence = muCombSequence(flags)
    return ChainStep( name=l2muCombSequence.name, Sequences=[l2muCombSequence], chainDicts=[chainDict] )

def muCombOvlpRmSequence(flags, is_probe_leg=False):
    log.warning("FAKE muCombOvlpRmSequence replaced by single muCombSequence")
    return muCombSequence(flags, is_probe_leg)



@AccumulatorCache
def _muEFCBStepSeq(flags, name='RoI', is_probe_leg=False):
    #EF combined muons
    selAccEFCB = SelectionCA("EFCBMuon_"+name, is_probe_leg)

    viewName = 'EFMuCBReco_'+name                                                       
    trackName = flags.Tracking.ActiveConfig.tracks_FTF
    muonCandName = "MuonCandidates"
    if 'FS' in name:
        muonCandName = "MuonCandidates_FS"
        ViewCreatorCentredOnIParticleROITool=CompFactory.ViewCreatorCentredOnIParticleROITool
        roiTool         = ViewCreatorCentredOnIParticleROITool(RoisWriteHandleKey="MuonCandidates_FS_ROIs")
        viewMakerAlg = CompFactory.EventViewCreatorAlgorithm("IM"+viewName,
                                                             ViewFallThrough = True,
                                                             mergeUsingFeature = True,
                                                             PlaceMuonInView = True,
                                                             RequireParentView = True,
                                                             InViewMuons = "InViewMuons",
                                                             InViewMuonCandidates = muonCandName,
                                                             RoITool         = roiTool,
                                                             InViewRoIs      = viewName+'RoIs',
                                                             Views           = viewName+'Views',
                                                             ViewNodeName    = viewName+"InView")
        recoCB = InViewRecoCA("EFMuCBReco_"+name, viewMaker=viewMakerAlg, isProbe=is_probe_leg)
        #ID tracking
        recoCB.mergeReco(trigInDetFastTrackingCfg( flags, roisKey=recoCB.inputMaker().InViewRoIs, signatureName="MuonFS" ))
        trackName = flags.Trigger.InDetTracking.MuonFS.tracks_FTF
    else:
        recoCB = InViewRecoCA(viewName, isProbe=is_probe_leg)
        recoCB.inputMaker().RequireParentView = True

    recoCB.mergeReco(EFMuonCBViewDataVerifierCfg(flags, name))
    
    indetCandCfg = MuonCombinedInDetCandidateAlgCfg(flags, name="TrigMuonCombinedInDetCandidateAlg_"+name, TrackParticleLocation=[trackName], 
                                                 InDetCandidateLocation="InDetCandidates_"+name, DoSiliconAssocForwardMuons=False, InDetForwardTrackSelector="")
    recoCB.mergeReco(indetCandCfg)
    muonCombCfg = MuonCombinedAlgCfg(flags, name="TrigMuonCombinedAlg_"+name, MuonCandidateLocation=muonCandName, 
                                     InDetCandidateLocation="InDetCandidates_"+name)
    recoCB.mergeReco(muonCombCfg)

    muonCreatorCBCfg = MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgCB_"+name, MuonCandidateLocation=[muonCandName], TagMaps=["muidcoTagMap"], 
                                         InDetCandidateLocation="InDetCandidates_"+name, MuonContainerLocation = "MuonsCB_"+name, 
                                         ExtrapolatedLocation = "CBExtrapolatedMuons", MSOnlyExtrapolatedLocation = "CBMSonlyExtrapolatedMuons", CombinedLocation = "HLT_CBCombinedMuon_"+name)
    recoCB.mergeReco(muonCreatorCBCfg)

    #Inside out recovery
    finalMuons = "MuonsCB_"+name
    if 'FS' not in name:
        acc = ComponentAccumulator()
        seqIO = seqOR("muonInsideOutSeq")
        acc.addSequence(seqIO)
        seqFilter = seqAND("muonFilterSeq")
        acc.addSequence(seqFilter, seqIO.name)
        muonFilterCfg = MuonFilterAlgCfg(flags, name="FilterZeroMuons", MuonContainerLocation="MuonsCB_"+name)
        acc.merge(muonFilterCfg, sequenceName=seqFilter.name)
        seqIOreco = parOR("muonInsideOutRecoSeq")
        acc.addSequence(seqIOreco, parentName=seqFilter.name)
        recoCB.mergeReco(MuonInsideOutViewDataVerifierCfg(flags))
        inDetExtensionCfg = MuonInDetToMuonSystemExtensionAlgCfg(flags, name="TrigInDetMuonExtensionAlg_"+name, InputInDetCandidates="InDetCandidates_"+name,
                                                                 WriteInDetCandidates="InDetCandidatesSystemExtended_"+name)
        acc.merge(inDetExtensionCfg, sequenceName=seqIOreco.name)
        muonInsideOutCfg = MuonInsideOutRecoAlgCfg(flags, name="TrigMuonInsideOutRecoAlg", InDetCandidateLocation = "InDetCandidatesSystemExtended_"+name)
        acc.merge(muonInsideOutCfg, sequenceName=seqIOreco.name)
        insideOutCreatorAlgCfg = MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgInsideOut", TagMaps=["muGirlTagMap"], InDetCandidateLocation="InDetCandidates_"+name,
                                                   MuonContainerLocation = "MuonsInsideOut_"+name, ExtrapolatedLocation = "InsideOutCBExtrapolatedMuons",
                                                   MSOnlyExtrapolatedLocation = "InsideOutCBMSOnlyExtrapolatedMuons", CombinedLocation = "InsideOutCBCombinedMuon")
        acc.merge(insideOutCreatorAlgCfg, sequenceName=seqIOreco.name)

        finalMuons = "MuonsCBMerged"
        muonMergeCfg = MergeEFMuonsAlgCfg(flags, name="MergeEFMuons", MuonCBContainerLocation = "MuonsCB_"+name,
                                          MuonInsideOutContainerLocation = "MuonsInsideOut_"+name, MuonOutputLocation = finalMuons)
        acc.merge(muonMergeCfg, sequenceName=seqIO.name)
        recoCB.mergeReco(acc)
    
    selAccEFCB.mergeReco(recoCB)

    efmuCBHypo = efMuHypoConf( flags,
                              name = 'TrigMuonEFCBHypo_'+name,
                              inputMuons = finalMuons )

    selAccEFCB.addHypoAlgo(efmuCBHypo)    
   
    return selAccEFCB

def muEFCBSequence(flags, is_probe_leg=False):
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muonflagsCBid = getInDetFlagsForSignature(muonflagsCB,'muon')
    selAccEFCB = _muEFCBStepSeq(muonflagsCBid, name='RoI', is_probe_leg=is_probe_leg)
    return MenuSequenceCA(flags, selAccEFCB, HypoToolGen = TrigMuonEFCombinerHypoToolFromDict)    

def muEFCBStep(flags, chainDict, name='RoI'):
    """ can be removed """
    efmuCBSequence = muEFCBSequence(flags)    
    return ChainStep( name=efmuCBSequence.name, Sequences=[efmuCBSequence], chainDicts=[chainDict] )

@AccumulatorCache
def _muEFIsoStepSeq(flags):
    #Track isolation
    selAccEFIso = SelectionCA("EFIsoMuon")

    viewName = 'EFMuIsoReco'                                                       
    ViewCreatorCentredOnIParticleROITool=CompFactory.ViewCreatorCentredOnIParticleROITool
    roiTool         = ViewCreatorCentredOnIParticleROITool(RoisWriteHandleKey="MuonIso_ROIs", RoIEtaWidth=0.15, RoIPhiWidth=0.15)
    viewMakerAlg = CompFactory.EventViewCreatorAlgorithm("IM"+viewName,
                                                         ViewFallThrough = True,
                                                         mergeUsingFeature = True,
                                                         PlaceMuonInView = True,
                                                         RequireParentView = True,
                                                         InViewMuons = "InViewIsoMuons",
                                                         InViewMuonCandidates = "IsoMuonCandidates",
                                                         RoITool         = roiTool,
                                                         InViewRoIs      = viewName+'RoIs',
                                                         Views           = viewName+'Views',
                                                         ViewNodeName    = viewName+"InView")
    recoIso = InViewRecoCA("EFMuIsoReco", viewMaker=viewMakerAlg)
    #ID tracking
    recoIso.mergeReco(trigInDetFastTrackingCfg( flags, roisKey=recoIso.inputMaker().InViewRoIs, signatureName="MuonIso" ))
    recoIso.mergeReco(MuIsoViewDataVerifierCfg())
    recoIso.mergeReco(TrigMuonEFTrackIsolationAlgCfg(flags, IdTrackParticles=flags.Trigger.InDetTracking.MuonIso.tracks_FTF,
                                                    MuonEFContainer="InViewIsoMuons", 
                                                    ptcone02Name="InViewIsoMuons.ptcone02", 
                                                    ptcone03Name="InViewIsoMuons.ptcone03"))

    selAccEFIso.mergeReco(recoIso)
    efmuIsoHypo = efMuIsoHypoConf( flags,
                                  name = 'TrigMuonIsoHypo',
                                  inputMuons = "MuonsIso" )
    selAccEFIso.addHypoAlgo(efmuIsoHypo)       
    return selAccEFIso

def muEFIsoSequence(flags, is_probe_leg=False):
    selAccEFIso = _muEFIsoStepSeq(flags)
    return MenuSequenceCA(flags, selAccEFIso, HypoToolGen = TrigMuonEFTrackIsolationHypoToolFromDict)

def muEFIsoStep(flags, chainDict):
    """ can be removed"""
    efmuIsoSequence=muEFIsoSequence(flags)
    return ChainStep( name=efmuIsoSequence.name, Sequences=[efmuIsoSequence], chainDicts=[chainDict] )

