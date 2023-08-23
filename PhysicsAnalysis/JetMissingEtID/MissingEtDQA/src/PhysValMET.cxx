///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// PhysValMET.cxx 
// Implementation file for class PhysValMET
// Author: Daniel Buescher <daniel.buescher@cern.ch>, Philipp Mogg <philipp.mogg@cern.ch>
/////////////////////////////////////////////////////////////////// 

// PhysVal includes
#include "PhysValMET.h"

// STL includes
#include <cmath>
#include <map>
#include <vector>

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "xAODMissingET/versions/MissingETBase.h" 
#include "xAODMissingET/MissingET.h" 
#include "xAODMissingET/MissingETContainer.h" 
#include "xAODMissingET/MissingETAuxContainer.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAssociationHelper.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "PATCore/AcceptData.h"
#include "METUtilities/METHelpers.h"

using namespace xAOD;

namespace MissingEtDQA {

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////

  PhysValMET::PhysValMET( const std::string& type, 
			  const std::string& name, 
			  const IInterface* parent) : 
    ManagedMonitorToolBase( type, name, parent ),
    m_metmaker(nullptr)
  {
    declareProperty( "InputElectrons", m_eleColl   = "Electrons"         );
    declareProperty( "InputPhotons",   m_gammaColl = "Photons"           );
    declareProperty( "InputTaus",      m_tauColl   = "TauJets"           );
    declareProperty( "InputMuons",     m_muonColl  = "Muons"             );
    declareProperty( "DoTruth", m_doTruth = false );
    declareProperty( "InputIsDAOD",    m_inputIsDAOD = false              );
    declareProperty( "DoMETRefPlots",  m_doMETRefPlots = false           );
    declareProperty( "METMapName",     m_mapname   = "METAssoc"          );
    declareProperty( "METCoreName",    m_corename  = "MET_Core"          );
  }
  
  // Destructor
  ///////////////
  PhysValMET::~PhysValMET()
  {
    m_names.clear();
    m_types.clear();
    m_terms.clear();
    m_MET_Ref.clear();
    m_MET_Ref_x.clear();
    m_MET_Ref_y.clear();
    m_MET_Ref_phi.clear();
    m_MET_Ref_sum.clear();
    m_MET_Diff_Ref.clear();
    m_MET_Diff_Ref_x.clear();
    m_MET_Diff_Ref_y.clear();
    m_MET_Diff_Ref_phi.clear();
    m_MET_Diff_Ref_sum.clear();
    m_MET_Cumu_Ref.clear();
    m_MET_Resolution_Ref.clear();
    m_MET_Significance_Ref.clear();
    m_MET_dPhi_Ref.clear();
    m_MET_CorrFinalTrk_Ref.clear();
    m_MET_CorrFinalClus_Ref.clear();
    m_MET_Reb.clear();
    m_MET_Reb_x.clear();
    m_MET_Reb_y.clear();
    m_MET_Reb_phi.clear();
    m_MET_Reb_sum.clear();
    m_MET_Diff_Reb.clear();
    m_MET_Diff_Reb_x.clear();
    m_MET_Diff_Reb_y.clear();
    m_MET_Diff_Reb_phi.clear();
    m_MET_Diff_Reb_sum.clear();
    m_MET_Cumu_Reb.clear();
    m_MET_Resolution_Reb.clear();
    m_MET_Significance_Reb.clear();
    m_MET_dPhi_Reb.clear();
    m_MET_CorrFinalTrk_Reb.clear();
    m_MET_CorrFinalClus_Reb.clear();
}
  
  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode PhysValMET::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");    
    ATH_CHECK(ManagedMonitorToolBase::initialize());

    m_names.clear();
    m_names["RefEle"] = "Electron term";
    m_names["RefGamma"] = "Photon term";
    m_names["RefTau"] = "Tau term";
    m_names["Muons"] = "Muon term";
    m_names["RefJet"] = "Jet term";
    m_names["SoftClus"] = "Cluster-based soft term";
    m_names["PVSoftTrk"] = "Track-based soft term (PV-matched)";
    m_names["FinalTrk"] = "Total MET with TST";
    m_names["FinalClus"] = "Total MET with CST";
    m_names["Track"] = "Track MET, loose selection";
    m_names["PVTrack_Nominal"] = "Track MET for highest sum p_{T}^{2} PV";
    m_names["PVTrack_Pileup"] = "Track MET for each pileup vertex";

    m_types.clear();
    m_types.emplace_back("AntiKt4EMTopo");
    m_types.emplace_back("AntiKt4EMPFlow");

    m_terms.clear();
    m_terms.emplace_back("RefEle");
    m_terms.emplace_back("RefGamma");
    m_terms.emplace_back("RefTau");
    m_terms.emplace_back("Muons");
    m_terms.emplace_back("RefJet");
    m_terms.emplace_back("SoftClus");
    m_terms.emplace_back("PVSoftTrk");
    m_terms.emplace_back("FinalTrk");
    m_terms.emplace_back("FinalClus");

    ATH_MSG_INFO("Retrieving tools...");

    ATH_CHECK( m_metmakerTopo.retrieve() ); 
    ATH_CHECK( m_metmakerPFlow.retrieve() );
    ATH_CHECK( m_muonSelTool.retrieve() );
    ATH_CHECK( m_elecSelLHTool.retrieve() );
    ATH_CHECK( m_photonSelIsEMTool.retrieve() );
    ATH_CHECK( m_tauSelTool.retrieve() );
    ATH_CHECK( m_jvtToolEM.retrieve() );
    ATH_CHECK( m_jvtToolPFlow.retrieve() );

