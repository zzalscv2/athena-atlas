/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// JpsiXPlusDisplaced.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/JpsiXPlusDisplaced.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "DerivationFrameworkBPhys/LocalVector.h"
#include "HepPDT/ParticleDataTable.hh"
#include <algorithm>
#include <functional>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  JpsiXPlusDisplaced::JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKey("InputV0Vertices"),
    m_vertexDisVContainerKey(""),
    m_cascadeOutputsKeys{ "JpsiXPlusDisplacedVtx1", "JpsiXPlusDisplacedVtx2_sub", "JpsiXPlusDisplacedVtx2", "JpsiXPlusDisplacedVtx3" },
    m_refitV0(false),
    m_v0VtxOutputsKey(""),
    m_disVtxOutputsKey(""),
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_jxMassLower(0.0),
    m_jxMassUpper(30000.0),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(20000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_V0Hypothesis("Lambda"),
    m_V0MassLower(0.0),
    m_V0MassUpper(20000.0),
    m_lxyV0_cut(-999.0),
    m_minMass_gamma(-1.0),
    m_chi2cut_gamma(-1.0),
    m_DisplacedMassLower(0.0),
    m_DisplacedMassUpper(30000.0),
    m_lxyDisV_cut(-999.0),
    m_MassLower(0.0),
    m_MassUpper(51000.0),
    m_jxDaug_num(4),
    m_jxDaug1MassHypo(-1),
    m_jxDaug2MassHypo(-1),
    m_jxDaug3MassHypo(-1),
    m_jxDaug4MassHypo(-1),
    m_disVDaug_num(3),
    m_disVDaug3MassHypo(-1),
    m_massJX(-1),
    m_massJpsi(-1),
    m_massX(-1),
    m_massDisV(-1),
    m_massV0(-1),
    m_constrJX(false),
    m_constrJpsi(false),
    m_constrX(false),
    m_constrDisV(false),
    m_constrV0(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut_DisV(-1.0),
    m_chi2cut(-1.0),
    m_useTRT(false),
    m_ptTRT(400),
    m_d0_cut(2),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxDisVCandidates(0),
    m_beamCondSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_trackToVertexTool("Reco::TrackToVertex"),
    m_trkSelector("InDet::TrackSelectorTool"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("JXVertices",               m_vertexJXContainerKey);
    declareProperty("V0Vertices",               m_vertexV0ContainerKey);
    declareProperty("DisplacedVertices",        m_vertexDisVContainerKey);
    declareProperty("JXVtxHypoNames",           m_vertexJXHypoNames);
    declareProperty("V0VtxHypoNames",           m_vertexV0HypoNames);
    declareProperty("CascadeVertexCollections", m_cascadeOutputsKeys); // size is 3 or 4 only
    declareProperty("RefitV0",                  m_refitV0);
    declareProperty("OutoutV0VtxCollection",    m_v0VtxOutputsKey);
    declareProperty("OutoutDisVtxCollection",   m_disVtxOutputsKey);
    declareProperty("TrackParticleCollection",  m_TrkParticleCollection);
    declareProperty("VxPrimaryCandidateName",   m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",       m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("JXMassLowerCut",           m_jxMassLower);
    declareProperty("JXMassUpperCut",           m_jxMassUpper);
    declareProperty("JpsiMassLowerCut",         m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",         m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",         m_diTrackMassLower); // only effective when m_jxDaug_num=4
    declareProperty("DiTrackMassUpper",         m_diTrackMassUpper); // only effective when m_jxDaug_num=4
    declareProperty("V0Hypothesis",             m_V0Hypothesis); // "Ks" or "Lambda"
    declareProperty("V0MassLowerCut",           m_V0MassLower);
    declareProperty("V0MassUpperCut",           m_V0MassUpper);
    declareProperty("LxyV0Cut",                 m_lxyV0_cut);
    declareProperty("MassCutGamma",             m_minMass_gamma);
    declareProperty("Chi2CutGamma",             m_chi2cut_gamma);
    declareProperty("DisplacedMassLowerCut",    m_DisplacedMassLower); // only effective when m_disVDaug_num=3
    declareProperty("DisplacedMassUpperCut",    m_DisplacedMassUpper); // only effective when m_disVDaug_num=3
    declareProperty("LxyDisVtxCut",             m_lxyDisV_cut); // only effective when m_disVDaug_num=3
    declareProperty("MassLowerCut",             m_MassLower);
    declareProperty("MassUpperCut",             m_MassUpper);
    declareProperty("HypothesisName",           m_hypoName = "TQ");
    declareProperty("NumberOfJXDaughters",      m_jxDaug_num); // 2, or 3, or 4 only
    declareProperty("JXDaug1MassHypo",          m_jxDaug1MassHypo);
    declareProperty("JXDaug2MassHypo",          m_jxDaug2MassHypo);
    declareProperty("JXDaug3MassHypo",          m_jxDaug3MassHypo);
    declareProperty("JXDaug4MassHypo",          m_jxDaug4MassHypo);
    declareProperty("NumberOfDisVDaughters",    m_disVDaug_num); // 2 or 3 only
    declareProperty("DisVDaug3MassHypo",        m_disVDaug3MassHypo); // only effective when m_disVDaug_num=3
    declareProperty("JXMass",                   m_massJX);
    declareProperty("JpsiMass",                 m_massJpsi);
    declareProperty("XMass",                    m_massX); // only effective when m_jxDaug_num=4
    declareProperty("DisVtxMass",               m_massDisV); // only effective when m_disVDaug_num=3
    declareProperty("V0Mass",                   m_massV0);
    declareProperty("ApplyJXMassConstraint",    m_constrJX);
    declareProperty("ApplyJpsiMassConstraint",  m_constrJpsi);
    declareProperty("ApplyXMassConstraint",     m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyDisVMassConstraint",  m_constrDisV); // only effective when m_disVDaug_num=3
    declareProperty("ApplyV0MassConstraint",    m_constrV0);
    declareProperty("Chi2CutJX",                m_chi2cut_JX);
    declareProperty("Chi2CutV0",                m_chi2cut_V0);
    declareProperty("Chi2CutDisV",              m_chi2cut_DisV); // only effective when m_disVDaug_num=3
    declareProperty("Chi2Cut",                  m_chi2cut);
    declareProperty("UseTRT",                   m_useTRT); // only effective when m_disVDaug_num=3
    declareProperty("PtTRT",                    m_ptTRT); // only effective when m_disVDaug_num=3
    declareProperty("Trackd0Cut",               m_d0_cut); // only effective when m_disVDaug_num=3
    declareProperty("MaxJXCandidates",          m_maxJXCandidates);
    declareProperty("MaxV0Candidates",          m_maxV0Candidates);
    declareProperty("MaxDisVCandidates",        m_maxDisVCandidates); // only effective when m_disVDaug_num=3
    declareProperty("RefitPV",                  m_refitPV         = true);
    declareProperty("MaxnPV",                   m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",           m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",             m_DoVertexType    = 7);
    declareProperty("BeamConditionsSvc",        m_beamCondSvc);
    declareProperty("TrkVertexFitterTool",      m_iVertexFitter);
    declareProperty("V0VertexFitterTool",       m_iV0Fitter);
    declareProperty("GammaFitterTool",          m_iGammaFitter);
    declareProperty("PVRefitter",               m_pvRefitter);
    declareProperty("V0Tools",                  m_V0Tools);
    declareProperty("TrackToVertexTool",        m_trackToVertexTool);
    declareProperty("TrackSelectorTool",        m_trkSelector);
    declareProperty("CascadeTools",             m_CascadeTools);
  }

  StatusCode JpsiXPlusDisplaced::initialize() {
    if(m_V0Hypothesis != "Ks" && m_V0Hypothesis != "Lambda") {
      ATH_MSG_FATAL("Incorrect V0 container hypothesis is not recognized");
      return StatusCode::FAILURE;
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4 || m_disVDaug_num<2 || m_disVDaug_num>3) {
      ATH_MSG_FATAL("Incorrect number of JX or DisVtx daughters");
      return StatusCode::FAILURE;
    }

    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );

    // retrieving V0 vertex Fitter
    ATH_CHECK( m_iV0Fitter.retrieve() );

    // retrieving photon conversion vertex Fitter
    ATH_CHECK( m_iGammaFitter.retrieve() );

    // retrieving primary vertex refitter
    ATH_CHECK( m_pvRefitter.retrieve() );

    // retrieving the V0 tool
    ATH_CHECK( m_V0Tools.retrieve() );

    // retrieving the TrackToVertex extrapolator tool
    ATH_CHECK( m_trackToVertexTool.retrieve() );

    // retrieving the track selector tool
    ATH_CHECK( m_trkSelector.retrieve() );

    // retrieving the Cascade tools
    ATH_CHECK( m_CascadeTools.retrieve() );

    // Get the beam conditon service for beamspot
    ATH_CHECK( m_beamCondSvc.retrieve() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // https://pkg.go.dev/go-hep.org/x/hep/heppdt#section-readme
    mass_e = BPhysPVCascadeTools::getParticleMass(pdt, PDG::e_minus);
    mass_mu = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    mass_pion = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    mass_proton = BPhysPVCascadeTools::getParticleMass(pdt, PDG::p_plus);
    mass_Lambda = BPhysPVCascadeTools::getParticleMass(pdt, PDG::Lambda0);
    mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, PDG::K_S0);
    mass_Xi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::Xi_minus);

    // retrieve particle masses
    if(m_constrJpsi && m_massJpsi<0) m_massJpsi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_constrJX && m_massJX<0) m_massJX = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);
    if(m_constrV0 && m_massV0<0) m_massV0 = m_V0Hypothesis=="Ks" ? mass_Ks : mass_Lambda;
    if(m_disVDaug_num==3 && m_constrDisV && m_massDisV<0) m_massDisV = mass_Xi;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = mass_pion;
    if(m_disVDaug_num==3 && m_disVDaug3MassHypo < 0.) m_disVDaug3MassHypo = mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, xAOD::VertexContainer* v0VtxOutputContainer, xAOD::VertexContainer* disVtxOutputContainer) const {
    ATH_MSG_DEBUG( "JpsiXPlusDisplaced::performSearch" );
    assert(cascadeinfoContainer!=nullptr);

    // Get TrackParticle container
    const xAOD::TrackParticleContainer* trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, m_TrkParticleCollection));

    // Get the PrimaryVertices container
    const xAOD::Vertex* primaryVertex(nullptr);
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
    if (pvContainer->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
      return StatusCode::RECOVERABLE;
    }
    else primaryVertex = (*pvContainer)[0];

    std::vector<double> massesJX;
    massesJX.push_back(m_jxDaug1MassHypo);
    massesJX.push_back(m_jxDaug2MassHypo);
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);
    std::vector<double> massesV0_ppi;
    massesV0_ppi.push_back(mass_proton);
    massesV0_ppi.push_back(mass_pion);
    std::vector<double> massesV0_pip;
    massesV0_pip.push_back(mass_pion);
    massesV0_pip.push_back(mass_proton);
    std::vector<double> massesV0_pipi;
    massesV0_pipi.push_back(mass_pion);
    massesV0_pipi.push_back(mass_pion);
    std::vector<const xAOD::TrackParticle*> tracksJX;
    std::vector<const xAOD::TrackParticle*> tracksJpsi;
    std::vector<const xAOD::TrackParticle*> tracksX;
    std::vector<const xAOD::TrackParticle*> tracksV0;
    std::vector<const xAOD::TrackParticle*> tracksDis3;

    // Get Jpsi+X container
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));

    // Get V0 container
    const xAOD::VertexContainer *V0Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKey));

    // Get disV container
    const xAOD::VertexContainer *disVtxInputContainer(nullptr);
    if(m_disVDaug_num==3 && m_vertexDisVContainerKey != "") {
      ATH_CHECK(evtStore()->retrieve(disVtxInputContainer, m_vertexDisVContainerKey));
    }

    // select the 3rd track displaced from the beam spot center
    std::vector<const xAOD::TrackParticle*> tracksDisplaced;
    if(m_disVDaug_num==3 && !disVtxInputContainer) {
      for(xAOD::TrackParticleContainer::const_iterator tpIt=trackContainer->begin(); tpIt!=trackContainer->end(); ++tpIt) {
	const xAOD::TrackParticle* TP = (*tpIt);
	// V0 track selection (https://gitlab.cern.ch/atlas/athena/-/blob/21.2/InnerDetector/InDetRecTools/InDetTrackSelectorTool/src/InDetConversionTrackSelectorTool.cxx#L379)
	if(m_trkSelector->decision(*TP, primaryVertex)) {
	  uint8_t temp(0);
	  uint8_t nclus(0);
	  if(TP->summaryValue(temp, xAOD::numberOfPixelHits)) nclus += temp;
	  if(TP->summaryValue(temp, xAOD::numberOfSCTHits)  ) nclus += temp; 
	  if(!m_useTRT && nclus == 0) continue;

	  bool trk_cut = false;
	  if(nclus != 0) trk_cut = true;
	  if(nclus == 0 && TP->pt()>=m_ptTRT) trk_cut = true;
	  if(!trk_cut) continue;

	  // track is used if std::abs(d0/sig_d0) > d0_cut for PV
	  if(!d0Pass(TP,primaryVertex)) continue;
	  // only tracks not associated to a primary vertex are used
	  if(TP->vertex() != 0) continue;

	  tracksDisplaced.push_back(TP);
	}
      }
    }

    // Accessors of the V0 vertices with photon conversion hypothesis info
    SG::AuxElement::Accessor<int>    mAcc_gfit("gamma_fit");
    SG::AuxElement::Accessor<float>  mAcc_gmass("gamma_mass");
    SG::AuxElement::Accessor<float>  mAcc_gmasserr("gamma_massError");
    SG::AuxElement::Accessor<float>  mAcc_gchisq("gamma_chisq");
    SG::AuxElement::Accessor<int>    mAcc_gndof("gamma_ndof");
    SG::AuxElement::Accessor<float>  mAcc_gprob("gamma_probability");
    // Decorators of the V0 vertices
    SG::AuxElement::Decorator<int>   mDec_gfit("gamma_fit");
    SG::AuxElement::Decorator<float> mDec_gmass("gamma_mass");
    SG::AuxElement::Decorator<float> mDec_gmasserr("gamma_massError");
    SG::AuxElement::Decorator<float> mDec_gchisq("gamma_chisq");
    SG::AuxElement::Decorator<int>   mDec_gndof("gamma_ndof");
    SG::AuxElement::Decorator<float> mDec_gprob("gamma_probability");
    SG::AuxElement::Decorator<std::string> mDec_type("Type_V0Vtx");    

    // Select the V0 candidates before calling cascade fit
    std::vector<std::pair<xAOD::Vertex*,int> > selectedV0Candidates;
    for(auto vxcItr=V0Container->cbegin(); vxcItr!=V0Container->cend(); ++vxcItr) {
      xAOD::Vertex* vtx = *vxcItr;
      // Check the passed flags first
      bool passed = false;
      for(auto name : m_vertexV0HypoNames) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexV0HypoNames.size() && !passed) continue;

      int opt(-1);
      SG::AuxElement::Accessor<std::string> type_acc("Type_V0Vtx");
      if(!type_acc.isAvailable(*vtx)) {
	// determine V0 candidate track masses
	double massSig_V0_Lambda(0);
	if(vtx->trackParticle(0)->p4().P() > vtx->trackParticle(1)->p4().P()) {
	  massSig_V0_Lambda = std::abs(m_V0Tools->invariantMass(vtx, massesV0_ppi)-mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_ppi);
	}
	else {
	  massSig_V0_Lambda = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pip)-mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_pip);
	}
	double massSig_V0_Ks = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pipi)-mass_Ks)/m_V0Tools->invariantMassError(vtx, massesV0_pipi);
	double massV0;
	if(massSig_V0_Lambda < massSig_V0_Ks) {
	  if(vtx->trackParticle(0)->p4().P() > vtx->trackParticle(1)->p4().P()) opt = 1;
	  else opt = 2;
	  massV0 = (opt == 1 ? m_V0Tools->invariantMass(vtx, massesV0_ppi) : m_V0Tools->invariantMass(vtx, massesV0_pip));
	}
	else {
	  opt = 3;
	  massV0 = m_V0Tools->invariantMass(vtx, massesV0_pipi);
	}
	if(massV0<m_V0MassLower || massV0>m_V0MassUpper) continue;
      }
      else {
	std::string type_V0Vtx = type_acc(*vtx);
	if(type_V0Vtx == "Lambda")         opt = 1;
	else if(type_V0Vtx == "Lambdabar") opt = 2;
	else if(type_V0Vtx == "Ks")        opt = 3;
      }

      if((opt==1 || opt==2) && m_V0Hypothesis != "Lambda") continue;
      if(opt==3 && m_V0Hypothesis != "Ks") continue;

      tracksV0.clear();
      for(size_t i=0; i<vtx->nTrackParticles(); i++) tracksV0.push_back(vtx->trackParticle(i));
      Amg::Vector3D vtxPos = m_V0Tools->vtx(vtx);

      int gamma_fit = 0; int gamma_ndof = 0;
      double gamma_chisq = 999999., gamma_prob = -1., gamma_mass = -1., gamma_massErr = -1.;
      if(mAcc_gfit.isAvailable(*vtx)) {
	gamma_fit     = mAcc_gfit.isAvailable(*vtx) ? mAcc_gfit(*vtx) : 0;
	gamma_mass    = mAcc_gmass.isAvailable(*vtx) ? mAcc_gmass(*vtx) : -1;
	gamma_massErr = mAcc_gmasserr.isAvailable(*vtx) ? mAcc_gmasserr(*vtx) : -1;
	gamma_chisq   = mAcc_gchisq.isAvailable(*vtx) ? mAcc_gchisq(*vtx) : 999999;
	gamma_ndof    = mAcc_gndof.isAvailable(*vtx) ? mAcc_gndof(*vtx) : 0;
	gamma_prob    = mAcc_gprob.isAvailable(*vtx) ? mAcc_gprob(*vtx) : -1;
      }
      else {
	std::unique_ptr<xAOD::Vertex> gammaVtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iGammaFitter->fit(tracksV0, vtxPos) ));
	if (gammaVtx) {
	  gamma_fit     = 1;
	  gamma_mass    = m_V0Tools->invariantMass(gammaVtx.get(),mass_e,mass_e);
	  gamma_massErr = m_V0Tools->invariantMassError(gammaVtx.get(),mass_e,mass_e);
	  gamma_chisq   = m_V0Tools->chisq(gammaVtx.get());
	  gamma_ndof    = m_V0Tools->ndof(gammaVtx.get());
	  gamma_prob    = m_V0Tools->vertexProbability(gammaVtx.get());
	}
      }
      if(gamma_fit==1 && gamma_mass<m_minMass_gamma && gamma_chisq/gamma_ndof<m_chi2cut_gamma) continue;

      if(m_refitV0) {
	std::unique_ptr<xAOD::Vertex> V0vtx;
	if(m_constrV0) {
	  std::vector<double> massesV0;
	  if(opt == 1)      massesV0 = massesV0_ppi;
	  else if(opt == 2) massesV0 = massesV0_pip;
	  else if(opt == 3) massesV0 = massesV0_pipi;
	  // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkV0Fitter/TrkV0Fitter/TrkV0VertexFitter.h
	  V0vtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, massesV0, m_massV0, 0, vtxPos) ));
	}
	else {
	  V0vtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, vtxPos) ));
	}
	if(V0vtx) {
	  double chi2DOF = V0vtx->chiSquared()/V0vtx->numberDoF();
	  if(m_chi2cut_V0>0 && chi2DOF>m_chi2cut_V0) continue;
	  
	  V0vtx->clearTracks();
	  ElementLink<xAOD::TrackParticleContainer> newLink1;
	  newLink1.setElement(tracksV0[0]);
	  newLink1.setStorableObject(*trackContainer);
	  ElementLink<xAOD::TrackParticleContainer> newLink2;
	  newLink2.setElement(tracksV0[1]);
	  newLink2.setStorableObject(*trackContainer);
	  V0vtx->addTrackAtVertex(newLink1);
	  V0vtx->addTrackAtVertex(newLink2);

	  mDec_gfit(*V0vtx.get())     = gamma_fit;
	  mDec_gmass(*V0vtx.get())    = gamma_mass;
	  mDec_gmasserr(*V0vtx.get()) = gamma_massErr;
	  mDec_gchisq(*V0vtx.get())   = gamma_chisq;
	  mDec_gndof(*V0vtx.get())    = gamma_ndof;
	  mDec_gprob(*V0vtx.get())    = gamma_prob;
	  if(opt==1)      mDec_type(*V0vtx.get()) = "Lambda";
	  else if(opt==2) mDec_type(*V0vtx.get()) = "Lambdabar";
	  else if(opt==3) mDec_type(*V0vtx.get()) = "Ks";
	  selectedV0Candidates.push_back(std::pair<xAOD::Vertex*,int>{V0vtx.release(),opt+3});
	}
      }
      else {
	double chi2DOF = vtx->chiSquared()/vtx->numberDoF();
	if(m_chi2cut_V0>0 && chi2DOF>m_chi2cut_V0) continue;
	if(!mAcc_gfit.isAvailable(*vtx)) {
	  mDec_gfit(*vtx)     = gamma_fit;
	  mDec_gmass(*vtx)    = gamma_mass;
	  mDec_gmasserr(*vtx) = gamma_massErr;
	  mDec_gchisq(*vtx)   = gamma_chisq;
	  mDec_gndof(*vtx)    = gamma_ndof;
	  mDec_gprob(*vtx)    = gamma_prob;
	}
	if(!type_acc.isAvailable(*vtx)) {
	  if(opt==1)      mDec_type(*vtx) = "Lambda";
	  else if(opt==2) mDec_type(*vtx) = "Lambdabar";
	  else if(opt==3) mDec_type(*vtx) = "Ks";
	}
	selectedV0Candidates.push_back(std::pair<xAOD::Vertex*,int>{vtx,opt});
      }
    }
    if(selectedV0Candidates.size()<1) return StatusCode::SUCCESS;

    std::sort( selectedV0Candidates.begin(), selectedV0Candidates.end(), [](std::pair<xAOD::Vertex*,int> a, std::pair<xAOD::Vertex*,int> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
    if(m_maxV0Candidates>0 && selectedV0Candidates.size()>m_maxV0Candidates) {
      for(auto it=selectedV0Candidates.begin()+m_maxV0Candidates; it!=selectedV0Candidates.end(); it++) if(it->second>3) delete it->first;
      selectedV0Candidates.erase(selectedV0Candidates.begin()+m_maxV0Candidates, selectedV0Candidates.end());
    }

    if(v0VtxOutputContainer) {
      for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) v0VtxOutputContainer->push_back(v0VItr->first);
    }

    // Make the displaced candidates if needed
    std::vector<std::pair<xAOD::Vertex*,size_t> > displacedVertices;
    if(m_disVDaug_num==3) {
      if(!disVtxInputContainer) {
	for(size_t it=0; it<selectedV0Candidates.size(); ++it) {
	  std::pair<xAOD::Vertex*,int> elem = selectedV0Candidates[it];
	  tracksV0.clear();
	  for(size_t i=0; i<elem.first->nTrackParticles(); i++) tracksV0.push_back(elem.first->trackParticle(i));
	  std::vector<double> massesV0;
	  if((elem.second-1)%3+1 == 1)      massesV0 = massesV0_ppi;
	  else if((elem.second-1)%3+1 == 2) massesV0 = massesV0_pip;
	  else if((elem.second-1)%3+1 == 3) massesV0 = massesV0_pipi;

	  for(auto trkItr=tracksDisplaced.cbegin(); trkItr!=tracksDisplaced.cend(); ++trkItr) {
	    // Check identical tracks in input
	    if(std::find(tracksV0.cbegin(), tracksV0.cend(), *trkItr) != tracksV0.cend()) continue;

	    TLorentzVector p4_moth;
	    TLorentzVector tmp;
	    for(std::vector<const xAOD::TrackParticle*>::iterator itr = tracksV0.begin(); itr != tracksV0.end(); itr++) {
	      tmp.SetPtEtaPhiM((*itr)->pt(),(*itr)->eta(),(*itr)->phi(),massesV0[itr-tracksV0.begin()]);
	      p4_moth += tmp;
	    }
	    tmp.SetPtEtaPhiM((*trkItr)->pt(),(*trkItr)->eta(),(*trkItr)->phi(),m_disVDaug3MassHypo);
	    p4_moth += tmp;
	    if (p4_moth.M() < m_DisplacedMassLower || p4_moth.M() > m_DisplacedMassUpper) continue;

	    m_iVertexFitter->setDefault();
	    int robustness = 0;
	    m_iVertexFitter->setRobustness(robustness);
	    std::vector<Trk::VertexID> vrtList;
	    // V0 vertex
	    Trk::VertexID vID;
	    if(m_constrV0) vID = m_iVertexFitter->startVertex(tracksV0,massesV0,m_massV0);
	    else           vID = m_iVertexFitter->startVertex(tracksV0,massesV0);
	    vrtList.push_back(vID);
	    // Mother vertex
	    tracksDis3.clear(); tracksDis3.push_back(*trkItr);
	    std::vector<double> massesDis3{m_disVDaug3MassHypo};
	    if (m_constrDisV) m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList,m_massDisV);
	    else              m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList);
	    // Do the work
	    std::unique_ptr<Trk::VxCascadeInfo> cascade_info = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));
	    if(cascade_info != nullptr) {
	      BPhysPVCascadeTools::PrepareVertexLinks(cascade_info.get(), trackContainer);
	      cascade_info->getSVOwnership(false);
	      const std::vector<xAOD::Vertex*>& disVertices = cascade_info->vertices();
	      double chi2NDF = cascade_info->fitChi2()/cascade_info->nDoF();
	      if(m_chi2cut_DisV<=0 || chi2NDF < m_chi2cut_DisV) {
		SG::AuxElement::Decorator<float> chi2_decor("ChiSquared_Cascade");
		SG::AuxElement::Decorator<int> ndof_decor("nDoF_Cascade");
		chi2_decor(*disVertices[1]) = cascade_info->fitChi2();
		ndof_decor(*disVertices[1]) = cascade_info->nDoF();

		displacedVertices.push_back(std::pair<xAOD::Vertex*,size_t>{disVertices[1],it});
	      }
	      else delete disVertices[1];
	      delete disVertices[0]; // always delete the newly fitted V0 vertices
	    }
	  } // tracksDisplaced
	} // selectedV0Candidates
      } // disVtxInputContainer==0
      else { // disVtxInputContainer!=0
	for(auto vxcItr=disVtxInputContainer->cbegin(); vxcItr!=disVtxInputContainer->cend(); ++vxcItr) {
	  xAOD::Vertex* vtx = *vxcItr;
	  static SG::AuxElement::Accessor<VertexLink> cascadeVertexLinkAcc("CascadeVertexLink");
	  if(cascadeVertexLinkAcc.isAvailable(*vtx)) {
	    const VertexLink& cascadeVertexLink = cascadeVertexLinkAcc(*vtx);
	    if(cascadeVertexLink.isValid()) {
	      xAOD::Vertex* V0Vtx = const_cast<xAOD::Vertex*>(*cascadeVertexLink);
	      for(size_t it=0; it<selectedV0Candidates.size(); ++it) {
		std::pair<xAOD::Vertex*,int> elem = selectedV0Candidates[it];
		if(BPhysPVCascadeTools::VerticesMatchTracks<2>(elem.first,V0Vtx)) {
		  displacedVertices.push_back(std::pair<xAOD::Vertex*,size_t>{vtx,it});
		  break;
		}
	      }
	    }
	  }
	}
	if(disVtxInputContainer->size() != displacedVertices.size()) ATH_MSG_ERROR("Input and selected displaced vertex containers have different sizes");
      }
      if(displacedVertices.size()<1) return StatusCode::SUCCESS;

      std::sort( displacedVertices.begin(), displacedVertices.end(), [](std::pair<xAOD::Vertex*,size_t> a, std::pair<xAOD::Vertex*,size_t> b) {
	SG::AuxElement::Accessor<float> ChiSquared("ChiSquared_Cascade");
	SG::AuxElement::Accessor<int>   nDoF("nDoF_Cascade");
	float chi2_a = ChiSquared.isAvailable(*a.first) ? ChiSquared(*a.first) : 999999;
	int ndof_a = nDoF.isAvailable(*a.first) ? nDoF(*a.first) : 1;
	float chi2_b = ChiSquared.isAvailable(*b.first) ? ChiSquared(*b.first) : 999999;
	int ndof_b = nDoF.isAvailable(*b.first) ? nDoF(*b.first) : 1;
	return chi2_a/ndof_a < chi2_b/ndof_b; } );
      if(m_maxDisVCandidates>0 && displacedVertices.size()>m_maxDisVCandidates) {
	if(!disVtxInputContainer) {
	  for(auto it=displacedVertices.begin()+m_maxDisVCandidates; it!=displacedVertices.end(); it++) delete it->first;
	}
	displacedVertices.erase(displacedVertices.begin()+m_maxDisVCandidates, displacedVertices.end());
      }

      if(disVtxOutputContainer) {
	for(auto disVItr=displacedVertices.cbegin(); disVItr!=displacedVertices.cend(); ++disVItr) {
	  disVtxOutputContainer->push_back(disVItr->first);
	  static SG::AuxElement::Decorator<VertexLink> cascadeVertexLinkDecor("CascadeVertexLink");
	  VertexLink vertexLink;
          if(v0VtxOutputContainer) {
	    vertexLink.setElement(v0VtxOutputContainer->at(disVItr->second));
	    vertexLink.setStorableObject(*v0VtxOutputContainer);
	  }
	  else {
	    xAOD::Vertex* V0Vtx = BPhysPVCascadeTools::FindVertex<2>(V0Container, selectedV0Candidates[disVItr->second].first);
	    vertexLink.setElement(V0Vtx);
	    vertexLink.setStorableObject(*V0Container);
	  }
	  cascadeVertexLinkDecor(*disVItr->first) = vertexLink;
	} 
      }
    }

    // Select the JX candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJXCandidates;
    for(auto vxcItr=jxContainer->cbegin(); vxcItr!=jxContainer->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(auto name : m_vertexJXHypoNames) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexJXHypoNames.size() && !passed) continue;

      // Check Psi candidate invariant mass and skip if need be
      double mass_jx = m_V0Tools->invariantMass(*vxcItr,massesJX);
      if(mass_jx < m_jxMassLower || mass_jx > m_jxMassUpper) continue;

      // Add loose cut on Jpsi mass from JX -> Jpsi pi+ pi-, or on phi mass from Ds+ -> phi pi+
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( vtx->trackParticle(0)->pt(), 
			   vtx->trackParticle(0)->eta(),
			   vtx->trackParticle(0)->phi(), m_jxDaug1MassHypo);
      p4_mu2.SetPtEtaPhiM( vtx->trackParticle(1)->pt(),
			   vtx->trackParticle(1)->eta(),
			   vtx->trackParticle(1)->phi(), m_jxDaug2MassHypo);
      double mass_jpsi = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi < m_jpsiMassLower || mass_jpsi > m_jpsiMassUpper) continue;

      if(m_jxDaug_num==4 && m_diTrackMassLower>=0 && m_diTrackMassUpper>=0) {
	TLorentzVector p4_trk1, p4_trk2;
	p4_trk1.SetPtEtaPhiM( vtx->trackParticle(2)->pt(),
			      vtx->trackParticle(2)->eta(),
			      vtx->trackParticle(2)->phi(), m_jxDaug3MassHypo);
	p4_trk2.SetPtEtaPhiM( vtx->trackParticle(3)->pt(),
			      vtx->trackParticle(3)->eta(),
			      vtx->trackParticle(3)->phi(), m_jxDaug4MassHypo);
	double mass_diTrk = (p4_trk1 + p4_trk2).M();
	if (mass_diTrk < m_diTrackMassLower || mass_diTrk > m_diTrackMassUpper) continue;
      }

      double chi2DOF = vtx->chiSquared()/vtx->numberDoF();
      if(m_chi2cut_JX>0 && chi2DOF>m_chi2cut_JX) continue;

      selectedJXCandidates.push_back(vtx);
    }
    if(selectedJXCandidates.size()<1) return StatusCode::SUCCESS;

    std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [](const xAOD::Vertex* a, const xAOD::Vertex* b) { return a->chiSquared()/a->numberDoF() < b->chiSquared()/b->numberDoF(); } );
    if(m_maxJXCandidates>0 && selectedJXCandidates.size()>m_maxJXCandidates) {
      selectedJXCandidates.erase(selectedJXCandidates.begin()+m_maxJXCandidates, selectedJXCandidates.end());
    }

    // Select JX+DisV candidates
    // Iterate over JX vertices
    for(auto jxItr=selectedJXCandidates.cbegin(); jxItr!=selectedJXCandidates.cend(); ++jxItr) {
      tracksJX.clear();
      for(size_t i=0; i<(*jxItr)->nTrackParticles(); i++) tracksJX.push_back((*jxItr)->trackParticle(i));
      if (tracksJX.size() != massesJX.size()) {
	ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      }
      tracksJpsi.clear();
      tracksJpsi.push_back((*jxItr)->trackParticle(0));
      tracksJpsi.push_back((*jxItr)->trackParticle(1));
      tracksX.clear();
      if(m_jxDaug_num>=3) tracksX.push_back((*jxItr)->trackParticle(2));
      if(m_jxDaug_num==4) tracksX.push_back((*jxItr)->trackParticle(3));

      // Iterate over displaced vertices
      if(m_disVDaug_num==2) {
	for(auto V0Itr=selectedV0Candidates.cbegin(); V0Itr!=selectedV0Candidates.cend(); ++V0Itr) {
	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr->first->trackParticle(0)) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr->first->trackParticle(1)) != tracksJX.cend()) continue;
	  tracksV0.clear();
	  for(size_t j=0; j<V0Itr->first->nTrackParticles(); j++) tracksV0.push_back(V0Itr->first->trackParticle(j));
	  std::vector<double> massesV0;
	  if((V0Itr->second-1)%3+1 == 1)      massesV0 = massesV0_ppi;
	  else if((V0Itr->second-1)%3+1 == 2) massesV0 = massesV0_pip;
	  else if((V0Itr->second-1)%3+1 == 3) massesV0 = massesV0_pipi;

	  TLorentzVector p4_moth;
	  TLorentzVector tmp;
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    tmp.SetPtEtaPhiM((*jxItr)->trackParticle(it)->pt(),(*jxItr)->trackParticle(it)->eta(),(*jxItr)->trackParticle(it)->phi(),massesJX[it]);
	    p4_moth += tmp;
	  }
	  for(size_t it=0; it<V0Itr->first->nTrackParticles(); it++) {
	    tmp.SetPtEtaPhiM(V0Itr->first->trackParticle(it)->pt(),V0Itr->first->trackParticle(it)->eta(),V0Itr->first->trackParticle(it)->phi(),massesV0[it]);
	    p4_moth += tmp;
	  }
	  if (p4_moth.M() < m_MassLower || p4_moth.M() > m_MassUpper) continue;

	  // Apply the user's settings to the fitter
	  // Reset
	  m_iVertexFitter->setDefault();
	  // Robustness: http://cdsweb.cern.ch/record/685551
	  int robustness = 0;
	  m_iVertexFitter->setRobustness(robustness);
	  // Build up the topology
	  // Vertex list
	  std::vector<Trk::VertexID> vrtList;
	  // JX vertex
	  Trk::VertexID vID1;
	  // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	  if (m_constrJX) {
	    vID1 = m_iVertexFitter->startVertex(tracksJX,massesJX,m_massJX);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksJX,massesJX);
	  }
	  vrtList.push_back(vID1);
	  // V0 vertex
	  Trk::VertexID vID2;
	  if (m_constrV0) {
	    vID2 = m_iVertexFitter->nextVertex(tracksV0,massesV0,m_massV0);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksV0,massesV0);
	  }
	  vrtList.push_back(vID2);
	  // Mother vertex including JX and Dis
	  std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	  std::vector<double> tp_masses; tp_masses.clear();
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
	  if (m_constrJpsi) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID1,tracksJpsi,cnstV,m_massJpsi).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	    }
	  }
	  if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID1,tracksX,cnstV,m_massX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for X failed");
	    }
	  }
	  // Do the work
	  std::unique_ptr<Trk::VxCascadeInfo> result = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));

	  if (result != nullptr) {
	    for(auto v : result->vertices()) {
	      if(v->nTrackParticles()==0) {
		std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
		v->setTrackParticleLinks(nullLinkVector);
	      }
	    }
	    // reset links to original tracks
	    BPhysPVCascadeTools::PrepareVertexLinks(result.get(), trackContainer);

	    // necessary to prevent memory leak
	    result->getSVOwnership(true);

	    // Chi2/DOF cut
	    double chi2DOF = result->fitChi2()/result->nDoF();
	    bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);

	    const std::vector<std::vector<TLorentzVector> > &moms = result->getParticleMoms();
	    const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
	    double lxy_SV2 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);

	    if(chi2CutPassed && lxy_SV2>m_lxyV0_cut) {
              SG::AuxElement::Decorator<float>       chi2_decor("ChiSquared_V2");
              SG::AuxElement::Decorator<int>         ndof_decor("nDoF_V2");
	      SG::AuxElement::Decorator<std::string> type_decor("Type_V2");
              chi2_decor(*cascadeVertices[1]) = V0Itr->first->chiSquared();
              ndof_decor(*cascadeVertices[1]) = V0Itr->first->numberDoF();
	      if((V0Itr->second-1)%3+1 == 1)      type_decor(*cascadeVertices[1]) = "Lambda";
	      else if((V0Itr->second-1)%3+1 == 2) type_decor(*cascadeVertices[1]) = "Lambdabar";
	      else if((V0Itr->second-1)%3+1 == 3) type_decor(*cascadeVertices[1]) = "Ks";

	      mDec_gfit(*cascadeVertices[1])     = mAcc_gfit.isAvailable(*V0Itr->first) ? mAcc_gfit(*V0Itr->first) : 0;
	      mDec_gmass(*cascadeVertices[1])    = mAcc_gmass.isAvailable(*V0Itr->first) ? mAcc_gmass(*V0Itr->first) : -1;
	      mDec_gmasserr(*cascadeVertices[1]) = mAcc_gmasserr.isAvailable(*V0Itr->first) ? mAcc_gmasserr(*V0Itr->first) : -1;
	      mDec_gchisq(*cascadeVertices[1])   = mAcc_gchisq.isAvailable(*V0Itr->first) ? mAcc_gchisq(*V0Itr->first) : 999999;
	      mDec_gndof(*cascadeVertices[1])    = mAcc_gndof.isAvailable(*V0Itr->first) ? mAcc_gndof(*V0Itr->first) : 0;
	      mDec_gprob(*cascadeVertices[1])    = mAcc_gprob.isAvailable(*V0Itr->first) ? mAcc_gprob(*V0Itr->first) : -1;

	      cascadeinfoContainer->push_back(result.release());
	    }
	  }
	} // Iterate over V0Itr
      } // m_disVDaug_num==2
      else if(m_disVDaug_num==3) {
	for(auto disVItr=displacedVertices.cbegin(); disVItr!=displacedVertices.cend(); ++disVItr) {
	  xAOD::Vertex* disVtx = disVItr->first;
	  xAOD::Vertex* v0Vtx = selectedV0Candidates[disVItr->second].first;
	  int opt = selectedV0Candidates[disVItr->second].second;

	  tracksV0.clear();
	  for(size_t i=0; i<v0Vtx->nTrackParticles(); i++) tracksV0.push_back(v0Vtx->trackParticle(i));
	  if(tracksV0.size()!=2) ATH_MSG_ERROR("Number of retrieved V0 tracks is not 2");

	  tracksDis3.clear();
	  for(size_t i=0; i<disVtx->nTrackParticles(); i++) tracksDis3.push_back(disVtx->trackParticle(i));
	  if(tracksDis3.size()!=1) ATH_MSG_ERROR("Number of tracksDis3 tracks is not 1");

	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksV0[0]) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksV0[1]) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksDis3[0]) != tracksJX.cend()) continue;

	  std::vector<double> massesV0;
	  if((opt-1)%3+1 == 1)      massesV0 = massesV0_ppi;
	  else if((opt-1)%3+1 == 2) massesV0 = massesV0_pip;
	  else if((opt-1)%3+1 == 3) massesV0 = massesV0_pipi;

	  std::vector<double> massesDis3 = {m_disVDaug3MassHypo};

	  TLorentzVector p4_moth;
	  TLorentzVector tmp;
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    tmp.SetPtEtaPhiM((*jxItr)->trackParticle(it)->pt(),(*jxItr)->trackParticle(it)->eta(),(*jxItr)->trackParticle(it)->phi(),massesJX[it]);
	    p4_moth += tmp;
	  }
	  for(std::vector<const xAOD::TrackParticle*>::iterator itr = tracksV0.begin(); itr != tracksV0.end(); itr++) {
	    tmp.SetPtEtaPhiM((*itr)->pt(),(*itr)->eta(),(*itr)->phi(),massesV0[itr-tracksV0.begin()]);
	    p4_moth += tmp;
	  }
	  tmp.SetPtEtaPhiM(tracksDis3[0]->pt(),tracksDis3[0]->eta(),tracksDis3[0]->phi(),massesDis3[0]);
	  p4_moth += tmp;
	  if (p4_moth.M() < m_MassLower || p4_moth.M() > m_MassUpper) continue;

	  // Apply the user's settings to the fitter
	  // Reset
	  m_iVertexFitter->setDefault();
	  // Robustness: http://cdsweb.cern.ch/record/685551
	  int robustness = 0;
	  m_iVertexFitter->setRobustness(robustness);
	  // Build up the topology
	  // Vertex list
	  std::vector<Trk::VertexID> vrtList;
	  std::vector<Trk::VertexID> vrtList2;
	  // JX vertex
	  Trk::VertexID vID1;
	  // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	  if (m_constrJX) {
	    vID1 = m_iVertexFitter->startVertex(tracksJX,massesJX,m_massJX);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksJX,massesJX);
	  }
	  vrtList.push_back(vID1);
	  // V0 vertex
	  Trk::VertexID vID2;
	  if (m_constrV0) {
	    vID2 = m_iVertexFitter->nextVertex(tracksV0,massesV0,m_massV0);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksV0,massesV0);
	  }
	  vrtList2.push_back(vID2);
	  // Displaced vertex
	  Trk::VertexID vID3;
	  if (m_constrDisV) {
	    vID3 = m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList2,m_massDisV);
	  } else {
	    vID3 = m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList2);
	  }
	  vrtList.push_back(vID3);
	  // Mother vertex including JX and Dis
	  std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	  std::vector<double> tp_masses; tp_masses.clear();
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
	  if (m_constrJpsi) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID1,tracksJpsi,cnstV,m_massJpsi).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	    }
	  }
	  if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID1,tracksX,cnstV,m_massX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for X failed");
	    }
	  }
	  // Do the work
	  std::unique_ptr<Trk::VxCascadeInfo> result = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));

	  if (result != nullptr) {
	    for(auto v : result->vertices()) {
	      if(v->nTrackParticles()==0) {
		std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
		v->setTrackParticleLinks(nullLinkVector);
	      }
	    }
	    // reset links to original tracks
	    BPhysPVCascadeTools::PrepareVertexLinks(result.get(), trackContainer);

	    // necessary to prevent memory leak
	    result->getSVOwnership(true);

	    // Chi2/DOF cut
	    double chi2DOF = result->fitChi2()/result->nDoF();
	    bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);

	    const std::vector<std::vector<TLorentzVector> > &moms = result->getParticleMoms();
	    const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
	    double lxy_SV2_sub = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	    double lxy_SV2 = m_CascadeTools->lxy(moms[2],cascadeVertices[2],cascadeVertices[3]);

	    if(chi2CutPassed && lxy_SV2>m_lxyDisV_cut && lxy_SV2_sub>m_lxyV0_cut) {
              SG::AuxElement::Decorator<float> chi2_V2_decor("ChiSquared_V2");
              SG::AuxElement::Decorator<int>   ndof_V2_decor("nDoF_V2");
	      SG::AuxElement::Accessor<float> ChiSquared("ChiSquared_Cascade");
	      SG::AuxElement::Accessor<int>   nDoF("nDoF_Cascade");
              chi2_V2_decor(*cascadeVertices[2]) = ChiSquared.isAvailable(*disVtx) ? ChiSquared(*disVtx) : 999999;
              ndof_V2_decor(*cascadeVertices[2]) = nDoF.isAvailable(*disVtx) ? nDoF(*disVtx) : 1;

              SG::AuxElement::Decorator<float>       chi2_V1_decor("ChiSquared_V1");
              SG::AuxElement::Decorator<int>         ndof_V1_decor("nDoF_V1");
	      SG::AuxElement::Decorator<std::string> type_V1_decor("Type_V1");
              chi2_V1_decor(*cascadeVertices[1]) = v0Vtx->chiSquared();
	      ndof_V1_decor(*cascadeVertices[1]) = v0Vtx->numberDoF();
              if((opt-1)%3+1 == 1)      type_V1_decor(*cascadeVertices[1]) = "Lambda";
              else if((opt-1)%3+1 == 2) type_V1_decor(*cascadeVertices[1]) = "Lambdabar";
              else if((opt-1)%3+1 == 3) type_V1_decor(*cascadeVertices[1]) = "Ks";

	      mDec_gfit(*cascadeVertices[1])     = mAcc_gfit.isAvailable(*v0Vtx) ? mAcc_gfit(*v0Vtx) : 0;
	      mDec_gmass(*cascadeVertices[1])    = mAcc_gmass.isAvailable(*v0Vtx) ? mAcc_gmass(*v0Vtx) : -1;
	      mDec_gmasserr(*cascadeVertices[1]) = mAcc_gmasserr.isAvailable(*v0Vtx) ? mAcc_gmasserr(*v0Vtx) : -1;
	      mDec_gchisq(*cascadeVertices[1])   = mAcc_gchisq.isAvailable(*v0Vtx) ? mAcc_gchisq(*v0Vtx) : 999999;
	      mDec_gndof(*cascadeVertices[1])    = mAcc_gndof.isAvailable(*v0Vtx) ? mAcc_gndof(*v0Vtx) : 0;
	      mDec_gprob(*cascadeVertices[1])    = mAcc_gprob.isAvailable(*v0Vtx) ? mAcc_gprob(*v0Vtx) : -1;

	      cascadeinfoContainer->push_back(result.release());
	    }
	  }
	} // Iterate over disVItr
      } // m_disVDaug_num==3
    } // Iterate over JX vertices

    // clean up transient objects
    if(!disVtxOutputContainer && !disVtxInputContainer) {
      for(auto disVItr=displacedVertices.cbegin(); disVItr!=displacedVertices.cend(); ++disVItr) delete disVItr->first;
    }
    if(!v0VtxOutputContainer) {
      for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) if(v0VItr->second > 3) delete v0VItr->first;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::addBranches() const {
    std::array<std::unique_ptr<xAOD::VertexContainer>, 4> VtxWriteHandles;
    std::array<std::unique_ptr<xAOD::VertexAuxContainer>, 4> VtxWriteHandlesAux;
    size_t topoN = (m_disVDaug_num==2 ? 3 : 4);
    if(m_cascadeOutputsKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output cascade vertices");
      return StatusCode::FAILURE;
    }

    for(size_t i=0; i<topoN; i++){
      VtxWriteHandles[i] = std::make_unique<xAOD::VertexContainer>();
      VtxWriteHandlesAux[i] = std::make_unique<xAOD::VertexAuxContainer>();
      VtxWriteHandles[i]->setStore(VtxWriteHandlesAux[i].get());
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
    ATH_MSG_DEBUG("Found " << m_VxPrimaryCandidateName << " in StoreGate!");
    if (pvContainer->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
      return StatusCode::RECOVERABLE;
    }

    //----------------------------------------------------
    // Try to retrieve refitted primary vertices
    //----------------------------------------------------
    std::unique_ptr<xAOD::VertexContainer> refPvContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> refPvAuxContainer;
    bool ref_record = false;
    if (m_refitPV) {
      if (evtStore()->contains<xAOD::VertexContainer>(m_refPVContainerName)) {
	// refitted PV container exists. Get it from the store gate
	xAOD::VertexContainer*    _refPvContainer    = nullptr;
	xAOD::VertexAuxContainer* _refPvAuxContainer = nullptr;
	ATH_CHECK(evtStore()->retrieve(_refPvContainer   , m_refPVContainerName       ));
	ATH_CHECK(evtStore()->retrieve(_refPvAuxContainer, m_refPVContainerName + "Aux."));
	refPvContainer.reset(_refPvContainer);
	refPvAuxContainer.reset(_refPvAuxContainer);
      } else {
	// refitted PV container does not exist. Create a new one.
	refPvContainer = std::make_unique<xAOD::VertexContainer>();
	refPvAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
	refPvContainer->setStore(refPvAuxContainer.get());
	ref_record = true;
      }
    }

    // output V0 vertices
    std::unique_ptr<xAOD::VertexContainer> v0VtxOutputContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> v0VtxOutputAuxContainer;
    if(m_v0VtxOutputsKey != "") {
      v0VtxOutputContainer = std::make_unique<xAOD::VertexContainer>();
      v0VtxOutputAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
      v0VtxOutputContainer->setStore(v0VtxOutputAuxContainer.get());
    }

    // output displaced vertices
    std::unique_ptr<xAOD::VertexContainer> disVtxOutputContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> disVtxOutputAuxContainer;
    if(m_disVDaug_num==3 && m_disVtxOutputsKey != "") {
      disVtxOutputContainer = std::make_unique<xAOD::VertexContainer>();
      disVtxOutputAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
      disVtxOutputContainer->setStore(disVtxOutputAuxContainer.get());
    }

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    ATH_CHECK(performSearch(&cascadeinfoContainer,v0VtxOutputContainer.get(),disVtxOutputContainer.get()));

    BPhysPVCascadeTools helper(&(*m_CascadeTools), &m_beamCondSvc);
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the V0 vertex variables
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> JXLinksDecor("JXVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> V0LinksDecor("V0VertexLinks");
    SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    SG::AuxElement::Decorator<int> ndof_decor("nDoF");
    SG::AuxElement::Decorator<float> Pt_decor("Pt");
    SG::AuxElement::Decorator<float> PtErr_decor("PtErr");

    SG::AuxElement::Decorator<float> chi2_V1_decor("ChiSquared_V1");
    SG::AuxElement::Decorator<int> ndof_V1_decor("nDoF_V1");
    SG::AuxElement::Decorator<float> chi2_SV1_decor("ChiSquared_SV1");
    SG::AuxElement::Decorator<float> lxy_SV1_decor("lxy_SV1");
    SG::AuxElement::Decorator<float> lxyErr_SV1_decor("lxyErr_SV1");
    SG::AuxElement::Decorator<float> a0xy_SV1_decor("a0xy_SV1");
    SG::AuxElement::Decorator<float> a0xyErr_SV1_decor("a0xyErr_SV1");
    SG::AuxElement::Decorator<float> a0z_SV1_decor("a0z_SV1");
    SG::AuxElement::Decorator<float> a0zErr_SV1_decor("a0zErr_SV1");

    SG::AuxElement::Decorator<float> chi2_SV2_decor("ChiSquared_SV2");
    SG::AuxElement::Decorator<float> lxy_SV2_decor("lxy_SV2");
    SG::AuxElement::Decorator<float> lxyErr_SV2_decor("lxyErr_SV2");
    SG::AuxElement::Decorator<float> a0xy_SV2_decor("a0xy_SV2");
    SG::AuxElement::Decorator<float> a0xyErr_SV2_decor("a0xyErr_SV2");
    SG::AuxElement::Decorator<float> a0z_SV2_decor("a0z_SV2");
    SG::AuxElement::Decorator<float> a0zErr_SV2_decor("a0zErr_SV2");

    // Get the input containers
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));
    const xAOD::VertexContainer *V0Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKey));

    for(auto cascade_info : cascadeinfoContainer) {
      if(cascade_info==nullptr) ATH_MSG_ERROR("CascadeInfo is null");

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      for(size_t i=0; i<topoN; i++) {
	if(cascadeVertices[i]==nullptr) ATH_MSG_ERROR("Error null vertex");
      }

      cascade_info->getSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[topoN-1]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();

      // Identify the input V0
      xAOD::Vertex* v0Vtx = BPhysPVCascadeTools::FindVertex<2>(V0Container, cascadeVertices[1]);
      // Identify the input JX
      xAOD::Vertex* jxVtx(0);
      if(m_jxDaug_num==4) jxVtx = BPhysPVCascadeTools::FindVertex<4>(jxContainer, cascadeVertices[0]);
      else if(m_jxDaug_num==3) jxVtx = BPhysPVCascadeTools::FindVertex<3>(jxContainer, cascadeVertices[0]);
      else jxVtx = BPhysPVCascadeTools::FindVertex<2>(jxContainer, cascadeVertices[0]);

      // transfer hypotheses to output vertices
      for(auto name : m_vertexV0HypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
        if(flagAcc.isAvailable(*v0Vtx) && flagAcc(*v0Vtx)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[1]) = true;
        }
      }

      for(auto name : m_vertexJXHypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
        if(flagAcc.isAvailable(*jxVtx) && flagAcc(*jxVtx)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[0]) = true;
        }
      }

      // reset beamspot cache
      helper.GetBeamSpot(true);

      xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);

      // Get refitted track momenta from all vertices, charged tracks only
      BPhysPVCascadeTools::SetVectorInfo(vtx, cascade_info);
      vtx.setPass(true);

      //
      // Decorate main vertex
      //
      // mass, mass error
      // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/VxCascadeInfo.h
      BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[topoN-1])) );
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[topoN-1],cascade_info->getCovariance()[topoN-1])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[topoN-1]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[topoN-1],cascade_info->getCovariance()[topoN-1]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info->nDoF();

      // decorate the newly fitted JX vertex
      chi2_SV1_decor(*cascadeVertices[0])    = m_V0Tools->chisq(cascadeVertices[0]);
      chi2_V1_decor(*cascadeVertices[0])     = m_V0Tools->chisq(jxVtx);
      ndof_V1_decor(*cascadeVertices[0])     = m_V0Tools->ndof(jxVtx);
      lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
      lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
      a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
      a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);

      // decorate the newly fitted V0 or DisV vertex
      int idx = topoN==3 ? 1 : 2;
      chi2_SV2_decor(*cascadeVertices[idx])    = m_V0Tools->chisq(cascadeVertices[idx]);
      lxy_SV2_decor(*cascadeVertices[idx])     = m_CascadeTools->lxy(moms[idx],cascadeVertices[idx],mainVertex);
      lxyErr_SV2_decor(*cascadeVertices[idx])  = m_CascadeTools->lxyError(moms[idx],cascade_info->getCovariance()[idx],cascadeVertices[idx],mainVertex);
      a0z_SV2_decor(*cascadeVertices[idx])     = m_CascadeTools->a0z(moms[idx],cascadeVertices[idx],mainVertex);
      a0zErr_SV2_decor(*cascadeVertices[idx])  = m_CascadeTools->a0zError(moms[idx],cascade_info->getCovariance()[idx],cascadeVertices[idx],mainVertex);
      a0xy_SV2_decor(*cascadeVertices[idx])    = m_CascadeTools->a0xy(moms[idx],cascadeVertices[idx],mainVertex);
      a0xyErr_SV2_decor(*cascadeVertices[idx]) = m_CascadeTools->a0xyError(moms[idx],cascade_info->getCovariance()[idx],cascadeVertices[idx],mainVertex);

      if(m_disVDaug_num==3) {
	SG::AuxElement::Decorator<float> chi2_SV1_dec("ChiSquared_SV1");
	SG::AuxElement::Decorator<float> chi2_nc_SV1_dec("ChiSquared_nc_SV1");
	SG::AuxElement::Decorator<float> lxy_SV1_dec("lxy_SV1");
	SG::AuxElement::Decorator<float> lxyErr_SV1_dec("lxyErr_SV1");
	SG::AuxElement::Decorator<float> a0xy_SV1_dec("a0xy_SV1");
	SG::AuxElement::Decorator<float> a0xyErr_SV1_dec("a0xyErr_SV1");
	SG::AuxElement::Decorator<float> a0z_SV1_dec("a0z_SV1");
	SG::AuxElement::Decorator<float> a0zErr_SV1_dec("a0zErr_SV1");
	chi2_SV1_dec(*cascadeVertices[1])    = m_V0Tools->chisq(cascadeVertices[1]);
	lxy_SV1_dec(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	lxyErr_SV1_dec(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0xy_SV1_dec(*cascadeVertices[1])    = m_CascadeTools->a0z(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0xyErr_SV1_dec(*cascadeVertices[1]) = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0z_SV1_dec(*cascadeVertices[1])     = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0zErr_SV1_dec(*cascadeVertices[1])  = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
      }

      double Mass_Moth = m_CascadeTools->invariantMass(moms[topoN-1]);
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer.get(), &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, topoN-1, Mass_Moth, vtx));

      for(size_t i=0; i<topoN; i++) {
        VtxWriteHandles[i]->push_back(cascadeVertices[i]);
      }

      // Set links to cascade vertices
      VertexLinkVector precedingVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].get());
      if( vertexLink1.isValid() ) precedingVertexLinks.push_back( vertexLink1 );
      VertexLink vertexLink2;
      vertexLink2.setElement(cascadeVertices[1]);
      vertexLink2.setStorableObject(*VtxWriteHandles[1].get());
      if( vertexLink2.isValid() ) precedingVertexLinks.push_back( vertexLink2 );
      if(topoN==4) {
	VertexLink vertexLink3;
	vertexLink3.setElement(cascadeVertices[2]);
	vertexLink3.setStorableObject(*VtxWriteHandles[2].get());
	if( vertexLink3.isValid() ) precedingVertexLinks.push_back( vertexLink3 );
      }
      CascadeLinksDecor(*mainVertex) = precedingVertexLinks;
    } // loop over cascadeinfoContainer

    for(size_t i=0; i<topoN; i++){
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandles[i])   , m_cascadeOutputsKeys[i]));
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandlesAux[i]), m_cascadeOutputsKeys[i] + "Aux."));
    }
    if(ref_record) {
      ATH_CHECK(evtStore()->record(std::move(refPvContainer)   , m_refPVContainerName));
      ATH_CHECK(evtStore()->record(std::move(refPvAuxContainer), m_refPVContainerName + "Aux."));
    }
    if(m_v0VtxOutputsKey != "") {
      ATH_CHECK(evtStore()->record(std::move(v0VtxOutputContainer)   , m_v0VtxOutputsKey));
      ATH_CHECK(evtStore()->record(std::move(v0VtxOutputAuxContainer), m_v0VtxOutputsKey + "Aux."));
    }
    if(m_disVDaug_num==3 && m_disVtxOutputsKey != "") {
      ATH_CHECK(evtStore()->record(std::move(disVtxOutputContainer)   , m_disVtxOutputsKey));
      ATH_CHECK(evtStore()->record(std::move(disVtxOutputAuxContainer), m_disVtxOutputsKey + "Aux."));
    }

    // Deleting cascadeinfo since this won't be stored.
    // Vertices have been kept in m_cascadeOutputs and should be owned by their container
    for (auto cascade_info : cascadeinfoContainer) delete cascade_info;

    return StatusCode::SUCCESS;
  }

  bool JpsiXPlusDisplaced::d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const {
    bool pass = false;
    if(PV) {
      const Trk::Perigee* per = m_trackToVertexTool->perigeeAtVertex(*track, PV->position());
      if(per == 0) return pass;
      double d0 = per->parameters()[Trk::d0];
      double sig_d0 = sqrt((*per->covariance())(0,0));
      if(std::abs(d0/sig_d0) > m_d0_cut) pass = true; 
      delete per;
    }
    else {
      Amg::Vector3D beamspot = Amg::Vector3D(m_beamCondSvc->beamVtx().position());
      const Trk::Perigee* per = m_trackToVertexTool->perigeeAtVertex(*track, beamspot);
      if(per == 0) return pass;
      double d0 = per->parameters()[Trk::d0];
      double sig_d0 = sqrt((*per->covariance())(0,0));
      if(std::abs(d0/sig_d0) > m_d0_cut) pass = true; 
      delete per;
    }
    return pass;
  }
}
