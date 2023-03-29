/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRONPHOTONFOURMOMENTUMCORRECTION_GAINUNCERTAINTY_H
#define ELECTRONPHOTONFOURMOMENTUMCORRECTION_GAINUNCERTAINTY_H

#include <AsgTools/AsgMessaging.h>
#include <PATCore/PATCoreEnums.h>
#include <memory>
#include <string>

class TH1;

namespace egGain {

class GainUncertainty : public asg::AsgMessaging {
public:

  GainUncertainty(const std::string& filename, bool splitGainUnc = false, const std::string& name = "GainUncertainty");
  ~GainUncertainty() {};
  enum class GainType {MEDIUM, LOW, MEDIUMLOW};
  // return relative uncertainty on energy from gain uncertainty
  // input etaCalo_input = eta in Calo frame
  //       et_input = Et in MeV
  //       ptype    = particle type
  //       useUncertainty : instead of the value, if a correction has been applied
  //       gaintyp : response function considering only Medium, only Low or the sum of Low and Medium bias 
  double getUncertainty(double etaCalo_input, double et_input,
			PATCore::ParticleType::Type ptype = PATCore::ParticleType::Electron,
			bool useUncertainty = false, GainType gainType = GainType::MEDIUMLOW) const;
      void setInterpolation() { m_useInterpolation = true; }
private:

  static const int s_nEtaBins=5;
  std::unique_ptr<TH1> m_alpha_specialGainRun;
  std::unique_ptr<TH1> m_gain_impact_Zee;
  std::unique_ptr<TH1> m_gain_Impact_elec[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_conv[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_unco[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_elec_medium[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_conv_medium[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_unco_medium[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_elec_low[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_conv_low[s_nEtaBins]{};
  std::unique_ptr<TH1> m_gain_Impact_unco_low[s_nEtaBins]{};

  bool m_useInterpolation = false;
  
};

}

#endif
