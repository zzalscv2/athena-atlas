/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "EgammaMonitoring.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ITHistSvc.h"

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

EgammaMonitoring::EgammaMonitoring(const std::string &name, ISvcLocator *pSvcLocator) :
  AthAlgorithm(name, pSvcLocator){
    declareProperty("sampleType", m_sampleType = "Unknown",
		    "Descriptive name for the processed type of particle");
}

// ******

StatusCode EgammaMonitoring::initialize() {
  ATH_MSG_INFO("********************  Running over " << m_sampleType << " ******************");
  ATH_CHECK(service("THistSvc", rootHistSvc));

  showerShapesAll = std::make_unique<egammaMonitoring::ShowerShapesHistograms>(
    "showerShapesAll","Shower Shapes ", "/MONITORING/showerShapesAll/", rootHistSvc);

  showerShapes10GeV = std::make_unique<egammaMonitoring::ShowerShapesHistograms>(
    "showerShapes10GeV","Shower Shapes - 10 GeV", "/MONITORING/showerShapes10GeV/", rootHistSvc);

  clusterAll = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clustersAll","Clusters", "/MONITORING/clusterAll/", rootHistSvc);

  cluster10GeV= std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusters10GeV","Clusters - 10 GeV", "/MONITORING/cluster10GeV/", rootHistSvc);

  clusterPromptAll = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clustersPromptAll","Clusters from Prompt", "/MONITORING/clusterPromptAll/", rootHistSvc);

  clusterPrompt10GeV = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clustersPrompt10GeV","Clusters from Prompt - 10 GeV", "/MONITORING/clusterPrompt10GeV/", rootHistSvc);

  isolationAll = std::make_unique<egammaMonitoring::IsolationHistograms>(
    "isolationAll","Isolation ", "/MONITORING/isolationAll/", rootHistSvc);

  ATH_CHECK(showerShapesAll->initializePlots());
  ATH_CHECK(showerShapes10GeV->initializePlots());
  ATH_CHECK(clusterAll->initializePlots());
  ATH_CHECK(cluster10GeV->initializePlots());
  ATH_CHECK(clusterPromptAll->initializePlots());
  ATH_CHECK(clusterPrompt10GeV->initializePlots());
  ATH_CHECK(isolationAll->initializePlots(m_sampleType == "electron"));

  if ("electron" == m_sampleType) {

    truthElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthElectronAll","All Truth Electrons", "/MONITORING/truthElectronAll/", rootHistSvc);

    truthPromptElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthPromptElectronAll","All Truth Prompt Electrons", "/MONITORING/truthPromptElectronAll/", rootHistSvc);

    truthElectronRecoElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthElectronRecoElectronAll","Truth Electrons Reco Electron", "/MONITORING/truthElectronRecoElectronAll/", rootHistSvc);

    truthPromptElectronWithTrack = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthPromptElectronWithTrack","Truth Prompt Electrons With Track", "/MONITORING/truthPromptElectronWithTrack/", rootHistSvc);

    truthPromptElectronWithGSFTrack = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthPromptElectronWithGSFTrack","Truth Prompt Electrons With GSFTrack", "/MONITORING/truthPromptElectronWithGSFTrack/", rootHistSvc);

    truthPromptElectronWithReco = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthPromptElectronWithReco","Truth Prompt Electrons With GSFTrack", "/MONITORING/truthPromptElectronWithReco/", rootHistSvc);

    recoElectronAll = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::RecoElectronHistograms(
      "recoElectronAll","Electrons Reco Electron", "/MONITORING/recoElectronAll/", rootHistSvc));

    truthRecoElectronLooseLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthRecoElectronLooseLH","LLH Electrons Reco Electron", "/MONITORING/truthRecoElectronLooseLH/", rootHistSvc);

    truthRecoElectronMediumLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
        "truthRecoElectronMediumLH","MLH Electrons Reco Electron", "/MONITORING/truthRecoElectronMediumLH/", rootHistSvc);

    truthRecoElectronTightLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthRecoElectronTightLH","TLH Electrons Reco Electron", "/MONITORING/truthRecoElectronTightLH/", rootHistSvc);

    ATH_CHECK(recoElectronAll->initializePlots());
    ATH_CHECK(truthRecoElectronLooseLH->initializePlots());
    ATH_CHECK(truthRecoElectronMediumLH->initializePlots());
    ATH_CHECK(truthRecoElectronTightLH->initializePlots());
    ATH_CHECK(truthElectronAll->initializePlots(true));
    ATH_CHECK(truthPromptElectronAll->initializePlots(true));
    ATH_CHECK(truthElectronRecoElectronAll->initializePlots());
    ATH_CHECK(truthPromptElectronWithTrack->initializePlots(true));
    ATH_CHECK(truthPromptElectronWithGSFTrack->initializePlots(true));
    ATH_CHECK(truthPromptElectronWithReco->initializePlots());
  } // electron Hists

  if ("gamma" == m_sampleType) {

    recoPhotonAll = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::RecoPhotonHistograms(
        "recoPhotonAll","Reco Photon", "/MONITORING/recoPhotonAll/", rootHistSvc));

    clusterConvPhoton = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhoton","Clusters from Converted Photons", "/MONITORING/clusterConvPhoton/", rootHistSvc);

    clusterConvPhotonSi = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhotonSi","Clusters from Converted Photons - Si", "/MONITORING/clusterConvPhotonSi/", rootHistSvc);

    clusterConvPhotonSiSi = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhotonSiSi","Clusters from Converted Photons - SiSi", "/MONITORING/clusterConvPhotonSiSi/", rootHistSvc);
   
    clusterConvPhotonTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhotonTRT","Clusters from Converted Photons - TRT", "/MONITORING/clusterConvPhotonTRT/", rootHistSvc);

    clusterConvPhotonTRTTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhotonTRTTRT","Clusters from Converted Photons - TRTTRT", "/MONITORING/clusterConvPhotonTRTTRT/", rootHistSvc);
 
    clusterConvPhotonSiTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterConvPhotonSiTRT","Clusters from Converted Photons - SiTRT", "/MONITORING/clusterConvPhotonSiTRT/", rootHistSvc);
    
    
    clusterUnconvPhoton = std::make_unique<egammaMonitoring::ClusterHistograms>(
    "clusterUnconvPhoton","Clusters from Converted Photons", "/MONITORING/clusterUnconvPhoton/", rootHistSvc);


    truthPhotonAll = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthPhotonAll","truthPhotonAll", "/MONITORING/truthPhotonAll/", rootHistSvc));

    truthPhotonAllUnconv = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthPhotonAllUnconv","truthPhotonAllUnconv", "/MONITORING/truthPhotonAllUnconv/", rootHistSvc));

    truthPhotonAllConv = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthPhotonAllConv","truthPhotonAllConv", "/MONITORING/truthPhotonAllConv/", rootHistSvc));

    truthPhotonRecoPhoton = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthPhotonRecoPhoton","truthPhotonRecoPhoton", "/MONITORING/truthPhotonRecoPhoton/", rootHistSvc));

    truthPhotonRecoPhotonOrElectron = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthPhotonRecoPhotonOrElectron","truthPhotonRecoPhotonOrElectron", "/MONITORING/truthPhotonRecoPhotonOrElectron/", rootHistSvc));

    truthPhotonConvPhoton = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvPhoton","truthConvPhoton", "/MONITORING/truthConvPhoton/", rootHistSvc));

    truthPhotonConvRecoConv = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv","truthConvRecoConv", "/MONITORING/truthConvRecoConv/", rootHistSvc));

    truthPhotonConvRecoConv1Si = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv1Si","truthConvRecoConv1Si", "/MONITORING/truthConvRecoConv1Si/", rootHistSvc));

    truthPhotonConvRecoConv1TRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv1TRT","truthConvRecoConv1TRT", "/MONITORING/truthConvRecoConv1TRT/", rootHistSvc));

    truthPhotonConvRecoConv2Si = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv2Si","truthConvRecoConv2Si", "/MONITORING/truthConvRecoConv2Si/", rootHistSvc));

    truthPhotonConvRecoConv2TRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv2TRT","truthConvRecoConv2TRT", "/MONITORING/truthConvRecoConv2TRT/", rootHistSvc));

    truthPhotonConvRecoConv2SiTRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoConv2SiTRT","truthConvRecoConv2SiTRT", "/MONITORING/truthConvRecoConv2SiTRT/", rootHistSvc));

    truthPhotonConvRecoUnconv= std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthConvRecoUnconv","truthConvRecoUnconv", "/MONITORING/truthConvRecoUnconv/", rootHistSvc));

    truthPhotonUnconvPhoton= std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvPhoton","truthUnconvPhoton", "/MONITORING/truthUnconvPhoton/", rootHistSvc));

    truthPhotonUnconvRecoConv= std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv","truthUnconvRecoConv", "/MONITORING/truthUnconvRecoConv/", rootHistSvc));

    truthPhotonUnconvRecoConv1Si = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv1Si","truthUnconvRecoConv1Si", "/MONITORING/truthUnconvRecoConv1Si/", rootHistSvc));

    truthPhotonUnconvRecoConv1TRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv1TRT","truthUnconvRecoConv1TRT", "/MONITORING/truthUnconvRecoConv1TRT/", rootHistSvc));

    truthPhotonUnconvRecoConv2Si = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv2Si","truthUnconvRecoConv2Si", "/MONITORING/truthUnconvRecoConv2Si/", rootHistSvc));

    truthPhotonUnconvRecoConv2TRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv2TRT","truthUnconvRecoConv2TRT", "/MONITORING/truthUnconvRecoConv2TRT/", rootHistSvc));

    truthPhotonUnconvRecoConv2SiTRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoConv2SiTRT","truthUnconvRecoConv2SiTRT", "/MONITORING/truthUnconvRecoConv2SiTRT/", rootHistSvc));

    truthPhotonUnconvRecoUnconv = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
        "truthUnconvRecoUnconv","truthUnconvRecoUnconv", "/MONITORING/truthUnconvRecoUnconv/", rootHistSvc));

    recoPhotonUnconvLooseLH = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
      "recoPhotonUnconvLooseLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonUnconvLooseLH/", rootHistSvc));

    recoPhotonUnconvTightLH = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
      "recoPhotonUnconvTightLH","TLH Photons Reco Photon", "/MONITORING/recoPhotonUnconvTightLH/", rootHistSvc));

    recoPhotonConvLooseLH = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
      "recoPhotonConvLooseLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonConvLooseLH/", rootHistSvc));

    recoPhotonConvTightLH = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
      "recoPhotonConvTightLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonConvTightLH/", rootHistSvc));

    recoPhotonUnconvIsoFixedCutTight = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonUnconvIsoFixedCutTight","Isolation Fixed Cut Tight Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutTight/", rootHistSvc));

    recoPhotonUnconvIsoFixedCutTightCaloOnly = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonUnconvIsoFixedCutTightCaloOnly","Isolation Fixed Cut Tight Calo Only Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutTightCaloOnly/", rootHistSvc));

    recoPhotonUnconvIsoFixedCutLoose = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonUnconvIsoFixedCutLoose","Isolation Fixed Cut Loose Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutLoose/", rootHistSvc));

    recoPhotonConvIsoFixedCutTight = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonConvIsoFixedCutTight","Isolation Fixed Cut Tight Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutTight/", rootHistSvc));

    recoPhotonConvIsoFixedCutTightCaloOnly = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonConvIsoFixedCutTightCaloOnly","Isolation Fixed Cut Tight Calo Only Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutTightCaloOnly/", rootHistSvc));

    recoPhotonConvIsoFixedCutLoose = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TruthPhotonHistograms(
    "recoPhotonConvIsoFixedCutLoose","Isolation Fixed Cut Loose Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutLoose/", rootHistSvc));

    InDetTracks = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracks","InDet Tracks", "/MONITORING/InDetTracks/", rootHistSvc));

    InDetTracksMatchElectron = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksMatchElectron","InDet Tracks matched to electrons", "/MONITORING/InDetTracksMatchElectron/", rootHistSvc));

    InDetTracksNotElectron = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksNotElectron","InDet Tracks not matched to electrons", "/MONITORING/InDetTracksNotElectron/", rootHistSvc));

    InDetTracksMatchPion = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksMatchPion","InDet Tracks matched to pions", "/MONITORING/InDetTracksMatchPion/", rootHistSvc));

    InDetTracksNotMatched = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksNotMatched","InDet Tracks not matched", "/MONITORING/InDetTracksNotMatched/", rootHistSvc));

    InDetTracksTRT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRT","InDet Tracks TRTSA", "/MONITORING/InDetTracksTRT/", rootHistSvc));

    InDetTracksTRTMatchElectron = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTMatchElectron","InDet Tracks TRTSA matched to electrons", "/MONITORING/InDetTracksTRTMatchElectron/", rootHistSvc));

    InDetTracksTRTNotElectron = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTNotElectron","InDet Tracks TRTSA not matched to electrons", "/MONITORING/InDetTracksTRTNotElectron/", rootHistSvc));

    InDetTracksTRTMatchPion = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTMatchPion","InDet Tracks TRTSA matched to pions", "/MONITORING/InDetTracksTRTMatchPion/", rootHistSvc));

    InDetTracksTRTNotMatched = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTNotMatched","InDet Tracks TRTSA not matched", "/MONITORING/InDetTracksTRTNotMatched/", rootHistSvc));

    InDetTrackshighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTrackshighpT","InDet Tracks pT > 3 GeV", "/MONITORING/InDetTrackshighpT/", rootHistSvc));

    InDetTracksMatchElectronhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksMatchElectronhighpT","InDet Tracks matched to electrons pT > 3 GeV", "/MONITORING/InDetTracksMatchElectronhighpT/", rootHistSvc));

    InDetTracksNotElectronhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksNotElectronhighpT","InDet Tracks not matched to electrons pT > 3 GeV", "/MONITORING/InDetTracksNotElectronhighpT/", rootHistSvc));

    InDetTracksMatchPionhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksMatchPionhighpT","InDet Tracks matched to pions pT > 3 GeV", "/MONITORING/InDetTracksMatchPionhighpT/", rootHistSvc));

    InDetTracksNotMatchedhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksNotMatchedhighpT","InDet Tracks not matched pT > 3 GeV", "/MONITORING/InDetTracksNotMatchedhighpT/", rootHistSvc));

    InDetTracksTRThighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRThighpT","InDet Tracks TRTSA pT > 3 GeV", "/MONITORING/InDetTracksTRThighpT/", rootHistSvc));

    InDetTracksTRTMatchElectronhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTMatchElectronhighpT","InDet Tracks TRTSA matched to electrons pT > 3 GeV", "/MONITORING/InDetTracksTRTMatchElectronhighpT/", rootHistSvc));

    InDetTracksTRTNotElectronhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTNotElectronhighpT","InDet Tracks TRTSA not matched to electrons pT > 3 GeV", "/MONITORING/InDetTracksTRTNotElectronhighpT/", rootHistSvc));

    InDetTracksTRTMatchPionhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTMatchPionhighpT","InDet Tracks TRTSA matched to pions pT > 3 GeV", "/MONITORING/InDetTracksTRTMatchPionhighpT/", rootHistSvc));

    InDetTracksTRTNotMatchedhighpT = std::unique_ptr<egammaMonitoring::IHistograms>(new egammaMonitoring::TrackHistograms(
    "InDetTracksTRTNotMatchedhighpT","InDet Tracks TRTSA not matched pT > 3 GeV", "/MONITORING/InDetTracksTRTNotMatchedhighpT/", rootHistSvc));

    ATH_CHECK(recoPhotonAll->initializePlots());
    ATH_CHECK(truthPhotonAll->initializePlots());
    ATH_CHECK(truthPhotonAllUnconv->initializePlots());
    ATH_CHECK(truthPhotonAllConv->initializePlots());
    ATH_CHECK(truthPhotonRecoPhoton->initializePlots());
    ATH_CHECK(truthPhotonRecoPhotonOrElectron->initializePlots());
    ATH_CHECK(truthPhotonConvPhoton->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv1Si->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv1TRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2Si->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2TRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2SiTRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoUnconv->initializePlots());
    ATH_CHECK(truthPhotonUnconvPhoton->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv1Si->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv1TRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2Si->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2TRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2SiTRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoUnconv->initializePlots());
    ATH_CHECK(recoPhotonUnconvLooseLH->initializePlots());
    ATH_CHECK(recoPhotonUnconvTightLH->initializePlots());
    ATH_CHECK(recoPhotonConvLooseLH->initializePlots());
    ATH_CHECK(recoPhotonConvTightLH->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTight->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightCaloOnly->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutLoose->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutTight->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutTightCaloOnly->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutLoose->initializePlots());

    ATH_CHECK(clusterConvPhoton->initializePlots());
    ATH_CHECK(clusterConvPhotonSi->initializePlots());
    ATH_CHECK(clusterConvPhotonSiSi->initializePlots());
    ATH_CHECK(clusterConvPhotonTRT->initializePlots());
    ATH_CHECK(clusterConvPhotonTRTTRT->initializePlots());
    ATH_CHECK(clusterConvPhotonSiTRT->initializePlots());
    ATH_CHECK(clusterUnconvPhoton->initializePlots());

    ATH_CHECK(InDetTracks->initializePlots());
    ATH_CHECK(InDetTracksMatchElectron->initializePlots());
    ATH_CHECK(InDetTracksNotElectron->initializePlots());
    ATH_CHECK(InDetTracksMatchPion->initializePlots());
    ATH_CHECK(InDetTracksNotMatched->initializePlots());
    ATH_CHECK(InDetTracksTRT->initializePlots());
    ATH_CHECK(InDetTracksTRTMatchElectron->initializePlots());
    ATH_CHECK(InDetTracksTRTNotElectron->initializePlots());
    ATH_CHECK(InDetTracksTRTMatchPion->initializePlots());
    ATH_CHECK(InDetTracksTRTNotMatched->initializePlots());
    ATH_CHECK(InDetTrackshighpT->initializePlots());
    ATH_CHECK(InDetTracksMatchElectronhighpT->initializePlots());
    ATH_CHECK(InDetTracksNotElectronhighpT->initializePlots());
    ATH_CHECK(InDetTracksMatchPionhighpT->initializePlots());
    ATH_CHECK(InDetTracksNotMatchedhighpT->initializePlots());
    ATH_CHECK(InDetTracksTRThighpT->initializePlots());
    ATH_CHECK(InDetTracksTRTMatchElectronhighpT->initializePlots());
    ATH_CHECK(InDetTracksTRTNotElectronhighpT->initializePlots());
    ATH_CHECK(InDetTracksTRTMatchPionhighpT->initializePlots());
    ATH_CHECK(InDetTracksTRTNotMatchedhighpT->initializePlots());

  } // gamma Hists

  if ("electron" == m_sampleType) {
    //*****************ID selectors (3 levels)********************
    ATH_CHECK(m_LooseLH.retrieve());
    ATH_CHECK(m_MediumLH.retrieve());
    ATH_CHECK(m_TightLH.retrieve());

    ATH_CHECK(m_ElectronsKey.initialize());
    ATH_CHECK(m_FwdElectronsKey.initialize(!m_FwdElectronsKey.empty()));
    ATH_CHECK(m_GSFTrackParticlesKey.initialize());
  }

  if ("gamma" == m_sampleType) {
    //*****************ID selectors (2 levels)********************
    ATH_CHECK(m_Loose_Photon.retrieve());
    ATH_CHECK(m_Tight_Photon.retrieve());

    //*****************Iso Requirements********************
    ATH_CHECK(m_IsoFixedCutTight.retrieve());
    ATH_CHECK(m_IsoFixedCutTightCaloOnly.retrieve());
    ATH_CHECK(m_IsoFixedCutLoose.retrieve());

    ATH_CHECK(m_PhotonsKey.initialize());
  }

  //*****************MC Truth Classifier Requirement********************
  ATH_CHECK(m_mcTruthClassifier.retrieve());

  //***************** The handles used whatever the sample********************
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_egTruthParticlesKey.initialize());
  ATH_CHECK(m_truthParticlesKey.initialize());
  ATH_CHECK(m_InDetTrackParticlesKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode EgammaMonitoring::execute() {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  // Retrieve things from the event store
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
  const float mu = eventInfo->averageInteractionsPerCrossing();

  // Retrieve egamma truth particles
  SG::ReadHandle<xAOD::TruthParticleContainer> egTruthParticles (m_egTruthParticlesKey, ctx);

  // Retrieve truth particles
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_truthParticlesKey, ctx);

  // Retrieve indet track particles
  SG::ReadHandle<xAOD::TrackParticleContainer > InDetTPs(m_InDetTrackParticlesKey, ctx);

  if ("electron" == m_sampleType) {

    SG::ReadHandle<xAOD::ElectronContainer > RecoEl(m_ElectronsKey, ctx);
    SG::ReadHandle<xAOD::TrackParticleContainer > GSFTracks(m_GSFTrackParticlesKey, ctx);

    ATH_MSG_DEBUG( "------------ Truth Egamma Container ---------------" );
    for (const auto *egtruth : *egTruthParticles) {

      if (!egtruth) continue;

      const xAOD::Electron *electron = xAOD::EgammaHelpers::getRecoElectron(egtruth);

      if (!electron) continue;

      clusterPromptAll->fill(*electron,mu);
      if (egtruth->pt() > 10*Gaudi::Units::GeV) {
        clusterPrompt10GeV->fill(*electron,mu);
      }
    }

    ATH_MSG_DEBUG( "------------ Truth Particles Container ---------------" );
    unsigned int promptElectronTruthIndex = - 9;
    for (const auto *truth : *truthParticles) {

      if (!truth) continue;
      if (std::abs(truth->pdgId()) != 11) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;

      ATH_MSG_DEBUG( " ******** Truth particle associated to Electron Found: "
                << " STATUS  " << truth->status()
                << " type  " << truth->type()
                << " barcode  " << truth->barcode()
                << " PDG id   " << truth->pdgId()
                << " index    " << truth->index()
                << " TO  " << TO
                << " TT   " << TT
                << " eventNumber  " << eventInfo->eventNumber() );

      // Check if it is the prompt electron
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron && truth->barcode() == 10001) {
        truthPromptElectronAll->fill(truth);
        promptElectronTruthIndex = truth->index();

      }

      // Check that it is not from geant4
      if (TT != MCTruthPartClassifier::NonPrimary) truthElectronAll->fill(truth);

    }

    ATH_MSG_DEBUG( "------------ InDetTracks ---------------" );

    for (const auto *tp : *InDetTPs) {

      if (!tp) continue;

      uint8_t nPi = 0;
      uint8_t nSCT = 0;

      tp->summaryValue(nPi, xAOD::numberOfPixelHits);
      tp->summaryValue(nSCT, xAOD::numberOfSCTHits);

      if ((nPi + nSCT) < 7) continue;

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*tp);

      if (!truth || std::abs(truth->pdgId()) != 11) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {

        // we found the track from the prompt electron
        // let's count it
        truthPromptElectronWithTrack->fill(truth);
        break;

      } else {
        const xAOD::TruthParticle *firstElTruth = xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that

        if (firstElTruth->index() == promptElectronTruthIndex) {
	  truthPromptElectronWithTrack->fill(firstElTruth);
	  break;
        }

      }

    }

    ATH_MSG_DEBUG( "------------ GSFTracks ---------------" );
    for (const auto *gsf : *GSFTracks) {

      if (!gsf) continue;

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*gsf);

      if (!truth) continue;

      uint8_t nPi = 0;
      uint8_t nSCT = 0;

      gsf->summaryValue(nPi, xAOD::numberOfPixelHits);
      gsf->summaryValue(nSCT, xAOD::numberOfSCTHits);

      if ((nPi + nSCT) < 7) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (msgLvl(MSG::DEBUG)) {
	auto res2 = m_mcTruthClassifier->checkOrigOfBkgElec(truth);
	MCTruthPartClassifier::ParticleOrigin bkgTO = res2.second;
	MCTruthPartClassifier::ParticleType bkgTT = res2.first;
	ATH_MSG_DEBUG(" ** Truth particle associated to track Found: "
		      << " STATUS  " << truth->status()
		      << " type  " << truth->type()
		      << " barcode  " << truth->barcode()
		      << " PDG id   " << truth->pdgId()
		      << " index    " << truth->index()
		      << " bkg T0  " << bkgTO
		      << " bkg TT   " << bkgTT
		      << " T0  " << TO
		      << " TT   " << TT
		      << " eventNumber  " << eventInfo->eventNumber() );
      }
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {

        // we found the track from the prompt electron
        // let's count it
        truthPromptElectronWithGSFTrack->fill(truth);
        break;

      } else {
        const xAOD::TruthParticle *firstElTruth = xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        ATH_MSG_DEBUG( "********----- Getting Mother-----********" );
        ATH_MSG_DEBUG( " STATUS  " << firstElTruth->status()
                << " type  " << firstElTruth->type()
                << " barcode  " << firstElTruth->barcode()
                << " PDG id   " << firstElTruth->pdgId()
                << " index    " << firstElTruth->index() );
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that
        if (firstElTruth->index() == promptElectronTruthIndex) {
	  truthPromptElectronWithGSFTrack->fill(firstElTruth);
	  break;
        }

      }

    }

    ATH_MSG_DEBUG( "------------ Reco central electrons ---------------" );
    bool foundPromptElectron = false;

    for (const auto *elrec : *RecoEl) {

      if (!elrec) continue;

      bool toFill = false;

      clusterAll->fill(*elrec,mu);
      recoElectronAll->fill(*elrec);
      showerShapesAll->fill(*elrec);
      isolationAll->fill(*elrec);
      if (elrec->pt() > 10*Gaudi::Units::GeV) {
	cluster10GeV->fill(*elrec,mu);
        showerShapes10GeV->fill(*elrec);
      }

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*elrec);
      if (!truth ) continue;
      truthElectronRecoElectronAll->fill(truth, elrec);
      if (std::abs(truth->pdgId()) != 11 || foundPromptElectron)
	continue;
      const xAOD::TruthParticle *elTruth(nullptr);
      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {
	toFill = true;
	elTruth = truth;
      } else {
        const xAOD::TruthParticle *firstElTruth =
	  xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that
        if (firstElTruth->index() == promptElectronTruthIndex) {
	  toFill = true;
	  elTruth = firstElTruth;
	}
      }
      if (toFill) {
	foundPromptElectron = true;
	truthPromptElectronWithReco->fill(elTruth,elrec);
	if (m_LooseLH->accept(elrec))
	  truthRecoElectronLooseLH->fill(elTruth,elrec);
	if (m_MediumLH->accept(elrec))
	  truthRecoElectronMediumLH->fill(elTruth,elrec);
	if (m_TightLH->accept(elrec))
	  truthRecoElectronTightLH->fill(elTruth,elrec);
      }

    } // RecoEl Loop

    if (!m_FwdElectronsKey.empty()) {
      SG::ReadHandle<xAOD::ElectronContainer > RecoFwdEl(m_FwdElectronsKey, ctx);
      for (const auto *el : *RecoFwdEl) {
	// This would be very weird ??
	if (!el)
	  continue;

	bool toFill = false;

	const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*el);
	if (!truth ) continue;
	//truthElectronRecoFwdElectronAll->fill(truth, elrec); // to be done
	if (std::abs(truth->pdgId()) != 11)
	  continue;
	const xAOD::TruthParticle *elTruth(nullptr);
	auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
	if (res.second == MCTruthPartClassifier::SingleElec &&
	    res.first == MCTruthPartClassifier::IsoElectron &&
	    truth->index() == promptElectronTruthIndex) {
	  toFill = true;
	  elTruth = truth;
	} else {
	  const xAOD::TruthParticle *firstElTruth =
	    xAOD::EgammaHelpers::getBkgElectronMother(truth);
	  if (!firstElTruth) continue;
	  if (firstElTruth->index() == promptElectronTruthIndex) {
	    toFill = true;
	    elTruth = firstElTruth;
	  }
	}
	if (toFill) {
	  if (foundPromptElectron) {
	    ATH_MSG_WARNING("A fwd electron also reconstructed as central "
			    "true eta = " << elTruth->eta() << " event = "
			    << eventInfo->eventNumber());
	  } else {
	    foundPromptElectron = true;
	    truthPromptElectronWithReco->fill(elTruth,el);
	  }
	}
      } // loop on fwdEl
    }

  } // if electron

  if ("gamma" == m_sampleType) {

    ATH_MSG_DEBUG( "------------ Photons ---------------" );
    SG::ReadHandle<xAOD::PhotonContainer > RecoPh(m_PhotonsKey, ctx);

    for (const auto *phrec : *RecoPh) {

      if (!phrec) continue;

      recoPhotonAll->fill(*phrec);
      isolationAll->fill(*phrec);
      showerShapesAll->fill(*phrec);
      clusterAll->fill(*phrec,mu);
      if (phrec->pt() > 10*Gaudi::Units::GeV) {
        cluster10GeV->fill(*phrec,mu);
      }
      if (phrec->pt() > 10*Gaudi::Units::GeV){ 
        showerShapes10GeV->fill(*phrec);
      }

    } // RecoPh Loop

    for (const auto *egtruth : *egTruthParticles) {

      if (!egtruth) continue;

      truthPhotonAll->fill(*egtruth, mu);

      bool isTrueConv = xAOD::EgammaHelpers::isTrueConvertedPhoton(egtruth);
      bool isTrueLateConv = xAOD::EgammaHelpers::isTrueConvertedPhoton(egtruth, 1200) and !isTrueConv;
      const xAOD::Photon *photon = xAOD::EgammaHelpers::getRecoPhoton(egtruth);
      const xAOD::Electron *electron = xAOD::EgammaHelpers::getRecoElectron(egtruth);

      if(isTrueConv) truthPhotonAllConv->fill(*egtruth, mu);
      if(!isTrueConv && !isTrueLateConv) truthPhotonAllUnconv->fill(*egtruth, mu);

      if(photon || electron)
        truthPhotonRecoPhotonOrElectron->fill(*egtruth, mu);

      if (!photon) continue;

      truthPhotonRecoPhoton->fill(*egtruth, mu);
      clusterPromptAll->fill(*photon,mu);
      if (egtruth->pt() > 10*Gaudi::Units::GeV) {
        clusterPrompt10GeV->fill(*photon,mu);
      }

      bool isRecoConv = xAOD::EgammaHelpers::isConvertedPhoton(photon);
      xAOD::EgammaParameters::ConversionType convType = xAOD::EgammaHelpers::conversionType(photon);

      if (isTrueConv) {

        truthPhotonConvPhoton->fill(*egtruth, mu);

        if (isRecoConv) {

          truthPhotonConvRecoConv->fill(*egtruth, mu);

          clusterConvPhoton->fill(*photon,mu);

          if (convType == xAOD::EgammaParameters::singleSi) {
            truthPhotonConvRecoConv1Si->fill(*egtruth, mu);
            clusterConvPhotonSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::singleTRT) {
            truthPhotonConvRecoConv1TRT->fill(*egtruth, mu);
            clusterConvPhotonTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSi) {
            truthPhotonConvRecoConv2Si->fill(*egtruth, mu);
            clusterConvPhotonSiSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleTRT) {
            truthPhotonConvRecoConv2TRT->fill(*egtruth, mu);
            clusterConvPhotonTRTTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSiTRT) {
            truthPhotonConvRecoConv2SiTRT->fill(*egtruth, mu);
            clusterConvPhotonSiTRT->fill(*photon,mu); 
          }

          if (m_IsoFixedCutTight->accept(*photon)) recoPhotonConvIsoFixedCutTight->fill(*egtruth, mu);
          if (m_IsoFixedCutTightCaloOnly->accept(*photon)) recoPhotonConvIsoFixedCutTightCaloOnly->fill(*egtruth, mu);
          if (m_IsoFixedCutLoose->accept(*photon)) recoPhotonConvIsoFixedCutLoose->fill(*egtruth, mu);
          if (m_Loose_Photon->accept(photon)) recoPhotonConvLooseLH->fill(*egtruth, mu);
          if (m_Tight_Photon->accept(photon)) recoPhotonConvTightLH->fill(*egtruth, mu);
        } // isRecoConv
        else {
          truthPhotonConvRecoUnconv->fill(*egtruth, mu);
          clusterUnconvPhoton->fill(*photon,mu); 
        } 

      } //isTrueConv
      else if (!isTrueLateConv) {

        truthPhotonUnconvPhoton->fill(*egtruth, mu);

        if (isRecoConv) {
          truthPhotonUnconvRecoConv->fill(*egtruth, mu);

          if (convType == xAOD::EgammaParameters::singleSi) {
            truthPhotonUnconvRecoConv1Si->fill(*egtruth, mu);
            clusterConvPhotonSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::singleTRT) {
            truthPhotonUnconvRecoConv1TRT->fill(*egtruth, mu);
            clusterConvPhotonTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSi) {
            truthPhotonUnconvRecoConv2Si->fill(*egtruth, mu);
            clusterConvPhotonSiSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleTRT) {
            truthPhotonUnconvRecoConv2TRT->fill(*egtruth, mu);
            clusterConvPhotonTRTTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSiTRT) {
            truthPhotonUnconvRecoConv2SiTRT->fill(*egtruth, mu);
            clusterConvPhotonSiTRT->fill(*photon,mu); 
          }
        }
        else           truthPhotonUnconvRecoUnconv->fill(*egtruth, mu);

        if (m_IsoFixedCutTight->accept(*photon)) recoPhotonUnconvIsoFixedCutTight->fill(*egtruth, mu);
        if (m_IsoFixedCutTightCaloOnly->accept(*photon)) recoPhotonUnconvIsoFixedCutTightCaloOnly->fill(*egtruth, mu);
        if (m_IsoFixedCutLoose->accept(*photon)) recoPhotonUnconvIsoFixedCutLoose->fill(*egtruth, mu);
        if (m_Loose_Photon->accept(photon)) recoPhotonUnconvLooseLH->fill(*egtruth, mu);
        if (m_Tight_Photon->accept(photon)) recoPhotonUnconvTightLH->fill(*egtruth, mu);
      } // !isTrueLateConv
    } //egtruth Loop

    //loop over InDetTrackParticles
    for (const auto *tp : *InDetTPs) {

      InDetTracks->fill(*tp, mu);
      if (matchedToElectron(*tp)) InDetTracksMatchElectron->fill(*tp, mu);
      else {
	InDetTracksNotElectron->fill(*tp, mu);
	if (matchedToPion(*tp)) InDetTracksMatchPion->fill(*tp, mu);
	else if (notMatchedToTruth(*tp)) InDetTracksNotMatched->fill(*tp, mu);
      }
      if (tp->pt() > 3000.){
        InDetTrackshighpT->fill(*tp, mu);
        if (matchedToElectron(*tp)) {
          InDetTracksMatchElectronhighpT->fill(*tp, mu);
        } else {
          InDetTracksNotElectronhighpT->fill(*tp, mu);
	  if (matchedToPion(*tp)){
	    InDetTracksMatchPionhighpT->fill(*tp, mu);
	  }
	  else if (notMatchedToTruth(*tp)){
	    InDetTracksNotMatchedhighpT->fill(*tp, mu);
	  }
	}
      }

      if (xAOD::EgammaHelpers::numberOfSiHits(tp)==0) { //TRTSA tracks
	InDetTracksTRT->fill(*tp, mu);
	if (matchedToElectron(*tp)) InDetTracksTRTMatchElectron->fill(*tp, mu);
	else {
	  InDetTracksTRTNotElectron->fill(*tp, mu);
	  if (matchedToPion(*tp)) InDetTracksTRTMatchPion->fill(*tp, mu);
	  else if (notMatchedToTruth(*tp)) InDetTracksTRTNotMatched->fill(*tp, mu);
	}
	if (tp->pt() > 3000.){
	  InDetTracksTRThighpT->fill(*tp, mu);
	  if (matchedToElectron(*tp)) InDetTracksTRTMatchElectronhighpT->fill(*tp, mu);
	  else {
	    InDetTracksTRTNotElectronhighpT->fill(*tp, mu);
	    if (matchedToPion(*tp)) InDetTracksTRTMatchPionhighpT->fill(*tp, mu);
	    else if (notMatchedToTruth(*tp)) InDetTracksTRTNotMatchedhighpT->fill(*tp, mu);
	  }
	}
      }

    }//loop over InDetTPs

  } // if gamma

  return StatusCode::SUCCESS;
}

