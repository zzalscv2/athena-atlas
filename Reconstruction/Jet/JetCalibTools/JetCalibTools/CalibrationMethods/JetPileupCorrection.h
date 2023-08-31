/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_JETPILEUPCORRECTION_H
#define JETCALIBTOOLS_JETPILEUPCORRECTION_H 1

/* Implementation of JetAreaSubtraction class
 * This class will apply the jet area pile up correction
 *
 * Author: Joe Taenzer (joseph.taenzer@cern.ch)
 * Date: June 27 2013
 */


#include "JetCalibTools/JetCalibrationStep.h"
#include "TString.h"
#include <string>
#include <memory>
class TEnv;
class ResidualOffsetCorrection;

namespace PUCorrection {
  struct PU3DCorrectionHelper;
}

class JetPileupCorrection
  : virtual public ::JetCalibrationStep
{

 public:
  JetPileupCorrection();
  JetPileupCorrection(const std::string& name, TEnv * config, TString jetAlgo, TString calibAreaTag, bool doResidual, bool doJetArea, bool doOrigin, const std::string& originScale, bool isData, bool dev);
  virtual ~JetPileupCorrection();

  virtual StatusCode initialize() override;
  virtual StatusCode calibrate(xAOD::Jet& jet, JetEventInfo& jetEventInfo) const override;
 
 private:
  TEnv * m_config{};
  TString m_jetAlgo;
  TString m_calibAreaTag;
  bool m_dev{};
  bool m_doResidual{};
  bool m_doJetArea{};
  bool m_doOrigin{};
  bool m_isData{};
  bool m_doMuOnly{};
  bool m_doNPVOnly{};
  bool m_doNJetOnly{};
  bool m_doSequentialResidual{};

  bool m_do3Dcorrection{};

  bool m_useFull4vectorArea{};
  ResidualOffsetCorrection * m_residualOffsetCorr{};

  std::unique_ptr<PUCorrection::PU3DCorrectionHelper> m_residual3DCorr;

  bool m_doOnlyResidual{};

  std::string m_originScale;

};

#endif
