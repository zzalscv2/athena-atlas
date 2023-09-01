/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "IsolationCorrections/ShowerDepthTool.h"

// Project include(s).
#include "AsgMessaging/MessageCheck.h"
#include "PathResolver/PathResolver.h"

// ROOT include(s).
#include <TFile.h>
#include <TH1.h>

// System include(s).
#include <cmath>
#include <string>

namespace CP{

  // Calibration file / histogram name(s).
  static const char* const CONFIG_FILE_NAME = "ElectronIsolationSelection/v1/CaloDeltaRZ.root";
  static const char* const DATA_HISTO_NAME = "hData";
  static const char* const MC_HISTO_NAME = "hMC";

  // Make the messaging functions available.
  ANA_MSG_SOURCE(ShowerDepthToolMessaging, "CP::ShowerDepthTool");
  using namespace ShowerDepthToolMessaging;

  ShowerDepthTool::ShowerDepthTool() = default;

  ShowerDepthTool::~ShowerDepthTool() = default;

  bool ShowerDepthTool::initialize()
  {
    const std::string filename = PathResolverFindCalibFile( CONFIG_FILE_NAME );

    m_hData = getHistoFromFile( filename.c_str(), DATA_HISTO_NAME );
    m_hMC = getHistoFromFile( filename.c_str(), MC_HISTO_NAME );

    return (m_hData && m_hMC);
  }

  /** Shower depth (in mm) on EM1 vs. eta, considering misalignments **/
  float ShowerDepthTool::getCorrectedShowerDepthEM1(float etas1, float phi, bool isData) const
  {
    return getShowerDepthEM1(etas1) - getRZCorrection(etas1, phi, isData);
  }

  /** Shower depth (in mm) on EM2 vs. eta, considering misalignments **/
  float ShowerDepthTool::getCorrectedShowerDepthEM2(float etas2, float phi, bool isData) const
  {
    return getShowerDepthEM2(etas2) - getRZCorrection(etas2, phi, isData);
  }

  /** Return the shower depth on sampling 1 given etas1. From:
  https://svnweb.cern.ch/trac/atlasoff/browser/Calorimeter/CaloDetDescr/trunk/src/CaloDepthTool.cxx#L347 **/
  float ShowerDepthTool::getShowerDepthEM1(float etas1)
  {
    float radius, aetas1 = std::fabs(etas1);
    if (aetas1 < 0.8)
      radius = (1558.859292 - 4.990838*aetas1 - 21.144279*aetas1*aetas1);
    else if (aetas1<1.5)
      radius = (1522.775373 + 27.970192*aetas1 - 21.104108*aetas1*aetas1);
    else
      radius = 3790.671754;
    if (etas1 < 0. and aetas1 > 1.5)
      return -radius;
    return radius;
  }

  /** Return the shower depth on sampling 2 given etas2. From:
  https://svnweb.cern.ch/trac/atlasoff/browser/Calorimeter/CaloDetDescr/trunk/src/CaloDepthTool.cxx#L347 **/
  float ShowerDepthTool::getShowerDepthEM2(float etas2)
  {
    float radius, aetas2 = std::fabs(etas2);
    if (aetas2 < 1.425) // Barrel, my definition
      radius = (1698.990944 - 49.431767*aetas2 - 24.504976*aetas2*aetas2);
    else if (aetas2 < 1.5) // EME2 in tool
      radius = (8027.574119 - 2717.653528*aetas2);
    else
      radius = (3473.473909 + 453.941515*aetas2 - 119.101945*aetas2*aetas2);
    if (etas2 < 0. and aetas2 > 1.5)
      return -radius;
    return radius;
  }


  float ShowerDepthTool::getCorrectedEtaDirection(float zvertex,
                                                  float eta,
                                                  float phi,
                                                  bool isData,
                                                  int sampling) const
  {
    std::pair<float, float> RZ = getCorrectedRZ(eta, phi, isData, sampling);
    return getEtaDirection(zvertex, RZ.first, RZ.second);
  }

