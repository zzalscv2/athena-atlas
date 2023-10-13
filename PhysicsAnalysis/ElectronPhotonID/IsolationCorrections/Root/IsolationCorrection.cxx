/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Header for the ROOT tool of this package
#include "IsolationCorrections/IsolationCorrection.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TVector.h"
// xAOD
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODPrimitives/IsolationHelpers.h"

#include "PathResolver/PathResolver.h"

#include <typeinfo>
#include <utility>
#include <cmath>


namespace{
const float GeV(1000);
}

namespace CP {

  IsolationCorrection::IsolationCorrection(const std::string& name)
    : asg::AsgMessaging(name),
      m_tool_ver(REL21),
      m_nBinsEtaFine(10),
      m_nBinsEtaCoarse(5),
      m_corr_file(""),
      m_corr_ddshift_file(""),
      m_corr_ddsmearing_file(""), // in run I, there was a shift and a smearing
      m_is_mc(false),
      m_AFII_corr(false),
      m_set_mc(false),
      m_correct_etcone(false),
      m_trouble_categories(false),
      m_useLogLogFit(false),
      m_forcePartType(-1),
      m_shower(nullptr),
      m_previousYear("2000")
  {
    // 3 possible periods
    for (int i = 0; i < 3; i++) {
      // 2 simulation flavour
      for (int j = 0; j < 2; j++) 
	      m_corrInitialized[i][j] = false;
    }
  }

  void IsolationCorrection::SetCorrectionFile(const std::string& corr_file, const std::string& corr_ddshift_file, const std::string& corr_ddsmearing_file) {
    // the leakage parameterisation
    m_corr_file            = PathResolverFindCalibFile(corr_file);
    // the DD shifts (for photons)
    m_corr_ddshift_file    = PathResolverFindCalibFile(corr_ddshift_file);
    // And an eventual smearing (run I)
    m_corr_ddsmearing_file = PathResolverFindCalibFile(corr_ddsmearing_file);
  }

  StatusCode IsolationCorrection::initialize() {

    setEtaBins();
    setIsolCorr();
    m_shower = new CP::ShowerDepthTool();
    m_shower->initialize();

    return StatusCode::SUCCESS;
  }

  void IsolationCorrection::SetToolVer(CP::IsolationCorrection::Version ver){
    m_tool_ver = ver;
  }


  float IsolationCorrection::GetPtCorrectedIsolation(const xAOD::Egamma& input, const xAOD::Iso::IsolationType isol) const{
    float corrected_isolation = input.isolationValue(isol) - GetPtCorrection(input, isol);
    return corrected_isolation;

  }


  float IsolationCorrection::GetPtCorrection(const xAOD::Egamma& input, const xAOD::Iso::IsolationType isol) const {
    /*
      - nPV: number of primary vertices (corrections were derived requiring >= 2 tracks/vertex in 2011 data)

      - Etcone40: (ph_Etcone40) *** in MeV!
      - Etcone40_ED_corrected: ED corrected version of Etcone40 (ph_Etcone40_ED_corrected) *** in Mev!

      - energy: cluster energy (ph_cl_E) *** in MeV!
      - etaS2: eta in the 2nd calo sampling (ph_etas2)
      - etaPointing: pointing eta -> from 1st and 2nd calo samplings (ph_etap)
      - etaCluster: cluster eta (ph_cl_eta)
      - radius: radius of the input EtconeXX variable.  Can be given as, e.g., .20 or 20
      - is_mc: set to true if running on MC
      - Etcone_value: value of uncorrected EtconeXX variable (ph_EtconeXX) *** in MeV!
      - isConversion: photons only: conversion flag (ph_isConv)
      - parttype: ELECTRON or PHOTON, enum defined below
      - version: REL22, REL21, REL20_2, REL17_2, REL17 or REL16, enum defined below
    */
    float isolation_ptcorrection = 0;
    float energy = 0;

    //energy = input.caloCluster()->energyBE(1) + input.caloCluster()->energyBE(2) + input.caloCluster()->energyBE(3); //input.e()
    if (input.caloCluster() == nullptr) {
      ATH_MSG_WARNING("The associated cluster of the object does not exist ! Maybe the thinning was too agressive... No leakage correction computed.");
      return 0.;
    }
    if (m_tool_ver == REL22 || m_tool_ver == REL21 || m_tool_ver == REL20_2)
      energy = input.caloCluster()->energyBE(1) + input.caloCluster()->energyBE(2) + input.caloCluster()->energyBE(3);
    else
      energy = input.caloCluster()->e();

    bool is_mc = m_is_mc;
    ParticleType part_type = ( (input.type() == xAOD::Type::Electron) ? IsolationCorrection::ELECTRON : IsolationCorrection::PHOTON);
    bool convFlag = false;

    float etaS2 = input.caloCluster()->etaBE(2);
    float etaS1 = input.caloCluster()->etaBE(1);
    float etaCluster = input.caloCluster()->eta();
    float phiCluster = input.caloCluster()->phi();
    
    if(part_type == IsolationCorrection::PHOTON && fabs(etaS2) > 2.37) return 0.;
    if(part_type == IsolationCorrection::ELECTRON && fabs(etaS2) > 2.47) return 0.;
    
    if(fabs(etaS1) > 2.5) return 0.;
    if(fabs(phiCluster) > float(M_PI)) return 0.;

    if (part_type == IsolationCorrection::ELECTRON && energy > 15e3)
      ATH_MSG_VERBOSE("Electron ? " << (part_type == IsolationCorrection::ELECTRON) << " Input E = " << input.caloCluster()->e() << " E used " << energy << " author = " << input.author() << " pT = " << input.pt() << " phi = " << input.phi());

    int convFlag_int = 0;
    float conv_radius = 0.;
    float conv_ratio = 0.;

    if(part_type == IsolationCorrection::PHOTON){
      const xAOD::Photon* ph_input = ((const xAOD::Photon_v1*) &input);

      convFlag_int = xAOD::EgammaHelpers::conversionType(ph_input);
      if(convFlag_int == 3 ) convFlag_int = 2;
      else if(convFlag_int != 0) convFlag_int = 1;

      if(convFlag_int != 0) convFlag = true;
      conv_radius = xAOD::EgammaHelpers::conversionRadius(ph_input);

      if(convFlag_int == 2){
        const xAOD::Vertex* phVertex = ph_input->vertex();

        static const SG::AuxElement::Accessor<float> accPt1("pt1");
        static const SG::AuxElement::Accessor<float> accPt2("pt2");
        const xAOD::TrackParticle* tp0 = phVertex->trackParticle(0);
        const xAOD::TrackParticle* tp1 = phVertex->trackParticle(1);

        float pt1conv, pt2conv;
        if (accPt1.isAvailable(*phVertex) && accPt2.isAvailable(*phVertex) ){
            pt1conv = accPt1(*phVertex);
            pt2conv = accPt2(*phVertex);
        }else{
            pt1conv = getPtAtFirstMeasurement( tp0 );
            pt2conv = getPtAtFirstMeasurement( tp1 );
        }

        if(pt1conv > pt2conv) conv_ratio = pt2conv/pt1conv;
        else conv_ratio = pt1conv/pt2conv;
      }
    }
    int author = input.author();
    
    // for test
    if (part_type == IsolationCorrection::ELECTRON && m_forcePartType) {
      if (input.phi() > 0)
	      author = 1;
      else
	      author = 16;
    }
    
    //using std::optional
    auto etaPointing = m_shower->getCaloPointingEta(etaS1, etaS2, phiCluster, is_mc);
    if (!etaPointing.has_value()) return 0.;

    float radius = xAOD::Iso::coneSize(isol);
    bool is_topo = xAOD::Iso::isolationFlavour(isol) == xAOD::Iso::topoetcone;

    if(is_topo){
    
      isolation_ptcorrection = GetPtCorrectionTopo(  energy,
						     etaS2,
						     etaPointing.value(),
						     etaCluster,
						     radius,
						     is_mc,
						     convFlag,
						     part_type,
						     m_tool_ver,
						     convFlag_int,
						     author,
						     conv_radius,
						     conv_ratio);
    }else{
      isolation_ptcorrection = GetPtCorrection(  energy,
						 etaS2,
						 etaPointing.value(),
						 etaCluster,
						 radius,
						 is_mc,
						 convFlag,
						 part_type);
    }

    return isolation_ptcorrection;
  }