// ******

StatusCode EgammaMonitoring::finalize() {

  if ("electron" == m_sampleType) {

    egammaMonitoring::EfficiencyPlot trackEfficiency("trackingEfficiency", "/MONITORING/trackingEfficiency/", rootHistSvc );
    ATH_CHECK(trackEfficiency.divide(truthPromptElectronWithTrack.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot GSFEfficiency("GSFEfficiency", "/MONITORING/GSFEfficiency/", rootHistSvc );
    ATH_CHECK(GSFEfficiency.divide(truthPromptElectronWithGSFTrack.get(),truthPromptElectronWithTrack.get()));
    egammaMonitoring::EfficiencyPlot matchingEfficiency("matchingEfficiency", "/MONITORING/matchingEfficiency/", rootHistSvc );
    ATH_CHECK(matchingEfficiency.divide(truthPromptElectronWithReco.get(), truthPromptElectronWithGSFTrack.get()));
    egammaMonitoring::EfficiencyPlot reconstructionEfficiency("reconstructionEfficiency", "/MONITORING/reconstructionEfficiency/", rootHistSvc );
    ATH_CHECK(reconstructionEfficiency.divide(truthPromptElectronWithReco.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronLooseLHEfficiency("recoElectronLooseLHEfficiency", "/MONITORING/recoElectronLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronLooseLHEfficiency.divide(truthRecoElectronLooseLH.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronMediumLHEfficiency("recoElectronMediumLHEfficiency", "/MONITORING/recoElectronMediumLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronMediumLHEfficiency.divide(truthRecoElectronMediumLH.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronTightLHEfficiency("recoElectronTightLHEfficiency", "/MONITORING/recoElectronTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronTightLHEfficiency.divide( truthRecoElectronTightLH.get(), truthPromptElectronAll.get()));

  }

  if ("gamma" == m_sampleType) {

    egammaMonitoring::EfficiencyPlot truthPhotonRecoPhotonEfficiency("truthPhotonRecoPhotonEfficiency", "/MONITORING/truthPhotonRecoPhotonEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoPhotonEfficiency.divide(truthPhotonRecoPhoton.get(),truthPhotonAll.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoPhotonOrElectronEfficiency("truthPhotonRecoPhotonOrElectronEfficiency", "/MONITORING/truthPhotonRecoPhotonOrElectronEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoPhotonOrElectronEfficiency.divide(truthPhotonRecoPhotonOrElectron.get(),truthPhotonAll.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoEfficiency("truthPhotonConvRecoEfficiency", "/MONITORING/truthPhotonConvRecoEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoEfficiency.divide(truthPhotonConvPhoton.get(),truthPhotonAllConv.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoEfficiency("truthPhotonUnconvRecoEfficiency", "/MONITORING/truthPhotonUnconvRecoEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoEfficiency.divide(truthPhotonUnconvPhoton.get(),truthPhotonAllUnconv.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConvEfficiency("truthPhotonConvRecoConvEfficiency", "/MONITORING/truthPhotonConvRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConvEfficiency.divide(truthPhotonConvRecoConv.get(),truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv1SiEfficiency("truthPhotonConvRecoConv1SiEfficiency", "/MONITORING/truthPhotonConvRecoConv1SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv1SiEfficiency.divide( truthPhotonConvRecoConv1Si.get()   , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv1TRTEfficiency("truthPhotonConvRecoConv1TRTEfficiency", "/MONITORING/truthPhotonConvRecoConv1TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv1TRTEfficiency.divide(truthPhotonConvRecoConv1TRT.get()  , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2SiEfficiency("truthPhotonConvRecoConv2SiEfficiency", "/MONITORING/truthPhotonConvRecoConv2SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2SiEfficiency.divide(truthPhotonConvRecoConv2Si.get()   , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2TRTEfficiency("truthPhotonConvRecoConv2TRTEfficiency", "/MONITORING/truthPhotonConvRecoConv2TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2TRTEfficiency.divide(truthPhotonConvRecoConv2TRT.get()  , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2SiTRTEfficiency("truthPhotonConvRecoConv2SiTRTEfficiency", "/MONITORING/truthPhotonConvRecoConv2SiTRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2SiTRTEfficiency.divide(truthPhotonConvRecoConv2SiTRT.get(), truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoUnconvEfficiency("truthPhotonConvRecoUnconvEfficiency", "/MONITORING/truthPhotonConvRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoUnconvEfficiency.divide(truthPhotonConvRecoUnconv.get(), truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConvEfficiency("truthPhotonUnconvRecoConvEfficiency", "/MONITORING/truthPhotonUnconvRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConvEfficiency.divide(truthPhotonUnconvRecoConv.get(),truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv1SiEfficiency("truthPhotonUnconvRecoConv1SiEfficiency", "/MONITORING/truthPhotonUnconvRecoConv1SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv1SiEfficiency.divide( truthPhotonUnconvRecoConv1Si.get()   , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv1TRTEfficiency("truthPhotonUnconvRecoConv1TRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv1TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv1TRTEfficiency.divide(truthPhotonUnconvRecoConv1TRT.get()  , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2SiEfficiency("truthPhotonUnconvRecoConv2SiEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2SiEfficiency.divide(truthPhotonUnconvRecoConv2Si.get()   , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2TRTEfficiency("truthPhotonUnconvRecoConv2TRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2TRTEfficiency.divide(truthPhotonUnconvRecoConv2TRT.get()  , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2SiTRTEfficiency("truthPhotonUnconvRecoConv2SiTRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2SiTRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2SiTRTEfficiency.divide(truthPhotonUnconvRecoConv2SiTRT.get(), truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoUnconvEfficiency("truthPhotonUnconvRecoUnconvEfficiency", "/MONITORING/truthPhotonUnconvRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoUnconvEfficiency.divide(truthPhotonUnconvRecoUnconv.get(), truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoConvEfficiency("truthPhotonRecoConvEfficiency", "/MONITORING/truthPhotonRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoConvEfficiency.divide(truthPhotonConvRecoConv.get(), truthPhotonRecoPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoUnconvEfficiency("truthPhotonRecoUnconvEfficiency", "/MONITORING/truthPhotonRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoUnconvEfficiency.divide(truthPhotonUnconvRecoUnconv.get(), truthPhotonRecoPhoton.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutTightEfficiency("recoPhotonUnconvIsoFixedCutTightEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutTightEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightEfficiency.divide( recoPhotonUnconvIsoFixedCutTight.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency("recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency.divide( recoPhotonUnconvIsoFixedCutTightCaloOnly.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutLooseEfficiency("recoPhotonUnconvIsoFixedCutLooseEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutLooseEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutLooseEfficiency.divide( recoPhotonUnconvIsoFixedCutLoose.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutTightEfficiency("recoPhotonConvIsoFixedCutTightEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutTightEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutTightEfficiency.divide( recoPhotonConvIsoFixedCutTight.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency("recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency.divide( recoPhotonConvIsoFixedCutTightCaloOnly.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutLooseEfficiency("recoPhotonConvIsoFixedCutLooseEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutLooseEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutLooseEfficiency.divide( recoPhotonConvIsoFixedCutLoose.get(), truthPhotonConvRecoConv.get()));
  
    egammaMonitoring::EfficiencyPlot recoPhotonConvLooseLHEfficiency("recoPhotonConvLooseLHEfficiency", "/MONITORING/recoPhotonConvLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvLooseLHEfficiency.divide( recoPhotonConvLooseLH.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvTightLHEfficiency("recoPhotonConvTightLHEfficiency", "/MONITORING/recoPhotonConvTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvTightLHEfficiency.divide( recoPhotonConvTightLH.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvLooseLHEfficiency("recoPhotonUnconvLooseLHEfficiency", "/MONITORING/recoPhotonUnconvLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvLooseLHEfficiency.divide( recoPhotonUnconvLooseLH.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvTightLHEfficiency("recoPhotonUnconvTightLHEfficiency", "/MONITORING/recoPhotonUnconvTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvTightLHEfficiency.divide( recoPhotonUnconvTightLH.get(), truthPhotonUnconvRecoUnconv.get()));

    egammaMonitoring::WidthPlot truthPhotonRecoPhotonWidth("truthPhotonRecoPhotonWidth", "/MONITORING/truthPhotonRecoPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonRecoPhotonWidth.fill(truthPhotonRecoPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonConvPhotonWidth("truthPhotonConvPhotonWidth", "/MONITORING/truthPhotonConvPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvPhotonWidth.fill(truthPhotonConvPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConvWidth("truthPhotonConvRecoConvWidth", "/MONITORING/truthPhotonConvRecoConvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConvWidth.fill(truthPhotonConvRecoConv.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv1SiWidth("truthPhotonConvRecoConv1SiWidth", "/MONITORING/truthPhotonConvRecoConv1SiWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv1SiWidth.fill(truthPhotonConvRecoConv1Si.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv1TRTWidth("truthPhotonConvRecoConv1TRTWidth", "/MONITORING/truthPhotonConvRecoConv1TRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv1TRTWidth.fill(truthPhotonConvRecoConv1TRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2SiWidth("truthPhotonConvRecoConv2SiWidth", "/MONITORING/truthPhotonConvRecoConv2SiWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2SiWidth.fill(truthPhotonConvRecoConv2Si.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2TRTWidth("truthPhotonConvRecoConv2TRTWidth", "/MONITORING/truthPhotonConvRecoConv2TRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2TRTWidth.fill(truthPhotonConvRecoConv2TRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2SiTRTWidth("truthPhotonConvRecoConv2SiTRTWidth", "/MONITORING/truthPhotonConvRecoConv2SiTRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2SiTRTWidth.fill(truthPhotonConvRecoConv2SiTRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoUnconvWidth("truthPhotonConvRecoUnconvWidth", "/MONITORING/truthPhotonConvRecoUnconvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoUnconvWidth.fill(truthPhotonConvRecoUnconv.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvPhotonWidth("truthPhotonUnconvPhotonWidth", "/MONITORING/truthPhotonUnconvPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvPhotonWidth.fill(truthPhotonUnconvPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvRecoConvWidth("truthPhotonUnconvRecoConvWidth", "/MONITORING/truthPhotonUnconvRecoConvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvRecoConvWidth.fill(truthPhotonUnconvRecoConv.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvRecoUnconvWidth("truthPhotonUnconvRecoUnconvWidth", "/MONITORING/truthPhotonUnconvRecoUnconvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvRecoUnconvWidth.fill(truthPhotonUnconvRecoUnconv.get()));
  }

  return StatusCode::SUCCESS;
}


bool EgammaMonitoring::matchedToElectron(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return truth && abs(truth->pdgId())==11;
}


bool EgammaMonitoring::matchedToPion(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return truth && abs(truth->pdgId())==211;
}


bool EgammaMonitoring::notMatchedToTruth(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return !truth;
}
