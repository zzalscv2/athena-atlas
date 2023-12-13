/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 *  JMS Calibration
 *
 *  Author: Jonathan Bossio (jbossios@cern.ch)
 *
 */

#include <TAxis.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <cmath>
#include <utility>

#include "JetCalibTools/CalibrationMethods/JMSCorrection.h"
#include "JetCalibTools/JetCalibUtils.h"
#include "JetCalibTools/RootHelpers.h"
#include "PathResolver/PathResolver.h"


JMSCorrection::JMSCorrection()
  : JetCalibrationStep::JetCalibrationStep(),
    m_config(nullptr), m_jetAlgo(""), m_calibAreaTag(""), m_dev(false)
{ }


JMSCorrection::JMSCorrection(const std::string& name, TEnv* config, TString jetAlgo, TString calibAreaTag, bool dev)
  : JetCalibrationStep::JetCalibrationStep(name.c_str()),
    m_config(config), m_jetAlgo(std::move(jetAlgo)), m_calibAreaTag(std::move(calibAreaTag)), m_dev(dev)
{ }

StatusCode JMSCorrection::initialize() {

  ATH_MSG_INFO("Initializing the JMS Calibration tool");

  if ( !m_config ) { ATH_MSG_FATAL("Config file not specified. Aborting."); return StatusCode::FAILURE; }

  m_jetStartScale = m_config->GetValue("JMSStartingScale","JetEtaJESScaleMomentum");
  m_jetOutScale = m_config->GetValue("JMSOutScale","JetJMSScaleMomentum");
  // Starting pT value to calibrate
  m_pTMinCorr = m_config->GetValue("MinpT",180); 

  m_pTfixed   = m_config->GetValue("pTfixed",false); //small-R:true, large-R:false

  if ( m_jetAlgo.EqualTo("") ) { ATH_MSG_FATAL("No jet algorithm specified. Aborting."); return StatusCode::FAILURE; }


  // Check if we are reading 2D histograms (one per eta bin) or 3D histograms
  // 3D histograms allow for interpolation also across the eta dimension
  // Defaults to "false" (use 2D) for backwards compatibility
  // *Note* that it is assumed the calo and TA masses use the same histogram dimensionality
  m_use3Dhisto = m_config->GetValue("MassCalibrationIs3D",false);

  // Should the mass combination be applied?
  m_combination = m_config->GetValue("Combination",false); // true: turn on combination of calo mass with track-assisted mass
  m_useCorrelatedWeights = m_config->GetValue("UseCorrelatedWeights",false); // true: turn on combination of calo mass with track-assisted mass
  // Should we only apply the combination (e.g after in situ calibration)
  m_onlyCombination = m_config->GetValue("OnlyCombination",false);

  //find the ROOT file containing response histograms, path comes from the config file.
  TString JMSFile;

  if(!m_onlyCombination){
    JMSFile =  m_config->GetValue("MassCalibrationFile","empty");
    if ( JMSFile.EqualTo("empty") ) { 
      ATH_MSG_FATAL("NO JMSFactorsFile specified. Aborting.");
      return StatusCode::FAILURE;
    }
    if(m_dev){
      JMSFile.Remove(0,33);
      JMSFile.Insert(0,"JetCalibTools/");
    }
    else{JMSFile.Insert(14,m_calibAreaTag);}
    TString fileName = PathResolverFindCalibFile(JMSFile.Data());
    std::unique_ptr<TFile> inputFile(TFile::Open(fileName));
    if (!inputFile){
      ATH_MSG_FATAL("Cannot open JMS factors file" << fileName);
      return StatusCode::FAILURE;
    }
    
    if (!m_use3Dhisto) setMassEtaBins( JetCalibUtils::VectorizeD( m_config->GetValue("MassEtaBins","") ) );
    
    //Get a TList of TKeys pointing to the histograms contained in the ROOT file
    inputFile->cd();
    TList *keys = inputFile->GetListOfKeys();
    //fill the names of the TKeys into a vector of TStrings
    TIter ikeys(keys);
    while ( TKey *iterobj = (TKey*)ikeys() ) {
      TString histoName = iterobj->GetName();
      if ( histoName.Contains(m_jetAlgo) ) 
	{ 
	  if (m_use3Dhisto)
	    m_respFactorMass3D = JetCalibUtils::GetHisto3(*inputFile,histoName.Data());
	  else
	    m_respFactorsMass.push_back( JetCalibUtils::GetHisto2(*inputFile,histoName.Data()) );
	}
    }
    
    //Make sure we put something in the vector of TH2Fs or we filled the TH3F
    if ( !m_use3Dhisto && m_respFactorsMass.size() < 3 ) {
      ATH_MSG_FATAL("Vector of mass correction histograms may be empty. Please check your mass calibration file: " << JMSFile);
      return StatusCode::FAILURE;
    }
    else if ( m_use3Dhisto && !m_respFactorMass3D)
      {
	ATH_MSG_FATAL("3D mass correction histogram may be missing.  Please check your mass calibration file: " << JMSFile);
	return StatusCode::FAILURE;
      }
    else ATH_MSG_INFO("JMS Tool has been initialized with binning and eta fit factors from: " << fileName);
    
    // Track-Assisted Jet Mass correction
    m_trackAssistedJetMassCorr = m_config->GetValue("TrackAssistedJetMassCorr",false);
    if(m_trackAssistedJetMassCorr){
      ATH_MSG_INFO("Track Assisted Jet Mass will be calibrated");
      TString JMS_TrackAssisted_File(m_config->GetValue("TrackAssistedMassCalibrationFile","empty"));
      if ( JMS_TrackAssisted_File.EqualTo("empty") ) { 
        ATH_MSG_FATAL("NO Track Assisted Mass Factors File specified. Aborting.");
        return StatusCode::FAILURE;
      }
      if(m_dev){
        JMS_TrackAssisted_File.Remove(0,33);
        JMS_TrackAssisted_File.Insert(0,"JetCalibTools/");
      }
      else{JMS_TrackAssisted_File.Insert(14,m_calibAreaTag);}
      TString file_trkAssisted_Name(PathResolverFindCalibFile(JMS_TrackAssisted_File.Data()));
      std::unique_ptr<TFile> inputFile_trkAssisted(TFile::Open(file_trkAssisted_Name));
      if (!inputFile_trkAssisted){
        ATH_MSG_FATAL("Cannot open Track Assisted Mass factors file" << fileName);
        return StatusCode::FAILURE;
      }
      
      //Get a TList of TKeys pointing to the histograms contained in the ROOT file
      inputFile_trkAssisted->cd();
      TList *keys_trkAssisted = inputFile_trkAssisted->GetListOfKeys();
      //fill the names of the TKeys into a vector of TStrings
      TIter ikeys_trkAssisted(keys_trkAssisted);
      while ( TKey *iterobj = (TKey*)ikeys_trkAssisted() ) {
	TString histoName = iterobj->GetName();
	if ( histoName.Contains(m_jetAlgo) ) 
	  { 
	    if (m_use3Dhisto)
	      m_respFactorTrackAssistedMass3D = JetCalibUtils::GetHisto3(*inputFile_trkAssisted,histoName);
	    else
	      m_respFactorsTrackAssistedMass.push_back( JetCalibUtils::GetHisto2(*inputFile_trkAssisted,histoName) );
	  }
      }
      
      //Make sure we put something in the vector of TH2Fs
      if ( !m_use3Dhisto && m_respFactorsTrackAssistedMass.size() < 3 ) {
	ATH_MSG_FATAL("Vector of track assisted mass correction histograms may be empty. Please check your track assisted mass calibration file: " << JMSFile);
	return StatusCode::FAILURE;
      }
      else if ( m_use3Dhisto && !m_respFactorTrackAssistedMass3D)
	{
	  ATH_MSG_FATAL("3D track assisted mass correction histogram may be missing.  Please check your mass calibration file: " << JMSFile);
	  return StatusCode::FAILURE;
	}
      else ATH_MSG_INFO("JMS Tool has been initialized with binning and eta fit factors from: " << file_trkAssisted_Name);
    }
  } //!m_onlyCombination

  // Combination
  if(m_combination){
    ATH_MSG_INFO("Mass Combination: ON");
    TString Combination_File(m_config->GetValue("CombinationFile","empty"));
    if ( Combination_File.EqualTo("empty") ) { 
      ATH_MSG_FATAL("NO Combination File specified. Aborting.");
      return StatusCode::FAILURE;
    }
    if(m_dev){
      Combination_File.Remove(0,33);
      Combination_File.Insert(0,"JetCalibTools/");
    }
    else{Combination_File.Insert(14,m_calibAreaTag);}
    TString file_combination_Name(PathResolverFindCalibFile(Combination_File.Data()));
    std::unique_ptr<TFile> inputFile_combination(TFile::Open(file_combination_Name));
    if (!inputFile_combination && !m_onlyCombination){
      ATH_MSG_FATAL("Cannot open Mass Combination file " << file_combination_Name);
      return StatusCode::FAILURE;
    }

    if (!m_use3Dhisto)
        setMassCombinationEtaBins( JetCalibUtils::VectorizeD( m_config->GetValue("MassCombinationEtaBins","") ) );

    // Identify which object is being tagged (QCD, Top, WZ, Hbb)
    TString combObj = "";
    if(m_jetOutScale.Contains("QCD")) combObj = "_QCD_";
    else if(m_jetOutScale.Contains("Top")){ combObj = "_Top_";}
    else if(m_jetOutScale.Contains("WZ")){ combObj = "_WZ_";}
    else if(m_jetOutScale.Contains("Hbb")){ combObj = "_WZ_";} // Temporary due to missing Hbb weights
    if(combObj==""){
      ATH_MSG_FATAL("Wrong JMS Outgoing Scale");
      return StatusCode::FAILURE;
    }

    //Get a TList of TKeys pointing to the histograms contained in the ROOT file
    inputFile_combination->cd();
    TList *keys_combination = inputFile_combination->GetListOfKeys();
    //fill the names of the TKeys into a vector of TStrings
    TIter ikeys_combination(keys_combination);
    while ( TKey *iterobj = (TKey*)ikeys_combination() ) {
      TString histoName = iterobj->GetName();
      if ( histoName.Contains("CaloMass") && histoName.Contains(combObj.Data()) )
      {
        if (!m_use3Dhisto) 
          m_caloResolutionMassCombination.push_back( JetCalibUtils::GetHisto2(*inputFile_combination,histoName) );
        else
          m_caloResolutionMassCombination3D = JetCalibUtils::GetHisto3(*inputFile_combination,histoName);
      }
      if ( histoName.Contains("TAMass") && histoName.Contains(combObj.Data()) )
      {
        if (!m_use3Dhisto)
          m_taResolutionMassCombination.push_back( JetCalibUtils::GetHisto2(*inputFile_combination,histoName) );
        else
          m_taResolutionMassCombination3D = JetCalibUtils::GetHisto3(*inputFile_combination,histoName);
      }
      if ( histoName.Contains("Correlation") && histoName.Contains(combObj.Data()) )
      {
        if (!m_use3Dhisto)
          m_correlationMapMassCombination.push_back( JetCalibUtils::GetHisto2(*inputFile_combination,histoName) );
        else
          m_correlationMapMassCombination3D = JetCalibUtils::GetHisto3(*inputFile_combination,histoName);
      }
    }

    //Make sure we put something in the vector of TH2Ds OR filled the TH3s
    if(!m_onlyCombination){
      if ( !m_use3Dhisto)
	{
	  if ( m_caloResolutionMassCombination.empty() ) {
	    ATH_MSG_FATAL("Vector of mass combination histograms with calo factors may be empty. Please check your mass combination file: " << JMSFile);
	    return StatusCode::FAILURE;
	  }
	  else if ( m_taResolutionMassCombination.empty() ) {
	    ATH_MSG_FATAL("Vector of mass combination histograms with trk-assisted factors may be empty. Please check your mass combination file: " << JMSFile);
	    return StatusCode::FAILURE;
	  }
	}
      else
	{
	  if (!m_caloResolutionMassCombination3D)
	    {
	      ATH_MSG_FATAL("Mass combination 3D histogram with calo factors was not filled.  Please check your mass combination file: " << JMSFile);
	      return StatusCode::FAILURE;
	    }
	  else if (!m_taResolutionMassCombination3D)
	    {
	      ATH_MSG_FATAL("Mass combination 3D histogram with trk-assisted factors was not filled.  Please check your mass combination file: " << JMSFile);
	      return StatusCode::FAILURE;
	    }
	}
    } //m_onlyCombination   
   
    ATH_MSG_INFO("JMS Tool has been initialized with mass combination weights from: " << file_combination_Name);

  }//m_combination

  // Determine the binning strategy
  // History is to use pt_mass_eta, with many past config files that don't specify
  // As such, if nothing is specified, assume pt_mass_eta
  // If something is specified and it's not understood, then that's a failure
  // *Note* that it is assumed the calo and TA masses use the same binning parametrization
  TString binParamString = m_config->GetValue("JMSBinningParam","");
  if (binParamString == "")
  {
    m_binParam = BinningParam::pt_mass_eta;
    ATH_MSG_INFO("JMS Tool will use the implied pt_mass_eta binning strategy");
  }
  else
  {
    // Check if we recognize what was specified
    if (!binParamString.CompareTo("pt_mass_eta",TString::kIgnoreCase))
      m_binParam = BinningParam::pt_mass_eta;
    else if (!binParamString.CompareTo("e_LOGmOe_eta",TString::kIgnoreCase))
      m_binParam = BinningParam::e_LOGmOe_eta;
    else if (!binParamString.CompareTo("e_LOGmOet_eta",TString::kIgnoreCase))
      m_binParam = BinningParam::e_LOGmOet_eta;
    else if (!binParamString.CompareTo("e_LOGmOpt_eta",TString::kIgnoreCase))
      m_binParam = BinningParam::e_LOGmOpt_eta;
    else if (!binParamString.CompareTo("et_LOGmOet_eta",TString::kIgnoreCase))
      m_binParam = BinningParam::et_LOGmOet_eta;
    else
    {
      // Failed to determine what was specified
      ATH_MSG_FATAL("JMSBinningParam was specified, but input was not understood: " << binParamString);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("JMS Tool will use the " << binParamString << " binning strategy");
  }



  return StatusCode::SUCCESS;

}

float JMSCorrection::getMassCorr3D(double pT_uncorr, double mass_uncorr, double eta) const
{
  const double pTMax = m_respFactorMass3D->GetXaxis()->GetBinLowEdge(m_respFactorMass3D->GetNbinsX()+1);
  const double pTMin = m_respFactorMass3D->GetXaxis()->GetBinLowEdge(1);
  const double massMax = m_respFactorMass3D->GetYaxis()->GetBinLowEdge(m_respFactorMass3D->GetNbinsY()+1);
  const double massMin = m_respFactorMass3D->GetYaxis()->GetBinLowEdge(1);
  const double etaMax = m_respFactorMass3D->GetZaxis()->GetBinLowEdge(m_respFactorMass3D->GetNbinsZ()+1);
  const double etaMin = m_respFactorMass3D->GetZaxis()->GetBinLowEdge(1);
  if ( pT_uncorr >= pTMax) pT_uncorr = pTMax-1e-6; // so it fits the up-most pt-bin
  if ( pT_uncorr <= m_pTMinCorr ) return 1; // no correction
  if ( pT_uncorr <= pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_uncorr)) return 1; // no correction if the input is NaN, can happen for log(X)
  if ( mass_uncorr >= massMax ) mass_uncorr = massMax-1e-6; //so it fits the up-most m-bin
  if ( mass_uncorr <= massMin ) mass_uncorr = massMin+1e-6; //so it fits the low-most m-bin
  if ( eta >= etaMax) eta = etaMax-1e-6; // so it fits the up-most eta-bin
  if ( eta <= etaMin) eta = etaMin+1e-6; // so it fits the low-most eta-bin

  float mass_corr = RootHelpers::Interpolate(m_respFactorMass3D.get(),pT_uncorr,mass_uncorr,eta);

  return mass_corr;
}

