#ifndef GENERATORFILTERS_DIPHOTONFILTER_H
#define GENERATORFILTERS_DIPHOTONFILTER_H

#include "GeneratorModules/GenFilter.h"

/// Filters and looks for di-photons
/// @author J Tanaka, Aug 2009
class DiPhotonFilter : public GenFilter {
public:

  DiPhotonFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize();
  virtual StatusCode filterEvent();

private:

  double m_Ptmin_1st;
  double m_EtaRange_1st;
  double m_Ptmin_2nd;
  double m_EtaRange_2nd;
  double m_Ptmin_others;
  double m_EtaRange_others;
  double m_diphoton_massmin;
  double m_diphoton_massmax;
	double m_deltaPhiMin;
  double m_deltaPhiMax;
  double m_diphoton_PtMin;
  double m_diphoton_PtMax;
  double m_deltaRMin ;
  double m_deltaRMax ;
  bool m_use1st2ndPhotonsforMassCut;

};

#endif