    m_MET_Ref.clear();
    m_MET_Ref_x.clear();
    m_MET_Ref_y.clear();
    m_MET_Ref_phi.clear();
    m_MET_Ref_sum.clear();
    m_MET_Diff_Ref.clear();
    m_MET_Diff_Ref_x.clear();
    m_MET_Diff_Ref_y.clear();
    m_MET_Diff_Ref_phi.clear();
    m_MET_Diff_Ref_sum.clear();
    m_MET_Cumu_Ref.clear();
    m_MET_Resolution_Ref.clear();
    m_MET_Significance_Ref.clear();
    m_MET_dPhi_Ref.clear();
    m_MET_CorrFinalTrk_Ref.clear();
    m_MET_CorrFinalClus_Ref.clear();
    m_MET_Reb.clear();
    m_MET_Reb_x.clear();
    m_MET_Reb_y.clear();
    m_MET_Reb_phi.clear();
    m_MET_Reb_sum.clear();
    m_MET_Diff_Reb.clear();
    m_MET_Diff_Reb_x.clear();
    m_MET_Diff_Reb_y.clear();
    m_MET_Diff_Reb_phi.clear();
    m_MET_Diff_Reb_sum.clear();
    m_MET_Cumu_Reb.clear();
    m_MET_Resolution_Reb.clear();
    m_MET_Significance_Reb.clear();
    m_MET_dPhi_Reb.clear();
    m_MET_CorrFinalTrk_Reb.clear();
    m_MET_CorrFinalClus_Reb.clear();
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValMET::bookHistograms()
  {
    ATH_MSG_INFO ("Booking hists " << name() << "...");
      
    // Physics validation plots are level 10

    int nbinp = 100;
    int nbinpxy = 100;
    int nbinphi = 63;
    int nbinE = 100;
    double suptmi = 500.;
    double suptmixy = 250.;
    double binphi = 3.15;
    double lowET = 0.;
    double suET = 2500.;

    if (m_detailLevel >= 10) {

     for (const auto& type : m_types){

      	std::string name_met = "MET_Reference_" + type;
      	m_dir_met.clear();
      	std::vector<TH1D*> v_MET_Ref;
      	std::vector<TH1D*> v_MET_Ref_x;
      	std::vector<TH1D*> v_MET_Ref_y;
      	std::vector<TH1D*> v_MET_Ref_phi;
      	std::vector<TH1D*> v_MET_Ref_sum;
      	std::vector<TH1D*> v_MET_Cumu_Ref;
      	std::vector<TH1D*> v_MET_Resolution_Ref;
      	std::vector<TH1D*> v_MET_Significance_Ref;
      	std::vector<TH1D*> v_MET_dPhi_Ref;
      	std::vector<TH2D*> v_MET_CorrFinalTrk_Ref;
      	std::vector<TH2D*> v_MET_CorrFinalClus_Ref;
      	std::vector<TH1D*> v_MET_Diff_Ref;
      	std::vector<TH1D*> v_MET_Diff_Ref_x;
      	std::vector<TH1D*> v_MET_Diff_Ref_y;
      	std::vector<TH1D*> v_MET_Diff_Ref_phi;
      	std::vector<TH1D*> v_MET_Diff_Ref_sum;

      	for(const auto& term : m_terms) {
      	  v_MET_Ref.push_back( new  TH1D((name_met + "_" + term).c_str(), (name_met + " " + m_names[term] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
      	  v_MET_Ref_x.push_back( new  TH1D((name_met + "_" + term +"_x").c_str(), (name_met + " " + m_names[term] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	  v_MET_Ref_y.push_back( new  TH1D((name_met + "_" + term + "_y").c_str(), (name_met + " " + m_names[term] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	  v_MET_Ref_phi.push_back( new  TH1D((name_met + "_" + term + "_phi").c_str(), (name_met + " " + m_names[term] + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
      	  v_MET_Ref_sum.push_back( new  TH1D((name_met + "_" + term + "_sum").c_str(), (name_met + " " + m_names[term] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET) );
      	  m_dir_met.push_back("MET/" + name_met + "/Terms/" + term + "/");
      	}

      	m_MET_Ref[type] = v_MET_Ref;
      	m_MET_Ref_x[type] = v_MET_Ref_x;
      	m_MET_Ref_y[type] = v_MET_Ref_y;
      	m_MET_Ref_phi[type] = v_MET_Ref_phi;
      	m_MET_Ref_sum[type] = v_MET_Ref_sum;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Ref[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Ref_x[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Ref_y[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Ref_phi[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Ref_sum[type].at(i),m_dir_met[i],all));
      	}

      	std::string name_sub = name_met + "/Cumulative";
      	v_MET_Cumu_Ref.push_back( new  TH1D((name_met + "_Cumulative_FinalClus").c_str(), (name_met + " CST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
      	v_MET_Cumu_Ref.push_back( new  TH1D((name_met + "_Cumulative_FinalTrk").c_str(), (name_met + " TST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
	
      	m_MET_Cumu_Ref[type] = v_MET_Cumu_Ref;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Cumu_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Cumu_Ref[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Residuals";
      	v_MET_Resolution_Ref.push_back(  new TH1D((name_met + "_Resolution_FinalClus_x").c_str(), ("x-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Ref.push_back(  new TH1D((name_met + "_Resolution_FinalClus_y").c_str(), ("y-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Ref.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_x").c_str(), ("x-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Ref.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_y").c_str(), ("y-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
	
      	m_MET_Resolution_Ref[type] = v_MET_Resolution_Ref;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Resolution_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Resolution_Ref[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Significance";
      	v_MET_Significance_Ref.push_back(  new TH1D((name_met + "_Significance_FinalClus").c_str(), ("MET / sqrt(sumet) for " + name_met + " CST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
      	v_MET_Significance_Ref.push_back(  new TH1D((name_met + "_Significance_FinalTrk").c_str(), ("MET / sqrt(sumet) for " + name_met + " TST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
	
      	m_MET_Significance_Ref[type] = v_MET_Significance_Ref;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Significance_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Significance_Ref[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/dPhi";
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalClus").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " CST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalClus").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " CST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalClus").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " CST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " TST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " TST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Ref.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalTrk").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " TST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
	
      	m_MET_dPhi_Ref[type] = v_MET_dPhi_Ref;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_dPhi_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_dPhi_Ref[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Correlations";
      	std::vector<std::string> corrClus_names;
      	corrClus_names.emplace_back("RefEle");
      	corrClus_names.emplace_back("RefGamma");
      	corrClus_names.emplace_back("RefTau");
      	corrClus_names.emplace_back("Muons");
      	corrClus_names.emplace_back("RefJet");
      	corrClus_names.emplace_back("SoftClus");
      	std::vector<std::string> corrTrk_names;
      	corrTrk_names.emplace_back("RefEle");
      	corrTrk_names.emplace_back("RefGamma");
      	corrTrk_names.emplace_back("RefTau");
      	corrTrk_names.emplace_back("Muons");
      	corrTrk_names.emplace_back("RefJet");
      	corrTrk_names.emplace_back("PVSoftTrk");

      	v_MET_CorrFinalClus_Ref.reserve(corrClus_names.size());

for(const auto& it : corrClus_names) {
      	  v_MET_CorrFinalClus_Ref.push_back( new  TH2D((name_met + "_" + it + "_FinalClus").c_str(), (name_met + " " + m_names[it] + " vs. CST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,CST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
      	}
      	v_MET_CorrFinalTrk_Ref.reserve(corrTrk_names.size());

for(const auto& it : corrTrk_names) {
      	  v_MET_CorrFinalTrk_Ref.push_back( new  TH2D((name_met + "_" + it + "_FinalTrk").c_str(), (name_met + " " + m_names[it] + " vs. TST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,TST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
      	}

      	m_MET_CorrFinalClus_Ref[type] = v_MET_CorrFinalClus_Ref;
      	m_MET_CorrFinalTrk_Ref[type] = v_MET_CorrFinalTrk_Ref;

      	for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalTrk_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_CorrFinalTrk_Ref[type].at(i),"MET/" + name_sub + "/",all));
	}
      	for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalClus_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_CorrFinalClus_Ref[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	std::vector<std::string> sum_names;
      	sum_names.emplace_back("RefEle");
      	sum_names.emplace_back("RefGamma");
      	sum_names.emplace_back("RefTau");
      	sum_names.emplace_back("Muons");
      	sum_names.emplace_back("RefJet");

      	m_dir_met.clear();

      	for(const auto& it : sum_names) {
      	  v_MET_Diff_Ref.push_back( new  TH1D((name_met + "_Diff_" + it).c_str(), ("MET_Diff " + m_names[it] + " in " + name_met +"; E_{T}^{miss} - #Sigma p_{T} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150));
      	  v_MET_Diff_Ref_x.push_back( new  TH1D((name_met + "_Diff_" + it +"_x").c_str(), ("MET_Diff x " + m_names[it] + " in " + name_met +"; E_{x}^{miss} - #Sigma p_{x} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
      	  v_MET_Diff_Ref_y.push_back( new  TH1D((name_met + "_Diff_" + it +"_y").c_str(), ("MET_Diff y " + m_names[it] + " in " + name_met +"; E_{y}^{miss} - #Sigma p_{y} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
      	  v_MET_Diff_Ref_phi.push_back( new  TH1D((name_met + "_Diff_" + it +"_phi").c_str(), ("MET_Diff phi " + m_names[it] + " in " + name_met +"; #Delta#Phi(E_{T}^{miss},#Sigma p_{T}); Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
      	  v_MET_Diff_Ref_sum.push_back( new  TH1D((name_met + "_Diff_" + it +"_sum").c_str(), ("MET_Diff sumet " + m_names[it] + " in " + name_met +"; E_{T}^{sum} - #Sigma |p_{T}| [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -250, 250) );
      	  m_dir_met.push_back("MET/" + name_met + "/Differences/" + it + "/");
      	}

      	m_MET_Diff_Ref[type] = v_MET_Diff_Ref;
      	m_MET_Diff_Ref_x[type] = v_MET_Diff_Ref_x;
      	m_MET_Diff_Ref_y[type] = v_MET_Diff_Ref_y;
      	m_MET_Diff_Ref_phi[type] = v_MET_Diff_Ref_phi;
      	m_MET_Diff_Ref_sum[type] = v_MET_Diff_Ref_sum;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Diff_Ref.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Diff_Ref[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Ref_x[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Ref_y[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Ref_phi[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Ref_sum[type].at(i),m_dir_met[i],all));
      	}
	

      	//-------------------------------------------------------------------------------------
      	//Now the same for Rebuilt MET

      	name_met = "MET_Rebuilt_" + type;
      	m_dir_met.clear();
      	std::vector<TH1D*> v_MET_Reb;
      	std::vector<TH1D*> v_MET_Reb_x;
      	std::vector<TH1D*> v_MET_Reb_y;
      	std::vector<TH1D*> v_MET_Reb_phi;
      	std::vector<TH1D*> v_MET_Reb_sum;
      	std::vector<TH1D*> v_MET_Cumu_Reb;
      	std::vector<TH1D*> v_MET_Resolution_Reb;
      	std::vector<TH1D*> v_MET_Significance_Reb;
      	std::vector<TH1D*> v_MET_dPhi_Reb;
      	std::vector<TH2D*> v_MET_CorrFinalTrk_Reb;
      	std::vector<TH2D*> v_MET_CorrFinalClus_Reb;
      	std::vector<TH1D*> v_MET_Diff_Reb;
      	std::vector<TH1D*> v_MET_Diff_Reb_x;
      	std::vector<TH1D*> v_MET_Diff_Reb_y;
      	std::vector<TH1D*> v_MET_Diff_Reb_phi;
      	std::vector<TH1D*> v_MET_Diff_Reb_sum;

      	for(const auto& term : m_terms) {
      	  v_MET_Reb.push_back( new  TH1D((name_met + "_" + term).c_str(), (name_met + " " + m_names[term] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
      	  v_MET_Reb_x.push_back( new  TH1D((name_met + "_" + term + "_x").c_str(), (name_met + " " + m_names[term] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	  v_MET_Reb_y.push_back( new  TH1D((name_met + "_" + term + "_y").c_str(), (name_met + " " + m_names[term] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	  v_MET_Reb_phi.push_back( new  TH1D((name_met + "_" + term + "_phi").c_str(), (name_met + " " + m_names[term] + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
      	  v_MET_Reb_sum.push_back( new  TH1D((name_met + "_" + term + "_sum").c_str(), (name_met + " " + m_names[term] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET) );
      	  m_dir_met.push_back("MET/" + name_met + "/Terms/" + term + "/");
      	}

      	m_MET_Reb[type] = v_MET_Reb;
      	m_MET_Reb_x[type] = v_MET_Reb_x;
      	m_MET_Reb_y[type] = v_MET_Reb_y;
      	m_MET_Reb_phi[type] = v_MET_Reb_phi;
      	m_MET_Reb_sum[type] = v_MET_Reb_sum;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Reb[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Reb_x[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Reb_y[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Reb_phi[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Reb_sum[type].at(i),m_dir_met[i],all));
      	}

      	name_sub = name_met + "/Cumulative";
      	v_MET_Cumu_Reb.push_back( new  TH1D((name_met + "_Cumulative_FinalClus").c_str(), (name_met + " CST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
      	v_MET_Cumu_Reb.push_back( new  TH1D((name_met + "_Cumulative_FinalTrk").c_str(), (name_met + " TST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
	
      	m_MET_Cumu_Reb[type] = v_MET_Cumu_Reb;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Cumu_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Cumu_Reb[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Residuals";
      	v_MET_Resolution_Reb.push_back(  new TH1D((name_met + "_Resolution_FinalClus_x").c_str(), ("x-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Reb.push_back(  new TH1D((name_met + "_Resolution_FinalClus_y").c_str(), ("y-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Reb.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_x").c_str(), ("x-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	v_MET_Resolution_Reb.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_y").c_str(), ("y-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
      	m_MET_Resolution_Reb[type] = v_MET_Resolution_Reb;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Resolution_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Resolution_Reb[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Significance";
      	v_MET_Significance_Reb.push_back(  new TH1D((name_met + "_Significance_FinalClus").c_str(), ("MET / sqrt(sumet) for " + name_met + " CST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
      	v_MET_Significance_Reb.push_back(  new TH1D((name_met + "_Significance_FinalTrk").c_str(), ("MET / sqrt(sumet) for " + name_met + " TST; MET/sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
	
      	m_MET_Significance_Reb[type] = v_MET_Significance_Reb;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Significance_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Significance_Reb[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/dPhi";
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalClus").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " CST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalClus").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " CST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalClus").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " CST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " TST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " TST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
      	v_MET_dPhi_Reb.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalTrk").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " TST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
	
      	m_MET_dPhi_Reb[type] = v_MET_dPhi_Reb;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_dPhi_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_dPhi_Reb[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	name_sub = name_met + "/Correlations";
      	v_MET_CorrFinalClus_Reb.reserve(corrClus_names.size());

for(const auto& it : corrClus_names) {
      	  v_MET_CorrFinalClus_Reb.push_back( new  TH2D((name_met + "_" + it + "_FinalClus").c_str(), (name_met + " " + m_names[it] + " vs. CST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,CST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
      	}
      	v_MET_CorrFinalTrk_Reb.reserve(corrTrk_names.size());

for(const auto& it : corrTrk_names) {
      	  v_MET_CorrFinalTrk_Reb.push_back( new  TH2D((name_met + "_" + it + "_FinalTrk").c_str(), (name_met + " " + m_names[it] + " vs. TST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,TST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
      	}

      	m_MET_CorrFinalClus_Reb[type] = v_MET_CorrFinalClus_Reb;
      	m_MET_CorrFinalTrk_Reb[type] = v_MET_CorrFinalTrk_Reb;

      	for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalTrk_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_CorrFinalTrk_Reb[type].at(i),"MET/" + name_sub + "/",all));
	}
      	for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalClus_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_CorrFinalClus_Reb[type].at(i),"MET/" + name_sub + "/",all));
      	}

      	m_dir_met.clear();

      	for(const auto& it : sum_names) {
      	  v_MET_Diff_Reb.push_back( new  TH1D((name_met + "_Diff_" + it).c_str(), ("MET_Diff " + m_names[it] + " in " + name_met +"; E_{T}^{miss} - #Sigma p_{T} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150));
      	  v_MET_Diff_Reb_x.push_back( new  TH1D((name_met + "_Diff_" + it + "_x").c_str(), ("MET_Diff x " + m_names[it] + " in " + name_met +"; E_{x}^{miss} - #Sigma p_{x} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
      	  v_MET_Diff_Reb_y.push_back( new  TH1D((name_met + "_Diff_" + it + "_y").c_str(), ("MET_Diff y " + m_names[it] + " in " + name_met +"; E_{y}^{miss} - #Sigma p_{y} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
      	  v_MET_Diff_Reb_phi.push_back( new  TH1D((name_met + "_Diff_" + it + "_phi").c_str(), ("MET_Diff phi " + m_names[it] + " in " + name_met +"; #Delta#Phi(E_{T}^{miss}, #Sigma p_{T}); Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
      	  v_MET_Diff_Reb_sum.push_back( new  TH1D((name_met + "_Diff_" + it + "_sum").c_str(), ("MET_Diff sumet " + m_names[it] + " in " + name_met +"; E_{T}^{sum} - #Sigma |p_{T}| [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -250, 250) );
      	  m_dir_met.push_back("MET/" + name_met + "/Differences/" + it + "/");
      	}

      	m_MET_Diff_Reb[type] = v_MET_Diff_Reb;
      	m_MET_Diff_Reb_x[type] = v_MET_Diff_Reb_x;
      	m_MET_Diff_Reb_y[type] = v_MET_Diff_Reb_y;
      	m_MET_Diff_Reb_phi[type] = v_MET_Diff_Reb_phi;
      	m_MET_Diff_Reb_sum[type] = v_MET_Diff_Reb_sum;

      	for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Diff_Reb.size(); ++i) {
      	  ATH_CHECK(regHist(m_MET_Diff_Reb[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Reb_x[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Reb_y[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Reb_phi[type].at(i),m_dir_met[i],all));
      	  ATH_CHECK(regHist(m_MET_Diff_Reb_sum[type].at(i),m_dir_met[i],all));
      	}
     }


	//-------------------------------------------------------------------------------------
	//Now MET_Track

      std::string name_met = "MET_Track";
      std::string dir = "MET/" + name_met + "/";

      std::string sub_dir = dir + "Track/";
      ATH_CHECK(regHist(m_MET_Track = new  TH1D("Track", (name_met + " " + m_names["Track"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_Track_x = new  TH1D("Track_x", (name_met + " " + m_names["Track"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_Track_y = new  TH1D("Track_y", (name_met + " " + m_names["Track"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_Track_phi = new  TH1D("Track_phi", (name_met + " " + m_names["Track"] + " phi;  #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_Track_sum = new  TH1D("Track_sum", (name_met + " " + m_names["Track"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), sub_dir, all));

      sub_dir = dir + "PVTrack_Nominal/";
      ATH_CHECK(regHist(m_MET_PVTrack_Nominal = new TH1D("PVTrack_Nominal", (name_met + " " + m_names["PVTrack_Nominal"] + " ; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Nominal_x = new  TH1D("PVTrack_Nominal_x", (name_met + " " + m_names["PVTrack_Nominal"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Nominal_y = new  TH1D("PVTrack_Nominal_y", (name_met + " " + m_names["PVTrack_Nominal"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Nominal_phi = new  TH1D("PVTrack_Nominal_phi", (name_met + " " + m_names["PVTrack_Nominal"] + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Nominal_sum = new  TH1D("PVTrack_Nominal_sum", (name_met + " " + m_names["PVTrack_Nominal"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), sub_dir, all));

      sub_dir = dir + "PVTrack_Pileup/";
      ATH_CHECK(regHist(m_MET_PVTrack_Pileup = new  TH1D("PVTrack_Pileup", (name_met + " " + m_names["PVTrack_Pileup"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Pileup_x = new  TH1D("PVTrack_Pileup_x", (name_met + " " + m_names["PVTrack_Pileup"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Pileup_y = new  TH1D("PVTrack_Pileup_y", (name_met +" " +  m_names["PVTrack_Pileup"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Pileup_phi = new  TH1D("PVTrack_Pileup_phi", (name_met + " " + m_names["PVTrack_Pileup"] + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), sub_dir, all));
      ATH_CHECK(regHist(m_MET_PVTrack_Pileup_sum = new  TH1D("PVTrack_Pileup_sum", (name_met + " " + m_names["PVTrack_Pileup"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), sub_dir, all));

	//-------------------------------------------------------------------------------------
	//Now MET_Calo

      name_met = "MET_Calo";
      dir = "MET/" + name_met + "/";

      ATH_CHECK(regHist(m_MET_Calo = new  TH1D("Calo", (name_met + " " + m_names["Calo"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_x = new  TH1D("Calo_x", (name_met + " " + m_names["Calo"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_y = new  TH1D("Calo_y", (name_met + " " + m_names["Calo"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_phi = new  TH1D("Calo_phi", (name_met + " " + m_names["Calo"] + " phi;  #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_sum = new  TH1D("Calo_sum", (name_met + " " + m_names["Calo"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), dir, all));

    }

    return StatusCode::SUCCESS;      
  }

  StatusCode PhysValMET::fillHistograms()
  {
    ATH_MSG_INFO ("Filling hists " << name() << "...");

    //If we're running over new AODs without MET containers, don't do anything!
    if(!m_inputIsDAOD && !m_doMETRefPlots)
      return StatusCode::SUCCESS;

    //Beamspot weight
    const xAOD::EventInfo* eventInfo(nullptr);
    ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));

    float weight = eventInfo->beamSpotWeight();

    //Retrieve MET Truth
    const xAOD::MissingETContainer* met_Truth = nullptr;
    if(m_doTruth) {
      ATH_CHECK( evtStore()->retrieve(met_Truth,"MET_Truth") );
      if (!met_Truth) {
	ATH_MSG_ERROR ( "Failed to retrieve MET_Truth. Exiting." );
	return StatusCode::FAILURE;
      }
    }

    //Physics Objects
    const xAOD::MuonContainer* muons = nullptr;
    ATH_CHECK( evtStore()->retrieve(muons,m_muonColl) );
    if (!muons) {
      ATH_MSG_ERROR ( "Failed to retrieve Muon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<MuonContainer> metMuons(SG::VIEW_ELEMENTS);
    bool is_muon = 0;
    for(const auto& mu : *muons) {
      if(Accept(mu)) {
	metMuons.push_back(mu);
	is_muon = 1;
      }
    }

    const xAOD::ElectronContainer* electrons = nullptr;
    ATH_CHECK( evtStore()->retrieve(electrons,m_eleColl) );
    if (!electrons) {
      ATH_MSG_ERROR ( "Failed to retrieve Electron container. Exiting." );
      return StatusCode::FAILURE;
    }
   ConstDataVector<ElectronContainer> metElectrons(SG::VIEW_ELEMENTS);
   bool is_electron = 0;
   for(const auto& el : *electrons) {
     if(Accept(el)) {
       metElectrons.push_back(el);
       is_electron = 1;
     }
   }

    const xAOD::PhotonContainer* photons = nullptr;
    ATH_CHECK( evtStore()->retrieve(photons,m_gammaColl) );
    if (!electrons) {
      ATH_MSG_ERROR ( "Failed to retrieve Photon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<PhotonContainer> metPhotons(SG::VIEW_ELEMENTS);
    for(const auto& ph : *photons) {
      if(Accept(ph)) {
	metPhotons.push_back(ph);
      }
    }

    const TauJetContainer* taus = nullptr;
    ATH_CHECK( evtStore()->retrieve(taus, m_tauColl) );
    if(!taus) {
      ATH_MSG_ERROR("Failed to retrieve TauJet container: " << m_tauColl);
      return StatusCode::SUCCESS;
    }
    ConstDataVector<TauJetContainer> metTaus(SG::VIEW_ELEMENTS);
    for(const auto& tau : *taus) {
      if(Accept(tau)) {
	metTaus.push_back(tau);
      }
    }

    // Overlap removal

    ConstDataVector<PhotonContainer>::iterator pho_itr;
    ConstDataVector<ElectronContainer>::iterator ele_itr;
    ConstDataVector<TauJetContainer>::iterator taujet_itr;
    ConstDataVector<MuonContainer>::iterator mu_itr;
    ConstDataVector<JetContainer>::iterator jetc_itr;

    //Photons
    bool is_photon = 0;
    ConstDataVector<PhotonContainer> metPhotonsOR(SG::VIEW_ELEMENTS);
    for(pho_itr = metPhotons.begin(); pho_itr != metPhotons.end(); ++pho_itr ) {
      TLorentzVector phtlv = (*pho_itr)->p4();
      bool passOR = 1;
      for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) {
	if(phtlv.DeltaR((*ele_itr)->p4()) < 0.2) {
	  passOR = 0;
	  break;
	}	
      }
      if(passOR){
	metPhotonsOR.push_back(*pho_itr);
	is_photon = 1;
      }
    }
  
    //TauJets
    ConstDataVector<TauJetContainer> metTausOR(SG::VIEW_ELEMENTS);
    bool is_tau = 0;
    for(taujet_itr = metTaus.begin(); taujet_itr != metTaus.end(); ++taujet_itr ) {
      TLorentzVector tautlv = (*taujet_itr)->p4();
      bool passOR = 1;
      for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) {
	if(tautlv.DeltaR((*ele_itr)->p4()) < 0.2) {
	  passOR = 0;
	  break;
	}	
      }
      for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr) {
	if(tautlv.DeltaR((*pho_itr)->p4()) < 0.2) {
	  passOR = 0;
	  break;
	}	
      }
      if(passOR){
	metTausOR.push_back(*taujet_itr);
	is_tau = 1;
      }
    }

    //Sum up the pT's of the objects
    TLorentzVector el_tlv;
    double sum_el = 0;
    for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr ) {
      el_tlv += (*ele_itr)->p4();
      sum_el += (*ele_itr)->pt();
    }
    
    TLorentzVector mu_tlv;
    double sum_mu = 0;
    for(mu_itr = metMuons.begin(); mu_itr != metMuons.end(); ++mu_itr ) {
      mu_tlv += (*mu_itr)->p4();
      sum_mu += (*mu_itr)->pt();
    }

    TLorentzVector tau_tlv;
    double sum_tau = 0;
    for(taujet_itr = metTausOR.begin(); taujet_itr != metTausOR.end(); ++taujet_itr ) {
      tau_tlv += (*taujet_itr)->p4();
      sum_tau += (*taujet_itr)->pt();
    }
  
    TLorentzVector photon_tlv;
    double sum_photon = 0;
    for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr ) {
      photon_tlv += (*pho_itr)->p4();
      sum_photon += (*pho_itr)->pt();
    }

    for (const auto& type : m_types){
      ToolHandle<IJetUpdateJvt>* jvtTool(nullptr);
      double JvtCut = 0.59;
      if (type == "AntiKt4EMPFlow"){
        JvtCut = 0.2;
        jvtTool = &m_jvtToolPFlow;
      }
      else if (type == "AntiKt4EMTopo"){
        jvtTool = &m_jvtToolEM;
      }
    
      if(jvtTool == nullptr){
        ATH_MSG_ERROR("Unrecognized jet container: " << type << "Jets");
        return StatusCode::FAILURE;
      }

      // Retrieve Jets
      std::string name_jet = type + "Jets";
      const xAOD::JetContainer* jets = nullptr;
      ATH_CHECK( evtStore()->retrieve(jets,name_jet) );
      if (!jets) {
    	ATH_MSG_ERROR ( "Failed to retrieve Jet container: " << name_jet << ". Exiting." );
    	return StatusCode::FAILURE;
      }
      for(auto jet : *jets) {
	float newjvt = (*jvtTool)->updateJvt(*jet); 
	jet->auxdecor<float>("NewJvt") = newjvt;
      }
      ConstDataVector<JetContainer> metJets(SG::VIEW_ELEMENTS);
      for(const auto& jet : *jets) {
	metJets.push_back(jet);
      }
      //Overlap Removal
      ConstDataVector<JetContainer> metJetsOR(SG::VIEW_ELEMENTS);
      bool is_jet = 0;
      for(jetc_itr = metJets.begin(); jetc_itr != metJets.end(); ++jetc_itr ) {
    	TLorentzVector jettlv = (*jetc_itr)->p4();
    	bool passOR = 1;
    	for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) {
    	  if(jettlv.DeltaR((*ele_itr)->p4()) < 0.2) {
    	    passOR = 0;
    	    break;
    	  }	
    	}
    	for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr) {
    	  if(jettlv.DeltaR((*pho_itr)->p4()) < 0.2) {
    	    passOR = 0;
    	    break;
    	  }	
    	}
    	for(taujet_itr = metTausOR.begin(); taujet_itr != metTausOR.end(); ++taujet_itr) {
    	  if(jettlv.DeltaR((*taujet_itr)->p4()) < 0.2) {
    	    passOR = 0;
    	    break;
    	  }	
    	}
    	if(passOR){
    	  metJetsOR.push_back(*jetc_itr);
    	  is_jet = 1;
    	}
      }

      TLorentzVector jet_tlv;
      double sum_jet = 0;
      for(jetc_itr = metJetsOR.begin(); jetc_itr != metJetsOR.end(); ++jetc_itr ) {
    	jet_tlv += (*jetc_itr)->p4();
    	sum_jet += (*jetc_itr)->pt();
      }

      // Fill MET_Ref 
      std::string name_met = "MET_Reference_" + type;
      const xAOD::MissingETContainer* met_Ref = nullptr;
      // We're not building METReference anymore in derivations

      if(m_doMETRefPlots){

	// Retrieve Reference MET
	ATH_CHECK( evtStore()->retrieve(met_Ref, name_met) );
	if (!met_Ref) {
	  ATH_MSG_ERROR ("Couldn't retrieve " << name_met);
	  return StatusCode::FAILURE;
	}

	ATH_MSG_INFO( "  MET_Ref_" << type << ":" );
	for(const auto& it : *met_Ref) {
	  const std::string& name = it->name();
	  if(name == "RefEle"){
	    (m_MET_Ref[type]).at(0)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(0)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(0)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(0)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(0)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "RefGamma"){
	    (m_MET_Ref[type]).at(1)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(1)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(1)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(1)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(1)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "RefTau"){
	    (m_MET_Ref[type]).at(2)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(2)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(2)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(2)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(2)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "Muons"){
	    (m_MET_Ref[type]).at(3)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(3)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(3)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(3)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(3)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "RefJet"){
	    (m_MET_Ref[type]).at(4)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(4)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(4)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(4)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(4)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "SoftClus"){
	    (m_MET_Ref[type]).at(5)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(5)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(5)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(5)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(5)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "PVSoftTrk"){
	    (m_MET_Ref[type]).at(6)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(6)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(6)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(6)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(6)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "FinalTrk"){
	    (m_MET_Ref[type]).at(7)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(7)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(7)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(7)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(7)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	  if(name == "FinalClus"){
	    (m_MET_Ref[type]).at(8)->Fill((*met_Ref)[name.c_str()]->met()/1000., weight);
	    (m_MET_Ref_x[type]).at(8)->Fill((*met_Ref)[name.c_str()]->mpx()/1000., weight);
	    (m_MET_Ref_y[type]).at(8)->Fill((*met_Ref)[name.c_str()]->mpy()/1000., weight);
	    (m_MET_Ref_phi[type]).at(8)->Fill((*met_Ref)[name.c_str()]->phi(), weight);
	    (m_MET_Ref_sum[type]).at(8)->Fill((*met_Ref)[name.c_str()]->sumet()/1000., weight);
	  }
	}
      }

      //Prepare Rebuilding MET
      ATH_MSG_INFO( "  Rebuilding MET_" << type );
      MissingETContainer* met_Reb = new MissingETContainer();
      if( evtStore()->record(met_Reb,("MET_Rebuilt"+type).c_str()).isFailure() ) {
    	ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Rebuilt_" << type);
    	return StatusCode::FAILURE;
      }
      MissingETAuxContainer* met_RebAux = new MissingETAuxContainer();
      if( evtStore()->record(met_RebAux,("MET_Rebuilt"+type+"Aux").c_str()).isFailure() ) {
    	ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Rebuilt" << type);
    	return StatusCode::FAILURE;
      }
      met_Reb->setStore(met_RebAux);

      m_mapname = "METAssoc_"+type;
      m_corename = "MET_Core_"+type;
      const MissingETAssociationMap* metMap = nullptr;
      if( evtStore()->retrieve(metMap, m_mapname).isFailure() ) {
    	ATH_MSG_WARNING("Unable to retrieve MissingETAssociationMap: " << m_mapname);
    	return StatusCode::SUCCESS;
      }
      MissingETAssociationHelper metHelper(metMap);
      const MissingETContainer* coreMet(nullptr);
      if( evtStore()->retrieve(coreMet, m_corename).isFailure() ) {
    	ATH_MSG_WARNING("Unable to retrieve MissingETContainer: " << m_corename);
    	return StatusCode::SUCCESS;
      }

      ATH_MSG_INFO( "  MET_Rebuilt_" << type << ":" );
      //Select and flag objects for final MET building ***************************
      if( type.find("PFlow") != std::string::npos) m_metmaker = &m_metmakerPFlow;
      else m_metmaker = &m_metmakerTopo;

      // Electrons
      if( (*m_metmaker)->rebuildMET("RefEle", xAOD::Type::Electron, met_Reb, metElectrons.asDataVector(), metHelper).isFailure() ) {
    	ATH_MSG_WARNING("Failed to build electron term.");
      }

      // Photons
      if( (*m_metmaker)->rebuildMET("RefGamma", xAOD::Type::Photon, met_Reb, metPhotons.asDataVector(), metHelper).isFailure() ) {
    	ATH_MSG_WARNING("Failed to build photon term.");
      }

      // Taus
      if( (*m_metmaker)->rebuildMET("RefTau", xAOD::Type::Tau, met_Reb,metTaus.asDataVector(),metHelper).isFailure() ){
    	ATH_MSG_WARNING("Failed to build tau term.");
      }

      // Muons
      if( (*m_metmaker)->rebuildMET("Muons", xAOD::Type::Muon, met_Reb, metMuons.asDataVector(), metHelper).isFailure() ) {
    	ATH_MSG_WARNING("Failed to build muon term.");
      }

      // Jets
      if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_Reb, jets, coreMet, metHelper, true).isFailure() ) {
    	ATH_MSG_WARNING("Failed to build jet and soft terms.");
      }
      MissingETBase::Types::bitmask_t trksource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track);
      if((*met_Reb)["PVSoftTrk"]) trksource = (*met_Reb)["PVSoftTrk"]->source();
      if( met::buildMETSum("FinalTrk", met_Reb, trksource).isFailure() ){
    	ATH_MSG_WARNING("Building MET FinalTrk sum failed.");
      }
      MissingETBase::Types::bitmask_t clsource;
      if (type == "AntiKt4EMTopo") clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::EMTopo);
      else clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::UnknownSignal);
      
      if((*met_Reb)["SoftClus"]) clsource = (*met_Reb)["SoftClus"]->source();
      if( met::buildMETSum("FinalClus", met_Reb, clsource).isFailure() ) {
    	ATH_MSG_WARNING("Building MET FinalClus sum failed.");
      }

      // Fill MET_Reb
      for(const auto& it : *met_Reb) {
    	std::string name = it->name();
    	if(name == "RefEle"){
	  (m_MET_Reb[type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(0)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(0)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(0)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(0)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "RefGamma"){
	  (m_MET_Reb[type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(1)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(1)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(1)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(1)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "RefTau"){
	  (m_MET_Reb[type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(2)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(2)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(2)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(2)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "Muons"){
	  (m_MET_Reb[type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(3)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(3)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(3)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(3)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "RefJet"){
	  (m_MET_Reb[type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(4)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(4)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(4)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(4)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "SoftClus"){
	  (m_MET_Reb[type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(5)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(5)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(5)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(5)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "PVSoftTrk"){
	  (m_MET_Reb[type]).at(6)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(6)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(6)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(6)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(6)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "FinalTrk"){
	  (m_MET_Reb[type]).at(7)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(7)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(7)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(7)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(7)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
    	if(name == "FinalClus"){
	  (m_MET_Reb[type]).at(8)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
	  (m_MET_Reb_x[type]).at(8)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
	  (m_MET_Reb_y[type]).at(8)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
	  (m_MET_Reb_phi[type]).at(8)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
	  (m_MET_Reb_sum[type]).at(8)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
    	}
      }

      //Fill MET Angles
      ATH_MSG_INFO( "  MET_Angles :" );

      double leadPt = 0., subleadPt = 0., leadPhi = 0., subleadPhi = 0.;


      for (auto jet_itr = jets->begin(); jet_itr != jets->end(); ++jet_itr) {
	if ((*jet_itr)->pt() > leadPt && Accept(*jet_itr,JvtCut,jvtTool)) {
	  subleadPt = leadPt;
	  subleadPhi = leadPhi;
	  leadPt = (*jet_itr)->pt();
	  leadPhi = (*jet_itr)->phi();
	}
	else if ((*jet_itr)->pt() > subleadPt && Accept(*jet_itr,JvtCut,jvtTool)) {
	  subleadPt = (*jet_itr)->pt();
	  subleadPhi = (*jet_itr)->phi();
	}
      }

      if(m_doMETRefPlots){
	(m_MET_dPhi_Ref[type]).at(0)->Fill( -remainder( leadPhi - (*met_Ref)["FinalClus"]->phi(), 2*M_PI ), weight );
	(m_MET_dPhi_Ref[type]).at(1)->Fill( -remainder( subleadPhi - (*met_Ref)["FinalClus"]->phi(), 2*M_PI ), weight );
	(m_MET_dPhi_Ref[type]).at(3)->Fill( -remainder( leadPhi - (*met_Ref)["FinalTrk"]->phi(), 2*M_PI ), weight );
	(m_MET_dPhi_Ref[type]).at(4)->Fill( -remainder( subleadPhi - (*met_Ref)["FinalTrk"]->phi(), 2*M_PI ), weight );
      }

      (m_MET_dPhi_Reb[type]).at(0)->Fill( -remainder( leadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi_Reb[type]).at(1)->Fill( -remainder( subleadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi_Reb[type]).at(3)->Fill( -remainder( leadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi_Reb[type]).at(4)->Fill( -remainder( subleadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );
  

      leadPt = 0.; leadPhi = 0.;

      xAOD::MuonContainer::const_iterator muon_itr = muons->begin();
      xAOD::MuonContainer::const_iterator muon_end = muons->end();

      for( ; muon_itr != muon_end; ++muon_itr ) {
    	if((*muon_itr)->pt() > leadPt) {
    	  leadPt = (*muon_itr)->pt();
    	  leadPhi = (*muon_itr)->phi();
    	}
      }

      xAOD::ElectronContainer::const_iterator electron_itr = electrons->begin();
      xAOD::ElectronContainer::const_iterator electron_end = electrons->end();

      for( ; electron_itr != electron_end; ++electron_itr ) {
    	if((*electron_itr)->pt() > leadPt) {
    	  leadPt = (*electron_itr)->pt();
    	  leadPhi = (*electron_itr)->phi();
    	}
      }

      (m_MET_dPhi_Reb[type]).at(2)->Fill( -remainder( leadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi_Reb[type]).at(5)->Fill( -remainder( leadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );

      if(m_doMETRefPlots){

	(m_MET_dPhi_Ref[type]).at(2)->Fill( -remainder( leadPhi - (*met_Ref)["FinalClus"]->phi(), 2*M_PI ), weight );
	(m_MET_dPhi_Ref[type]).at(5)->Fill( -remainder( leadPhi - (*met_Ref)["FinalTrk"]->phi(), 2*M_PI ), weight );
    
	//Fill Correlation Plots
	//Reference
	for(const auto& it : *met_Ref) {
	  const std::string& name = it->name();
	  if(name == "RefEle"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(0)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	    (m_MET_CorrFinalClus_Ref[type]).at(0)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	  if(name == "RefGamma"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(1)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	    (m_MET_CorrFinalClus_Ref[type]).at(1)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	  if(name == "RefTau"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(2)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	    (m_MET_CorrFinalClus_Ref[type]).at(2)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	  if(name == "Muons"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(3)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	    (m_MET_CorrFinalClus_Ref[type]).at(3)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	  if(name == "RefJet"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(4)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	    (m_MET_CorrFinalClus_Ref[type]).at(4)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	  if(name == "PVSoftTrk"){
	    (m_MET_CorrFinalTrk_Ref[type]).at(5)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalTrk"]->met()/1000., weight);
	  }
	  if(name == "SoftClus"){
	    (m_MET_CorrFinalClus_Ref[type]).at(5)->Fill((*met_Ref)[name.c_str()]->met()/1000.,(*met_Ref)["FinalClus"]->met()/1000., weight);
	  }
	}
      }

      //Rebuilt
      for(const auto& it : *met_Reb) {
    	std::string name = it->name();
    	if(name == "RefEle"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
	  (m_MET_CorrFinalClus_Reb[type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
    	if(name == "RefGamma"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
	  (m_MET_CorrFinalClus_Reb[type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
    	if(name == "RefTau"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
	  (m_MET_CorrFinalClus_Reb[type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
    	if(name == "Muons"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
	  (m_MET_CorrFinalClus_Reb[type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
    	if(name == "RefJet"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
	  (m_MET_CorrFinalClus_Reb[type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
    	if(name == "PVSoftTrk"){
	  (m_MET_CorrFinalTrk_Reb[type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
    	}
    	if(name == "SoftClus"){
	  (m_MET_CorrFinalClus_Reb[type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
    	}
      }

      // Fill Resolution
      if(m_doTruth)
    	{
    	  ATH_MSG_INFO( "  Resolution:" );
	  if(m_doMETRefPlots){
	    (m_MET_Resolution_Ref[type]).at(0)->Fill(((*met_Ref)["FinalClus"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
	    (m_MET_Resolution_Ref[type]).at(1)->Fill(((*met_Ref)["FinalClus"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
	    (m_MET_Resolution_Ref[type]).at(2)->Fill(((*met_Ref)["FinalTrk"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
	    (m_MET_Resolution_Ref[type]).at(3)->Fill(((*met_Ref)["FinalTrk"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
	  }
	  (m_MET_Resolution_Reb[type]).at(0)->Fill(((*met_Reb)["FinalClus"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
	  (m_MET_Resolution_Reb[type]).at(1)->Fill(((*met_Reb)["FinalClus"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
	  (m_MET_Resolution_Reb[type]).at(2)->Fill(((*met_Reb)["FinalTrk"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
	  (m_MET_Resolution_Reb[type]).at(3)->Fill(((*met_Reb)["FinalTrk"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
    	}

      //Fill MET significance
      if( (*met_Reb)["FinalClus"]->sumet() != 0) (m_MET_Significance_Reb[type]).at(0)->Fill((*met_Reb)["FinalClus"]->met()/sqrt((*met_Reb)["FinalClus"]->sumet()*1000.), weight);
      if( (*met_Reb)["FinalTrk"]->sumet() != 0) (m_MET_Significance_Reb[type]).at(1)->Fill((*met_Reb)["FinalTrk"]->met()/sqrt((*met_Reb)["FinalTrk"]->sumet()*1000.), weight);

      TLorentzVector target_tlv;
      if(m_doMETRefPlots){
	//Fill MET Significance
	ATH_MSG_INFO( "  MET_significance:" );
	if( (*met_Ref)["FinalClus"]->sumet() != 0) (m_MET_Significance_Ref[type]).at(0)->Fill((*met_Ref)["FinalClus"]->met()/sqrt((*met_Ref)["FinalClus"]->sumet()*1000.), weight);
	if( (*met_Ref)["FinalTrk"]->sumet() != 0) (m_MET_Significance_Ref[type]).at(1)->Fill((*met_Ref)["FinalTrk"]->met()/sqrt((*met_Ref)["FinalTrk"]->sumet()*1000.), weight);

	//Fill Diff histograms
	for(const auto& it : *met_Ref) {
	  if(it->name() == "RefEle"){
	    if(is_electron or (it->sumet() > 0)){
	      target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
	      (m_MET_Diff_Ref[type]).at(0)->Fill((target_tlv.Pt() - el_tlv.Pt())/1000., weight);
	      (m_MET_Diff_Ref_x[type]).at(0)->Fill((target_tlv.Px() - el_tlv.Px())/1000., weight);
	      (m_MET_Diff_Ref_y[type]).at(0)->Fill((target_tlv.Py() - el_tlv.Py())/1000., weight);
	      (m_MET_Diff_Ref_phi[type]).at(0)->Fill(el_tlv.DeltaPhi(target_tlv), weight);
	      (m_MET_Diff_Ref_sum[type]).at(0)->Fill((it->sumet() - sum_el)/1000., weight);
	    }
	  }
	  if(it->name() == "RefGamma"){
	    if(is_photon or (it->sumet() > 0)){
	      target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
	      (m_MET_Diff_Ref[type]).at(1)->Fill((target_tlv.Pt() - photon_tlv.Pt())/1000., weight);
	      (m_MET_Diff_Ref_x[type]).at(1)->Fill((target_tlv.Px() - photon_tlv.Px())/1000., weight);
	      (m_MET_Diff_Ref_y[type]).at(1)->Fill((target_tlv.Py() - photon_tlv.Py())/1000., weight);
	      (m_MET_Diff_Ref_phi[type]).at(1)->Fill(photon_tlv.DeltaPhi(target_tlv), weight);
	      (m_MET_Diff_Ref_sum[type]).at(1)->Fill((it->sumet() - sum_photon)/1000., weight);
	    }
	  }
	  if(it->name() == "RefTau"){
	    if(is_tau or (it->sumet() > 0)){
	      target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
	      (m_MET_Diff_Ref[type]).at(2)->Fill((target_tlv.Pt() - tau_tlv.Pt())/1000., weight);
	      (m_MET_Diff_Ref_x[type]).at(2)->Fill((target_tlv.Px() - tau_tlv.Px())/1000., weight);
	      (m_MET_Diff_Ref_y[type]).at(2)->Fill((target_tlv.Py() - tau_tlv.Py())/1000., weight);
	      (m_MET_Diff_Ref_phi[type]).at(2)->Fill(tau_tlv.DeltaPhi(target_tlv), weight);
	      (m_MET_Diff_Ref_sum[type]).at(2)->Fill((it->sumet() - sum_tau)/1000., weight);
	    }
	  }
	  if(it->name() == "Muons"){
	    if(is_muon or (it->sumet() > 0)){
	      target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
	      (m_MET_Diff_Ref[type]).at(3)->Fill((target_tlv.Pt() - mu_tlv.Pt())/1000., weight);
	      (m_MET_Diff_Ref_x[type]).at(3)->Fill((target_tlv.Px() - mu_tlv.Px())/1000., weight);
	      (m_MET_Diff_Ref_y[type]).at(3)->Fill((target_tlv.Py() - mu_tlv.Py())/1000., weight);
	      (m_MET_Diff_Ref_phi[type]).at(3)->Fill(mu_tlv.DeltaPhi(target_tlv), weight);
	      (m_MET_Diff_Ref_sum[type]).at(3)->Fill((it->sumet() - sum_mu)/1000., weight);
	    }
	  }
	  if(it->name() == "RefJet"){
	    if(is_jet or (it->sumet() > 0)){
	      target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
	      (m_MET_Diff_Ref[type]).at(4)->Fill((target_tlv.Pt() - jet_tlv.Pt())/1000., weight);
	      (m_MET_Diff_Ref_x[type]).at(4)->Fill((target_tlv.Px() - jet_tlv.Px())/1000., weight);
	      (m_MET_Diff_Ref_y[type]).at(4)->Fill((target_tlv.Py() - jet_tlv.Py())/1000., weight);
	      (m_MET_Diff_Ref_phi[type]).at(4)->Fill(jet_tlv.DeltaPhi(target_tlv), weight);
	      (m_MET_Diff_Ref_sum[type]).at(4)->Fill((it->sumet() - sum_jet)/1000., weight);
	    }
    	  }
    	}
      }

      // For rebuilt MET add only jets with pT>20e3 and JVT cut
      TLorentzVector jetReb_tlv;
      double sum_jetReb = 0;
      for(const auto jet : metJetsOR) {
    	if(Accept(jet, JvtCut, jvtTool)) {
    	  jetReb_tlv += jet->p4();
    	  sum_jetReb += jet->pt();
    	}
      }

      for(const auto& it : *met_Reb) {
    	if(it->name() == "RefEle"){
    	  if(is_electron or (it->sumet() > 0)){
    	    target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
    	    (m_MET_Diff_Reb[type]).at(0)->Fill((target_tlv.Pt() - el_tlv.Pt())/1000., weight);
    	    (m_MET_Diff_Reb_x[type]).at(0)->Fill((target_tlv.Px() - el_tlv.Px())/1000., weight);
    	    (m_MET_Diff_Reb_y[type]).at(0)->Fill((target_tlv.Py() - el_tlv.Py())/1000., weight);
    	    (m_MET_Diff_Reb_phi[type]).at(0)->Fill(el_tlv.DeltaPhi(target_tlv), weight);
    	    (m_MET_Diff_Reb_sum[type]).at(0)->Fill((it->sumet() - sum_el)/1000., weight);
    	  }
    	}
    	if(it->name() == "RefGamma"){
    	  if(is_photon or (it->sumet() > 0)){
    	    target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
    	    (m_MET_Diff_Reb[type]).at(1)->Fill((target_tlv.Pt() - photon_tlv.Pt())/1000., weight);
    	    (m_MET_Diff_Reb_x[type]).at(1)->Fill((target_tlv.Px() - photon_tlv.Px())/1000., weight);
    	    (m_MET_Diff_Reb_y[type]).at(1)->Fill((target_tlv.Py() - photon_tlv.Py())/1000., weight);
    	    (m_MET_Diff_Reb_phi[type]).at(1)->Fill(photon_tlv.DeltaPhi(target_tlv), weight);
    	    (m_MET_Diff_Reb_sum[type]).at(1)->Fill((it->sumet() - sum_photon)/1000., weight);
    	  }
    	}
    	if(it->name() == "RefTau"){
    	  if(is_tau or (it->sumet() > 0)){
    	    target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
    	    (m_MET_Diff_Reb[type]).at(2)->Fill((target_tlv.Pt() - tau_tlv.Pt())/1000., weight);
    	    (m_MET_Diff_Reb_x[type]).at(2)->Fill((target_tlv.Px() - tau_tlv.Px())/1000., weight);
    	    (m_MET_Diff_Reb_y[type]).at(2)->Fill((target_tlv.Py() - tau_tlv.Py())/1000., weight);
    	    (m_MET_Diff_Reb_phi[type]).at(2)->Fill(tau_tlv.DeltaPhi(target_tlv), weight);
    	    (m_MET_Diff_Reb_sum[type]).at(2)->Fill((it->sumet() - sum_tau)/1000., weight);
    	  }
    	}
    	if(it->name() == "Muons"){
    	  if(is_muon or (it->sumet() > 0)){
    	    target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
    	    (m_MET_Diff_Reb[type]).at(3)->Fill((target_tlv.Pt() - mu_tlv.Pt())/1000., weight);
    	    (m_MET_Diff_Reb_x[type]).at(3)->Fill((target_tlv.Px() - mu_tlv.Px())/1000., weight);
    	    (m_MET_Diff_Reb_y[type]).at(3)->Fill((target_tlv.Py() - mu_tlv.Py())/1000., weight);
    	    (m_MET_Diff_Reb_phi[type]).at(3)->Fill(mu_tlv.DeltaPhi(target_tlv), weight);
    	    (m_MET_Diff_Reb_sum[type]).at(3)->Fill((it->sumet() - sum_mu)/1000., weight);
    	  }
    	}
    	if(it->name() == "RefJet"){
    	  if(is_jet or (it->sumet() > 0)){
    	    target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
    	    (m_MET_Diff_Reb[type]).at(4)->Fill((target_tlv.Pt() - jetReb_tlv.Pt())/1000., weight);
    	    (m_MET_Diff_Reb_x[type]).at(4)->Fill((target_tlv.Px() - jetReb_tlv.Px())/1000., weight);
    	    (m_MET_Diff_Reb_y[type]).at(4)->Fill((target_tlv.Py() - jetReb_tlv.Py())/1000., weight);
    	    (m_MET_Diff_Reb_phi[type]).at(4)->Fill(jetReb_tlv.DeltaPhi(target_tlv), weight);
    	    (m_MET_Diff_Reb_sum[type]).at(4)->Fill((it->sumet() - sum_jetReb)/1000., weight);
    	  }
    	}
      }

      if(type == "AntiKt4EMTopo") {
     	//Calo MET
      	//const xAOD::JetContainer* emptyjets = 0;
	ConstDataVector<JetContainer> metJetsEmpty(SG::VIEW_ELEMENTS);
      	MissingETContainer* met_Calo = new MissingETContainer();
      	if( evtStore()->record(met_Calo,("MET_Calo"+type).c_str()).isFailure() ) {
      	  ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Calo_" << type);
      	  return StatusCode::FAILURE;
      	}
      	MissingETAuxContainer* met_CaloAux = new MissingETAuxContainer();
      	if( evtStore()->record(met_CaloAux,("MET_Calo"+type+"Aux").c_str()).isFailure() ) {
      	  ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Calo" << type);
      	  return StatusCode::FAILURE;
      	}
      	met_Calo->setStore(met_CaloAux);
        MissingETAssociationHelper metHelper(metMap);
	if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_Calo, metJetsEmpty.asDataVector(), coreMet, metHelper, true).isFailure() ) {
      	  ATH_MSG_WARNING("Failed to build jet and soft terms.");
      	}

      	if((*met_Calo)["SoftClus"]) clsource = (*met_Calo)["SoftClus"]->source();
      	if( met::buildMETSum("FinalClus", met_Calo, clsource).isFailure() ) {
      	  ATH_MSG_WARNING("Building MET FinalClus sum failed.");
      	}

	m_MET_Calo->Fill((*met_Calo)["FinalClus"]->met()/1000., weight);
	m_MET_Calo_x->Fill((*met_Calo)["FinalClus"]->mpx()/1000., weight);
	m_MET_Calo_y->Fill((*met_Calo)["FinalClus"]->mpy()/1000., weight);
	m_MET_Calo_phi->Fill((*met_Calo)["FinalClus"]->phi(), weight);
	m_MET_Calo_sum->Fill((*met_Calo)["FinalClus"]->sumet()/1000., weight);

      }

    }

    //Currently we don't store MET_Track in the derivations
    //if MET_Ref not present, then we also dont have MET_Track
    if(m_doMETRefPlots){

      //Retrieve MET Track
      const xAOD::MissingETContainer* met_Track = nullptr;
      ATH_CHECK( evtStore()->retrieve(met_Track,"MET_Track") );
      if (!met_Track) {
	ATH_MSG_ERROR ( "Failed to retrieve MET_Track. Exiting." );
	return StatusCode::FAILURE;
      }

      // Fill MET Track
      ATH_MSG_INFO( "  MET_Track:" );
      
      m_MET_Track->Fill((*met_Track)["Track"]->met()/1000., weight);
      m_MET_Track_x->Fill((*met_Track)["Track"]->mpx()/1000., weight);
      m_MET_Track_y->Fill((*met_Track)["Track"]->mpy()/1000., weight);
      m_MET_Track_phi->Fill((*met_Track)["Track"]->phi(), weight);
      m_MET_Track_sum->Fill((*met_Track)["Track"]->sumet()/1000., weight);
      
      const xAOD::VertexContainer *vxCont = nullptr;
      ATH_CHECK( evtStore()->retrieve(vxCont, "PrimaryVertices") );
      for(const auto& vx : *vxCont) {
	int N = vx->index();
	const std::string name = "PVTrack_vx"+std::to_string(N);
	if(vx->vertexType()!=xAOD::VxType::NoVtx) {
	  if(vx->vertexType()==xAOD::VxType::PriVtx) {
	    m_MET_PVTrack_Nominal->Fill((*met_Track)[name]->met()/1000., weight);
	    m_MET_PVTrack_Nominal_x->Fill((*met_Track)[name]->mpx()/1000., weight);
	    m_MET_PVTrack_Nominal_y->Fill((*met_Track)[name]->mpy()/1000., weight);
	    m_MET_PVTrack_Nominal_phi->Fill((*met_Track)[name]->phi(), weight);
	    m_MET_PVTrack_Nominal_sum->Fill((*met_Track)[name]->sumet()/1000., weight);
	  } else { 
	    m_MET_PVTrack_Pileup->Fill((*met_Track)[name]->met()/1000., weight);
	    m_MET_PVTrack_Pileup_x->Fill((*met_Track)[name]->mpx()/1000., weight);
	    m_MET_PVTrack_Pileup_y->Fill((*met_Track)[name]->mpy()/1000., weight);
	    m_MET_PVTrack_Pileup_phi->Fill((*met_Track)[name]->phi(), weight);
	    m_MET_PVTrack_Pileup_sum->Fill((*met_Track)[name]->sumet()/1000., weight);
	  }
	}
      }
    }

   return StatusCode::SUCCESS;
   //return StatusCode::FAILURE;
  }
  
  StatusCode PhysValMET::procHistograms()
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");

    for (const auto& type : m_types){
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Ref[type]).size(); ++i) {
    	(m_MET_Ref[type]).at(i)->Sumw2();
    	(m_MET_Ref_x[type]).at(i)->Sumw2();
    	(m_MET_Ref_y[type]).at(i)->Sumw2();
    	(m_MET_Ref_phi[type]).at(i)->Sumw2();
    	(m_MET_Ref_sum[type]).at(i)->Sumw2();
    	(m_MET_Reb[type]).at(i)->Sumw2();
    	(m_MET_Reb_x[type]).at(i)->Sumw2();
    	(m_MET_Reb_y[type]).at(i)->Sumw2();
    	(m_MET_Reb_phi[type]).at(i)->Sumw2();
    	(m_MET_Reb_sum[type]).at(i)->Sumw2();
      }

      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Diff_Ref[type]).size(); ++i) {
    	(m_MET_Diff_Ref[type]).at(i)->Sumw2();
    	(m_MET_Diff_Ref_x[type]).at(i)->Sumw2();
    	(m_MET_Diff_Ref_y[type]).at(i)->Sumw2();
    	(m_MET_Diff_Ref_phi[type]).at(i)->Sumw2();
    	(m_MET_Diff_Ref_sum[type]).at(i)->Sumw2();
    	(m_MET_Diff_Reb[type]).at(i)->Sumw2();
    	(m_MET_Diff_Reb_x[type]).at(i)->Sumw2();
    	(m_MET_Diff_Reb_y[type]).at(i)->Sumw2();
    	(m_MET_Diff_Reb_phi[type]).at(i)->Sumw2();
    	(m_MET_Diff_Reb_sum[type]).at(i)->Sumw2();
      }

      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalTrk_Ref[type]).size(); ++i) {
    	(m_MET_CorrFinalTrk_Ref[type]).at(i)->Sumw2();
    	(m_MET_CorrFinalTrk_Reb[type]).at(i)->Sumw2();
      }

      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalClus_Ref[type]).size(); ++i) {
    	(m_MET_CorrFinalClus_Ref[type]).at(i)->Sumw2();
    	(m_MET_CorrFinalClus_Reb[type]).at(i)->Sumw2();
      }

      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Significance_Ref[type]).size(); ++i) {
    	(m_MET_Significance_Ref[type]).at(i)->Sumw2();
    	(m_MET_Significance_Reb[type]).at(i)->Sumw2();
      }

      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Resolution_Ref[type]).size(); ++i) {
    	(m_MET_Resolution_Ref[type]).at(i)->Sumw2();
    	(m_MET_Resolution_Reb[type]).at(i)->Sumw2();
      }

      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_dPhi_Ref[type]).size(); ++i) {
    	(m_MET_dPhi_Ref[type]).at(i)->Sumw2();
    	(m_MET_dPhi_Reb[type]).at(i)->Sumw2();
      }

      int nBins = (m_MET_Ref[type]).at(7)->GetNbinsX();
      for(int i=1;i<=nBins;i++){
      	double err;
      	(m_MET_Cumu_Ref[type]).at(0)->SetBinContent(i, (m_MET_Ref[type]).at(8)->IntegralAndError(i,nBins+1,err));
      	(m_MET_Cumu_Ref[type]).at(0)->SetBinError(i, err);
      	(m_MET_Cumu_Ref[type]).at(1)->SetBinContent(i, (m_MET_Ref[type]).at(7)->IntegralAndError(i,nBins+1,err));
      	(m_MET_Cumu_Ref[type]).at(1)->SetBinError(i, err);
      	(m_MET_Cumu_Reb[type]).at(0)->SetBinContent(i, (m_MET_Reb[type]).at(8)->IntegralAndError(i,nBins+1,err));
      	(m_MET_Cumu_Reb[type]).at(0)->SetBinError(i, err);
      	(m_MET_Cumu_Reb[type]).at(1)->SetBinContent(i, (m_MET_Reb[type]).at(7)->IntegralAndError(i,nBins+1,err));
      	(m_MET_Cumu_Reb[type]).at(1)->SetBinError(i, err);
      }
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Cumu_Ref[type]).size(); ++i) {
      	m_MET_Cumu_Ref[type].at(i)->Scale(1./(m_MET_Cumu_Ref[type]).at(i)->GetBinContent(1));
      	m_MET_Cumu_Reb[type].at(i)->Scale(1./(m_MET_Cumu_Reb[type]).at(i)->GetBinContent(1));
      }

    }

    m_MET_Track->Sumw2();
    m_MET_Track_x->Sumw2();
    m_MET_Track_y->Sumw2();
    m_MET_Track_phi->Sumw2();
    m_MET_Track_sum->Sumw2();
    m_MET_PVTrack_Nominal->Sumw2();
    m_MET_PVTrack_Nominal_x->Sumw2();
    m_MET_PVTrack_Nominal_y->Sumw2();
    m_MET_PVTrack_Nominal_phi->Sumw2();
    m_MET_PVTrack_Nominal_sum->Sumw2();
    m_MET_PVTrack_Pileup->Sumw2();
    m_MET_PVTrack_Pileup_x->Sumw2();
    m_MET_PVTrack_Pileup_y->Sumw2();
    m_MET_PVTrack_Pileup_phi->Sumw2();
    m_MET_PVTrack_Pileup_sum->Sumw2();

    m_MET_Calo->Sumw2();
    m_MET_Calo_x->Sumw2();
    m_MET_Calo_y->Sumw2();
    m_MET_Calo_phi->Sumw2();
    m_MET_Calo_sum->Sumw2();

    return StatusCode::SUCCESS;
  }
  
  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 
  bool PhysValMET::Accept(const xAOD::Muon* mu)
  {
    if( mu->pt()<2.5e3 || mu->pt()/cosh(mu->eta())<4e3 ) return false;
    return static_cast<bool> (m_muonSelTool->accept(*mu));
  }

  bool PhysValMET::Accept(const xAOD::Electron* el)
  {
    if( fabs(el->eta())>2.47 || el->pt()<10e3 ) return false;
    return static_cast<bool> (m_elecSelLHTool->accept(el));
  }

  bool PhysValMET::Accept(const xAOD::Photon* ph)
  {
    if( !(ph->author()&20) || fabs(ph->eta())>2.47 || ph->pt()<10e3 ) return false;
    return static_cast<bool>(m_photonSelIsEMTool->accept(ph));
  }

  bool PhysValMET::Accept(const xAOD::TauJet* tau)
  { return static_cast<bool> (m_tauSelTool->accept( *tau )); }

  bool PhysValMET::Accept(const xAOD::Jet* jet, double JvtCut, ToolHandle<IJetUpdateJvt>* jvtTool)
  {
    if( jet->pt()<20e3 || jvtTool == nullptr) return false;
    return (fabs(jet->eta()) > 2.4 || jet->pt() > 60e3 || (*jvtTool)->updateJvt(*jet) > JvtCut);
  }

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 


}

//  LocalWords:  str 