float JMSCorrection::getMassCorr(double pT_uncorr, double mass_uncorr, int etabin) const {

  // Asymptotic values
  const double pTMax = m_respFactorsMass[etabin]->GetXaxis()->GetBinLowEdge(m_respFactorsMass[etabin]->GetNbinsX()+1);
  const double pTMin = m_respFactorsMass[etabin]->GetXaxis()->GetBinLowEdge(1);
  const double massMax = m_respFactorsMass[etabin]->GetYaxis()->GetBinLowEdge(m_respFactorsMass[etabin]->GetNbinsY()+1);
  const double massMin = m_respFactorsMass[etabin]->GetYaxis()->GetBinLowEdge(1);
  if ( pT_uncorr > pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr < m_pTMinCorr ) return 1; // no correction
  if ( pT_uncorr < pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_uncorr)) return 1; // no correction if the input is NaN, can happen for log(X)
  if ( mass_uncorr > massMax ) mass_uncorr = massMax-1e-6; //so it fits the up-most m-bin
  if ( mass_uncorr < massMin ) mass_uncorr = massMin+1e-6; //so it fits the low-most m-bin

  float mass_corr = m_respFactorsMass[etabin]->Interpolate( pT_uncorr, mass_uncorr );

  return mass_corr;
}

float JMSCorrection::getTrackAssistedMassCorr3D(double pT_uncorr, double mass_uncorr, double eta) const
{
  const double pTMax = m_respFactorTrackAssistedMass3D->GetXaxis()->GetBinLowEdge(m_respFactorTrackAssistedMass3D->GetNbinsX()+1);
  const double pTMin = m_respFactorTrackAssistedMass3D->GetXaxis()->GetBinLowEdge(1);
  const double massMax = m_respFactorTrackAssistedMass3D->GetYaxis()->GetBinLowEdge(m_respFactorTrackAssistedMass3D->GetNbinsY()+1);
  const double massMin = m_respFactorTrackAssistedMass3D->GetYaxis()->GetBinLowEdge(1);
  const double etaMax = m_respFactorTrackAssistedMass3D->GetZaxis()->GetBinLowEdge(m_respFactorTrackAssistedMass3D->GetNbinsZ()+1);
  const double etaMin = m_respFactorTrackAssistedMass3D->GetZaxis()->GetBinLowEdge(1);
  if ( pT_uncorr >= pTMax) pT_uncorr = pTMax-1e-6; // so it fits the up-most pt-bin
  if ( pT_uncorr <= m_pTMinCorr ) return 1; // no correction
  if ( pT_uncorr <= pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_uncorr)) return 1; // no correction if the input is NaN, can happen for log(X)
  if ( mass_uncorr >= massMax ) mass_uncorr = massMax-1e-6; //so it fits the up-most m-bin
  if ( mass_uncorr <= massMin ) mass_uncorr = massMin+1e-6; //so it fits the low-most m-bin
  if ( eta >= etaMax) eta = etaMax-1e-6; // so it fits the up-most eta-bin
  if ( eta <= etaMin) eta = etaMin+1e-6; // so it fits the low-most eta-bin

  float mass_corr = RootHelpers::Interpolate(m_respFactorTrackAssistedMass3D.get(),pT_uncorr,mass_uncorr,eta);

  return mass_corr;
}

