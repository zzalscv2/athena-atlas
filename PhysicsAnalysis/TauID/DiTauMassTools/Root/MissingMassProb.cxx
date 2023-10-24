/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Class handling the probability calculation of the MissingMassCalculator
// author Michael Huebner <michael.huebner@no.spam.cern.ch>

// Local include(s):
#include "DiTauMassTools/MissingMassProb.h"
#include "PathResolver/PathResolver.h"
#include <cmath>

using namespace DiTauMassTools;

// The wrapper functions make use of the ignore template defined in HelperFunctions.h.
// This is to avoid warnings during compilation.
// All wrapper functions need to have the same structure such that they can be
// handled within the probList vectors and the "user" from the Calculator only
// has to call Prob->apply() to have a common handling of all probability calculations.

double MissingMassProb::MetProbabilityWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
	ignore(tau_type1);
	ignore(tau_type2);
	ignore(tauvec1);
	ignore(tauvec2);
	ignore(nuvec1);
	ignore(nuvec2);
	return prob->MetProbability(preparedInput, 1.,1.,1.,1.);
}

double MissingMassProb::mEtAndTauProbabilityWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
	ignore(tau_type1);
	ignore(tau_type2);
	ignore(tauvec1);
	ignore(tauvec2);
	ignore(nuvec1);
	ignore(nuvec2);
	return prob->mEtAndTauProbability(preparedInput);
}

double MissingMassProb::dTheta3d_probabilityFastWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  double Prob = 1.;
  if (tau_type1>=0) {
    TLorentzVector totalTau1;
    totalTau1+=tauvec1;
    totalTau1+=nuvec1;
    const double tau1_tmpp = totalTau1.P();
    const double angle1 = Angle(nuvec1,tauvec1);
    Prob*=prob->dTheta3d_probabilityFast(preparedInput, tau_type1, angle1, tau1_tmpp);
  }
  if (tau_type2>=0) {
    TLorentzVector totalTau2;
    totalTau2+=tauvec2;
    totalTau2+=nuvec2;
    const double tau2_tmpp = totalTau2.P();
    const double angle2 = Angle(nuvec2,tauvec2);
    Prob*=prob->dTheta3d_probabilityFast(preparedInput, tau_type2, angle2, tau2_tmpp);
  }
  return Prob;
}

double MissingMassProb::TauProbabilityWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  if( prob->GetUseHT() || (preparedInput.m_tauTypes==TauTypes::hh) ) {
    return prob->TauProbability(preparedInput, tau_type1, tauvec1, nuvec1, tau_type2, tauvec2, nuvec2, preparedInput.m_MetVec.Mod()); // customized prob for Njet25=0
  } else {
    return prob->TauProbability(preparedInput, tau_type1, tauvec1, nuvec1, tau_type2, tauvec2, nuvec2);
  }
}

double MissingMassProb::MnuProbabilityWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  ignore(tauvec1);
  ignore(tauvec2);
  if(prob->GetUseMnuProbability()==1){
    if(preparedInput.m_tauTypes==TauTypes::ll) return prob->MnuProbability(preparedInput, nuvec1.M())*prob->MnuProbability(preparedInput, nuvec2.M()); // lep-lep
    else if(tau_type1==8 && (tau_type2>=0 && tau_type2<=5)) return prob->MnuProbability(preparedInput, nuvec1.M()); // lep-had: tau1==lepton
    else if((tau_type1>=0 && tau_type1<=5) && tau_type2==8) return prob->MnuProbability(preparedInput, nuvec2.M()); // lep-had: tau2==lepton
    else {
      Warning("DiTauMassTools", "something went really wrong in MNuProb...");
      return 1.;
    }
  } else {
    return 1.;
  }
}

double MissingMassProb::dTheta3d_probabilityNewWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  ignore(preparedInput);
  double Prob = 1.;
  if (tau_type1>=0) {
    TLorentzVector totalTau1;
    totalTau1+=tauvec1;
    totalTau1+=nuvec1;
    const double angle1 = Angle(nuvec1,tauvec1);
    double prob_tmp = 1e-10;
    if (angle1!=0.) prob_tmp=prob->GetFormulaAngle1()->Eval(angle1);
    if (prob_tmp<=0.) prob_tmp=1e-10;
    Prob*=prob_tmp;
  }
  if (tau_type2>=0) {
    TLorentzVector totalTau2;
    totalTau2+=tauvec2;
    totalTau2+=nuvec2;
    const double angle2 = Angle(nuvec2,tauvec2);
    double prob_tmp = 1e-10;
    if (angle2!=0.) prob_tmp=prob->GetFormulaAngle2()->Eval(angle2);
    if (prob_tmp<=0.) prob_tmp=1e-10;
    Prob*=prob_tmp;
  }
  // very rare cases where parametrisation flips
  if (std::isnan(Prob)) Prob = 0.;
  return Prob;
}

double MissingMassProb::TauProbabilityNewWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  ignore(preparedInput);
  ignore(tau_type1);
  ignore(tau_type2);
  double Prob = 1.;
  double R1 = nuvec1.P()/tauvec1.P();
  double R2 = nuvec2.P()/tauvec2.P();
  Prob*=prob->GetFormulaRatio1()->Eval(R1);
  Prob*=prob->GetFormulaRatio2()->Eval(R2);
  // not observed, just a protection
  if (std::isnan(Prob)) Prob = 0.;
  return Prob;
}

double MissingMassProb::MnuProbabilityNewWrapper( MissingMassProb* prob, MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2 ){
  ignore(tauvec1);
  ignore(tauvec2);
  double Prob = 1.;
  if(prob->GetUseMnuProbability()==1){
    if(preparedInput.m_tauTypes==TauTypes::ll) Prob*=(prob->GetFormulaNuMass()->Eval(nuvec1.M())*prob->GetFormulaNuMass()->Eval(nuvec2.M()));
    else if(tau_type1==8 && (tau_type2>=0 && tau_type2<=5)) Prob*=prob->GetFormulaNuMass()->Eval(nuvec1.M());
    else if((tau_type1>=0 && tau_type1<=5) && tau_type2==8) Prob*=prob->GetFormulaNuMass()->Eval(nuvec2.M());
    else {
      Warning("DiTauMassTools", "something went really wrong in MNuProb...");
    }
  }
  // not observed, just a protection
  if (std::isnan(Prob)) Prob = 0.;
  return Prob;
}

void MissingMassProb::setParamNuMass() {
  if (m_paramVectorNuMass.size() > 0){
    for(int i=0; i<m_formulaNuMass->GetNpar(); i++){
      m_formulaNuMass->SetParameter(i, m_paramVectorNuMass[0]->GetParameter(i));
    }
  }
}

void MissingMassProb::setParamAngle(const TLorentzVector& tauvec, int tau, int tautype) {
  double Pt_tau = tauvec.Pt();
  int type = tautype;
  if (tautype > 4 && tautype < 8) type = 4;
  if (tautype <= 5){
    if (m_paramVectorAngle.size() > 0){
      for(int i=0; i<=3; i++){
        double par = m_paramVectorAngle[i+(type)*4]->Eval(Pt_tau);
        if (tau==1){
          m_formulaAngle1->SetParameter(i, par);
        } else {
          m_formulaAngle2->SetParameter(i, par);
        }
      }
    }
  } else {
    if (m_paramVectorAngleLep.size() > 0){
      for(int i=0; i<=3; i++){
        double par = m_paramVectorAngleLep[i]->Eval(Pt_tau);
        if (tau==1){
          m_formulaAngle1->SetParameter(i, par);
        } else {
          m_formulaAngle2->SetParameter(i, par);
        }
      }
    }
  }
}

void MissingMassProb::setParamRatio(int tau, int tautype) {
  int type = tautype;
  if(tautype > 4 && tautype < 8) type = 4;
  if (tautype <= 5){
      if(tau==1){
	m_formulaRatio1 = m_formulaRatioHad1;
        for(int i=0; i<m_formulaRatio1->GetNpar(); i++){
        m_formulaRatio1->SetParameter(i, m_paramVectorRatio[5*(tau-1)+type]->GetParameter(i));
      }
      } else {
	m_formulaRatio2 = m_formulaRatioHad2;
        for(int i=0; i<m_formulaRatio2->GetNpar(); i++){
        m_formulaRatio2->SetParameter(i, m_paramVectorRatio[5*(tau-1)+type]->GetParameter(i));
      }
    }
  } else {
    if(tau==1){
	m_formulaRatio1 = m_formulaRatioLep1;
      for(int i=0; i<m_formulaRatio1->GetNpar(); i++){
        m_formulaRatio1->SetParameter(i, m_paramVectorRatioLep[0]->GetParameter(i));
      }
    } else {
	m_formulaRatio2 = m_formulaRatioLep2;
      for(int i=0; i<m_formulaRatio2->GetNpar(); i++){
        m_formulaRatio2->SetParameter(i, m_paramVectorRatioLep[0]->GetParameter(i));
      }
    }
  }
}

