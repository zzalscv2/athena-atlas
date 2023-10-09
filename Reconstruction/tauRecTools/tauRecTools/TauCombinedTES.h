/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUCOMBINEDTES_H
#define TAURECTOOLS_TAUCOMBINEDTES_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODTau/TauJet.h"

#include "TH1F.h"
#include "TF1.h"
#include "TGraph.h"

#include <array>
#include <string>

class TauCombinedTES : public TauRecToolBase {
public:
  ASG_TOOL_CLASS2( TauCombinedTES, TauRecToolBase, ITauToolBase )

  TauCombinedTES(const std::string& name="TauCombinedTES");
    
  virtual StatusCode initialize() override;
      
  virtual StatusCode execute(xAOD::TauJet& xTau) const override;

  /** Whether to use calo pt, invoked by TauSmearing tool */
  bool getUseCaloPtFlag(const xAOD::TauJet& tau) const;
  
  /** Get MVA Et resolution, invoked by METSignificance */
  double getMvaEnergyResolution(const xAOD::TauJet& tau) const;

private:
    
  /// Switch for decorating the intermediate results, for combined TES tuning
  bool m_addCalibrationResultVariables;
  
  /// Name of the calibration file 
  std::string m_calFileName;

  /// Use MVA TES resolution (for MET significance)
  bool m_useMvaResolution;
  
  struct Variables
  {
    double pt_constituent{0.0};
    double pt_tauRecCalibrated{0.0};
    double pt_weighted{0.0};
    double weight{-1111.0};
    //double sigma_combined{-1111.};
    double sigma_compatibility{-1111.};
    double sigma_tauRec{-1111.0};
    double sigma_constituent{-1111.0};
    double corrcoeff{-1111.0};
  };

  /** Get the weighted four momentum of calo TES and PanTau */
  TLorentzVector getCombinedP4(const xAOD::TauJet& tau, Variables& variables) const;

  /** Whether the tau candidate is valid for the calculation */
  bool isValid(const xAOD::TauJet& tau) const;

  /** Get the index of eta in the calibration histogram */
  int getEtaIndex(float eta) const;

  /** Get the decay mode of the tau candidate */
  xAOD::TauJetParameters::DecayMode getDecayMode(const xAOD::TauJet& tau) const;
  
  /** Get the index of decay mode in the calibration histogram */
  int getDecayModeIndex(xAOD::TauJetParameters::DecayMode decayMode) const;

  /** Get correlation coefficient between the calo TES and PanTau */
  double getCorrelation(int decayModeIndex, int etaIndex) const;
  
  /** Get the resolution of Et at the calo TES */
  double getCaloResolution(double et, int decayModeIndex, int etaIndex) const;
  
  /** Get the resolution of Et at PanTau */
  double getPanTauResolution(double et, int decayModeIndex, int etaIndex) const;
  
  /** Get the Et at the calo TES after calibration correction */ 
  double getCaloCalEt(double et, int decayModeIndex, int etaIndex) const;

  /** Get the Et at PanTau after calibration correction */ 
  double getPanTauCalEt(double panTauEt, int decayModeIndex, int etaIndex) const;
 
  /** Get the weight of calo TES */
  double getWeight(double caloSigma, double panTauSigma, double correlatioon) const;

  /** Get the combined sigma of calo TES and PanTau */
  double getCombinedSigma(double caloSigma, double panTauSigma, double correlation) const;

  /** Get the compatibility sigma of calo TES and PanTau */
  double getCompatibilitySigma(double caloSigma, double panTauSigma, double correlation) const;

  /** Get the combined Et of calo TES and PanTau */
  double getCombinedEt(double caloEt, double et_substructure,
		       xAOD::TauJetParameters::DecayMode decayMode, float eta,
                       Variables& variables) const;
  
  /** Get the allowed difference between calo TES and PanTau */ 
  double getNsigmaCompatibility(double caloEt, int decayModeIndex) const;
  
  /// Binning in the calibraction graph/hist
  enum Binning {DecayModeBinning = 5, EtaBinning = 5};

  /// Decay mode binning in the calibration graph/hist
  const std::array<std::string, DecayModeBinning> m_decayModeNames = {"1p0n","1p1n","1pXn","3p0n","3pXn"}; //!
  
  /// Eta binning in the calibration graph
  const std::array<std::string, EtaBinning> m_etaBinNames = {"0", "1", "2", "3", "4"}; //!
  
  /// Calibration graph: mean of bias/caloEt as a function of caloEt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_caloRelBias; //!

  /// Maximum Et of m_caloRelBias
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_caloRelBiasMaxEt; //!

  /// Calibration graph: resolution at Calo TES as a function of caloEt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_caloRes; //!

  /// Maximum Et of m_caloRes
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_caloResMaxEt; //!

  /// Calibration graph: mean of bias/panTauEt as a funtion of panTauEt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_panTauRelBias; //!
  
  /// Maximum Et of m_panTauRelBias
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_panTauRelBiasMaxEt; //!

  /// Calibration graph: resolution at PanTau as a function of panTauEt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_panTauRes; //!
  
  /// Maximum Et of m_panTauRes
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_panTauResMaxEt; //!

  /// Calibration graph: MVA TES resolution as a function of MVA pt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_mvaRes; //!
  
  /// Maximum Et of m_mvaRes
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_mvaResMaxEt; //!
  
  /// Calibration histogram: correlation coefficient of calo TES and PanTau
  std::array<std::unique_ptr<TH1F>, DecayModeBinning> m_correlationHists; //!

  /// Maximum tolerence in unit of combined sigma, as a function of calo Et
  std::array<std::unique_ptr<TF1>, DecayModeBinning> m_nSigmaCompatibility; //!
};

#endif // TAURECTOOLS_TAUCOMBINEDTES_H
