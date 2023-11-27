/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/TauFeature.h"

#include <cmath>
#include <stdexcept>

static const std::vector<double> s_defaultVec(0);

PanTau::TauFeature::TauFeature():
  m_featureMap(),
  m_vecFeatureMap()
{
}


PanTau::TauFeature::~TauFeature()
{
}


double PanTau::TauFeature::value(const std::string& name, bool& isValid) const
{
  FeatureMapConstIter iter = m_featureMap.find(name);
  if (m_featureMap.end() == iter) {
    isValid=false;
    return -999999.;
  }
  isValid=true;
  return (*iter).second; 
}


const std::vector<double>& PanTau::TauFeature::vecValue(const std::string& name) const {
  VectorFeatureMapConstIter iter = m_vecFeatureMap.find(name);
  if (m_vecFeatureMap.end() == iter) {
    return s_defaultVec;
  }
  return (*iter).second;
}


bool PanTau::TauFeature::addFeature(const std::string& name, const double value) {

  if (std::isnan(value)) {
    throw std::runtime_error("TauFeature::addFeature: Given " + name + " value is NaN!");
  }
  if (std::isinf(value)){
    throw std::runtime_error("TauFeature::addFeature: Given " + name + " value is inf!");
  }
  std::pair<FeatureMapConstIter, bool> result = m_featureMap.insert(make_pair(name, value));
  return result.second;
}


bool PanTau::TauFeature::addVecFeature(const std::string& name,
				       const std::vector<double>& value) {
  std::pair<VectorFeatureMapConstIter, bool> result = m_vecFeatureMap.insert(make_pair(name, value));
  return result.second;
}


int PanTau::TauFeature::nValues() const {
  return m_featureMap.size();
}


int PanTau::TauFeature::nVecValues() const {
  return m_vecFeatureMap.size();
}

// FIXME: use StatusCode instead of throwing exceptions
void PanTau::TauFeature::add(PanTau::TauFeature* otherFeatures) {
    
  //add the scalar features
  for (const auto& p : otherFeatures->m_featureMap) {
    if (!(this->addFeature(p.first, p.second))) {
      throw std::runtime_error("PanTau::TauFeature::add( PanTau::TauFeature* ): Error when adding scalar feature " + p.first);
    }
  }
    
  //add the vector features
  for (const auto& p : otherFeatures->m_vecFeatureMap) {
    if (!(this->addVecFeature(p.first, p.second))) {
      throw std::runtime_error("PanTau::TauFeature::add( PanTau::TauFeature* ): Error when adding vector feature " + p.first);
    }
  }    
}