float JMSCorrection::getTrackAssistedMassCorr(double pT_uncorr, double uncorr, int etabin) const {

  // Asymptotic values
  const double pTMax = m_respFactorsTrackAssistedMass[etabin]->GetXaxis()->GetBinLowEdge(m_respFactorsTrackAssistedMass[etabin]->GetNbinsX()+1);
  const double pTMin = m_respFactorsTrackAssistedMass[etabin]->GetXaxis()->GetBinLowEdge(1);
  const double massMax = m_respFactorsTrackAssistedMass[etabin]->GetYaxis()->GetBinLowEdge(m_respFactorsTrackAssistedMass[etabin]->GetNbinsY()+1);
  const double massMin = m_respFactorsTrackAssistedMass[etabin]->GetYaxis()->GetBinLowEdge(1);
  if ( pT_uncorr > pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr < m_pTMinCorr ) return 1; // no correction
  if ( pT_uncorr < pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(uncorr)) return 1; // no correction if the input is NaN, can happen for log(X)
  if ( uncorr > massMax ) uncorr = massMax-1e-6; //so it fits the up-most m-bin
  if ( uncorr < massMin ) uncorr = massMin+1e-6; //so it fits the low-most m-bin

  float mass_corr = m_respFactorsTrackAssistedMass[etabin]->Interpolate( pT_uncorr, uncorr );

  return mass_corr;
}