  float IsolationCorrection::getPtAtFirstMeasurement( const xAOD::TrackParticle* tp) 
  {
    if (!tp) return 0;
    for (unsigned int i = 0; i < tp->numberOfParameters(); ++i)
      if (tp->parameterPosition(i) == xAOD::FirstMeasurement)
        return hypot(tp->parameterPX(i), tp->parameterPY(i));
    return tp->pt();
  }

  // I also include the DD from 2015 study because it is done in the same way as 2015-2016 or 2017
StatusCode IsolationCorrection::setupDD(const std::string& year) {

  if (m_corr_ddshift_file.empty()){
    ATH_MSG_WARNING("IsolationCorrection::GetDDCorrection " << year << ", unknown correction file name.\nNo correction is applied.\n");
    return StatusCode::FAILURE;
  }

  int ii = year == "2017" ? 2 : (year == "2015_2016" ? 1 : 0);
  int jj = m_AFII_corr ? 1 : 0;

  if (year == m_previousYear && m_corrInitialized[ii][jj])
    return StatusCode::SUCCESS;

  ATH_MSG_VERBOSE("Will setup the tool to retrieve " << year << " DD corrections");
  
  m_previousYear  = year;
  if (year == "2015") {
    m_feta_bins_dd = &m_feta_bins_dd_2015;
    m_crackBin     = 2;
    m_graph_dd_cone40_unconv_photon_shift = &m_graph_dd_2015_cone40_unconv_photon_shift;
    m_graph_dd_cone40_conv_photon_shift   = &m_graph_dd_2015_cone40_conv_photon_shift;  
    m_graph_dd_cone20_unconv_photon_shift = &m_graph_dd_2015_cone20_unconv_photon_shift;
    m_graph_dd_cone20_conv_photon_shift   = &m_graph_dd_2015_cone20_conv_photon_shift;
  } else if (year == "2015_2016") {
    m_feta_bins_dd = &m_feta_bins_dd_2017;
    m_crackBin     = 4;
    m_graph_dd_cone40_unconv_photon_shift = m_AFII_corr ? &m_graph_afIIdd_2015_2016_cone40_unconv_photon_shift : &m_graph_dd_2015_2016_cone40_unconv_photon_shift;
    m_graph_dd_cone40_conv_photon_shift   = m_AFII_corr ? &m_graph_afIIdd_2015_2016_cone40_conv_photon_shift   : &m_graph_dd_2015_2016_cone40_conv_photon_shift;
    m_graph_dd_cone20_unconv_photon_shift = m_AFII_corr ? &m_graph_afIIdd_2015_2016_cone20_unconv_photon_shift : &m_graph_dd_2015_2016_cone20_unconv_photon_shift;
    m_graph_dd_cone20_conv_photon_shift   = m_AFII_corr ? &m_graph_afIIdd_2015_2016_cone20_conv_photon_shift   : &m_graph_dd_2015_2016_cone20_conv_photon_shift;
  } else if (year == "2017") {
    m_feta_bins_dd = &m_feta_bins_dd_2017;
    m_crackBin     = 4;
    m_graph_dd_cone40_unconv_photon_shift = m_AFII_corr ? &m_graph_afIIdd_2017_cone40_unconv_photon_shift : &m_graph_dd_2017_cone40_unconv_photon_shift;
    m_graph_dd_cone40_conv_photon_shift   = m_AFII_corr ? &m_graph_afIIdd_2017_cone40_conv_photon_shift   : &m_graph_dd_2017_cone40_conv_photon_shift;  
    m_graph_dd_cone20_unconv_photon_shift = m_AFII_corr ? &m_graph_afIIdd_2017_cone20_unconv_photon_shift : &m_graph_dd_2017_cone20_unconv_photon_shift;
    m_graph_dd_cone20_conv_photon_shift   = m_AFII_corr ? &m_graph_afIIdd_2017_cone20_conv_photon_shift   : &m_graph_dd_2017_cone20_conv_photon_shift;  
  } else {
    ATH_MSG_WARNING("Year " << year << " is not known in IsolationCorrection ! Check your input ! No correction is applied");
    return StatusCode::FAILURE;
  }

  if (!m_corrInitialized[ii][jj]) {

    ATH_MSG_VERBOSE("DD corrections for year " << year << " not loaded yet. Doing it know");
    
    std::unique_ptr< TFile > file_ptleakagecorr( TFile::Open( m_corr_ddshift_file.c_str(), "READ" ) );
    if (!file_ptleakagecorr) {
      ATH_MSG_ERROR("file " << m_corr_ddshift_file << " not found ! Check your inputs");
      return StatusCode::FAILURE;
    }
    TString baseN = year + "/" + (m_AFII_corr ? "AF2/" : "FullSim/");
    TVector *veta = (TVector*)file_ptleakagecorr->Get(baseN+"/etaBinning");
    m_feta_bins_dd->resize(veta->GetNrows());
    for (int ieta = 0; ieta < veta->GetNrows(); ieta++) m_feta_bins_dd->at(ieta) = (*veta)[ieta];
    TTree *tbinLabel = (TTree*)file_ptleakagecorr->Get(baseN+"tbinLabel");
    TBranch *bbinLabel(nullptr);
    TString *binLabel(nullptr); tbinLabel->SetBranchAddress("binLabel",&binLabel,&bbinLabel);
    for (unsigned int ieta = 0; ieta < m_feta_bins_dd->size()-2; ieta++) {
      tbinLabel->GetEntry(ieta);
      TString gN = "topoETcone40_DataDriven_unconverted_photon_eta_";
      gN += (*binLabel);
      m_graph_dd_cone40_unconv_photon_shift->push_back( (TGraph*) file_ptleakagecorr->Get(baseN+gN));
      gN = "topoETcone40_DataDriven_converted_photon_eta_";
      gN += (*binLabel);
      m_graph_dd_cone40_conv_photon_shift->push_back( (TGraph*) file_ptleakagecorr->Get(baseN+gN));
      gN = "topoETcone20_DataDriven_unconverted_photon_eta_";
      gN += (*binLabel);
      m_graph_dd_cone20_unconv_photon_shift->push_back( (TGraph*) file_ptleakagecorr->Get(baseN+gN));
      gN = "topoETcone20_DataDriven_converted_photon_eta_";
      gN += (*binLabel);
      m_graph_dd_cone20_conv_photon_shift->push_back( (TGraph*) file_ptleakagecorr->Get(baseN+gN));
      ATH_MSG_VERBOSE("Got graphs \n"
		      << " dR = 0.4, unconv " << m_graph_dd_cone40_unconv_photon_shift->at(ieta)->GetName() << "\n"
		      << " dR = 0.4,   conv " << m_graph_dd_cone40_conv_photon_shift->at(ieta)->GetName() << "\n"
		      << " dR = 0.2, unconv " << m_graph_dd_cone20_unconv_photon_shift->at(ieta)->GetName() << "\n"
		      << " dR = 0.2,   conv " << m_graph_dd_cone20_conv_photon_shift->at(ieta)->GetName());
		      
    }
    m_corrInitialized[ii][jj] = true;
  }
  
  return StatusCode::SUCCESS;
}

