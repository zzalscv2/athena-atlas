/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "CLHEP/Random/RandPoisson.h"

#include "ISF_FastCaloSimEvent/TFCSLateralShapeParametrizationHitNumberFromE.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"

#include "TMath.h"

//=============================================
//======= TFCSHistoLateralShapeParametrization =========
//=============================================

TFCSLateralShapeParametrizationHitNumberFromE::TFCSLateralShapeParametrizationHitNumberFromE(const char* name, const char* title,double stochastic,double constant) :
  TFCSLateralShapeParametrizationHitBase(name,title),m_stochastic(stochastic),m_stochastic_hadron(0.0),m_constant(constant)
{
  set_match_all_pdgid();
}

TFCSLateralShapeParametrizationHitNumberFromE::TFCSLateralShapeParametrizationHitNumberFromE(const char* name, const char* title,double stochastic,double stochastic_hadron,double constant) :
  TFCSLateralShapeParametrizationHitBase(name,title),m_stochastic(stochastic),m_stochastic_hadron(stochastic_hadron),m_constant(constant)
{
  set_match_all_pdgid();
}

double TFCSLateralShapeParametrizationHitNumberFromE::get_sigma2_fluctuation(TFCSSimulationState& simulstate,const TFCSTruthState* /*truth*/, const TFCSExtrapolationState* /*extrapol*/) const
{
  int cs=calosample();
  double energy=simulstate.E(cs);

  if (energy < 0) {
    return 1;
  }

  if (TMath::IsNaN(energy)) {
    ATH_MSG_DEBUG("Energy is NaN");
    return 1;
  }
  
  double sqrtE=sqrt(energy/1000.0);
  double sigma_stochastic=m_stochastic/sqrtE;
  double sigma_stochastic_hadron=m_stochastic_hadron/sqrtE;

  //Attention: linear sum of "hadron" stochastic term and constant term as emulation of fluctuations in EM component
  double sigma_hadron=m_constant+sigma_stochastic_hadron; 

  //Usual quadratic sum of "hardon" component and normal stochastic term
  double sigma2 = sigma_stochastic*sigma_stochastic + sigma_hadron*sigma_hadron;

  ATH_MSG_DEBUG("sigma^2 fluctuation="<<sigma2);

  return sigma2;
}

int TFCSLateralShapeParametrizationHitNumberFromE::get_number_of_hits(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol) const
{
  if (!simulstate.randomEngine()) {
    return -1;
  }

  double sigma2=get_sigma2_fluctuation(simulstate,truth,extrapol);
  int hits = CLHEP::RandPoisson::shoot(simulstate.randomEngine(), 1.0 / sigma2);

  ATH_MSG_DEBUG("#hits="<<hits);
  
  return hits;
}

void TFCSLateralShapeParametrizationHitNumberFromE::Print(Option_t *option) const
{
  TString opt(option);
  bool shortprint=opt.Index("short")>=0;
  bool longprint=msgLvl(MSG::DEBUG) || (msgLvl(MSG::INFO) && !shortprint);
  TString optprint=opt;optprint.ReplaceAll("short","");
  TFCSLateralShapeParametrizationHitBase::Print(option);

  if(longprint) ATH_MSG_INFO(optprint <<"  sigma^2=["<<m_stochastic<<"/sqrt(E/GeV)]^2 + ["<<m_constant<<" + "<<m_stochastic_hadron<<"/sqrt(E/GeV)]^2");
}
