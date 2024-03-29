/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_TFCSPCAEnergyParametrization_h
#define ISF_FASTCALOSIMEVENT_TFCSPCAEnergyParametrization_h

#include "ISF_FastCaloSimEvent/TFCSEnergyParametrization.h"
#include "ISF_FastCaloSimEvent/TFCS1DFunction.h"
#include "ISF_FastCaloSimEvent/IntArray.h"
#include "TMatrixF.h"
#include "TMatrixDSym.h"
#include "TVectorF.h"
#include "TFile.h"

class TH1;

class TFCSPCAEnergyParametrization : public TFCSEnergyParametrization {
public:
  enum FCSReturnCodePCA {
    // Assuming an extrem h_totalE_ratio histogram that would cause a retry in
    // 50% of the cases,
    // returning FCSRetry+20 will cause an accidental WARNING for every
    // 2^10=1024 simulated particles and
    // an accidental FATAL for every 2^20, which should be safe even for largest
    // scale productions
    FCSRetryPCA = FCSRetry + 20
  };

  TFCSPCAEnergyParametrization(const char *name = nullptr,
                               const char *title = nullptr);

  virtual FCSReturnCode
  simulate(TFCSSimulationState &simulstate, const TFCSTruthState *truth,
           const TFCSExtrapolationState *extrapol) const override;

  int n_pcabins() const { return m_numberpcabins; };
  virtual int n_bins() const override { return m_numberpcabins; };
  const std::vector<int> &get_layers() const { return m_RelevantLayers; };

  virtual bool is_match_Ekin_bin(int Ekin_bin) const override;
  virtual bool is_match_calosample(int calosample) const override;
  virtual bool is_match_all_Ekin_bin() const override { return true; };
  virtual bool is_match_all_calosample() const override { return false; };

  static void P2X(TVectorD *, TVectorD *, TMatrixD *, int, const double *,
                  double *, int);
  bool loadInputs(TFile *file);
  bool loadInputs(TFile *file, const std::string &);

  void clean();

  void Print(Option_t *option = "") const override;

  float get_total_energy_normalization() const {
    return m_total_energy_normalization;
  };
  void set_total_energy_normalization(float norm) {
    m_total_energy_normalization = norm;
  };

  void set_totalE_probability_ratio(int Ekin_bin, TH1 *hist);
  TH1 *get_totalE_probability_ratio(int Ekin_bin) const;

  int do_rescale;

private:
  std::vector<int> m_RelevantLayers;

  std::vector<TMatrixD *> m_EV;
  std::vector<TVectorD *> m_MeanValues;
  std::vector<TVectorD *> m_SigmaValues;
  std::vector<TVectorD *> m_Gauss_means;
  std::vector<TVectorD *> m_Gauss_rms;
  std::vector<std::vector<TFCS1DFunction *>> m_cumulative;

  std::vector<TH1 *> m_totalE_probability_ratio;

  int m_numberpcabins;

  float m_total_energy_normalization{1};

  ClassDefOverride(TFCSPCAEnergyParametrization,
                   3) // TFCSPCAEnergyParametrization
};

#endif