  float IsolationCorrection::GetDDCorrection(const xAOD::Egamma& input, const xAOD::Iso::IsolationType isol, const std::string& year) {

    ATH_MSG_VERBOSE("Getting DD correction");
    if (setupDD(year) == StatusCode::FAILURE) {
      return 0;
    }
    
    // corrections only for MC and photon
    if(!m_is_mc || input.type() == xAOD::Type::Electron) return 0;
    
    const xAOD::Photon* ph_input = ((const xAOD::Photon_v1*) &input);
    int convFlag_int = xAOD::EgammaHelpers::conversionType(ph_input);
    
    bool converted = false;
    if(convFlag_int > 0) converted = true;
    
    float etaS2  = input.eta();
    int eta_bin  = 0;
    
    double feta = fabs(etaS2);
    for (unsigned int i = 0; i < m_feta_bins_dd->size()-1; i++) {
      if (feta >= m_feta_bins_dd->at(i) && feta < m_feta_bins_dd->at(i+1))
        eta_bin = i;
    }
    if (eta_bin == m_crackBin)
      return 0;
    else if (eta_bin > m_crackBin)
      eta_bin -= 1;
    
    if (eta_bin < 0) {
      ATH_MSG_WARNING("Strange cluster S2 eta for photon isolation DD correction ! eta = " << etaS2 << ". No correction");
      return 0;
    }
    
    if (input.pt() > 25e3) 
      ATH_MSG_VERBOSE("Getting correction for photon pt: " <<input.pt()
		    << " eta: " << etaS2 << " etabin: " << eta_bin);
    
    float pt_gev = input.pt()*0.001;
    if (pt_gev > 999.) pt_gev = 999. ;
    
    float isolation_ddcorrection = 0;
    if (isol==xAOD::Iso::topoetcone40) {
      if (!converted)
        isolation_ddcorrection = 1e3*(*m_graph_dd_cone40_unconv_photon_shift)[eta_bin]->Eval(pt_gev);
      else
        isolation_ddcorrection = 1e3*(*m_graph_dd_cone40_conv_photon_shift)[eta_bin]->Eval(pt_gev);
    } else if (isol==xAOD::Iso::topoetcone20) {
      if (!converted)
        isolation_ddcorrection = 1e3*(*m_graph_dd_cone20_unconv_photon_shift)[eta_bin]->Eval(pt_gev);
      else
        isolation_ddcorrection = 1e3*(*m_graph_dd_cone20_conv_photon_shift)[eta_bin]->Eval(pt_gev);
    }

    return isolation_ddcorrection;
  }

  float IsolationCorrection::GetEtaPointing(const xAOD::Egamma* input){
    if (input->caloCluster() == nullptr) {
      ATH_MSG_WARNING("The associated cluster of the object does not exist ! Maybe the thinning was too agressive... use object eta for eta (instead of etaS2 or pointing).");
      return input->eta();
    }
    float etaS1 = input->caloCluster()->etaBE(1);
    float etaS2 = input->caloCluster()->etaBE(2);
    float phiCluster = input->caloCluster()->phi();
    ParticleType part_type = ( (input->type() == xAOD::Type::Electron) ? IsolationCorrection::ELECTRON : IsolationCorrection::PHOTON);

    if(part_type == IsolationCorrection::PHOTON && fabs(etaS2) > 2.37) return 0.;
    if(part_type == IsolationCorrection::ELECTRON && fabs(etaS2) > 2.47) return 0.;
    if(fabs(etaS1) > 2.5) return 0.;
    if(fabs(phiCluster) > 3.2) return 0.;

    auto etaPointing = m_shower->getCaloPointingEta(etaS1, etaS2, phiCluster);
    //using std::optional here
    return etaPointing.value_or(0.);
  }

  void IsolationCorrection::SetDataMC(bool is_mc){
    m_is_mc = is_mc;
    m_set_mc = true;
  }

  void IsolationCorrection::SetAFII(bool AFII_corr){
    m_AFII_corr = AFII_corr;
  }

  // obsolete
  /*
  void IsolationCorrection::SetDD(bool apply_dd){
    ATH_MSG_WARNING("The method SetDD is obsolete and not doing anything"); 
  }
  */

  void IsolationCorrection::SetCorrectEtcone(bool correct_etcone){
    m_correct_etcone = correct_etcone;
  }

  void IsolationCorrection::SetTroubleCategories(bool trouble_categories){
    m_trouble_categories = trouble_categories;
  }

  void IsolationCorrection::SetDataMC(const xAOD::EventInfo* event_info){
    SetDataMC(event_info->eventType(xAOD::EventInfo::IS_SIMULATION));
  }

  void IsolationCorrection::setEtaBins(){
    m_eta_bins_fine.reserve(m_nBinsEtaFine);
    m_eta_bins_coarse.reserve(m_nBinsEtaCoarse);
    m_eta_bins_fine = {0.0, 0.10, 0.60, 0.80, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47};
    m_eta_bins_coarse = {0.0, 0.60, 1.37, 1.52, 1.81, 2.47};
  }

  // initialize the leakage corrections and DD only for rel17.2.
  // Other DD are initialized once used for the first time 
  void IsolationCorrection::setIsolCorr(){
    if (m_tool_ver == REL17_2) {
      set2011Corr(); // in fact, this is for etcone
      set2012Corr();
      setDDCorr();
    } else if (m_tool_ver == REL20_2 || m_tool_ver == REL21 || m_tool_ver == REL22)
      set2015Corr();
  }

