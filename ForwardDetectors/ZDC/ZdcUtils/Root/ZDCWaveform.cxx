/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcUtils/ZDCWaveform.h"


ZDCWaveformBase::ZDCWaveformBase(std::string tag, double initialTauRise, double initialTauFall, const std::vector<std::string> &addtlShapeNames, 
				 const std::vector<double> &addtlShapeValues) :
  m_tag( std::move(tag) ),
  m_initialTauRise(initialTauRise),
  m_initialTauFall(initialTauFall),
  m_numAddtlShapePars(addtlShapeNames.size()),
  m_tauRise(initialTauRise),
  m_tauFall(initialTauFall)
{      
  setAddtlShapeParameters(addtlShapeNames, addtlShapeValues);
}

ZDCWaveformBase::ZDCWaveformBase(const ZDCWaveformBase& instance) :
  m_tag( std::move(instance.m_tag) ),
  m_addtlShapeNames(instance.m_addtlShapeNames),
  m_initialTauRise(instance.m_initialTauRise),
  m_initialTauFall(instance.m_initialTauFall),
  m_numAddtlShapePars(instance.m_numAddtlShapePars),
  m_addtlShapeInitialValues(instance.m_addtlShapeInitialValues),
  m_tauRise(instance.m_tauRise),
  m_tauFall(instance.m_tauFall)
{      
}

void ZDCWaveformBase::setAddtlShapeParameters(const std::vector<std::string> &addtlShapeNames, 
					      const std::vector<double> &addtlShapeValues)
{
  // Enforce consistency in vector lengths
  //
  m_numAddtlShapePars = addtlShapeNames.size();
  if (m_numAddtlShapePars != addtlShapeValues.size()) throw std::runtime_error("ZDCWaveformBase::setAddtlShapeParameters; Shape name and value size mismatch");

  m_addtlShapeNames.clear();
  
  // Insert names into map with index
  //
  for (unsigned int param = 0; param < addtlShapeNames.size(); param++) {
    m_addtlShapeNames.insert({addtlShapeNames[param], param});
  }

  // Save shape values, first to "initial", then to the actual
  //
  m_addtlShapeInitialValues = addtlShapeValues;
  m_addtlShapeValues = m_addtlShapeInitialValues;
}

void ZDCWaveformBase::setAddtlShapeValues(const double* values)
{
  unsigned int nvalues = m_addtlShapeValues.size();
  std::copy_n(values, nvalues, m_addtlShapeValues.begin());
}

void ZDCWaveformBase::restoreInitial()
{
  m_tauRise = m_initialTauRise;
  m_tauFall = m_initialTauFall;
  m_addtlShapeValues = m_addtlShapeInitialValues;
}