float JMSCorrection::getRelCalo3D(double pT_uncorr, double mass_over_pt_uncorr, double eta) const {

  // Asymptotic values
  double pTMax = m_caloResolutionMassCombination3D->GetXaxis()->GetBinLowEdge(m_caloResolutionMassCombination3D->GetNbinsX()+1);
  double pTMin = m_caloResolutionMassCombination3D->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_caloResolutionMassCombination3D->GetYaxis()->GetBinLowEdge(m_caloResolutionMassCombination3D->GetNbinsY()+1);
  double mass_over_pTMin = m_caloResolutionMassCombination3D->GetYaxis()->GetBinLowEdge(1);
  double etaMax = m_caloResolutionMassCombination3D->GetZaxis()->GetBinLowEdge(m_caloResolutionMassCombination3D->GetNbinsZ()+1);
  double etaMin = m_caloResolutionMassCombination3D->GetZaxis()->GetBinLowEdge(1);
  if ( pT_uncorr >= pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr <= pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr >= mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr <= mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin
  if (eta >= etaMax) eta = etaMax-1e-6; // so it fits the up-most eta-bin
  if (eta <= etaMin) eta = etaMin+1e-6; // so it fits the low-most eta-bin

  float rel = RootHelpers::Interpolate(m_caloResolutionMassCombination3D.get(),pT_uncorr,mass_over_pt_uncorr,eta);

  return rel;
}

float JMSCorrection::getRelCalo(double pT_uncorr, double mass_over_pt_uncorr, int etabin) const {

  // Asymptotic values
  double pTMax = m_caloResolutionMassCombination[etabin]->GetXaxis()->GetBinLowEdge(m_caloResolutionMassCombination[etabin]->GetNbinsX()+1);
  double pTMin = m_caloResolutionMassCombination[etabin]->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_caloResolutionMassCombination[etabin]->GetYaxis()->GetBinLowEdge(m_caloResolutionMassCombination[etabin]->GetNbinsY()+1);
  double mass_over_pTMin = m_caloResolutionMassCombination[etabin]->GetYaxis()->GetBinLowEdge(1);
  if ( pT_uncorr > pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr < pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr > mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr < mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin

  float rel = m_caloResolutionMassCombination[etabin]->Interpolate( pT_uncorr, mass_over_pt_uncorr );

  return rel;
}


float JMSCorrection::getRelTA3D(double pT_uncorr, double mass_over_pt_uncorr, double eta) const {

  // Asymptotic values
  double pTMax = m_taResolutionMassCombination3D->GetXaxis()->GetBinLowEdge(m_taResolutionMassCombination3D->GetNbinsX()+1);
  double pTMin = m_taResolutionMassCombination3D->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_taResolutionMassCombination3D->GetYaxis()->GetBinLowEdge(m_taResolutionMassCombination3D->GetNbinsY()+1);
  double mass_over_pTMin = m_taResolutionMassCombination3D->GetYaxis()->GetBinLowEdge(1);
  double etaMax = m_taResolutionMassCombination3D->GetZaxis()->GetBinLowEdge(m_taResolutionMassCombination3D->GetNbinsZ()+1);
  double etaMin = m_taResolutionMassCombination3D->GetZaxis()->GetBinLowEdge(1);
  if ( pT_uncorr >= pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr <= pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr >= mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr <= mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin
  if (eta >= etaMax) eta = etaMax-1e-6; // so it fits the up-most eta-bin
  if (eta <= etaMin) eta = etaMin+1e-6; // so it fits the low-most eta-bin

  float rel = RootHelpers::Interpolate(m_taResolutionMassCombination3D.get(),pT_uncorr,mass_over_pt_uncorr,eta);

  return rel;
}

float JMSCorrection::getRelTA(double pT_uncorr, double mass_over_pt_uncorr, int etabin) const {

  // Asymptotic values
  double pTMax = m_taResolutionMassCombination[etabin]->GetXaxis()->GetBinLowEdge(m_taResolutionMassCombination[etabin]->GetNbinsX()+1);
  double pTMin = m_taResolutionMassCombination[etabin]->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_taResolutionMassCombination[etabin]->GetYaxis()->GetBinLowEdge(m_taResolutionMassCombination[etabin]->GetNbinsY()+1);
  double mass_over_pTMin = m_taResolutionMassCombination[etabin]->GetYaxis()->GetBinLowEdge(1);
  if ( pT_uncorr > pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr < pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr > mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr < mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin

  float rel = m_taResolutionMassCombination[etabin]->Interpolate( pT_uncorr, mass_over_pt_uncorr );

  return rel;
}

float JMSCorrection::getRho3D(double pT_uncorr, double mass_over_pt_uncorr, double eta) const {

  // Asymptotic values
  double pTMax = m_correlationMapMassCombination3D->GetXaxis()->GetBinLowEdge(m_correlationMapMassCombination3D->GetNbinsX()+1);
  double pTMin = m_correlationMapMassCombination3D->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_correlationMapMassCombination3D->GetYaxis()->GetBinLowEdge(m_correlationMapMassCombination3D->GetNbinsY()+1);
  double mass_over_pTMin = m_correlationMapMassCombination3D->GetYaxis()->GetBinLowEdge(1);
  double etaMax = m_correlationMapMassCombination3D->GetZaxis()->GetBinLowEdge(m_correlationMapMassCombination3D->GetNbinsZ()+1);
  double etaMin = m_correlationMapMassCombination3D->GetZaxis()->GetBinLowEdge(1);
  if ( pT_uncorr >= pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr <= pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr >= mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr <= mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin
  if (eta >= etaMax) eta = etaMax-1e-6; // so it fits the up-most eta-bin
  if (eta <= etaMin) eta = etaMin+1e-6; // so it fits the low-most eta-bin

  float rel = RootHelpers::Interpolate(m_correlationMapMassCombination3D.get(),pT_uncorr,mass_over_pt_uncorr,eta);

  return rel;
}

float JMSCorrection::getRho(double pT_uncorr, double mass_over_pt_uncorr, int etabin) const {

  // Asymptotic values
  double pTMax = m_correlationMapMassCombination[etabin]->GetXaxis()->GetBinLowEdge(m_correlationMapMassCombination[etabin]->GetNbinsX()+1);
  double pTMin = m_correlationMapMassCombination[etabin]->GetXaxis()->GetBinLowEdge(1);
  double mass_over_pTMax = m_correlationMapMassCombination[etabin]->GetYaxis()->GetBinLowEdge(m_correlationMapMassCombination[etabin]->GetNbinsY()+1);
  double mass_over_pTMin = m_correlationMapMassCombination[etabin]->GetYaxis()->GetBinLowEdge(1);
  if ( pT_uncorr > pTMax ) pT_uncorr = pTMax-1e-6 ; //so it fits the up-most pt-bin
  if ( pT_uncorr < pTMin ) pT_uncorr = pTMin+1e-6; //so it fits the low-most pt-bin
  if ( std::isnan(mass_over_pt_uncorr)) return 0; // no weight if the input is NaN, can happen for log(X)
  if ( mass_over_pt_uncorr > mass_over_pTMax ) mass_over_pt_uncorr = mass_over_pTMax-1e-6; //so it fits the up-most m_over_pt-bin
  if ( mass_over_pt_uncorr < mass_over_pTMin ) mass_over_pt_uncorr = mass_over_pTMin+1e-6; //so it fits the low-most m_over_pt-bin

  float rho = m_correlationMapMassCombination[etabin]->Interpolate( pT_uncorr, mass_over_pt_uncorr );

  return rho;
}


StatusCode JMSCorrection::calibrate(xAOD::Jet& jet, JetEventInfo&) const {

  //Apply the JMS calibration scale factor

  //Takes the uncorrected jet eta (in case the origin and/or 4vector jet area corrections were applied)
  float detectorEta = jet.getAttribute<float>("DetectorEta");
  double absdetectorEta = fabs(detectorEta);

  xAOD::JetFourMom_t jetStartP4;
  ATH_CHECK( setStartP4(jet) );
  jetStartP4 = jet.jetP4();

  xAOD::JetFourMom_t calibP4 = jet.jetP4();

  float mass_corr = jetStartP4.mass();
  double pT_corr = jetStartP4.pt();

  TLorentzVector caloCalibJet;
  float mass_ta = 0;

  if(!m_onlyCombination){
    // Determine mass eta bin to use (if using 2D histograms)
    int etabin=-99;
    if (!m_use3Dhisto && (m_massEtaBins.empty() || m_respFactorsMass.size() != m_massEtaBins.size()-1)){
      ATH_MSG_FATAL("Please check that the mass correction eta binning is properly set in your config file");
      return StatusCode::FAILURE;
    }
    else if (m_use3Dhisto && !m_respFactorMass3D)
      {
	ATH_MSG_FATAL("Please check that the mass correction 3D histogram is provided");
	return StatusCode::FAILURE;
      }

    // Originally was jet.getConstituents().size() > 1
    // This essentially requires that the jet has a mass
    // However, constituents are not stored now in rel21 (LCOrigTopoClusters are transient)
    // Thus, getConstituents() breaks unless they are specifically written out
    // Instead, this has been changed to require a non-zero mass
    // Done by S. Schramm on Oct 21, 2017
    
    if ( ( ( !m_use3Dhisto && absdetectorEta < m_massEtaBins.back() ) ||
	   (  m_use3Dhisto && absdetectorEta < m_respFactorMass3D->GetZaxis()->GetBinLowEdge(m_respFactorMass3D->GetNbinsZ()+1))
	   ) && jetStartP4.mass() != 0 ) { // Fiducial cuts
      if (!m_use3Dhisto)
	{
	  for (uint i=0; i<m_massEtaBins.size()-1; ++i) {
            if(absdetectorEta >= m_massEtaBins[i] && absdetectorEta < m_massEtaBins[i+1]) etabin = i;
	  }
	  if (etabin< 0){
	    ATH_MSG_FATAL("There was a problem determining the eta bin to use for the mass correction");
	    return StatusCode::FAILURE;
	  }
	}
    
      // Use the correct histogram binning parametrisation when reading the corrected mass
      double massFactor = 1;
      switch (m_binParam)
	{
	case BinningParam::pt_mass_eta:
	  if (m_use3Dhisto)
            massFactor = getMassCorr3D( jetStartP4.pt()/m_GeV, jetStartP4.mass()/m_GeV, absdetectorEta );
	  else
            massFactor = getMassCorr( jetStartP4.pt()/m_GeV, jetStartP4.mass()/m_GeV, etabin );
	  break;
	case BinningParam::e_LOGmOe_eta:
          if (jetStartP4.mass() / jetStartP4.e() > 0)
          {
            if (m_use3Dhisto)
                massFactor = getMassCorr3D( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.e()), absdetectorEta);
            else
                massFactor = getMassCorr( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.e()), etabin);
          }
          else
            massFactor = 1; // Prevent log(X) for X <= 0
          break;
        case BinningParam::e_LOGmOet_eta:
          if (jetStartP4.mass() / jetStartP4.Et() > 0)
          {
              if (m_use3Dhisto)
                  massFactor = getMassCorr3D( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.Et()), absdetectorEta);
              else
                  massFactor = getMassCorr( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.Et()), etabin);
          }
          else
              massFactor = 1; // Prevent log(X) for X <= 0
          break;
        case BinningParam::e_LOGmOpt_eta:
          if (jetStartP4.mass() / jetStartP4.pt() > 0)
          {
              if (m_use3Dhisto)
                  massFactor = getMassCorr3D( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.pt()), absdetectorEta);
              else
                  massFactor = getMassCorr( jetStartP4.e()/m_GeV, log(jetStartP4.mass() / jetStartP4.pt()), etabin);
          }
          else
              massFactor = 1; // Prevent log(X) for X <= 0
          break;
        case BinningParam::et_LOGmOet_eta:
          if (jetStartP4.mass() / jetStartP4.Et() > 0)
          {
              if (m_use3Dhisto)
                  massFactor = getMassCorr3D( jetStartP4.Et()/m_GeV, log(jetStartP4.mass() / jetStartP4.Et()), absdetectorEta);
              else
                  massFactor = getMassCorr( jetStartP4.Et()/m_GeV, log(jetStartP4.mass() / jetStartP4.Et()), etabin);
          }
          else
              massFactor = 1; // Prevent log(X) for X <= 0
          break;
	default:
	  ATH_MSG_FATAL("This should never be reached - if it happens, it's because a new BinningParam enum option was added, but how to handle it for the calo mass was not.  Please contact the tool developer(s) to fix this.");
	  return StatusCode::FAILURE;
	  break;
	}

      mass_corr = jetStartP4.mass() / massFactor;
    
      if(!m_pTfixed) pT_corr = sqrt(jetStartP4.e()*jetStartP4.e()-mass_corr*mass_corr)/cosh( jetStartP4.eta() );
    }

    caloCalibJet.SetPtEtaPhiM(pT_corr, jetStartP4.eta(), jetStartP4.phi(), mass_corr);
    
    if(!m_combination){
      calibP4.SetPxPyPzE( caloCalibJet.Px(), caloCalibJet.Py(), caloCalibJet.Pz(), caloCalibJet.E() );
  
      //Transfer calibrated jet properties to the Jet object
      jet.setAttribute<xAOD::JetFourMom_t>("JetJMSScaleMomentum",calibP4);
      jet.setJetP4( calibP4 );
    }

    // Track Assisted Mass Correction
    if(m_trackAssistedJetMassCorr || m_combination){

      double E_corr = jetStartP4.e();
      
      // Determine mass eta bin to use
      if (!m_use3Dhisto)
	{
	  etabin=-99;
	  if (m_massEtaBins.empty() || m_respFactorsTrackAssistedMass.size() != m_massEtaBins.size()-1){
	    ATH_MSG_FATAL("Please check that the mass correction eta binning is properly set in your config file");
	    if(m_combination) return StatusCode::FAILURE;
	  }
	}
      else if (m_use3Dhisto && !m_respFactorTrackAssistedMass3D)
	{
	  ATH_MSG_FATAL("Please check that the track assisted mass correction 3D histogram is provided");
	  return StatusCode::FAILURE;
	}

      float trackSumMass;
      std::string TrackSumMassStr = "TrackSumMass";
      if(m_jetAlgo=="AntiKt4EMTopo" || m_jetAlgo=="AntiKt4LCTopo") TrackSumMassStr = "DFCommonJets_TrackSumMass";
      std::string TrackSumPtStr = "TrackSumPt";
      if(m_jetAlgo=="AntiKt4EMTopo" || m_jetAlgo=="AntiKt4LCTopo") TrackSumPtStr = "DFCommonJets_TrackSumPt";
      if( !jet.getAttribute<float>(TrackSumMassStr,trackSumMass) ) {
	if(!m_combination){
	  //ATH_MSG_WARNING("Failed to retrieve TrackSumMass! Track Assisted Mass Correction will NOT be applied\n\n");
      [[maybe_unused]] static const bool warnedOnce = [&] {
        ATH_MSG_WARNING("Failed to retrieve TrackSumMass! Track Assisted Mass Correction will NOT be applied");
        return true;
      }();
	  return StatusCode::SUCCESS;
	} else{
	  ATH_MSG_FATAL("Failed to retrieve TrackSumMass! Mass Combination can NOT be performed. Aborting.");
	  return StatusCode::FAILURE;
	}
      }
      float trackSumPt;
      if( !jet.getAttribute<float>(TrackSumPtStr,trackSumPt) ) {
	if(!m_combination){
	  //ATH_MSG_WARNING("Failed to retrieve TrackSumPt! Track Assisted Mass Correction will NOT be applied\n\n");
      [[maybe_unused]] static const bool warnedOnce = [&] {
        ATH_MSG_WARNING("Failed to retrieve TrackSumPt! Track Assisted Mass Correction will NOT be applied");
        return true;
      }();
	  return StatusCode::SUCCESS;
	} else{
	  ATH_MSG_FATAL("Failed to retrieve TrackSumPt! Mass Combination can NOT be performed. Aborting.");
	  return StatusCode::FAILURE;
	}
      }
      pT_corr = jetStartP4.pt();
      float mTA;
      if(trackSumPt==0) mTA = 0;
      else{mTA = (jetStartP4.pt()/trackSumPt)*trackSumMass;}
      if(mTA<0 || mTA > jetStartP4.e()) mTA = 0;
      mass_corr = mTA;
      
      if ( ( ( !m_use3Dhisto && absdetectorEta < m_massEtaBins.back() ) ||
	     (  m_use3Dhisto && absdetectorEta < m_respFactorMass3D->GetZaxis()->GetBinLowEdge(m_respFactorMass3D->GetNbinsZ()+1))
	     ) && jetStartP4.mass() != 0 ) { // Fiducial cuts
	if (!m_use3Dhisto)
	  {
	    for (uint i=0; i<m_massEtaBins.size()-1; ++i) {
              if(absdetectorEta >= m_massEtaBins[i] && absdetectorEta < m_massEtaBins[i+1]) etabin = i;
	    }
	    if (etabin< 0){
	      ATH_MSG_FATAL("There was a problem determining the eta bin to use for the track assisted mass correction");
	      return StatusCode::FAILURE;
	    }
	  }
      
	double mTAFactor = 1;

	if(mTA!=0){ // Read the calibration values from histograms only when this value is non-zero
	  // Use the correct histogram binning parametrisation when reading the corrected mass
	  switch (m_binParam)
	    {
	    case BinningParam::pt_mass_eta:
	      if (m_use3Dhisto)
		mTAFactor = getTrackAssistedMassCorr3D( jetStartP4.pt()/m_GeV, mTA/m_GeV, absdetectorEta );
	      else
		mTAFactor = getTrackAssistedMassCorr( jetStartP4.pt()/m_GeV, mTA/m_GeV, etabin );
	      break;
	    case BinningParam::e_LOGmOe_eta:
              if (mTA / jetStartP4.e() > 0)
              {
                if (m_use3Dhisto)
                  mTAFactor = getTrackAssistedMassCorr3D( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.e()), absdetectorEta);
                else
                  mTAFactor = getTrackAssistedMassCorr( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.e()), etabin);
              }
              else
                mTAFactor = 1; // Prevent log(X) for X <= 0
              break;
            case BinningParam::e_LOGmOet_eta:
              if (mTA / jetStartP4.Et() > 0)
              {
                if (m_use3Dhisto)
                  mTAFactor = getTrackAssistedMassCorr3D( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.Et()), absdetectorEta);
                else
                  mTAFactor = getTrackAssistedMassCorr( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.Et()), etabin);
              }
              else
                mTAFactor = 1; // Prevent log(X) for X <= 0
              break;
            case BinningParam::e_LOGmOpt_eta:
              if (mTA / jetStartP4.pt() > 0)
              {
                if (m_use3Dhisto)
                  mTAFactor = getTrackAssistedMassCorr3D( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.pt()), absdetectorEta);
                else
                  mTAFactor = getTrackAssistedMassCorr( jetStartP4.e()/m_GeV, log(mTA / jetStartP4.pt()), etabin);
              }
              else
                mTAFactor = 1; // Prevent log(X) for X <= 0
              break;
            case BinningParam::et_LOGmOet_eta:
              if (mTA / jetStartP4.Et() > 0)
              {
                if (m_use3Dhisto)
                  mTAFactor = getTrackAssistedMassCorr3D( jetStartP4.Et()/m_GeV, log(mTA / jetStartP4.Et()), absdetectorEta);
                else
                  mTAFactor = getTrackAssistedMassCorr( jetStartP4.Et()/m_GeV, log(mTA / jetStartP4.Et()), etabin);
              }
              else
                mTAFactor = 1; // Prevent log(X) for X <= 0
              break;
	    default:
	      ATH_MSG_FATAL("This should never be reached - if it happens, it's because a new BinningParam enum option was added, but how to handle it for the TA mass was not.  Please contact the tool developer(s) to fix this.");
	      return StatusCode::FAILURE;
	      break;
	    }  
	}

	if(mTAFactor!=0) mass_corr = mTA/mTAFactor;
	else{
	  ATH_MSG_FATAL("The calibration histogram may have a bad filling bin that is causing mTAFactor to be zero. This value should be different from zero in order to take the ratio. Please contact the tool developer to fix this since the calibration histogram may be corrupted. ");
	  return StatusCode::FAILURE;
	}
	
	if(!m_pTfixed) pT_corr = sqrt(jetStartP4.e()*jetStartP4.e()-mass_corr*mass_corr)/cosh( jetStartP4.eta() );
	else{E_corr  = sqrt(jetStartP4.P()*jetStartP4.P()+mass_corr*mass_corr);}
      }
      else{
	mTA       = 0;
	mass_corr = 0;
	if(!m_pTfixed) pT_corr = jetStartP4.e()/cosh( jetStartP4.eta() );
	else{E_corr = jetStartP4.P();}
      }

      TLorentzVector TACalibJet;
      xAOD::JetFourMom_t TACalibJet_pTfixed = jet.jetP4();
      if(!m_pTfixed){
	TACalibJet.SetPtEtaPhiM(pT_corr, jetStartP4.eta(), jetStartP4.phi(), mass_corr);
      }else{
	TACalibJet_pTfixed.SetPxPyPzE( jetStartP4.Px(), jetStartP4.Py(), jetStartP4.Pz(), E_corr );}
      
      //Transfer calibrated track assisted mass property to the Jet object
      jet.setAttribute<float>("JetTrackAssistedMassUnCalibrated",mTA);
      jet.setAttribute<float>("JetTrackAssistedMassCalibrated",mass_corr);
      if(!m_pTfixed) jet.setAttribute<float>("JetpTCorrByCalibratedTAMass",pT_corr);
      else{jet.setAttribute<float>("JetECorrByCalibratedTAMass",E_corr);}

      //float mass_ta;
      mass_ta = mass_corr;

      // Store calo and TA calibrated jets separetely to further apply insitu:
      //Transfer calibrated calo mass property to the Jet object
      xAOD::JetFourMom_t calibP4_calo = jet.jetP4();
      calibP4_calo.SetCoordinates( caloCalibJet.Pt(), jetStartP4.eta(), jetStartP4.phi(), caloCalibJet.M() );
      jet.setAttribute<xAOD::JetFourMom_t>("JetJMSScaleMomentumCalo",calibP4_calo);

      //Transfer calibrated TA mass property to the Jet object
      xAOD::JetFourMom_t calibP4_ta = jet.jetP4();
      if(!m_pTfixed){
	calibP4_ta.SetCoordinates( TACalibJet.Pt(), jetStartP4.eta(), jetStartP4.phi(), TACalibJet.M() );
      }else{
	calibP4_ta.SetPxPyPzE( TACalibJet_pTfixed.Px(), TACalibJet_pTfixed.Py(), TACalibJet_pTfixed.Pz(), TACalibJet_pTfixed.E() );}

      jet.setAttribute<xAOD::JetFourMom_t>("JetJMSScaleMomentumTA",calibP4_ta);
    } //m_trackAssistedJetMassCorr  
  } //!m_onlyCombination

  if(m_combination){
    float  mass_calo;  
    float  Mass_comb = 0.;
    double pT_calo;
    double E_calo;
    double Et_calo;
  
    if(m_onlyCombination){
      // Read input values (calo and TA insitu calibrated jets) for combination:
 
      xAOD::JetFourMom_t jetInsituP4_calo;
      xAOD::JetFourMom_t calibP4Insitu_calo;
      if(jet.getAttribute<xAOD::JetFourMom_t>("JetInsituScaleMomentumCalo",jetInsituP4_calo)){
	calibP4Insitu_calo=jetInsituP4_calo;
      }else{
	ATH_MSG_FATAL( "Cannot retrieve JetInsituScaleMomentumCalo jets" );
	return StatusCode::FAILURE;
      }
      TLorentzVector TLVCaloInsituCalib;
      TLVCaloInsituCalib.SetPtEtaPhiM(calibP4Insitu_calo.pt(), calibP4Insitu_calo.eta(), calibP4Insitu_calo.phi(), calibP4Insitu_calo.mass());
      mass_calo  = TLVCaloInsituCalib.M();  
      pT_calo = TLVCaloInsituCalib.Pt();
      E_calo = TLVCaloInsituCalib.E();
      Et_calo = TLVCaloInsituCalib.Et();
      
      xAOD::JetFourMom_t jetInsituP4_ta;
      xAOD::JetFourMom_t calibP4Insitu_ta;
      if(jet.getAttribute<xAOD::JetFourMom_t>("JetInsituScaleMomentumTA",jetInsituP4_ta)){
	calibP4Insitu_ta=jetInsituP4_ta;
      }else{
	ATH_MSG_FATAL( "Cannot retrieve JetInsituScaleMomentumTA jets" );
	return StatusCode::FAILURE;
      }
      TLorentzVector TLVTAInsituCalib;
      TLVTAInsituCalib.SetPtEtaPhiM(calibP4Insitu_ta.pt(), calibP4Insitu_ta.eta(), calibP4Insitu_ta.phi(), calibP4Insitu_ta.mass());
      mass_ta = TLVTAInsituCalib.M();
    }else{
      mass_calo = caloCalibJet.M();  // combined mass
      pT_calo = caloCalibJet.Pt();
      E_calo = caloCalibJet.E();
      Et_calo = caloCalibJet.Et();
      // mass_ta already defined above 
    }

    // if one of the mass is null, use the other one
    if( (mass_calo==0) || (mass_ta==0) ) { 
      Mass_comb = mass_ta+mass_calo;
    }
    else {
      // Determine mass combination eta bin to use
      int etabin=-99;
      if (!m_use3Dhisto)
        {
          if (m_massCombinationEtaBins.empty() || m_caloResolutionMassCombination.size() != m_massCombinationEtaBins.size()-1){
	    ATH_MSG_FATAL("Please check that the mass combination eta binning is properly set in your config file");
	    return StatusCode::FAILURE;
	  }
          if (m_massCombinationEtaBins.empty() || m_taResolutionMassCombination.size() != m_massCombinationEtaBins.size()-1){
	    ATH_MSG_FATAL("Please check that the mass combination eta binning is properly set in your config file");
	    return StatusCode::FAILURE;
	  }
        }
      else if (m_use3Dhisto && !m_caloResolutionMassCombination3D)
        {
          ATH_MSG_FATAL("Please check that the mass resolution 3D histogram is provided");
          return StatusCode::FAILURE;
        }
      else if (m_use3Dhisto && !m_taResolutionMassCombination3D)
        {
          ATH_MSG_FATAL("Please check that the track assisted mass resolution 3D histogram is provided");
          return StatusCode::FAILURE;
        }
    
      if ( ( ( !m_use3Dhisto && absdetectorEta < m_massCombinationEtaBins.back() ) ||
	     (  m_use3Dhisto && absdetectorEta < m_caloResolutionMassCombination3D->GetZaxis()->GetBinLowEdge(m_caloResolutionMassCombination3D->GetNbinsZ()+1)) ) ) {
	
	if (!m_use3Dhisto)
          {
            for (uint i=0; i<m_massCombinationEtaBins.size()-1; ++i) {
              if(absdetectorEta >= m_massCombinationEtaBins[i] && absdetectorEta < m_massCombinationEtaBins[i+1]) etabin = i;
            }
            if (etabin< 0){
	      ATH_MSG_FATAL("There was a problem determining the eta bin to use for the mass combination");
	      return StatusCode::FAILURE;
	    }
          }

	// Use the correct histogram binning parametrisation when reading the combined mass weights
	double relCalo = 0;
	double relTA = 0;
	double rho = 0;
	switch (m_binParam)
          {
	  case BinningParam::pt_mass_eta:
	    if (m_use3Dhisto)
              {
                relCalo = getRelCalo3D( pT_calo/m_GeV, mass_calo/pT_calo, absdetectorEta );
                relTA   = getRelTA3D( pT_calo/m_GeV, mass_ta/pT_calo, absdetectorEta );
                if (m_useCorrelatedWeights)
		  rho = getRho3D( pT_calo/m_GeV, mass_calo/pT_calo, absdetectorEta );
              }
	    else
              {
                relCalo = getRelCalo( pT_calo/m_GeV, mass_calo/pT_calo, etabin );
                relTA   = getRelTA( pT_calo/m_GeV, mass_ta/pT_calo, etabin );
                if (m_useCorrelatedWeights)
		  rho = getRho( pT_calo/m_GeV, mass_calo/pT_calo, etabin );
              }
	    break;
	  case BinningParam::e_LOGmOe_eta:
            if (m_use3Dhisto)
            {
              relCalo = mass_calo/E_calo > 0 ? getRelCalo3D( E_calo/m_GeV, log(mass_calo/E_calo), absdetectorEta ) : 0;
              relTA   = mass_ta/E_calo   > 0 ? getRelTA3D(   E_calo/m_GeV, log(mass_ta/E_calo), absdetectorEta )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/E_calo > 0 ? getRho3D(     E_calo/m_GeV, log(mass_calo/E_calo), absdetectorEta ) : 0;
            }
            else
            {
              relCalo = mass_calo/E_calo > 0 ? getRelCalo(   E_calo/m_GeV, log(mass_calo/E_calo), etabin ) : 0;
              relTA   = mass_ta/E_calo   > 0 ? getRelTA(     E_calo/m_GeV, log(mass_ta/E_calo), etabin )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/E_calo > 0 ? getRho(       E_calo/m_GeV, log(mass_calo/E_calo), etabin ) : 0;
            }
            break;
          case BinningParam::e_LOGmOet_eta:
            if (m_use3Dhisto)
            {
              relCalo = mass_calo/Et_calo > 0 ? getRelCalo3D( E_calo/m_GeV, log(mass_calo/Et_calo), absdetectorEta ) : 0;
              relTA   = mass_ta/Et_calo   > 0 ? getRelTA3D(   E_calo/m_GeV, log(mass_ta/Et_calo), absdetectorEta )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/Et_calo > 0 ? getRho3D(     E_calo/m_GeV, log(mass_calo/Et_calo), absdetectorEta ) : 0;
            }
            else
            {
              relCalo = mass_calo/Et_calo > 0 ? getRelCalo(   E_calo/m_GeV, log(mass_calo/Et_calo), etabin ) : 0;
              relTA   = mass_ta/Et_calo   > 0 ? getRelTA(     E_calo/m_GeV, log(mass_ta/Et_calo), etabin )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/Et_calo > 0 ? getRho(       E_calo/m_GeV, log(mass_calo/Et_calo), etabin ) : 0;
            }
            break;
          case BinningParam::e_LOGmOpt_eta:
            if (m_use3Dhisto)
            {
              relCalo = mass_calo/pT_calo > 0 ? getRelCalo3D( E_calo/m_GeV, log(mass_calo/pT_calo), absdetectorEta ) : 0;
              relTA   = mass_ta/pT_calo   > 0 ? getRelTA3D(   E_calo/m_GeV, log(mass_ta/pT_calo), absdetectorEta )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/pT_calo > 0 ? getRho3D(     E_calo/m_GeV, log(mass_calo/pT_calo), absdetectorEta ) : 0;
            }
            else
            {
              relCalo = mass_calo/pT_calo > 0 ? getRelCalo(   E_calo/m_GeV, log(mass_calo/pT_calo), etabin ) : 0;
              relTA   = mass_ta/pT_calo   > 0 ? getRelTA(     E_calo/m_GeV, log(mass_ta/pT_calo), etabin )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/pT_calo > 0 ? getRho(       E_calo/m_GeV, log(mass_calo/pT_calo), etabin ) : 0;
            }
            break;
          case BinningParam::et_LOGmOet_eta:
            if (m_use3Dhisto)
            {
              relCalo = mass_calo/Et_calo > 0 ? getRelCalo3D( Et_calo/m_GeV, log(mass_calo/Et_calo), absdetectorEta ) : 0;
              relTA   = mass_ta/Et_calo   > 0 ? getRelTA3D(   Et_calo/m_GeV, log(mass_ta/Et_calo), absdetectorEta )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/Et_calo > 0 ? getRho3D(     Et_calo/m_GeV, log(mass_calo/Et_calo), absdetectorEta ) : 0;
            }
            else
            {
              relCalo = mass_calo/Et_calo > 0 ? getRelCalo(   Et_calo/m_GeV, log(mass_calo/Et_calo), etabin ) : 0;
              relTA   = mass_ta/Et_calo   > 0 ? getRelTA(     Et_calo/m_GeV, log(mass_ta/Et_calo), etabin )   : 0;
              if (m_useCorrelatedWeights)
                  rho = mass_calo/Et_calo > 0 ? getRho(       Et_calo/m_GeV, log(mass_calo/Et_calo), etabin ) : 0;
            }
            break;
	  default:
	    ATH_MSG_FATAL("This should never be reached - if it happens, it's because a new BinningParam enum option was added, but how to handle it for the TA mass was not.  Please contact the tool developer(s) to fix this.");
	    return StatusCode::FAILURE;
	    break;
          }
          
	// Watch for division by zero
	if(m_useCorrelatedWeights && (relCalo*relCalo + relTA*relTA - 2 * rho* relCalo * relTA == 0)){
	  ATH_MSG_ERROR("Encountered division by zero when calculating mass combination weight using correlated weights");
	  return StatusCode::FAILURE;
	}
	const double Weight = ( relTA*relTA - rho *relCalo*relTA ) / ( relCalo*relCalo + relTA*relTA - 2 * rho* relCalo * relTA );

	// Zero should be only returned by resolution functions if jet mass is negative
	if(relCalo == 0 && relTA == 0)
	  Mass_comb = 0;
	else if(relCalo == 0)
	  Mass_comb = mass_ta;
	else if(relTA == 0)
	  Mass_comb = mass_calo;
	else
	  Mass_comb =  ( mass_calo * Weight ) + ( mass_ta * ( 1 - Weight) );
	// Protection
	if(Mass_comb>jetStartP4.e()) Mass_comb = mass_calo;
	else if(!m_pTfixed) pT_calo = sqrt(jetStartP4.e()*jetStartP4.e()-mass_calo*mass_calo)/cosh( jetStartP4.eta() );
      }
    }
  
    TLorentzVector TLVjet;
    TLVjet.SetPtEtaPhiM( pT_calo, jetStartP4.eta(), jetStartP4.phi(), Mass_comb );
    calibP4.SetPxPyPzE( TLVjet.Px(), TLVjet.Py(), TLVjet.Pz(), TLVjet.E() );
  
    //Transfer calibrated jet properties to the Jet object
    jet.setAttribute<xAOD::JetFourMom_t>(m_jetOutScale.Data(),calibP4);
    jet.setJetP4( calibP4 );

  } //m_combination

  return StatusCode::SUCCESS;

}