  // This for etcone !!! From 2011 !
  void IsolationCorrection::set2011Corr(){

    // -------------------------------------------------------------------------------------------
    // ------------- full 2011 (rel 17) leakage corrections --------------------------------------
    // CURRENT isolation corrections: fine grained in eta, derived from MC11
    m_mc_rel17_leakage_correction_slopes_electron_15 = { 0.01466, 0.01421, 0.01427, 0.01872, 0.02008, 0.02181, 0.02141, 0.02636, 0.03285, 0.03564 };
    m_mc_rel17_leakage_correction_slopes_electron_20 = { 0.01616, 0.01572, 0.01596, 0.02139, 0.02339, 0.03350, 0.02499, 0.03038, 0.03870, 0.04337 };
    m_mc_rel17_leakage_correction_slopes_electron_25 = { 0.01663, 0.01616, 0.01667, 0.02223, 0.02445, 0.03852, 0.02611, 0.03174, 0.04093, 0.04692 };
    m_mc_rel17_leakage_correction_slopes_electron_30 = { 0.01689, 0.01635, 0.01684, 0.02256, 0.02485, 0.04223, 0.02660, 0.03232, 0.04182, 0.04846 };
    m_mc_rel17_leakage_correction_slopes_electron_35 = { 0.01695, 0.01642, 0.01693, 0.02268, 0.02501, 0.04403, 0.02685, 0.03254, 0.04223, 0.04928 };
    m_mc_rel17_leakage_correction_slopes_electron_40 = { 0.01701, 0.01646, 0.01702, 0.02272, 0.02517, 0.04550, 0.02698, 0.03267, 0.04242, 0.04964 };

    m_mc_rel17_leakage_correction_offsets_electron_15 = { 21.71, 36.00, 132.56, 191.64, 263.46, 619.58, 288.75, 121.92, 102.35, 175.02 };
    m_mc_rel17_leakage_correction_offsets_electron_20 = { 76.67, 85.35, 184.90, 276.72, 384.97, 595.60, 657.99, 231.88, 170.48, 312.30 };
    m_mc_rel17_leakage_correction_offsets_electron_25 = { 90.44, 105.02, 267.95, 420.38, 555.09, 1014.50, 765.83, 283.50, 224.18, 357.30 };
    m_mc_rel17_leakage_correction_offsets_electron_30 = { 119.55, 127.09, 279.48, 430.96, 571.81, 846.86, 968.01, 354.46, 263.11, 455.21 };
    m_mc_rel17_leakage_correction_offsets_electron_35 = { 138.79, 161.87, 371.47, 572.08, 754.17, 1249.38, 1000.44, 389.22, 295.72, 464.28 };
    m_mc_rel17_leakage_correction_offsets_electron_40 = { 180.08, 187.89, 363.31, 553.46, 707.60, 1006.20, 1105.73, 434.21, 312.78, 535.90 };

    // photons: last eta bin isn't used
    m_mc_rel17_leakage_correction_slopes_photon_converted_15 = { 0.01450, 0.01410, 0.01410, 0.01860, 0.01990, 0.0, 0.02120, 0.02610, 0.03260, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_converted_20 = { 0.01600, 0.01560, 0.01580, 0.02130, 0.02320, 0.0, 0.02450, 0.03000, 0.03840, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_converted_25 = { 0.01630, 0.01600, 0.01620, 0.02210, 0.02420, 0.0, 0.02560, 0.03140, 0.04060, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_converted_30 = { 0.01630, 0.01600, 0.01630, 0.02240, 0.02460, 0.0, 0.02610, 0.03190, 0.04150, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_converted_35 = { 0.01660, 0.01600, 0.01630, 0.02240, 0.02470, 0.0, 0.02640, 0.03210, 0.04190, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_converted_40 = { 0.01610, 0.01590, 0.01620, 0.02250, 0.02480, 0.0, 0.02650, 0.03220, 0.04210, 0.0 };

    m_mc_rel17_leakage_correction_offsets_photon_converted_15 = { 36.50, 61.80, 176.90, 206.40, 300.70, 0.0, 277.40, 91.70, 126.60, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_converted_20 = { 93.30, 101.40, 270.60, 369.10, 514.70, 0.0, 586.10, 160.80, 193.80, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_converted_25 = { 195.80, 166.20, 386.50, 472.30, 637.30, 0.0, 739.40, 207.60, 240.60, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_converted_30 = { 286.60, 241.60, 501.60, 570.70, 739.50, 0.0, 860.00, 264.50, 270.40, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_converted_35 = { 329.90, 314.70, 585.60, 655.60, 835.70, 0.0, 934.30, 291.50, 291.90, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_converted_40 = { 478.70, 383.80, 679.20, 725.70, 938.70, 0.0, 999.30, 322.80, 316.20, 0.0 };

    m_mc_rel17_leakage_correction_slopes_photon_unconverted_15 = { 0.01480, 0.01410, 0.01400, 0.01820, 0.01950, 0.0, 0.02140, 0.02660, 0.03250, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_20 = { 0.01630, 0.01560, 0.01560, 0.02060, 0.02240, 0.0, 0.02480, 0.03060, 0.03830, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_25 = { 0.01670, 0.01610, 0.01620, 0.02140, 0.02330, 0.0, 0.02590, 0.03200, 0.04050, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_30 = { 0.01690, 0.01630, 0.01640, 0.02170, 0.02360, 0.0, 0.02630, 0.03260, 0.04140, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_35 = { 0.01700, 0.01640, 0.01650, 0.02180, 0.02380, 0.0, 0.02650, 0.03280, 0.04180, 0.0 };
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_40 = { 0.01710, 0.01640, 0.01650, 0.02190, 0.02390, 0.0, 0.02660, 0.03290, 0.04200, 0.0 };

    m_mc_rel17_leakage_correction_offsets_photon_unconverted_15 = { -27.80, 3.80, 67.50, 80.90, 114.90, 0.0, 82.60, 2.10, 39.80, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_20 = { -17.70, 12.60, 97.80, 126.50, 186.20, 0.0, 200.80, 24.00, 62.30, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_25 = { -8.60, 20.30, 118.20, 161.80, 244.30, 0.0, 271.80, 39.80, 79.10, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_30 = { 5.40, 33.80, 141.60, 199.50, 295.40, 0.0, 336.50, 64.80, 90.40, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_35 = { 9.60, 47.80, 154.10, 231.10, 346.10, 0.0, 384.60, 77.80, 96.90, 0.0 };
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_40 = { 13.30, 62.00, 177.00, 267.10, 406.20, 0.0, 419.80, 89.40, 105.90, 0.0 };
  }

  void IsolationCorrection::set2012Corr() {
    if( !m_corr_file.empty()){
      load2012Corr();
    }else{
      ATH_MSG_WARNING("Correction file for 2017 data/mc not specified, tool not initialized for 2017 corrections\n");
    }
  }

  void IsolationCorrection::load2012Corr() {
    std::unique_ptr<TFile> file_ptleakagecorr(TFile::Open(m_corr_file.c_str(), "READ"));
    if(!file_ptleakagecorr){
    	ATH_MSG_WARNING("Correction file for 2012 data/mc not found, tool not initialized for 2012 corrections\n");
      m_corr_file = "";
      return;
    }else{
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin0_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin1_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin2_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin3_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin4_extrap") );
      m_graph_cone40_photon_unconverted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin6_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin7_extrap") );
      m_graph_cone40_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_unconverted_etabin8_extrap") );
      m_graph_cone40_photon_unconverted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin0_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin1_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin2_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin3_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin4_extrap") );
      m_graph_cone40_photon_converted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin6_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin7_extrap") );
      m_graph_cone40_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_photon_converted_etabin8_extrap") );
      m_graph_cone40_photon_converted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin0_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin1_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin2_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin3_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin4_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin5_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin6_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin7_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin8_extrap") );
      m_graph_cone40_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone40_electron_etabin9_extrap") );

      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin0_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin1_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin2_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin3_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin4_extrap") );
      m_graph_cone30_photon_unconverted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin6_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin7_extrap") );
      m_graph_cone30_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_unconverted_etabin8_extrap") );
      m_graph_cone30_photon_unconverted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin0_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin1_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin2_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin3_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin4_extrap") );
      m_graph_cone30_photon_converted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin6_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin7_extrap") );
      m_graph_cone30_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_photon_converted_etabin8_extrap") );
      m_graph_cone30_photon_converted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin0_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin1_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin2_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin3_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin4_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin5_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin6_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin7_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin8_extrap") );
      m_graph_cone30_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone30_electron_etabin9_extrap") );

      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin0_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin1_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin2_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin3_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin4_extrap") );
      m_graph_cone20_photon_unconverted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin6_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin7_extrap") );
      m_graph_cone20_photon_unconverted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_unconverted_etabin8_extrap") );
      m_graph_cone20_photon_unconverted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin0_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin1_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin2_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin3_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin4_extrap") );
      m_graph_cone20_photon_converted.push_back( new TGraph() );//No corrections for photons in the crack region
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin6_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin7_extrap") );
      m_graph_cone20_photon_converted.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_photon_converted_etabin8_extrap") );
      m_graph_cone20_photon_converted.push_back( new TGraph() );//No corrections for photons with |eta|>2.37

      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin0_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin1_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin2_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin3_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin4_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin5_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin6_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin7_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin8_extrap") );
      m_graph_cone20_electron.push_back( (TGraph*) file_ptleakagecorr->Get("graph_cone20_electron_etabin9_extrap") );
    }
  }

  void IsolationCorrection::set2015Corr() {
    if( !m_corr_file.empty()){
      load2015Corr();
    }else{
      ATH_MSG_WARNING("Correction file for 2015 data/mc not specified, tool not initialized for 2015 corrections\n");
    }
  }

  void IsolationCorrection::load2015Corr() {
    std::unique_ptr<TFile> file_ptleakagecorr(TFile::Open(m_corr_file.c_str(), "READ"));
    if(!file_ptleakagecorr){
      ATH_MSG_WARNING("Correction file for 2015 data/mc not found, "<<m_corr_file<<". tool not initialized for 2015 corrections\n");
      m_corr_file = "";
    }else{
      if(!file_ptleakagecorr->GetListOfKeys()->Contains("mean_f_topoetcone40_eta_1.15_1.37_converted_ok")){
        ATH_MSG_ERROR("Correction file for 2015 data/mc is not right, "<<m_corr_file<<". Tool not initialized for 2015 corrections\n");
        m_corr_file = "";
      } else {

        // ******************************
	// Global fit corrections *******
	// ******************************
	
	static const int nEta = 10;
	static const TString etaLab[nEta] = { "0.0_0.1", "0.1_0.6", "0.6_0.8", "0.8_1.15", "1.15_1.37", "1.37_1.52", "1.52_1.81", "1.81_2.01", "2.01_2.37", "2.37_2.47" };
	static const TString pLab[5]      = { "_unconverted", "_converted_ok", "_converted_trouble", "_author_1_electron", "_author_16_electron" };
	static const TString dRLab[3]     = { "20", "30", "40" };

	m_function_2015_cone40_photon_unconverted.resize(nEta);
	m_function_2015_cone30_photon_unconverted.resize(nEta);
	m_function_2015_cone20_photon_unconverted.resize(nEta);
	m_function_2015_cone40_photon_converted_ok.resize(nEta);
	m_function_2015_cone30_photon_converted_ok.resize(nEta);
	m_function_2015_cone20_photon_converted_ok.resize(nEta);
	m_function_2015_cone40_photon_converted_trouble.resize(nEta);
	m_function_2015_cone30_photon_converted_trouble.resize(nEta);
	m_function_2015_cone20_photon_converted_trouble.resize(nEta);

	m_function_2015_cone40_author_1_electron.resize(nEta);
	m_function_2015_cone30_author_1_electron.resize(nEta);
	m_function_2015_cone20_author_1_electron.resize(nEta);
	m_function_2015_cone40_author_16_electron.resize(nEta);
	m_function_2015_cone30_author_16_electron.resize(nEta);
	m_function_2015_cone20_author_16_electron.resize(nEta);

	if (m_trouble_categories) {
	  m_graph_histoMean_2015_cone40_photon_converted_trouble.resize(nEta);
	  m_graph_histoMean_2015_cone30_photon_converted_trouble.resize(nEta);
	  m_graph_histoMean_2015_cone20_photon_converted_trouble.resize(nEta);

	  m_graph_histoMean_2015_cone40_author_16_electron.resize(nEta);
	  m_graph_histoMean_2015_cone30_author_16_electron.resize(nEta);
	  m_graph_histoMean_2015_cone20_author_16_electron.resize(nEta);
	}
	
	for (int iPart = 0; iPart < 5; iPart++) {

	  for (int iEta = 0; iEta < 10; iEta++) {

	    // No parametrisation in the crack or above |eta| = 2.37 for photons
	    if (iPart < 3 && (iEta == 5 || iEta == 9))
	      continue;

	    for (int idR = 0; idR < 3; idR++) {
	      // The LogLog fit is a quick-and-dirty solution to get rid of negative leakage at low ET for unconverted and converted_ok photons
	      TString fN = (m_useLogLogFit && iPart < 2) ? "mean_flog_histo_topoetcone" : "mean_f_topoetcone";
	      fN += dRLab[idR]; fN += "_eta_"; fN += etaLab[iEta]; fN += pLab[iPart];
	      TString gN;
	      // Trouble categories are converted_trouble photons and author 16 electrons. Below 250 GeV, one uses the mean of the leakage histo
	      // instead of a fit to the mean of the CB vs ET 
	      if (m_trouble_categories && (iPart == 2 || iPart == 4)) {
		gN = "mean_g_histo_topoetcone"; gN += dRLab[idR]; gN += "_eta_"; gN += etaLab[iEta]; gN += pLab[iPart];
	      }
	      if (idR == 2) {
		if (iPart == 0) {
		  m_function_2015_cone40_photon_unconverted.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone40_photon_unconverted.at(iEta));
		} else if (iPart == 1) {
		  m_function_2015_cone40_photon_converted_ok.at(iEta)             = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone40_photon_converted_ok.at(iEta));
		} else if (iPart == 2) {
		  m_function_2015_cone40_photon_converted_trouble.at(iEta)        = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone40_photon_converted_trouble.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone40_photon_converted_trouble.at(iEta) = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone40_photon_converted_trouble.at(iEta));
		  }
		} else if (iPart == 3) {
		  m_function_2015_cone40_author_1_electron.at(iEta)               = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone40_author_1_electron.at(iEta));
		} else if (iPart == 4) {
		  m_function_2015_cone40_author_16_electron.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone40_author_16_electron.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone40_author_16_electron.at(iEta)     = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone40_author_16_electron.at(iEta));
		  }
		}

	      } else if (idR == 1) {
		if (iPart == 0) {
		  m_function_2015_cone30_photon_unconverted.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone30_photon_unconverted.at(iEta));
		} else if (iPart == 1) {
		  m_function_2015_cone30_photon_converted_ok.at(iEta)             = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone30_photon_converted_ok.at(iEta));
		} else if (iPart == 2) {
		  m_function_2015_cone30_photon_converted_trouble.at(iEta)        = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone30_photon_converted_trouble.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone30_photon_converted_trouble.at(iEta) = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone30_photon_converted_trouble.at(iEta));
		  }
		} else if (iPart == 3) {
		  m_function_2015_cone30_author_1_electron.at(iEta)               = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone30_author_1_electron.at(iEta));
		} else if (iPart == 4) {
		  m_function_2015_cone30_author_16_electron.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone30_author_16_electron.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone30_author_16_electron.at(iEta)     = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone30_author_16_electron.at(iEta));
		  }
		}

	      } else {
		if (iPart == 0) {
		  m_function_2015_cone20_photon_unconverted.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone20_photon_unconverted.at(iEta));
		} else if (iPart == 1) {
		  m_function_2015_cone20_photon_converted_ok.at(iEta)             = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone20_photon_converted_ok.at(iEta));
		} else if (iPart == 2) {
		  m_function_2015_cone20_photon_converted_trouble.at(iEta)        = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone20_photon_converted_trouble.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone20_photon_converted_trouble.at(iEta) = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone20_photon_converted_trouble.at(iEta));
		  }
		}  else if (iPart == 3) {
		  m_function_2015_cone20_author_1_electron.at(iEta)               = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone20_author_1_electron.at(iEta));
		} else if (iPart == 4) {
		  m_function_2015_cone20_author_16_electron.at(iEta)              = ((TF1*)file_ptleakagecorr->Get(fN));
		  ATH_MSG_DEBUG("TF1 name : " << fN << " ptr = " << m_function_2015_cone20_author_16_electron.at(iEta));
		  if (m_trouble_categories) {
		    m_graph_histoMean_2015_cone20_author_16_electron.at(iEta)     = ((TGraph*)file_ptleakagecorr->Get(gN));
		    ATH_MSG_DEBUG("TGraph name (trouble categories at low ET) : " << gN << " ptr = " << m_graph_histoMean_2015_cone20_author_16_electron.at(iEta));
		  }
		}
	      }

	    } // dR bin

	  }   // eta bin

	}     // particle type

      }       // root file : no good format
    }         // root file not found
  }

  void IsolationCorrection::setDDCorr() {
    if( !m_corr_ddshift_file.empty() && !m_corr_ddsmearing_file.empty()){
      loadDDCorr();
    }else{
      ATH_MSG_WARNING("Data-driven correction files not specified, tool not initialized for data-driven corrections\n");
    }
  }

  void IsolationCorrection::loadDDCorr() {
    std::unique_ptr< TFile > file_ddshift_corr( TFile::Open( m_corr_ddshift_file.c_str(), "READ" ) );
    std::unique_ptr< TFile > file_ddsmearingcorr( TFile::Open( m_corr_ddsmearing_file.c_str(), "READ" ) );
    
    if(!file_ddshift_corr || !file_ddsmearingcorr){
      ATH_MSG_WARNING("Correction file for data-driven corrections not found, tool not initialized for data-driven corrections\n");
      m_corr_ddshift_file = "";
      m_corr_ddsmearing_file = "";
    }else{

      // **********************************
      // Data driven corrections **********
      // https://cds.cern.ch/record/2008664
      // **********************************

      // Photon shift corrections
      std::vector< std::shared_ptr<TGraphAsymmErrors> > graph_shift;
      // Photon smearing corrections (to be applied in end caps only)
      std::vector< std::shared_ptr<TGraphAsymmErrors> > graph_smearing;
      for (int ig = 0; ig <= 13; ig++) {
	graph_shift.emplace_back( dynamic_cast<TGraphAsymmErrors*>(file_ddshift_corr->Get(Form("graph_%i",ig))) );
	graph_smearing.emplace_back( dynamic_cast<TGraphAsymmErrors*>(file_ddsmearingcorr->Get(Form("graph_%i",ig))) );
      }
      for (int ig = 0; ig <= 13; ig++) {
	if (ig <= 7)
	  m_graph_dd_cone40_photon_shift.push_back( graph_shift.at(ig)->GetFunction("f") );
	else
	  m_graph_dd_cone40_photon_shift.push_back( graph_shift.at(ig)->GetFunction("f_2") );
	m_graph_dd_cone40_photon_smearing.push_back( graph_smearing.at(ig)->GetFunction("f_3") );
      }

      for (const auto& gr : graph_shift) {
        if (gr == nullptr)
	  ATH_MSG_ERROR("Null pointer for one of the DD correction graphs");
      }
      for (const auto& gr : graph_smearing) {
        if (gr == nullptr)
	  ATH_MSG_ERROR("Null pointer for one of the smearing graphs");
      }
    }
  }

  template <class T> void IsolationCorrection::FreeClear( T & cntr ) {
    ATH_MSG_DEBUG("FreeClearing the container " << cntr.size());
    for ( typename T::iterator it = cntr.begin();
	  it != cntr.end(); ++it ) {
      if (*it) {
	if (msgLvl(MSG::DEBUG)) {
	  ATH_MSG_DEBUG("Deleting " << *it << " " << typeid(*it).name());
	  ATH_MSG_DEBUG((*it)->GetName());
	  (*it)->Print();
	}
	delete * it;
      }
    }
    cntr.clear();
  }

  IsolationCorrection::~IsolationCorrection() {
    m_eta_bins_fine.clear();
    m_eta_bins_coarse.clear();

    if (m_tool_ver == REL17_2) {
      FreeClear( m_graph_cone40_photon_unconverted );
      FreeClear( m_graph_cone30_photon_unconverted );
      FreeClear( m_graph_cone20_photon_unconverted );

      FreeClear( m_graph_cone40_photon_converted );
      FreeClear( m_graph_cone30_photon_converted );
      FreeClear( m_graph_cone20_photon_converted );

      FreeClear( m_graph_cone40_electron );
      FreeClear( m_graph_cone30_electron );
      FreeClear( m_graph_cone20_electron );

      FreeClear( m_graph_dd_cone40_photon_shift );
      FreeClear( m_graph_dd_cone40_photon_smearing );

    } else if (m_tool_ver == REL20_2 || m_tool_ver == REL21 || m_tool_ver == REL22) {

      //---- Rel 20_2 pT leakage correction file

      FreeClear( m_function_2015_cone40_photon_unconverted );
      FreeClear( m_function_2015_cone30_photon_unconverted );
      FreeClear( m_function_2015_cone20_photon_unconverted );

      FreeClear( m_function_2015_cone40_photon_converted_ok );
      FreeClear( m_function_2015_cone30_photon_converted_ok );
      FreeClear( m_function_2015_cone20_photon_converted_ok );

      FreeClear( m_function_2015_cone40_photon_converted_trouble );
      FreeClear( m_function_2015_cone30_photon_converted_trouble );
      FreeClear( m_function_2015_cone20_photon_converted_trouble );

      FreeClear( m_function_2015_cone40_author_1_electron );
      FreeClear( m_function_2015_cone30_author_1_electron );
      FreeClear( m_function_2015_cone20_author_1_electron );

      FreeClear( m_function_2015_cone40_author_16_electron );
      FreeClear( m_function_2015_cone30_author_16_electron );
      FreeClear( m_function_2015_cone20_author_16_electron );


      //---- Rel 20_2 pT leakage correction with histogram mean file

      FreeClear( m_graph_histoMean_2015_cone40_photon_converted_trouble );
      FreeClear( m_graph_histoMean_2015_cone30_photon_converted_trouble );
      FreeClear( m_graph_histoMean_2015_cone20_photon_converted_trouble );

      FreeClear( m_graph_histoMean_2015_cone40_author_16_electron );
      FreeClear( m_graph_histoMean_2015_cone30_author_16_electron );
      FreeClear( m_graph_histoMean_2015_cone20_author_16_electron );

    }

    if (!m_graph_dd_2015_2016_cone40_unconv_photon_shift.empty()) {
      FreeClear(m_graph_dd_2015_2016_cone40_unconv_photon_shift);
      FreeClear(m_graph_dd_2015_2016_cone40_conv_photon_shift);
      FreeClear(m_graph_dd_2015_2016_cone20_unconv_photon_shift);
      FreeClear(m_graph_dd_2015_2016_cone20_conv_photon_shift);
    }
    if (!m_graph_dd_2017_cone40_unconv_photon_shift.empty()) {
      FreeClear(m_graph_dd_2017_cone40_unconv_photon_shift);
      FreeClear(m_graph_dd_2017_cone40_conv_photon_shift);
      FreeClear(m_graph_dd_2017_cone20_unconv_photon_shift);
      FreeClear(m_graph_dd_2017_cone20_conv_photon_shift);
    }

    // For etcone, we only have very old corrections...
    m_mc_rel17_leakage_correction_slopes_electron_15.clear();
    m_mc_rel17_leakage_correction_slopes_electron_20.clear();
    m_mc_rel17_leakage_correction_slopes_electron_25.clear();
    m_mc_rel17_leakage_correction_slopes_electron_30.clear();
    m_mc_rel17_leakage_correction_slopes_electron_35.clear();
    m_mc_rel17_leakage_correction_slopes_electron_40.clear();

    m_mc_rel17_leakage_correction_offsets_electron_15.clear();
    m_mc_rel17_leakage_correction_offsets_electron_20.clear();
    m_mc_rel17_leakage_correction_offsets_electron_25.clear();
    m_mc_rel17_leakage_correction_offsets_electron_30.clear();
    m_mc_rel17_leakage_correction_offsets_electron_35.clear();
    m_mc_rel17_leakage_correction_offsets_electron_40.clear();

    // photons: last eta bin isn't used
    m_mc_rel17_leakage_correction_slopes_photon_converted_15.clear();
    m_mc_rel17_leakage_correction_slopes_photon_converted_20.clear();
    m_mc_rel17_leakage_correction_slopes_photon_converted_25.clear();
    m_mc_rel17_leakage_correction_slopes_photon_converted_30.clear();
    m_mc_rel17_leakage_correction_slopes_photon_converted_35.clear();
    m_mc_rel17_leakage_correction_slopes_photon_converted_40.clear();

    m_mc_rel17_leakage_correction_offsets_photon_converted_15.clear();
    m_mc_rel17_leakage_correction_offsets_photon_converted_20.clear();
    m_mc_rel17_leakage_correction_offsets_photon_converted_25.clear();
    m_mc_rel17_leakage_correction_offsets_photon_converted_30.clear();
    m_mc_rel17_leakage_correction_offsets_photon_converted_35.clear();
    m_mc_rel17_leakage_correction_offsets_photon_converted_40.clear();

    m_mc_rel17_leakage_correction_slopes_photon_unconverted_15.clear();
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_20.clear();
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_25.clear();
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_30.clear();
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_35.clear();
    m_mc_rel17_leakage_correction_slopes_photon_unconverted_40.clear();

    m_mc_rel17_leakage_correction_offsets_photon_unconverted_15.clear();
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_20.clear();
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_25.clear();
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_30.clear();
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_35.clear();
    m_mc_rel17_leakage_correction_offsets_photon_unconverted_40.clear();

    // -------------------------------------------------------------------------------------------
    //Delete m_shower instance
    delete m_shower;

  }

  //-----------------------------------------------------------------------
  // User function
  // Returns the pt leakage corrected isolation
  //
  float IsolationCorrection::GetPtCorrectedIsolation(float energy,
						     float etaS2,
						     float etaPointing,
						     float etaCluster,
						     float radius,
						     bool is_mc,
						     float Etcone_value,
						     bool isConversion,
						     ParticleType parttype){

    float pt_correction = GetPtCorrection(energy, etaS2, etaPointing, etaCluster, radius, is_mc, isConversion, parttype);
    return Etcone_value - pt_correction;
  }
  //-----------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // User function
  // Returns the pt leakage corrected topo isolation
  //
  float IsolationCorrection::GetPtCorrectedTopoIsolation(float energy,
							 float etaS2,
							 float etaPointing,
							 float etaCluster,
							 float radius,
							 bool is_mc,
							 float Etcone_value,
							 bool isConversion,
							 ParticleType parttype,
							 Version ver){

    float pt_correction = GetPtCorrectionTopo(energy, etaS2, etaPointing, etaCluster, radius, is_mc, isConversion, parttype, ver);
    return Etcone_value - pt_correction;
  }
  //-----------------------------------------------------------------------


  // ***************************************************************************************************************************
  // ***************************************************************************************************************************
  // Internal Functions Below.
  // ***************************************************************************************************************************
  // ***************************************************************************************************************************

  //-----------------------------------------------------------------------
  // Internal function
  // Gets the pt correction factor
  //
  float IsolationCorrection::GetPtCorrection(  float energy,
					       float etaS2, float etaPointing, float etaCluster,
					       float radius,
					       bool is_mc,
					       bool isConversion, ParticleType parttype) const {

    int newrad = GetRadius(radius);
    std::vector<float> mc_correction_slopes_ptr;
    std::vector<float> mc_correction_offsets_ptr;
    std::vector<float> data_correction_slopes_ptr;

    //TODO: when implementing ptcorr for etconeXX change this

    switch(newrad){
    case 20:
      if (parttype == PHOTON) {
	// photons
	if(isConversion) {
	  // converted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_converted_20;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_converted_20;
	} else {
	  // unconverted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_unconverted_20;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_unconverted_20;
	}
      } else {
	// electrons
	mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_electron_20;
	mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_electron_20;
      }
      break;
    case 30:
      if (parttype == PHOTON) {
	// photons
	if(isConversion) {
	  // converted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_converted_30;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_converted_30;
	} else {
	  // unconverted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_unconverted_30;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_unconverted_30;
	}
      } else {
	// electrons
	mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_electron_30;
	mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_electron_30;
      }
      break;
    case 40:
      if (parttype == PHOTON) {
	// photons
	if(isConversion) {
	  // converted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_converted_40;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_converted_40;
	} else {
	  // unconverted
	  mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_photon_unconverted_40;
	  mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_photon_unconverted_40;
	}
      } else {
	// electrons
	mc_correction_slopes_ptr = m_mc_rel17_leakage_correction_slopes_electron_40;
	mc_correction_offsets_ptr = m_mc_rel17_leakage_correction_offsets_electron_40;
      }
      break;
    default:
    	ATH_MSG_WARNING("Unable to retrieve leakage correction for topoIso cone with radius = " << radius << ".\n--- Radii must be one of {.20, .30, .40} OR {20, 30, 40}.\n");
      return 0.;
    }

    float scale_factor = GetPtCorrectionFactor(etaS2, mc_correction_slopes_ptr, data_correction_slopes_ptr);
    float offset = 0.;
    if (!mc_correction_offsets_ptr.empty()) offset = GetPtCorrectionFactor(etaS2, mc_correction_offsets_ptr);

    //avoid warnings
    if(is_mc) is_mc = true;
    return offset + GetPtCorrectionValue(energy, etaPointing, etaCluster, scale_factor);

  }
  //-----------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // Internal function
  // Gets the pt correction factor for topoIso
  //
  float IsolationCorrection::GetPtCorrectionTopo(float energy,
						 float etaS2, float etaPointing, float etaCluster,
						 float radius,
						 bool /*is_mc*/,
						 bool isConversion, ParticleType parttype, Version ver,
						 int convFlag_int, int author, float conv_radius, float conv_ratio) const {
    double correction_value = 0.;
    if (ver== REL17_2) {
      correction_value = GetPtCorrection_FromGraph(energy,etaS2,etaPointing,etaCluster,radius,isConversion,parttype);
    } else if (m_tool_ver == REL20_2 || m_tool_ver == REL21 || m_tool_ver == REL22){
      correction_value = GetPtCorrection_FromGraph_2015(energy, etaS2, radius, convFlag_int, author, conv_radius, conv_ratio, parttype);
    }

    return correction_value;
  }
  //-----------------------------------------------------------------------


  //-----------------------------------------------------------------------
  // Internal function
  // Used to retrieve the correct radius
  int IsolationCorrection::GetRadius(float radius) {
    int newrad = 0;
    // avoid roundoff errors by adding 0.1
    if(radius < 1)    newrad = (int)(radius * 100 + 0.1);
    else                        newrad = (int)radius;
    return newrad;
  }
  //-----------------------------------------------------------------------


  //-----------------------------------------------------------------------
  // Internal function
  // Used to retrieve the correct (fine) eta bin number
  //
  int IsolationCorrection::GetEtaBinFine(float eta) const {
    int eta_bin=-1;
    float fabs_eta = fabs(eta);
    for (unsigned int i=0; i < m_nBinsEtaFine; ++i) {
      if ((fabs_eta >= m_eta_bins_fine[i]) && (fabs_eta < m_eta_bins_fine[i+1])) {
	eta_bin = i;
	break;
      }
    }
    return eta_bin;
  }
  //-----------------------------------------------------------------------


  //-----------------------------------------------------------------------
  // Internal function
  // Used to retrieve the correct (coarse) eta bin number
  //
  int IsolationCorrection::GetEtaBinCoarse(float eta) const {
    int eta_bin=-1;
    float fabs_eta = fabs(eta);
    for (unsigned int i=0; i < m_nBinsEtaCoarse; ++i) {
      if ((fabs_eta >= m_eta_bins_coarse[i]) && (fabs_eta < m_eta_bins_coarse[i+1])) {
	eta_bin = i;
	break;
      }
    }
    return eta_bin;
  }
  //-----------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // Internal function
  // Returns the appropriate corrections value
  //
  float IsolationCorrection::GetPtCorrectionFactor(float eta,
						   const std::vector<float>& mc_leakage_corrections_ptr,
						   const std::vector<float>& data_leakage_corrections_ptr) const {
    if(mc_leakage_corrections_ptr.empty() && data_leakage_corrections_ptr.empty())
      return 0.;

    int eta_bin_fine = GetEtaBinFine(eta);
    int eta_bin_coarse = GetEtaBinCoarse(eta);

    float correction = 0.;
    if (!mc_leakage_corrections_ptr.empty() && (eta_bin_fine >= 0)) correction += mc_leakage_corrections_ptr[eta_bin_fine];
    if (!data_leakage_corrections_ptr.empty() && (eta_bin_coarse >= 0)) correction += data_leakage_corrections_ptr[eta_bin_coarse];

    return correction;
  }
  //-----------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // Internal function
  // Does the final pt scaling
  float IsolationCorrection::GetPtCorrectionValue(float energy, float etaPointing, float etaCluster, float scale_factor) {
    // apply the correction to et
    double etaForPt = ((fabs(etaPointing - etaCluster) < 0.15) ? etaPointing : etaCluster);
    double et = (fabs(etaForPt)<99.) ? energy/cosh(etaForPt) : 0.;

    return scale_factor * et;
  }
  //-----------------------------------------------------------------------


  //-----------------------------------------------------------------------
  // Internal function
  // Does the correction for REL17_2 from TGraph stored into isolation_leakage_corrections.root file
  // Returns the correction value in MeV
  float IsolationCorrection::GetPtCorrection_FromGraph(float energy,float etaS2,float etaPointing,float etaCluster,float radius,bool isConversion,ParticleType parttype) const
  {
    int newrad = GetRadius(radius);
    double etaForPt = ((fabs(etaPointing - etaCluster) < 0.15) ? etaPointing : etaCluster);
    double et = (fabs(etaForPt)<99.) ? energy/cosh(etaForPt) : 0.;
    int etabin = GetEtaBinFine(etaS2);
    if( m_corr_file.empty() ){
      ATH_MSG_WARNING("IsolationCorrection::GetPtCorrection_FromGraph: the file containing the isolation leakage corrections is not initialized.\nNo correction is applied.\n");
      return 0;
    }
    if (etabin < 0) return 0; // must have eta in fiducial region

    if( (etabin == 9 || etabin == 5) && parttype == PHOTON ){
      //No correction for photons with |etaS2|>2.37 or 1.37<|etaS2|<1.52
      return 0.;
    }
    double correction_value_mev = -1e6;
    switch(newrad){
    case 20:
      if(parttype == ELECTRON)
	correction_value_mev = GeV*m_graph_cone20_electron[etabin]->Eval(et/GeV);
      else
	if(isConversion) correction_value_mev = GeV*m_graph_cone20_photon_converted[etabin]->Eval(et/GeV);
	else correction_value_mev = GeV*m_graph_cone20_photon_unconverted[etabin]->Eval(et/GeV);
      break;
    case 30:
      if(parttype == ELECTRON)
	correction_value_mev = GeV*m_graph_cone30_electron[etabin]->Eval(et/GeV);
      else
	if(isConversion) correction_value_mev = GeV*m_graph_cone30_photon_converted[etabin]->Eval(et/GeV);
	else correction_value_mev = GeV*m_graph_cone30_photon_unconverted[etabin]->Eval(et/GeV);
      break;
    case 40:
      if(parttype == ELECTRON)
	correction_value_mev = GeV*m_graph_cone40_electron[etabin]->Eval(et/GeV);
      else
	if(isConversion) correction_value_mev = GeV*m_graph_cone40_photon_converted[etabin]->Eval(et/GeV);
	else correction_value_mev = GeV*m_graph_cone40_photon_unconverted[etabin]->Eval(et/GeV);
      break;
    default:
    	ATH_MSG_WARNING("Unable to retrieve leakage correction for topoIso cone with radius = " << radius << ".\n--- Radii must be one of {.20, .30, .40} OR {20, 30, 40}.\n");
      return 0.;
    }
    return correction_value_mev;

  }

  //-----------------------------------------------------------------------
  // Internal function
  // Does the correction for REL18 from TGraph stored into isolation_ptcorrections.root file
  // Returns the correction value in MeV

  int IsolationCorrection::GetConversionType(int conversion_flag, float conv_radius, float conv_ratio) {
    // ph_convFlag==0
    if(conversion_flag == 0) return 0;
    // ((ph_convFlag==1)||(ph_convFlag==2&&ph_ptconv_ratio>0.3)||(ph_convFlag==2&&ph_ptconv_ratio<0.3&&ph_truth_Rconv>140.))
    if( (conversion_flag == 1) || ((conversion_flag == 2 && conv_ratio > 0.3) || (conversion_flag == 2 && conv_ratio < 0.3 && conv_radius > 140.)) ) return 1;
    // (ph_convFlag==2&&ph_ptconv_ratio<0.3&&ph_truth_Rconv<140.)
    if(conversion_flag == 2 && conv_radius < 140.) return 2;
    return 0;
  }

  float IsolationCorrection::GetPtCorrection_FromGraph_2015(float energy, float etaS2, float radius, int conversion_flag, int author, float conv_radius, float conv_ratio, ParticleType parttype) const {
    int newrad = GetRadius(radius);
    double etaForPt = etaS2; //((fabs(etaPointing - etaCluster) < 0.15) ? etaPointing : etaCluster);
    double et = energy/cosh(etaForPt); //(fabs(etaForPt)<99.) ? energy/cosh(etaForPt) : 0.;
    int etabin = GetEtaBinFine(etaS2);

    int conversion_type = 0;
    if(parttype == PHOTON) conversion_type = GetConversionType(conversion_flag, conv_radius, conv_ratio);

    // for test
    if (m_forcePartType && parttype == PHOTON) conversion_type = conv_radius > 800 ? 0 : (conv_radius > 140 ? 1 : 2);

    if( m_corr_file.empty() ){
      ATH_MSG_WARNING("In IsolationCorrection::GetPtCorrection_FromGraph_2015: the file containing the isolation leakage corrections is not initialized.\nNo correction is applied.\n");
      return 0;
    }

    if (etabin < 0) return 0; // must have eta in fiducial region
    if(parttype == PHOTON) if( (etabin == 9 || etabin == 5) ) return 0; // No correction for photons with |etaS2|>2.37 or 1.37<|etaS2|<1.52

    double correction_value_mev = 0;

    float pt_threshold = 250000.;

    //if(parttype == PHOTON) ATH_MSG_INFO("Applying 2015 corrections photon etaBin "<<etabin<<" Pt: "<<et<<" || "<< conversion_type <<" "<< author <<" "<< conv_radius <<" "<< conv_ratio <<"\n");
    //else ATH_MSG_INFO("Applying 2015 corrections electron etaBin "<<etabin<<" Pt: "<<et<<" || "<< conversion_type <<" "<< author <<" "<< conv_radius <<" "<< conv_ratio <<"\n");
    //TODO: switch author_1 with author_16 in the right places, invert function and histomean
    switch(newrad){
      case 20:
        if(parttype == ELECTRON){
          if(author == 1) correction_value_mev = m_function_2015_cone20_author_1_electron[etabin]->Eval(et);
          if(author == 16){
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone20_author_16_electron[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone20_author_16_electron[etabin]->Eval(et);
          }
        }else{
          if(conversion_type == 0) correction_value_mev = m_function_2015_cone20_photon_unconverted[etabin]->Eval(et);
          if(conversion_type == 1) correction_value_mev = m_function_2015_cone20_photon_converted_ok[etabin]->Eval(et);
          if(conversion_type == 2){
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone20_photon_converted_trouble[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone20_photon_converted_trouble[etabin]->Eval(et);
          }
        }
        break;
      case 30:
        if(parttype == ELECTRON){
          if(author == 1) correction_value_mev = m_function_2015_cone30_author_1_electron[etabin]->Eval(et);
          if(author == 16) {
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone30_author_16_electron[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone30_author_16_electron[etabin]->Eval(et);
          }
        }else{
          if(conversion_type == 0) correction_value_mev = m_function_2015_cone30_photon_unconverted[etabin]->Eval(et);
          if(conversion_type == 1) correction_value_mev = m_function_2015_cone30_photon_converted_ok[etabin]->Eval(et);
          if(conversion_type == 2){
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone30_photon_converted_trouble[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone30_photon_converted_trouble[etabin]->Eval(et);
          }
        }
        break;
      case 40:
        if(parttype == ELECTRON){
          if(author == 1) correction_value_mev = m_function_2015_cone40_author_1_electron[etabin]->Eval(et);
          if(author == 16){
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone40_author_16_electron[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone40_author_16_electron[etabin]->Eval(et);
          }
        }else{
          if(conversion_type == 0) correction_value_mev = m_function_2015_cone40_photon_unconverted[etabin]->Eval(et);
          if(conversion_type == 1) correction_value_mev = m_function_2015_cone40_photon_converted_ok[etabin]->Eval(et);
          if(conversion_type == 2){
            if(m_trouble_categories && et < pt_threshold) correction_value_mev = m_graph_histoMean_2015_cone40_photon_converted_trouble[etabin]->Eval(et);
            else correction_value_mev = m_function_2015_cone40_photon_converted_trouble[etabin]->Eval(et);
          }
        }
        break;
      default:
        ATH_MSG_WARNING("Unable to retrieve leakage correction for topoIso cone with radius = " << radius << ".\n--- Radii must be one of {.20, .30, .40} OR {20, 30, 40}.\n");
        return 0.;
    }
    //ATH_MSG_INFO("  Correction: "<< correction_value_mev <<"\n");
    if (et > 20e3)
      ATH_MSG_VERBOSE("Electron ? " << (parttype == ELECTRON) << " author / conversion type = " << (parttype == ELECTRON ? author : conversion_type) << " et = " << et << " eta = " << etaS2 << " etabin = " << etabin << " correction: "<< correction_value_mev);
    return correction_value_mev;
  }

  void IsolationCorrection::Print() {
    ATH_MSG_INFO("Print properties of the parametrisation");
    for (auto *i : m_function_2015_cone20_photon_unconverted) {
      ATH_MSG_INFO("ptr = " << i);
      if (i) {
	ATH_MSG_INFO(typeid(i).name());
	i->Print();
      }
    }
  }


}