// Default Constructor
MissingMassProb::MissingMassProb(MMCCalibrationSetV2::e aset, std::string m_paramFilePath) {
  m_mmcCalibrationSet = aset;
  m_allowUseHT = false;
  m_UseHT = false;

  m_fParams = NULL;
  if (!m_paramFilePath.empty()){
     std::string total_path = "DiTauMassTools/"+m_paramFilePath;
     m_fParams = TFile::Open( (const char*) PathResolverFindCalibFile(total_path).c_str() ,"READ");
  }
  if (aset == MMCCalibrationSetV2::MMC2019) {
    m_probListConstant.push_back( std::bind(&mEtAndTauProbabilityWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListOneTau.push_back( std::bind(&dTheta3d_probabilityNewWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListTwoTau.push_back( std::bind(&TauProbabilityNewWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListTwoTau.push_back( std::bind(&MnuProbabilityNewWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    if (m_fParams){
	readInParams(m_fParams, aset, m_paramVectorNuMass, m_paramVectorAngleLep, m_paramVectorRatioLep, m_paramVectorAngle, m_paramVectorRatio);
    }
  }
  else {
    m_probListConstant.push_back( std::bind(&mEtAndTauProbabilityWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListOneTau.push_back( std::bind(&dTheta3d_probabilityFastWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListTwoTau.push_back( std::bind(&TauProbabilityWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
    m_probListTwoTau.push_back( std::bind(&MnuProbabilityWrapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7) );
  }

  if (m_fParams) if (m_fParams->IsOpen()) m_fParams->Close();

  /// MMC2011 parameterisation
  // [tau_type][parLG][par]
  // leptonic tau
  //-par[0]
  s_fit_param[0][0][0][0]=-9.82013E-1;
  s_fit_param[0][0][0][1]=9.09874E-1;
  s_fit_param[0][0][0][2]=0.0;
  s_fit_param[0][0][0][3]=0.0;
  //-par[1]
  s_fit_param[0][0][1][0]=9.96303E1;
  s_fit_param[0][0][1][1]=1.68873E1;
  s_fit_param[0][0][1][2]=3.23798E-2;
  s_fit_param[0][0][1][3]=0.0;
  //-par[2]
  s_fit_param[0][0][2][0]=0.0;
  s_fit_param[0][0][2][1]=0.0;
  s_fit_param[0][0][2][2]=0.0;
  s_fit_param[0][0][2][3]=0.3; // fit value is 2.8898E-1, I use 0.3
  //-par[3]
  s_fit_param[0][0][3][0]=9.70055E1;
  s_fit_param[0][0][3][1]=6.46175E1;
  s_fit_param[0][0][3][2]=3.20679E-2;
  s_fit_param[0][0][3][3]=0.0;
  //-par[4]
  s_fit_param[0][0][4][0]=1.56865;
  s_fit_param[0][0][4][1]=2.28336E-1;
  s_fit_param[0][0][4][2]=1.09396E-1;
  s_fit_param[0][0][4][3]=1.99975E-3;
  //-par[5]
  s_fit_param[0][0][5][0]=0.0;
  s_fit_param[0][0][5][1]=0.0;
  s_fit_param[0][0][5][2]=0.0;
  s_fit_param[0][0][5][3]=0.66;
  //-----------------------------------------------------------------
  // 1-prong hadronic tau
  //-par[0]
  s_fit_param[0][1][0][0]=-2.42674;
  s_fit_param[0][1][0][1]=7.69124E-1;
  s_fit_param[0][1][0][2]=0.0;
  s_fit_param[0][1][0][3]=0.0;
  //-par[1]
  s_fit_param[0][1][1][0]=9.52747E1;
  s_fit_param[0][1][1][1]=1.26319E1;
  s_fit_param[0][1][1][2]=3.09643E-2;
  s_fit_param[0][1][1][3]=0.0;
  //-par[2]
  s_fit_param[0][1][2][0]=1.71302E1;
  s_fit_param[0][1][2][1]=3.00455E1;
  s_fit_param[0][1][2][2]=7.49445E-2;
  s_fit_param[0][1][2][3]=0.0;
  //-par[3]
  s_fit_param[0][1][3][0]=1.06137E2;
  s_fit_param[0][1][3][1]=6.01548E1;
  s_fit_param[0][1][3][2]=3.50867E-2;
  s_fit_param[0][1][3][3]=0.0;
  //-par[4]
  s_fit_param[0][1][4][0]=4.26079E-1;
  s_fit_param[0][1][4][1]=1.76978E-1;
  s_fit_param[0][1][4][2]=1.43419;
  s_fit_param[0][1][4][3]=0.0;
  //-par[5]
  s_fit_param[0][1][5][0]=0.0;
  s_fit_param[0][1][5][1]=0.0;
  s_fit_param[0][1][5][2]=0.0;
  s_fit_param[0][1][5][3]=0.4;
  //-----------------------------------------------------------------
  // 3-prong hadronic tau
  //-par[0]
  s_fit_param[0][2][0][0]=-2.43533;
  s_fit_param[0][2][0][1]=6.12947E-1;
  s_fit_param[0][2][0][2]=0.0;
  s_fit_param[0][2][0][3]=0.0;
  //-par[1]
  s_fit_param[0][2][1][0]=9.54202;
  s_fit_param[0][2][1][1]=2.80011E-1;
  s_fit_param[0][2][1][2]=2.49782E-1;
  s_fit_param[0][2][1][3]=0.0;
  //-par[2]
  s_fit_param[0][2][2][0]=1.61325E1;
  s_fit_param[0][2][2][1]=1.74892E1;
  s_fit_param[0][2][2][2]=7.05797E-2;
  s_fit_param[0][2][2][3]=0.0;
  //-par[3]
  s_fit_param[0][2][3][0]=1.17093E2;
  s_fit_param[0][2][3][1]=4.70000E1;
  s_fit_param[0][2][3][2]=3.87085E-2;
  s_fit_param[0][2][3][3]=0.0;
  //-par[4]
  s_fit_param[0][2][4][0]=4.16557E-1;
  s_fit_param[0][2][4][1]=1.58902E-1;
  s_fit_param[0][2][4][2]=1.53516;
  s_fit_param[0][2][4][3]=0.0;
  //-par[5]
  s_fit_param[0][2][5][0]=0.0;
  s_fit_param[0][2][5][1]=0.0;
  s_fit_param[0][2][5][2]=0.0;
  s_fit_param[0][2][5][3]=0.95;


  /// MMC2012 parameterisation
  // [tau_type][parLG][par]
  // leptonic tau
  //-par[0]
  s_fit_param[1][0][0][0]=-9.82013E-1;
  s_fit_param[1][0][0][1]=9.09874E-1;
  s_fit_param[1][0][0][2]=0.0;
  s_fit_param[1][0][0][3]=0.0;
  s_fit_param[1][0][0][4]=0.0;
  //-par[1]
  s_fit_param[1][0][1][0]=9.96303E1;
  s_fit_param[1][0][1][1]=1.68873E1;
  s_fit_param[1][0][1][2]=3.23798E-2;
  s_fit_param[1][0][1][3]=0.0;
  s_fit_param[1][0][1][4]=0.0;
  //-par[2]
  s_fit_param[1][0][2][0]=0.0;
  s_fit_param[1][0][2][1]=0.0;
  s_fit_param[1][0][2][2]=0.0;
  s_fit_param[1][0][2][3]=0.3; // fit value is 2.8898E-1, I use 0.3
  s_fit_param[1][0][2][4]=0.0;
  //-par[3]
  s_fit_param[1][0][3][0]=9.70055E1;
  s_fit_param[1][0][3][1]=6.46175E1;
  s_fit_param[1][0][3][2]=3.20679E-2;
  s_fit_param[1][0][3][3]=0.0;
  s_fit_param[1][0][3][4]=0.0;
  //-par[4]
  s_fit_param[1][0][4][0]=1.56865;
  s_fit_param[1][0][4][1]=2.28336E-1;
  s_fit_param[1][0][4][2]=1.09396E-1;
  s_fit_param[1][0][4][3]=1.99975E-3;
  s_fit_param[1][0][4][4]=0.0;
  //-par[5]
  s_fit_param[1][0][5][0]=0.0;
  s_fit_param[1][0][5][1]=0.0;
  s_fit_param[1][0][5][2]=0.0;
  s_fit_param[1][0][5][3]=0.66;
  s_fit_param[1][0][5][4]=0.0;
  //-----------------------------------------------------------------
  //-----------------------
  // Only hadronic tau's were re-parametrized in MC12. The parameterization now
  // goes to P(tau)=1 TeV.
  //-----------------------------------------------------------------
  // 1-prong hadronic tau
  //---par[0]
  s_fit_param[1][1][0][0]=0.7568;
  s_fit_param[1][1][0][1]=-0.0001469;
  s_fit_param[1][1][0][2]=5.413E-7;
  s_fit_param[1][1][0][3]=-6.754E-10;
  s_fit_param[1][1][0][4]=2.269E-13;
  //---par[1]
  s_fit_param[1][1][1][0]=-0.0288208;
  s_fit_param[1][1][1][1]=0.134174;
  s_fit_param[1][1][1][2]=-142.588;
  s_fit_param[1][1][1][3]=-0.00035606;
  s_fit_param[1][1][1][4]=-6.94567E-20;
  //---par[2]
  s_fit_param[1][1][2][0]=-0.00468927;
  s_fit_param[1][1][2][1]=0.0378737;
  s_fit_param[1][1][2][2]=-260.284;
  s_fit_param[1][1][2][3]=0.00241158;
  s_fit_param[1][1][2][4]=-6.01766E-7;
  //---par[3]
  s_fit_param[1][1][3][0]=-0.170424;
  s_fit_param[1][1][3][1]=0.135764;
  s_fit_param[1][1][3][2]=-50.2361;
  s_fit_param[1][1][3][3]=0.00735544;
  s_fit_param[1][1][3][4]=-7.34073E-6;
  //---par[4]
  s_fit_param[1][1][4][0]=-0.0081364;
  s_fit_param[1][1][4][1]=0.0391428;
  s_fit_param[1][1][4][2]=-141.936;
  s_fit_param[1][1][4][3]=0.0035034;
  s_fit_param[1][1][4][4]=-1.21956E-6;
  //-par[5]
  s_fit_param[1][1][5][0]=0.0;
  s_fit_param[1][1][5][1]=0.0;
  s_fit_param[1][1][5][2]=0.0;
  s_fit_param[1][1][5][3]=0.624*0.00125; // multiplied by a bin size
  s_fit_param[1][1][5][4]=0.0;
  //-----------------------------------------------------------------
  // 3-prong hadronic tau
  //---par[0]
  s_fit_param[1][2][0][0]=0.7562;
  s_fit_param[1][2][0][1]=-1.168E-5;
  s_fit_param[1][2][0][2]=0.0;
  s_fit_param[1][2][0][3]=0.0;
  s_fit_param[1][2][0][4]=0.0;
  //---par[1]
  s_fit_param[1][2][1][0]=-0.0420458;
  s_fit_param[1][2][1][1]=0.15917;
  s_fit_param[1][2][1][2]=-80.3259;
  s_fit_param[1][2][1][3]=0.000125729;
  s_fit_param[1][2][1][4]=-2.43945E-18;
  //---par[2]
  s_fit_param[1][2][2][0]=-0.0216898;
  s_fit_param[1][2][2][1]=0.0243497;
  s_fit_param[1][2][2][2]=-63.8273;
  s_fit_param[1][2][2][3]=0.0148339;
  s_fit_param[1][2][2][4]=-4.45351E-6;
  //---par[3]
  s_fit_param[1][2][3][0]=-0.0879411;
  s_fit_param[1][2][3][1]=0.110092;
  s_fit_param[1][2][3][2]=-75.4901;
  s_fit_param[1][2][3][3]=0.0116915;
  s_fit_param[1][2][3][4]=-1E-5;
  //---par[4]
  s_fit_param[1][2][4][0]=-0.0118324;
  s_fit_param[1][2][4][1]=0.0280538;
  s_fit_param[1][2][4][2]=-85.127;
  s_fit_param[1][2][4][3]=0.00724948;
  s_fit_param[1][2][4][4]=-2.38792E-6;
  //-par[5]
  s_fit_param[1][2][5][0]=0.0;
  s_fit_param[1][2][5][1]=0.0;
  s_fit_param[1][2][5][2]=0.0;
  s_fit_param[1][2][5][3]=0.6167*0.00125; // multiplied by a bin size
  s_fit_param[1][2][5][4]=0.0;

  // TER parameterization, for now based on p1130, to be checked and updated with new tag
  // [tau_type][eta_bin][parameter]
  // for 1-prongs
  s_ter_sigma_par[0][0][0]=0.311717;
  s_ter_sigma_par[0][0][1]=0.0221615;
  s_ter_sigma_par[0][0][2]=0.859698;
  s_ter_sigma_par[0][1][0]=0.290019;
  s_ter_sigma_par[0][1][1]=0.0225794;
  s_ter_sigma_par[0][1][2]=0.883407;
  s_ter_sigma_par[0][2][0]=0.352312;
  s_ter_sigma_par[0][2][1]=0.0196381;
  s_ter_sigma_par[0][2][2]=0.629708;
  s_ter_sigma_par[0][3][0]=0.342059;
  s_ter_sigma_par[0][3][1]=0.0275107;
  s_ter_sigma_par[0][3][2]=0.48065;
  s_ter_sigma_par[0][4][0]=0.481564;
  s_ter_sigma_par[0][4][1]=0.0197219;
  s_ter_sigma_par[0][4][2]=0.0571714;
  s_ter_sigma_par[0][5][0]=0.41264;
  s_ter_sigma_par[0][5][1]=0.0233964;
  s_ter_sigma_par[0][5][2]=0.515674;
  s_ter_sigma_par[0][6][0]=0.20112;
  s_ter_sigma_par[0][6][1]=0.0339914;
  s_ter_sigma_par[0][6][2]=0.944524;
  s_ter_sigma_par[0][7][0]=0.0892094;
  s_ter_sigma_par[0][7][1]=0.0210225;
  s_ter_sigma_par[0][7][2]=1.34014;
  s_ter_sigma_par[0][8][0]=0.175554;
  s_ter_sigma_par[0][8][1]=0.0210968;
  s_ter_sigma_par[0][8][2]=0.813925;
  s_ter_sigma_par[0][9][0]=0.0;
  s_ter_sigma_par[0][9][1]=0.0340279;
  s_ter_sigma_par[0][9][2]=1.30856;
  // for 3-prongs
  s_ter_sigma_par[1][0][0]=0.303356;
  s_ter_sigma_par[1][0][1]=0.0299807;
  s_ter_sigma_par[1][0][2]=1.25388;
  s_ter_sigma_par[1][1][0]=0.358106;
  s_ter_sigma_par[1][1][1]=0.0229604;
  s_ter_sigma_par[1][1][2]=1.02222;
  s_ter_sigma_par[1][2][0]=0.328643;
  s_ter_sigma_par[1][2][1]=0.025684;
  s_ter_sigma_par[1][2][2]=1.02594;
  s_ter_sigma_par[1][3][0]=0.497332;
  s_ter_sigma_par[1][3][1]=0.0215113;
  s_ter_sigma_par[1][3][2]=0.30055;
  s_ter_sigma_par[1][4][0]=0.4493;
  s_ter_sigma_par[1][4][1]=0.0280311;
  s_ter_sigma_par[1][4][2]=0.285793;
  s_ter_sigma_par[1][5][0]=0.427811;
  s_ter_sigma_par[1][5][1]=0.0316536;
  s_ter_sigma_par[1][5][2]=0.457286;
  s_ter_sigma_par[1][6][0]=0.165288;
  s_ter_sigma_par[1][6][1]=0.0376361;
  s_ter_sigma_par[1][6][2]=1.3913;
  s_ter_sigma_par[1][7][0]=0.289798;
  s_ter_sigma_par[1][7][1]=0.0140801;
  s_ter_sigma_par[1][7][2]=0.83603;
  s_ter_sigma_par[1][8][0]=0.186823;
  s_ter_sigma_par[1][8][1]=0.0213053;
  s_ter_sigma_par[1][8][2]=0.968934;
  s_ter_sigma_par[1][9][0]=0.301673;
  s_ter_sigma_par[1][9][1]=0.0145606;
  s_ter_sigma_par[1][9][2]=0.514022;
  
}

// Default Destructor
MissingMassProb::~MissingMassProb() {
}

double MissingMassProb::apply(MissingMassInput& preparedInput, const int & tau_type1, const int & tau_type2, const TLorentzVector & tauvec1, const TLorentzVector & tauvec2, const TLorentzVector nuvec1, const TLorentzVector & nuvec2, bool constant, bool oneTau, bool twoTau) {
  double prob = 1.;
  if (constant == true) {
    for (auto& f: m_probListConstant) {
      prob*=f(preparedInput, tau_type1, tau_type2, tauvec1, tauvec2, nuvec1, nuvec2);
    }
  } else if (oneTau == true) {
    for (auto& f: m_probListOneTau) {
      prob*=f(preparedInput, tau_type1, tau_type2, tauvec1, tauvec2, nuvec1, nuvec2);
    }
  } else if (twoTau == true) {
    for (auto& f: m_probListTwoTau) {
      prob*=f(preparedInput, tau_type1, tau_type2, tauvec1, tauvec2, nuvec1, nuvec2);
    }
  }
  return prob;
}


double MissingMassProb::MetProbability(MissingMassInput& preparedInput, const double & met1,const  double & met2,const  double & MetSigma1,const  double & MetSigma2) {


  double metprob;
  if(MetSigma1>1.0 && MetSigma2>1.0) // it doesn't make sense if MET resolution sigma is <1 GeV
    {
      //SpeedUp
      metprob=exp(-0.5*(met1*met1/(MetSigma1*MetSigma1)+met2*met2/(MetSigma2*MetSigma2)))/(MetSigma1*MetSigma2*2*TMath::Pi());
    }
  else
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "MissingMassCalculator::MetProbability: either MetSigma1 or MetSigma2 are <1 GeV--- too low, returning prob=1");
      metprob=1.;
    }


  return metprob;


}

double MissingMassProb::mEtAndTauProbability(MissingMassInput& preparedInput)
{
  double proba=1.;
  double metprob;

  // deltaMEt is the difference between the neutrino sum and the MEt (or -HT if useHT),
  //corrected possibly from tau E scanning

  double deltaMetX=preparedInput.m_metVec.X()-preparedInput.m_inputMEtX;
  double deltaMetY=preparedInput.m_metVec.Y()-preparedInput.m_inputMEtY;

  // possibly correct for subtract tau scanning here
  //  const double met_smearL=(deltaMetVec.X()*m_metCovPhiCos+deltaMetVec.Y()*m_metCovPhiSin;
  //  const double met_smearP=-deltaMetVec.X()*m_metCovPhiSin+deltaMetVec.Y()*m_metCovPhiCos;

  // rotate into error ellipse axis
  const double met_smearL=deltaMetX*cos(preparedInput.m_METcovphi)+deltaMetY*sin(preparedInput.m_METcovphi);
  const double met_smearP=-deltaMetX*sin(preparedInput.m_METcovphi)+deltaMetY*cos(preparedInput.m_METcovphi);


  if (m_UseHT)
    {
      if ( preparedInput.m_tauTypes==TauTypes::hh ) {

        metprob=MHtProbabilityHH(preparedInput, met_smearL,met_smearP,preparedInput.m_inputMEtT,preparedInput.m_MEtT,preparedInput.m_htOffset); // for had-had
      }
      else {
        metprob=MHtProbability(preparedInput, met_smearL,met_smearP,preparedInput.m_inputMEtT,preparedInput.m_MEtT,preparedInput.m_htOffset); // for lep-had Winter 2012 analysis
      }
    }
  else {
    metprob=MetProbability(preparedInput, met_smearL,met_smearP,preparedInput.m_METsigmaL,preparedInput.m_METsigmaP);
  }

  proba=metprob;

  return proba;
}

//------------------- simple TauProbability for LFV
double MissingMassProb::TauProbabilityLFV(MissingMassInput& preparedInput, const int & type1, const TLorentzVector & vis1, const TLorentzVector & nu1)
{
  double prob=1.0;
  if(m_fUseTauProbability==0) return prob; // don't apply TauProbability
  double prob1=1.0;
  const double mtau=1.777;
  const double R1=nu1.E()/vis1.E();
  //--- dealing with 1st tau
  double m1=nu1.M();
  double m2=vis1.M();
  double E1=0.5*(mtau*mtau+m1*m1-m2*m2)/mtau;
  double E2=mtau-E1;
  if(E1<=m1 || E1>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E1, returning 0 ");
      return 0.0;
    }
  if(E2<=m2 || E2>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E2, returning 0 ");
      return 0.0;
    }
  preparedInput.m_tlv_tmp.SetPxPyPzE(0.,0.,0.,0.);
  preparedInput.m_tlv_tmp+=nu1;
  preparedInput.m_tlv_tmp+=vis1;
  //  double p=(nu1+vis1).P();
  double p=preparedInput.m_tlv_tmp.P();
  double V=p/sqrt(p*p+mtau*mtau);
  double p0;
  if(type1==8) p0=sqrt(E2*E2-m2*m2); // leptonic tau
  else p0=E1; // hadronic tau
  prob1=0.5*mtau/(p0*V*pow(R1+1.0,2));
  // avoid too large values
  prob=std::min(prob1,1.);
  return prob;
}

double MissingMassProb::TauProbability(MissingMassInput& preparedInput, const int & type1, const TLorentzVector & vis1, const TLorentzVector & nu1,
                                             const int & type2, const TLorentzVector & vis2, const TLorentzVector & nu2)
{
  double prob=1.0;
  if(m_fUseTauProbability==0) return prob; // don't apply TauProbability
  double prob1=1.0;
  double prob2=1.0;
  const double mtau=1.777;
  const double R1=nu1.E()/vis1.E();
  const double R2=nu2.E()/vis2.E();
  //--- dealing with 1st tau
  double m1=nu1.M();
  double m2=vis1.M();
  double E1=0.5*(mtau*mtau+m1*m1-m2*m2)/mtau;
  double E2=mtau-E1;
  if(E1<=m1 || E1>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E1, returning 0 ");
      return 0.0;
    }
  if(E2<=m2 || E2>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E2, returning 0 ");
      return 0.0;
    }
  preparedInput.m_tlv_tmp.SetPxPyPzE(0.,0.,0.,0.);
  preparedInput.m_tlv_tmp+=nu1;
  preparedInput.m_tlv_tmp+=vis1;
  //  double p=(nu1+vis1).P();
  double p=preparedInput.m_tlv_tmp.P();
  double V=p/sqrt(p*p+mtau*mtau);
  double p0;
  if(type1==8) p0=sqrt(E2*E2-m2*m2); // leptonic tau
  else p0=E1; // hadronic tau
  prob1=0.5*mtau/(p0*V*pow(R1+1.0,2));
  // avoid too large values
  prob1=std::min(prob1,1.);


  //--- dealing with 2nd tau
  m1=nu2.M();
  m2=vis2.M();
  E1=0.5*(mtau*mtau+m1*m1-m2*m2)/mtau;
  E2=mtau-E1;
  if(E1<=m1 || E1>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E1, returning 0 ");
      return 0.0;
    }
  if(E2<=m2 || E2>=mtau)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::TauProbability: bad E2, returning 0 ");
      return 0.0;
    }
  preparedInput.m_tlv_tmp.SetPxPyPzE(0.,0.,0.,0.);
  preparedInput.m_tlv_tmp+=nu2;
  preparedInput.m_tlv_tmp+=vis2;
  //  p=(nu2+vis2).P();
  p=preparedInput.m_tlv_tmp.P();


  V=p/sqrt(p*p+mtau*mtau);
  if(type2==8) p0=sqrt(E2*E2-m2*m2); // leptonic tau
  else p0=E1; // hadronic tau
  prob2=0.5*mtau/(p0*V*pow(R2+1.0,2));
  // avoid too large values
  prob2=std::min(prob2,1.);
  prob=prob1*prob2;
  return prob;
}


// --------- Updated version of TauProbability for lep-had events with Njet25=0, takes into account Winter-2012 analysis cuts
double MissingMassProb::TauProbability(MissingMassInput& preparedInput, const int & type1, const TLorentzVector & vis1, const TLorentzVector & nu1,
                                             const int & type2, const TLorentzVector & vis2, const TLorentzVector & nu2, const double & detmet) {
  double prob=1.0;

  if(m_fUseTauProbability==0) return prob; // don't apply TauProbability

  if(m_UseHT)
    {
      if(detmet<20.0) // low MET Njet=0 category
        {
          const double R1=nu1.P()/vis1.P();
          const double R2=nu2.P()/vis2.P();
          const double lep_p1[4]={0.417,0.64,0.52,0.678};
          const double lep_p2[4]={0.23,0.17,0.315,0.319};
          const double lep_p3[4]={0.18,0.33,0.41,0.299};
          const double lep_p4[4]={0.033,0.109,0.129,0.096};
          const double lep_p5[4]={0.145,0.107,0.259,0.304};
          int ind=3;
          int indT=3;
          const double n_1pr[4]={-0.15,-0.13,-0.25,-0.114};
          const double s_1pr[4]={0.40,0.54,0.62,0.57};
          const double n_3pr[4]={-1.08,-1.57,-0.46,-0.39};
          const double s_3pr[4]={0.53,0.85,0.61,0.53};
          double Ptau=0.0;
          double Plep=0.0;
          if(type1>=0 && type1<=5)
            {
              Ptau=(nu1+vis1).P();
              Plep=(nu2+vis2).P();
            }
          if(type2>=0 && type2<=5)
            {
              Ptau=(nu2+vis2).P();
              Plep=(nu1+vis1).P();
            }
          if(Plep<50.0 && Plep>=45.0) ind=2;
          if(Plep<45.0 && Plep>=40.0) ind=1;
          if(Plep<40.0) ind=0;
          if(Ptau<50.0 && Ptau>=45.0) indT=2;
          if(Ptau<45.0 && Ptau>=40.0) indT=1;
          if(Ptau<40.0) indT=0;
          if(type1==8) prob=prob*(lep_p5[ind]*TMath::Gaus(R1,lep_p1[ind],lep_p2[ind])+TMath::Landau(R1,lep_p3[ind],lep_p4[ind]))/(1+lep_p5[ind]);
          if(type2==8) prob=prob*(lep_p5[ind]*TMath::Gaus(R2,lep_p1[ind],lep_p2[ind])+TMath::Landau(R2,lep_p3[ind],lep_p4[ind]))/(1+lep_p5[ind]);

          if(type1>=0 && type1<=2) prob=prob*TMath::Gaus(R1,n_1pr[indT],s_1pr[indT]);
          if(type2>=0 && type2<=2) prob=prob*TMath::Gaus(R2,n_1pr[indT],s_1pr[indT]);
          if(type1>=3 && type1<=5) prob=prob*TMath::Gaus(R1,n_3pr[indT],s_3pr[indT]);
          if(type2>=3 && type2<=5) prob=prob*TMath::Gaus(R2,n_3pr[indT],s_3pr[indT]);

        }
      else // high MET Njet=0 category
        {
          const double R1=nu1.P()/vis1.P();
          const double R2=nu2.P()/vis2.P();
          const double lep_p1[4]={0.441,0.64,0.79,0.8692};
          const double lep_p2[4]={0.218,0.28,0.29,0.3304};
          const double lep_p3[4]={0.256,0.33,0.395,0.4105};
          const double lep_p4[4]={0.048,0.072,0.148,0.1335};
          const double lep_p5[4]={0.25,0.68,0.10,0.2872};
          int ind=3;
          const double p_1prong=-3.706;
          const double p_3prong=-5.845;
          double Ptau=0.0;
          double Plep=0.0;
          if(type1>=0 && type1<=5)
            {
              Ptau=(nu1+vis1).P();
              Plep=(nu2+vis2).P();
            }
          if(type2>=0 && type2<=5)
            {
              Ptau=(nu2+vis2).P();
              Plep=(nu1+vis1).P();
            }
          if(Plep<50.0 && Plep>=45.0) ind=2;
          if(Plep<45.0 && Plep>=40.0) ind=1;
          if(Plep<40.0) ind=0;
          const double scale1prong=Ptau>45.0 ? 1.0 : -1.019/((Ptau*0.0074-0.059)*p_1prong);
          const double scale3prong=Ptau>40.0 ? 1.0 : -1.24/((Ptau*0.0062-0.033)*p_3prong);
          if(type1==8) prob=prob*(lep_p5[ind]*TMath::Gaus(R1,lep_p1[ind],lep_p2[ind])+TMath::Landau(R1,lep_p3[ind],lep_p4[ind]))/(1+lep_p5[ind]);
          if(type2==8) prob=prob*(lep_p5[ind]*TMath::Gaus(R2,lep_p1[ind],lep_p2[ind])+TMath::Landau(R2,lep_p3[ind],lep_p4[ind]))/(1+lep_p5[ind]);

          if(type1>=0 && type1<=2) prob=prob*exp(p_1prong*R1*scale1prong)*std::abs(p_1prong*scale1prong)*0.02; // introduced normalization to account for sharpening of probability at low E(tau)
          if(type2>=0 && type2<=2) prob=prob*exp(p_1prong*R2*scale1prong)*std::abs(p_1prong*scale1prong)*0.02;
          if(type1>=3 && type1<=5) prob=prob*exp(p_3prong*R1*scale3prong)*std::abs(p_3prong*scale3prong)*0.02;
          if(type2>=3 && type2<=5) prob=prob*exp(p_3prong*R2*scale3prong)*std::abs(p_3prong*scale3prong)*0.02;
        }
    }
  //----------------- had-had channel ---------------------------------------
  if( preparedInput.m_tauTypes==TauTypes::hh )
    {

      if(m_UseHT) // Events with no jets
        {

          const double R[2]={nu1.P()/vis1.P(),nu2.P()/vis2.P()};
          const double E[2]={(nu1+vis1).E(),(nu2+vis2).E()};
          const int tau_type[2]={type1,type2};
          int order1= vis1.Pt()>vis2.Pt() ? 0 : 1;
          int order2= vis1.Pt()>vis2.Pt() ? 1 : 0;

          double par_1p[2][6]; // P(tau)-scaling; lead, sub-lead
          double par_3p[2][6]; // P(tau)-scaling; lead, sub-lead

          par_1p[0][0]=0.1273; par_1p[0][1]=-0.2479; par_1p[0][2]=1.0; par_1p[0][3]=-43.16; par_1p[0][4]=0.0; par_1p[0][5]=0.0;
          par_1p[1][0]=0.3736; par_1p[1][1]=-1.441; par_1p[1][2]=1.0; par_1p[1][3]=-29.82; par_1p[1][4]=0.0; par_1p[1][5]=0.0;
          par_3p[0][0]=0.1167; par_3p[0][1]=-0.1388; par_3p[0][2]=1.0; par_3p[0][3]=-44.77; par_3p[0][4]=0.0; par_3p[0][5]=0.0;
          par_3p[1][0]=0.3056; par_3p[1][1]=-2.18; par_3p[1][2]=1.0; par_3p[1][3]=-19.09; par_3p[1][4]=0.0; par_3p[1][5]=0.0;
          // parameters for sub-leading tau
          const double C1p=0.062;
          const double C3p=0.052;
          const double G1p=1.055;
          const double G3p=1.093;
          // Probability for leading tau

          if( tau_type[order1]>=0 && tau_type[order1]<=2 ) // 1-prong
            {
              //double x=std::min(300.,std::max(E[order1],45.0));
              // 50 to be safe. TO be finalised.
              //              double x=std::min(300.,std::max(E[order1],50.0));
              double x=std::min(E[order1],300.0);
              const double slope=par_1p[0][0]+par_1p[0][1]/(par_1p[0][2]*x+par_1p[0][3])+par_1p[0][4]*x > 0.01 ?
                par_1p[0][0]+par_1p[0][1]/(par_1p[0][2]*x+par_1p[0][3])+par_1p[0][4]*x : 0.01;
              prob=prob*exp(-R[order1]/slope)*0.04/std::abs(slope);
            }
          if( tau_type[order1]>=3 && tau_type[order1]<=5 ) // 3-prong
            {
              //double x=std::min(300.,std::max(E[order1],45.0));
              //              double x=std::min(300.,std::max(E[order1],50.0));
              double x=std::min(E[order1],300.0);
              const double slope=par_3p[0][0]+par_3p[0][1]/(par_3p[0][2]*x+par_3p[0][3])+par_3p[0][4]*x > 0.01 ?
                par_3p[0][0]+par_3p[0][1]/(par_3p[0][2]*x+par_3p[0][3])+par_3p[0][4]*x : 0.01;
              prob=prob*exp(-R[order1]/slope)*0.04/std::abs(slope);
            }
          // Probability for sub-leading tau
          if( tau_type[order2]>=0 && tau_type[order2]<=2 ) // 1-prong
            {
              const double par[4]={0.1147,-0.09675,-35.0,3.711E-11};
              double x=std::min(300.,std::max(E[order2],30.0));
              //              double x=std::min(300.,std::max(E[order2],50.0));
              const double sigma=G1p*(par_1p[1][0]+par_1p[1][1]/(par_1p[1][2]*x+par_1p[1][3])+par_1p[1][4]*x+par_1p[1][5]*x*x) > 0.01 ?
                G1p*(par_1p[1][0]+par_1p[1][1]/(par_1p[1][2]*x+par_1p[1][3])+par_1p[1][4]*x+par_1p[1][5]*x*x) : 0.01;
              if(x<36.0) x=36.0;
              const double mean=par[0]+par[1]/(x+par[2])+par[3]*pow(x,4);
              prob=prob*C1p*TMath::Gaus(R[order2],mean,sigma);
            }
          if( tau_type[order2]>=3 && tau_type[order2]<=5 ) // 3-prong
            {
              double x=std::min(300.,std::max(E[order2],20.0));
              //              double x=std::min(300.,std::max(E[order2],50.0));
              const double sigma=G3p*(par_3p[1][0]+par_3p[1][1]/(par_3p[1][2]*x+par_3p[1][3])+par_3p[1][4]*x+par_3p[1][5]*x*x) > 0.01 ?
                G3p*(par_3p[1][0]+par_3p[1][1]/(par_3p[1][2]*x+par_3p[1][3])+par_3p[1][4]*x+par_3p[1][5]*x*x) : 0.01;
              const double par[4]={0.2302,-2.012,-36.08,-0.000373};
              if(x<37.0) x=37.0;
              const double mean=par[0]+par[1]/(x+par[2])+par[3]*x;
              prob=prob*C3p*TMath::Gaus(R[order2],mean,sigma);
            }
        }
      if(!m_UseHT) // Events with jets
        {
          const double R1=nu1.P()/vis1.P();
          const double R2=nu2.P()/vis2.P();
          const double E1=(nu1+vis1).E();
          const double E2=(nu2+vis2).E();
          int order1= vis1.Pt()>vis2.Pt() ? 0 : 1;
          int order2= vis1.Pt()>vis2.Pt() ? 1 : 0;
          const double slope_1p[2]={-3.185,-2.052}; // lead, sub-lead
          const double slope_3p[2]={-3.876,-2.853}; // lead, sub-lead
          double par_1p[2][3]; // scaling below 100 GeV; lead, sub-lead
          double par_3p[2][3]; // scaling below 100 GeV; lead, sub-lead
          par_1p[0][0]=-0.3745; par_1p[0][1]=0.01417; par_1p[0][2]=-7.285E-5; // [0][i] is always adjusted to match slope at 100 GeV
          par_1p[1][0]=-0.3683; par_1p[1][1]=0.01807; par_1p[1][2]=-9.514E-5;
          par_3p[0][0]=-0.3055; par_3p[0][1]=0.01149; par_3p[0][2]=-5.855E-5;
          par_3p[1][0]=-0.3410; par_3p[1][1]=0.01638; par_3p[1][2]=-9.465E-5;
          double scale1;
          double scale2;
          if(type1>=0 && type1<=2) // 1-prong
            {
              scale1=E1>100.0 ? 1.0 : 1.0/std::abs((par_1p[order1][0]+par_1p[order1][1]*E1+par_1p[order1][2]*E1*E1)*slope_1p[order1]);
              if(scale1<1.0) scale1=1.0;
              if(scale1>100.0) scale1=100.0;
              prob=prob*std::abs(slope_1p[order1])*scale1*exp(slope_1p[order1]*scale1*R1)*0.04;
            }
          if(type1>=3 && type1<=5) // 3-prong
            {
              scale1=E1>100.0 ? 1.0 : 1.0/std::abs((par_3p[order1][0]+par_3p[order1][1]*E1+par_3p[order1][2]*E1*E1)*slope_3p[order1]);
              if(scale1<1.0) scale1=1.0;
              if(scale1>100.0) scale1=100.0;
              prob=prob*std::abs(slope_3p[order1])*scale1*exp(slope_3p[order1]*scale1*R1)*0.04;
            }
          if(type2>=0 && type2<=2) // 1-prong
            {
              scale2=E2>100.0 ? 1.0 : 1.0/std::abs((par_1p[order2][0]+par_1p[order2][1]*E2+par_1p[order2][2]*E2*E2)*slope_1p[order2]);
              if(scale2<1.0) scale2=1.0;
              if(scale2>100.0) scale2=100.0;
              prob=prob*std::abs(slope_1p[order2])*scale2*exp(slope_1p[order2]*scale2*R2)*0.04;
            }
          if(type2>=3 && type2<=5) // 3-prong
            {
              scale2=E2>100.0 ? 1.0 : 1.0/std::abs((par_3p[order2][0]+par_3p[order2][1]*E2+par_3p[order2][2]*E2*E2)*slope_3p[order2]);
              if(scale2<1.0) scale2=1.0;
              if(scale2>100.0) scale2=100.0;
              prob=prob*std::abs(slope_3p[order2])*scale2*exp(slope_3p[order2]*scale2*R2)*0.04;

            }
        }

    }
  //   prob=std::min(prob,1.); // Sasha commented out this line, it was introduced by David. Have to ask about its purpose.

  return prob;
}


// returns Mnu probability according pol6 parameterization
double MissingMassProb::MnuProbability(MissingMassInput& preparedInput, double mnu, double binsize)
{
  double prob=1.0;
  double norm=4851900.0;
  double p[7];
  p[0]=-288.6/norm; p[1]=6.217E4/(2.0*norm); p[2]=2.122E4/(3.0*norm); p[3]=-9.067E4/(4.0*norm);
  p[4]=1.433E5/(5.0*norm); p[5]=-1.229E5/(6.0*norm); p[6]=3.434E4/(7.0*norm);
  double int1=0.0;
  double int2=0.0;
  double x1= mnu+0.5*binsize < 1.777-0.113 ? mnu+0.5*binsize : 1.777-0.113;
  double x2= mnu-0.5*binsize > 0.0 ? mnu-0.5*binsize : 0.0;
  for(int i=0; i<7; i++)
    {
      int1=p[i]*pow(x1,i+1)+int1;
      int2=p[i]*pow(x2,i+1)+int2;
    }
  prob=int1-int2;
  if(prob<0.0)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::MnuProbability: negative probability!!! ");
      return 0.0;
    }
  if(prob>1.0)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::MnuProbability: probability > 1!!! ");
      return 1.0;
    }
  return prob;
}

// returns Mnu probability according pol6 parameterization
double MissingMassProb::MnuProbability(MissingMassInput& preparedInput, double mnu)
{
  if(m_fUseMnuProbability==0) return 1.0;
  double prob=1.0;
  const double norm=4851900.0;
  double p[7];
  p[0]=-288.6; p[1]=6.217E4; p[2]=2.122E4; p[3]=-9.067E4;
  p[4]=1.433E5; p[5]=-1.229E5; p[6]=3.434E4;
  double int1=0.0;
  for(int i=0; i<7; i++)
    {
      int1+=p[i]*pow(mnu,i);
    }
  prob=int1/norm;
  if(prob<0.0)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::MnuProbability: negative probability!!! ");
      return 0.0;
    }
  if(prob>1.0)
    {
      if(preparedInput.m_fUseVerbose==1) Warning("DiTauMassTools", "Warning in MissingMassCalculator::MnuProbability: probability > 1!!! ");
      return 1.0;
    }
  return prob;
}

double MissingMassProb::MHtProbability(MissingMassInput& preparedInput, const double & d_mhtX, const double & d_mhtY, const double & mht,
                                             const double & trueMetGuess, const double & mht_offset) {
  // all param except trueMetguess unchanged in one event. So can cache agaisnt this one.
  //disable cache if (trueMetGuess==trueMetGuesscache) return mhtprobcache;
  double mhtprob;
  //  if(MHtSigma1>0.0 && MHtSigma2>0.0 && MHtGaussFr>0.0)

  // if RANDOMNONUNIF MET already follow the double gaussian parameterisation. So weight should not include it to avoid double counting
  // formula to be checked IMHO the two gaussian should be correlated
    mhtprob=exp(-0.5*pow(d_mhtX/preparedInput.m_MHtSigma1,2))+preparedInput.m_MHtGaussFr*exp(-0.5*pow(d_mhtX/preparedInput.m_MHtSigma2,2));
    mhtprob*=(exp(-0.5*pow(d_mhtY/preparedInput.m_MHtSigma1,2))+preparedInput.m_MHtGaussFr*exp(-0.5*pow(d_mhtY/preparedInput.m_MHtSigma2,2)));

  const double n_arg=(mht-trueMetGuess-mht_offset)/preparedInput.m_MHtSigma1;
  mhtprob*=exp(-0.25*pow(n_arg,2)); // assuming sqrt(2)*sigma
  return mhtprob;
}

double MissingMassProb::MHtProbabilityHH(MissingMassInput& preparedInput, const double & d_mhtX, const double & d_mhtY, const double & mht,
                                               const double & trueMetGuess, const double & mht_offset) {
  double prob=1.0;

  // updated for final cuts, May 21 2012
  // should be merged
  prob=prob*(0.0256*TMath::Gaus(d_mhtX,0.0,preparedInput.m_MHtSigma1)+0.01754*TMath::Gaus(d_mhtX,0.0,preparedInput.m_MHtSigma2));
  prob=prob*(0.0256*TMath::Gaus(d_mhtY,0.0,preparedInput.m_MHtSigma1)+0.01754*TMath::Gaus(d_mhtY,0.0,preparedInput.m_MHtSigma2));
  const double n_arg=(mht-trueMetGuess-mht_offset)/5.7; // this sigma is different from MHtSigma1; actually it depends on dPhi
  prob=prob*exp(-0.5*pow(n_arg,2))/(5.7*sqrt(2.0*TMath::Pi())); // assuming sigma from above line

  return prob;
}

//SpeedUp static instantation
// first index is the calibration set : 0: MMC2011, 1:MMC2012
// second index is the decay 0 : lepton, 1 : 1 prong, 2 3 prong
double MissingMassProb::s_fit_param[2][3][6][5];
// first parameter: 0- for 1-prong; 1- for 3-prong
double MissingMassProb::s_ter_sigma_par[2][10][3];

// returns dTheta3D probability based on ATLAS parameterization
double MissingMassProb::dTheta3d_probabilityFast(MissingMassInput& preparedInput, const int & tau_type,const double & dTheta3d,const  double & P_tau) {
  double prob=1.0E-10;
  int tau_code;  // 0: l, 1:1-prong, 2:3-prong
  if(tau_type==8) tau_code = 0;
  else if(tau_type>=0 && tau_type<=2) tau_code = 1;
  else if(tau_type>=3 && tau_type<=5) tau_code = 2;
  else
    {
      Warning("DiTauMassTools", "---- WARNING in MissingMassCalculator::dTheta3d_probabilityFast() ----");
      Warning("DiTauMassTools", "%s", ("..... wrong tau_type="+std::to_string(tau_type)).c_str());
      Warning("DiTauMassTools", "%s", ("..... returning prob="+std::to_string(prob)).c_str());
      Warning("DiTauMassTools", "____________________________________________________________");
      return prob;
    }


  double myDelThetaParam[6];

  for (int i=0;i<6;++i)
    {
      if(m_mmcCalibrationSet==MMCCalibrationSetV2::UPGRADE
         || m_mmcCalibrationSet==MMCCalibrationSetV2::LFVMMC2012
         || m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS)
        myDelThetaParam[i]=dTheta3Dparam(i,tau_code,P_tau,s_fit_param[1][tau_code][i]);
    }
  double dTheta3dVal=dTheta3d;

  if (tau_type==8) prob=myDelThetaLepFunc(&dTheta3dVal,  myDelThetaParam);
  else prob=myDelThetaHadFunc(&dTheta3dVal,  myDelThetaParam);

  if (false)
    {

      if( preparedInput.m_fUseVerbose==1 && (prob>1.0 || prob<0.0))
        {
          Warning("DiTauMassTools", "---- WARNING in MissingMassCalculator::dTheta3d_probabilityFast() ----");
          Warning("DiTauMassTools", "%s", ("..... wrong probability="+std::to_string(prob)).c_str());
          Warning("DiTauMassTools", "%s", ("..... debugging: tau_type="+std::to_string(tau_type)+"dTheta3d="+std::to_string(dTheta3d)+"  P_tau="+std::to_string(P_tau)).c_str());
          Warning("DiTauMassTools", "____________________________________________________________");
          prob=1.0E-10;
        }
    }

  return prob;
}

// dTheta probability density function for hadronic taus
double MissingMassProb::myDelThetaHadFunc(double *x, double *par)
{
  double fitval=1.0E-10;
  if(x[0]>TMath::Pi() || x[0]<0.0) return fitval;
  const double arg=x[0];
  const double arg_L=arg;
  const double mean=par[1];
  const double sigmaG=par[2];
  const double mpv=par[3];
  const double sigmaL=par[4];

  if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS
             || m_mmcCalibrationSet==MMCCalibrationSetV2::UPGRADE
             || m_mmcCalibrationSet==MMCCalibrationSetV2::LFVMMC2012){
    const double norm=sqrt(2.0*TMath::Pi());
    const double g1=TMath::Gaus(arg,mean,sigmaG)/norm;
    const double g2=TMath::Landau(arg_L,mpv,sigmaL)/norm;
    fitval=par[0]*g1/sigmaG+par[5]*g2/sigmaL;
  }

  if(fitval<0.0) return 0.0;
  return fitval;
}

// dTheta probability density function for leptonic taus
double MissingMassProb::myDelThetaLepFunc(double *x, double *par)
{
  double fitval=1.0E-10;
  if(x[0]>TMath::Pi() || x[0]<0.0) return fitval;
  double arg=x[0]/par[1];

  double normL=par[5];
  if(normL<0.0) normL=0.0;

  if(arg<1) arg=sqrt(std::abs(arg));
  else arg=arg*arg;
  const double arg_L=x[0];
  const double mean=1.0;
  const double sigmaG=par[2];
  const double mpv=par[3];
  const double sigmaL=par[4];
  const double g1=normL*TMath::Gaus(arg,mean,sigmaG);
  const double g2=TMath::Landau(arg_L,mpv,sigmaL);
  fitval=par[0]*(g1+g2)/(1.0+normL);
  if(fitval<0.0) return 0.0;
  return fitval;
}

// returns parameters for dTheta3D pdf
double MissingMassProb::dTheta3Dparam(const int & parInd, const int & tau_type, const double & P_tau, const double *par) {
  //tau_type 0: l, 1:1-prong, 3:3-prong
  if(P_tau<0.0) return 0.0;


  if(parInd==0) {
    if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS
             || m_mmcCalibrationSet==MMCCalibrationSetV2::UPGRADE
             || m_mmcCalibrationSet==MMCCalibrationSetV2::LFVMMC2012){
      return (par[0]+par[1]*P_tau+par[2]*pow(P_tau,2)+par[3]*pow(P_tau,3)+par[4]*pow(P_tau,4))*0.00125;
    }
  }
  else { // parInd==0
    if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS
             || m_mmcCalibrationSet==MMCCalibrationSetV2::UPGRADE
             || m_mmcCalibrationSet==MMCCalibrationSetV2::LFVMMC2012){
      if(tau_type==0) return par[0]*(exp(-par[1]*P_tau)+par[2]/P_tau)+par[3]+par[4]*P_tau;
      else return par[0]*(exp(-par[1]*sqrt(P_tau))+par[2]/P_tau)+par[3]+par[4]*P_tau;
    }
  }
  return 0.;
}

/*
// Sasha: keep this for now, may need in the future
// returns analytical P(theta)-probability for given tau-topology
// decayType==1 for leptonic decays and 0 for hadronic decays
// uses product of probabilities
double MissingMassProb::AngularProbability(TLorentzVector nu_vec, TLorentzVector vis_vec, int decayType) {
  double prob=0.0;
  double M=1.777;
  double angl=0.0;
  double P=(vis_vec+nu_vec).P();
  double V=P/sqrt(P*P+M*M); // tau speed
  double dA=dRmax_tau/(2.0*Niter_fit1);

  if(decayType==1) // leptonic tau for now
    {
      // exact formular for energy probability is sqrt(1-V^2)/(2*V*p_0)*dE
      double m_1=nu_vec.M();
      double m_2=vis_vec.M();
      double E_nu=(M*M+m_1*m_1-m_2*m_2)/(2.0*M);
      if(E_nu<=m_1) return 0.0;
      double P_nu=sqrt(pow(E_nu,2)-pow(m_1,2));
      double prob1=0.5*sqrt(1-V*V)/(P_nu*V); // energy probability

      angl=Angle(vis_vec,vis_vec+nu_vec); // using lepton direction
      double det1=1.0-V*cos(angl+dA);
      double det2= (angl-dA)>0.0 ? 1.0-V*cos(angl-dA) : 1.0-V;
      double prob2=std::abs(1.0/det1-1.0/det2)*(1.0-V*V)/(2.0*V); // using massless approximation for leptons
      prob=prob1*prob2;
    }
  if(decayType==0) // hadronic tau
    {
      // exact formula for energy probability is sqrt(1-V^2)/(2*V*p_0)*dE
      // drop p_0 because it's a contstant for a given hadronic tau
      double prob1=0.5*sqrt(1-V*V)/V; // "energy" probability

      angl=Angle(nu_vec,vis_vec+nu_vec); // using neutrino direction
      double det1=1.0-V*cos(angl+dA);
      double det2= (angl-dA)>0.0 ? 1.0-V*cos(angl-dA) : 1.0-V;
      double prob2=std::abs(1.0/det1-1.0/det2)*(1.0-V*V)/(2.0*V);
      prob=prob1*prob2;
    }
  return prob;
}

// returns probability of angle between two tau's
// assuming massless tau's for now, should be small effect for M>M_Z
double MissingMassProb::ResonanceProbability(TLorentzVector vec1, TLorentzVector vec2) {

  double prob=1.0;
  double boson_P=(vec1+vec2).P();
  if(boson_P==0.0) return 1.0;
  double boson_E=(vec1+vec2).E();
  double boson_V=0.0;
  if(boson_E>0.0) boson_V=boson_P/boson_E;
  else return 1.0E-10;

  double testMass=(vec1+vec2).M();
  double m=1.777; // tau mass
  double E_tau=testMass/2.0;
  double P_tau=sqrt(pow(E_tau,2)-m*m);
  double Evis_lim[2];
  Evis_lim[0]=(E_tau-boson_V*P_tau)/sqrt(1.0-boson_V*boson_V);
  Evis_lim[1]=(E_tau+boson_V*P_tau)/sqrt(1.0-boson_V*boson_V);
  if(vec1.E()<Evis_lim[0] || vec1.E()>Evis_lim[1]) return 1.0E-20;
  if(vec2.E()<Evis_lim[0] || vec2.E()>Evis_lim[1]) return 1.0E-20;

  double prob1=0.5*sqrt(1-boson_V*boson_V)/(P_tau*boson_V);

  if(vec1.P()*vec2.P()>0)
    {
      double theta=acos((vec1.Px()*vec2.Px()+vec1.Py()*vec2.Py()+vec1.Pz()*vec2.Pz())/(vec1.P()*vec2.P()));
      if(boson_V>0.0 && boson_V<1.0)
        {
          if(boson_V<cos(theta/2)) return 1.0E-10;
          double num=(1.0-boson_V*boson_V)*cos(theta/2);
          double denom=4*boson_V*sin(theta/2)*sin(theta/2)*sqrt(boson_V*boson_V-cos(theta/2)*cos(theta/2));
          prob=num/denom;
        }
      else
        {
          if(std::abs(theta-TMath::Pi())>0.0001) return 1.0E-10;
        }
    }
  else return 1.0E-10;
  prob=prob*prob1;
  if(prob<1.0E-20) prob=1.0E-20;
  return prob;
}
*/

void MissingMassProb::MET(MissingMassInput& preparedInput){
  // compute MET resolution (eventually use HT)
  if(preparedInput.m_METsigmaP<0.1 || preparedInput.m_METsigmaL<0.1)
    {
      if(m_mmcCalibrationSet==MMCCalibrationSetV2::LFVMMC2012)
        {
          if(preparedInput.m_fUseVerbose==1) { Info("DiTauMassTools", "Attempting to set LFV MMC settings"); }
          double mT1 = mT(preparedInput.m_vistau1,preparedInput.m_MetVec);
          double mT2 = mT(preparedInput.m_vistau2,preparedInput.m_MetVec);
          int sr_switch = 0;
          if(preparedInput.m_vistau1.M()<0.12 && preparedInput.m_vistau2.M()<0.12) // lep-lep case
            {
              if(preparedInput.m_LFVmode==0) // e+tau(->mu) case
                {
                  if(preparedInput.m_vistau1.M()<0.05 && preparedInput.m_vistau2.M()>0.05)
                    {
                      if(mT1>mT2) sr_switch = 0; // SR1
                      else sr_switch = 1; // SR2
                    }
                  else
                    {
                      if(mT1>mT2) sr_switch = 1; // SR2
                      else sr_switch = 0; // SR1
                    }
                }
              if(preparedInput.m_LFVmode==1) // mu+tau(->e) case
                {
                  if(preparedInput.m_vistau1.M()>0.05 && preparedInput.m_vistau2.M()<0.05)
                    {
                      if(mT1>mT2) sr_switch = 0; // SR1
                      else sr_switch = 1; // SR2
                    }
                  else
                    {
                      if(mT1>mT2) sr_switch = 1; // SR2
                      else sr_switch = 0; // SR1
                    }
                }
            }
          if((preparedInput.m_vistau1.M()<0.12 && preparedInput.m_vistau2.M()>0.12) || (preparedInput.m_vistau2.M()<0.12 && preparedInput.m_vistau1.M()>0.12)) // lep-had case
            {
              if(preparedInput.m_vistau1.M()<0.12 && preparedInput.m_vistau2.M()>0.12)
                {
                  if(mT1>mT2) sr_switch = 0; // SR1
                  else sr_switch = 1; // SR2
                }
              else
                {
                  if(mT1>mT2) sr_switch = 1; // SR2
                  else sr_switch = 0; // SR1
                }
            }

          m_UseHT = false;
          if(preparedInput.m_Njet25==0) // 0-jet
            {
              double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
              double METresScale;
              double METoffset;
              if(sr_switch==0)
                {
                  METresScale = 0.41*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                  METoffset = 7.36*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                }
              else
                {
                  METresScale = 0.34*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                  METoffset = 6.61*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                }
              if(preparedInput.m_fUseVerbose==1) {
                Info("DiTauMassTools", "%s", ("SumEt = "+std::to_string(preparedInput.m_SumEt)).c_str());
                Info("DiTauMassTools", "%s", ("METoffset = "+std::to_string(METoffset)).c_str());
                Info("DiTauMassTools", "%s", ("METresScale = "+std::to_string(METresScale)).c_str());
              }

              double sigma = preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
              preparedInput.m_METsigmaP = sigma;
              preparedInput.m_METsigmaL = sigma;
              if(preparedInput.m_fUseVerbose==1) {
                Info("DiTauMassTools", "%s", ("=> METsigmaP = "+std::to_string(preparedInput.m_METsigmaP)).c_str());
                Info("DiTauMassTools", "%s", ("=> METsigmaL = "+std::to_string(preparedInput.m_METsigmaL)).c_str());
              }
            }
          if(preparedInput.m_Njet25>0) // Inclusive 1-jet
            {
              double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
              double sigma = 0.;
              double METresScale;
              double METoffset;
              if(sr_switch==0)
                {
                  METresScale = 0.38*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                  METoffset = 7.96*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                }
              else
                {
                  METresScale = 0.39*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                  METoffset = 6.61*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                }

              // MET resolution can't be perfect in presence of other objects (i.e., electrons, jets, taus), so assume minSumEt = 5.0 for now
              sigma = preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
              preparedInput.m_METsigmaP = sigma;
              preparedInput.m_METsigmaL = sigma;
            } // Njet25>0
        }
      else //LFV
        {
          //DRDRMERGE end addition

          if(preparedInput.m_METScanScheme==1) // default for Winter 2012 and further
            {
	      //LEP-HAD
              if ( preparedInput.m_tauTypes==TauTypes::lh ) // lephad case
                {
		  //0-jet
                  if(preparedInput.m_Njet25==0)//0-jet
                    {
                      // placeholder for 2019 tune
                      if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2019){
                        if(preparedInput.m_MetVec.Mod()<20.0) // 0-jet low MET case
                          {
                            if(std::abs(preparedInput.m_DelPhiTT)>2.95 && m_allowUseHT) // use mHt only if dPhi(lep-tau)>2.95
                              {
                                m_UseHT = true;
                                // giving priority to external settings
                                if(preparedInput.m_MHtSigma1<0.0) preparedInput.m_MHtSigma1 = 4.822;
                                if(preparedInput.m_MHtSigma2<0.0) preparedInput.m_MHtSigma2 = 10.31;
                                if(preparedInput.m_MHtGaussFr<0.0) preparedInput.m_MHtGaussFr = 6.34E-5;
                              }
                            else
                              {
                                m_UseHT = false;
                                double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
                                double METresScale = 0.32*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                                double METoffset = 5.38*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                                double sigma =  preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
                                preparedInput.m_METsigmaP = sigma;
                                preparedInput.m_METsigmaL = sigma;
                              }
                          }
                        else // 0-jet high MET case
                          {
                            if(std::abs(preparedInput.m_DelPhiTT)>2.8 && m_allowUseHT) // use mHt only if dPhi(lep-tau)>2.8
                              {
                                m_UseHT = true;
                                // giving priority to external settings
                                if(preparedInput.m_MHtSigma1<0.0) preparedInput.m_MHtSigma1 = 7.5;
                                if(preparedInput.m_MHtSigma2<0.0) preparedInput.m_MHtSigma2 = 13.51;
                                if(preparedInput.m_MHtGaussFr<0.0) preparedInput.m_MHtGaussFr = 6.81E-4;
                                preparedInput.m_METsigmaP = preparedInput.m_MHtSigma2; // sigma of 2nd Gaussian for missing Ht resolution
                                preparedInput.m_METsigmaL = preparedInput.m_MHtSigma2;
                              }
                            else
                              {
                                m_UseHT = false;
                                double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
                                double METresScale = 0.87*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                                double METoffset = 4.16*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                                double sigma =  preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
                                preparedInput.m_METsigmaP = sigma;
                                preparedInput.m_METsigmaL = sigma;
                              }
                          } // high MET
                      } // MMC2019
                      // 2015 high-mass tune; avergare MET resolution for Mh=600,1000 mass points
                      else if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS)
                        {
                          m_UseHT = false;
                          double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
                          double METresScale = 0.65*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double METoffset = 5.0*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double sigma =  preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
                          preparedInput.m_METsigmaP = sigma;
                          preparedInput.m_METsigmaL = sigma;
                        } // MMC2015HIGHMASS
                    } // 0 jet
		  //1-jet
                  else if(preparedInput.m_Njet25>0) // Inclusive 1-jet and VBF lep-had categories for Winter 2012
                    {
                      double sigmaSyst=0.10; // 10% systematics for now (be conservative)
                      double sigma=0.;
                      // 2015 high-mass tune; average MET resolution for Mh=400,600 mass points (they look consistent);
                      if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS)
                        {
                          double METresScale=0.86*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double METoffset=3.0*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          // MET resolution can't be perfect in presence of other objects (i.e., electrons, jets, taus), so assume minSumEt=5.0 for now
                          sigma= preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
                        }
                      //2019
		      else if (m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2019)
                        {
			  double x = preparedInput.m_DelPhiTT;
			  double dphi_scale = x > 0.3 ? 0.9429 - 0.059*x + 0.054*x*x : 0.728;
			  double METoffset    = 1.875*(1.0+preparedInput.m_METresSyst*sigmaSyst);
			  double METresScale1 = 8.914*(1.0+preparedInput.m_METresSyst*sigmaSyst);
			  double METresScale2 = -8.53*(1.0+preparedInput.m_METresSyst*sigmaSyst);


			  sigma = preparedInput.m_SumEt > 80.0 ? METoffset + METresScale1*TMath::Log(sqrt(preparedInput.m_SumEt)+METresScale2) : 5.0;
			  sigma = sigma * dphi_scale;
                        }
		      //

                      preparedInput.m_METsigmaP=sigma;
                      preparedInput.m_METsigmaL=sigma;
                    } // Njet25>0
		  
                } // lep-had

	      //HAD-HAD
              else if(preparedInput.m_tauTypes==TauTypes::hh) // had-had events
                {
                  if(preparedInput.m_Njet25==0 && m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS) //0-jet high mass hadhad
                    {
                      // 2015 high-mass tune; average of all mass points
                      //                      double METresScale=-1.;
                      //                      double METoffset=-1.;
                      double sigmaSyst=0.10; // 10% systematics for now (be conservative)

                      double METresScale=0.9*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      double METoffset=-1.8*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      double sigma= preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : std::abs(METoffset);
                      preparedInput.m_METsigmaP=sigma;
                      preparedInput.m_METsigmaL=sigma;

                    }
                  else if(preparedInput.m_Njet25==0 && m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2019)
                    {
                      double sigmaSyst=0.10; // 10% systematics for now (be conservative)
                      double x = preparedInput.m_DelPhiTT;
                      double dphi_scale = x > 2.5 ? 11.0796 - 4.61236*x + 0.423617*x*x : 2.;
                      double METoffset = -8.51013*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      double METresScale1 = 8.54378*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      double METresScale2 = -3.97146*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      double sigma= preparedInput.m_SumEt>80.0 ? METoffset+METresScale1*TMath::Log(sqrt(preparedInput.m_SumEt)+METresScale2) : 5.;
                      sigma = sigma*dphi_scale;

                      preparedInput.m_METsigmaP=sigma;
                      preparedInput.m_METsigmaL=sigma;

                    }
                  else if(preparedInput.m_Njet25==0 && m_allowUseHT) // 0-jet high MET had-had category for Winter 2012
                    {

                      m_UseHT=true; // uncomment this line to enable HT also for HH (crucial)
                      // updated for final cuts, may 21 2012
                      if(preparedInput.m_MHtSigma1<0.0) preparedInput.m_MHtSigma1=5.972;
                      if(preparedInput.m_MHtSigma2<0.0) preparedInput.m_MHtSigma2=13.85;
                      //                  if(MHtGaussFr<0.0) MHtGaussFr=0.4767; // don't directly use 2nd fraction
                    }
		  //1-jet
                  else  // Inclusive 1-jet and VBF categories
                    {
                      double METresScale=-1.;
                      double METoffset=-1.;
                      double sigmaSyst=0.10; // 10% systematics for now (be conservative)

                      // previous value in trunk
                      if(m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS) {
                        // 2015 high-mass tune; average of all mass points
                        METresScale = 1.1*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                        METoffset = -5.0*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                      }
                      // MET resolution can't be perfect in presence of other objects (i.e., electrons, jets, taus), so assume minSumEt=5.0 for now
                      double sigma =  preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : std::abs(METoffset);

                      if(m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2019) {
                        double x = preparedInput.m_DelPhiTT;
                        double dphi_scale = x > 0.6 ? 1.42047 - 0.666644*x + 0.199986*x*x : 1.02;
                        METoffset = 1.19769*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                        double METresScale1 = 5.61687*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                        double METresScale2 = -4.2076*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                        sigma= preparedInput.m_SumEt>115.0 ? METoffset+METresScale1*TMath::Log(sqrt(preparedInput.m_SumEt)+METresScale2) : 12.1;
                        sigma = sigma*dphi_scale;
                      } //for hh 2016 mc15c

                      preparedInput.m_METsigmaP = sigma;
                      preparedInput.m_METsigmaL = sigma;

                    }// 1 jet
                } // had-had
	      //LEP-LEP
              else if(preparedInput.m_tauTypes==TauTypes::ll) // setup for LEP-LEP channel
                {
                  if(m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2015HIGHMASS) // placeholder for 2015 high-mass tune; for now it is the same as 2012
                    {
                      m_UseHT = false;
                      double sigmaSyst = 0.10; // 10% systematics for now (be conservative)
                      double METresScale = -1.0;
                      double METoffset = -1.0;
                      double sigma = 5.0;
                      // tune is based on cuts for Run-1 paper analysis
                      if(preparedInput.m_Njet25==0)
                        {
                          // use tune for emebedding
                          METresScale=-0.4307*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          METoffset=7.06*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double METresScale2=0.07693*(1.0+preparedInput.m_METresSyst*sigmaSyst); // quadratic term
                          // this is a tune for Higgs125
                          //                      METresScale=-0.5355*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          //                      METoffset=11.5*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          //                      double METresScale2=0.07196*(1.0+preparedInput.m_METresSyst*sigmaSyst); // quadratic term
                          sigma= preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt)+METresScale2*preparedInput.m_SumEt : METoffset;
                        }
                      if(preparedInput.m_Njet25>0)
                        {
                          // use tune for embedding
                          METresScale=0.8149*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          METoffset=5.343*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          // this is a tune for Higgs125
                          //                      METresScale=0.599*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          //                      METoffset=8.223*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          sigma= preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt) : METoffset;
                        }
                      preparedInput.m_METsigmaP = sigma;
                      preparedInput.m_METsigmaL = sigma;
                    } // end of MMC2015HIGHMASS

                  if(m_mmcCalibrationSet==MMCCalibrationSetV2::MMC2019) // 2019 leplep
                    {
		      m_UseHT=false;
                      double sigmaSyst=0.10; // 10% systematics for now (be conservative)
                      double METresScale=-1.0;
                      double METoffset=-1.0;
                      double sigma=5.0;
                      double min_sigma = 2.0;
                      // tune is based on cuts for Run-1 paper analysis
                      if(preparedInput.m_Njet25==0)
                        {
                          // Madgraph Ztautau MET param
                          METoffset = 4.22581*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          METresScale = 0.03818*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double METresScale2= 0.12623;
                          sigma= preparedInput.m_SumEt>0.0 ? METoffset+METresScale*sqrt(preparedInput.m_SumEt)+METresScale2*preparedInput.m_SumEt : min_sigma;
                          if (m_fUseDphiLL) {
                            double p0 = 2.60131;
                            double p1const = 1.22427;
                            double p2quad = -1.71261;
                            double DphiLL = std::abs(TVector2::Phi_mpi_pi(preparedInput.m_vistau1.Phi()-preparedInput.m_vistau2.Phi()));
                            sigma *= (DphiLL < p0) ? p1const : p1const+
                              p2quad*p0*p0 - 2*p2quad*p0*DphiLL+p2quad*DphiLL*DphiLL;
                          }
                          if (sigma < min_sigma) sigma = min_sigma;
                        }
		      if(preparedInput.m_Njet25>0)
                        {
                          // Madgraph Ztautau MET param
                          METoffset = 5.42506*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          METresScale = 5.36760*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          double METoffset2 = -4.86808*(1.0+preparedInput.m_METresSyst*sigmaSyst);
                          if (preparedInput.m_SumEt > 0.0) {
                            double x = sqrt(preparedInput.m_SumEt);
                            sigma = (x+METoffset2 > 1) ? METoffset+METresScale*log(x+METoffset2) : METoffset;
                          } else {
                            sigma = METoffset;
                          }
                          if (m_fUseDphiLL) {
                            double p0 = 2.24786;
                            double p1const = 0.908597;
                            double p2quad = 0.544577;
                            double DphiLL = std::abs(TVector2::Phi_mpi_pi(preparedInput.m_vistau1.Phi()-preparedInput.m_vistau2.Phi()));
                            sigma *= (DphiLL < p0) ? p1const : p1const+
                              p2quad*p0*p0 - 2*p2quad*p0*DphiLL+p2quad*DphiLL*DphiLL;
                          }
                          if (sigma < min_sigma) sigma = min_sigma;
                        }
                      preparedInput.m_METsigmaP=sigma;
                      preparedInput.m_METsigmaL=sigma;
                    }//2016 mc15c

                } // lep-lep

            } //preparedInput.METScanScheme

          if(preparedInput.m_METScanScheme==0) // old scheme with JER
            {
              if(preparedInput.m_dataType==0 || preparedInput.m_dataType==1) preparedInput.SetMetScanParamsUE(preparedInput.m_SumEt,preparedInput.m_METcovphi,preparedInput.m_dataType);
              else preparedInput.SetMetScanParamsUE(preparedInput.m_SumEt,preparedInput.m_METcovphi,0);
            }
        }
    } // end else LFV

  return;
}