  std::pair<float,float> ShowerDepthTool::getRZ(float eta, int sampling)
  {
    if ((sampling != 1 && sampling != 2) || (std::fabs(eta)>10))
    {
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Invalid sampling, eta: " << sampling << ", " << eta);
      return std::make_pair(0., 0.);
    }
    float depth = (sampling == 1 ? getShowerDepthEM1(eta) : getShowerDepthEM2(eta) );
    if (std::fabs(eta) <  1.5)
      return std::make_pair( depth, depth*std::sinh(eta) );
    return std::make_pair( depth/std::sinh(eta), depth );
  }


  std::optional<float> ShowerDepthTool::getCaloPointingEta(float etas1, float etas2, float phi, bool isData) const
  {
    std::pair<float, float> RZ1 = getCorrectedRZ(etas1, phi, isData, 1);
    std::pair<float, float> RZ2 = getCorrectedRZ(etas2, phi, isData, 2);

    //Sanity check
    constexpr float epsilon=1e-6;
    if (std::fabs(RZ2.first - RZ1.first) < epsilon) return std::nullopt;

    return std::optional<float>(std::asinh( (RZ2.second - RZ1.second) / (RZ2.first - RZ1.first)));
  }


  std::pair<float, float> ShowerDepthTool::getCorrectedRZ(float eta,
                                                          float phi,
                                                          bool isData,
                                                          int sampling) const
  {
    if ((sampling != 1 && sampling != 2) || (std::fabs(eta)>10))
    {
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Invalid sampling, eta: " << sampling << ", " << eta);
      return std::make_pair(0., 0.);
    }
    float depth = (sampling == 1 ? getCorrectedShowerDepthEM1(eta, phi, isData) :
      getCorrectedShowerDepthEM2(eta, phi, isData) );
    if (std::fabs(eta) <  1.5)
      return std::make_pair( depth, depth*std::sinh(eta) );
    return std::make_pair( depth/std::sinh(eta), depth );
  }


  /** Return the calorimeter displacement in R(Z) for barrel (endcap) **/
  float ShowerDepthTool::getRZCorrection(float eta, float phi, bool isData) const
  {
    // Get the correct histogram.
    const TH1* histo = (isData ? m_hData.get() : m_hMC.get());
    if (!histo) {
      return 0;
    }
    // Make sure that we can perform the interpolation in both eta and phi.
    // Note that std::numeric_limits<float>::epsilon() is just not large enough
    // for the following. :-(
    static constexpr float epsilon = 1e-6f;
    const Int_t etaBin = histo->GetXaxis()->FindFixBin(eta);
    if (etaBin < 1) {
      const float etaOld = eta;
      eta = histo->GetXaxis()->GetBinLowEdge(1) + epsilon;
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Using eta " << eta << " instead of "
                          << etaOld);
    }
    else if (etaBin > histo->GetNbinsX()) {
      const float etaOld = eta;
      eta = histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX()) - epsilon;
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Using eta " << eta << " instead of "
                          << etaOld);
    }
    const Int_t phiBin = histo->GetYaxis()->FindFixBin(phi);
    if (phiBin < 1) {
      const float phiOld = phi;
      phi = histo->GetYaxis()->GetBinLowEdge(1) + epsilon;
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Using phi " << phi << " instead of "
                          << phiOld);
    }
    else if (phiBin > histo->GetNbinsY()) {
      const float phiOld = phi;
      phi = histo->GetYaxis()->GetBinUpEdge(histo->GetNbinsY()) - epsilon;
      ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Using phi " << phi << " instead of "
                          << phiOld);
    }
    // Get the correction as an interpolation.
    return histo->Interpolate(eta, phi);
  }


  float ShowerDepthTool::getEtaDirection(float zvertex, float R, float z)
  {
    return std::asinh( (z- zvertex)/R );
  }


  std::unique_ptr<TH1> ShowerDepthTool::getHistoFromFile(const char* fileName, const char* histoName)
  {
    std::unique_ptr<TFile> f(TFile::Open(fileName, "READ"));
    if (!f){
      ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                          "Could not open file: \"" << fileName << "\"");
      return {};
    }
    TH1 *h = dynamic_cast<TH1*>( f->Get(histoName) );
    if (!h){
      ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                          "Could not get histogram: \"" << histoName
                          << "\" from file: \"" << fileName << "\"");
      return {};
    }
    //The file we be deleted so use SetDirectory
    h->SetDirectory(nullptr);
    return std::unique_ptr<TH1>(h);
  }

} // namespace CP
