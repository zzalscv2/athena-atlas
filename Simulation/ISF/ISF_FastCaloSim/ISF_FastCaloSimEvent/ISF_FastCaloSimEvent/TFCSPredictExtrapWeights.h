/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_TFCSPREDICTEXTRAPWEIGHTS_h
#define ISF_FASTCALOSIMEVENT_TFCSPREDICTEXTRAPWEIGHTS_h

#include "ISF_FastCaloSimEvent/TFCSLateralShapeParametrizationHitBase.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include <string>
#include "TH2D.h"

// Flexable network class
#include "VNetworkBase.h"

class TFCSPredictExtrapWeights : public TFCSLateralShapeParametrizationHitBase {
public:
  TFCSPredictExtrapWeights(const char *name = nullptr,
                           const char *title = nullptr);
  virtual ~TFCSPredictExtrapWeights();

  virtual bool operator==(const TFCSParametrizationBase &ref) const override;

  // Used to decorate simulstate with extrapolation weights
  virtual FCSReturnCode
  simulate(TFCSSimulationState &simulstate, const TFCSTruthState *truth,
           const TFCSExtrapolationState *extrapol) const override;

  // Used to decorate Hit with extrapolated center positions
  virtual FCSReturnCode
  simulate_hit(Hit &hit, TFCSSimulationState &simulstate,
               const TFCSTruthState *truth,
               const TFCSExtrapolationState *extrapol) override;

  // Status bit for chain persistency
  enum FCSfreemem {
    kfreemem = BIT(17) ///< Set this bit in the TObject bit if the memory for
                       ///< m_input should be freed after reading in athena
  };
  bool freemem() const { return TestBit(kfreemem); };
  void set_freemem() { SetBit(kfreemem); };

  // Initialize Neural Network
  bool initializeNetwork(int pid, const std::string &etaBin,
                         const std::string &FastCaloNNInputFolderName);

  // Get inputs needed to normalize data
  bool getNormInputs(const std::string &etaBin,
                     const std::string &FastCaloTXTInputFolderName);

  // Test function
  static void test_path(std::string &net_path, std::string const &norm_path,
                        TFCSSimulationState *simulstate = nullptr,
                        const TFCSTruthState *truth = nullptr,
                        const TFCSExtrapolationState *extrapol = nullptr);
  static void unit_test(TFCSSimulationState *simulstate = nullptr,
                        const TFCSTruthState *truth = nullptr,
                        const TFCSExtrapolationState *extrapol = nullptr);

  // Prepare inputs to the Neural Network
  VNetworkBase::NetworkInputs prepareInputs(TFCSSimulationState &simulstate,
                                            const float truthE) const;

  // Print()
  void Print(Option_t *option = "") const override;

  // Use extrapWeight=0.5 or r and z when constructing a hit?
  enum TFCSPredictExtrapWeightsStatusBits { kUseHardcodedWeight = BIT(15) };
  bool UseHardcodedWeight() const { return TestBit(kUseHardcodedWeight); };
  void set_UseHardcodedWeight() { SetBit(kUseHardcodedWeight); };
  void reset_UseHardcodedWeight() { ResetBit(kUseHardcodedWeight); };

private:
  // Old way to save and load the net
  // Persistify configuration in string m_input.
  // A custom Streamer(...) builds
  // m_nn on the fly when reading from file.
  // Inside Athena, if freemem() is true, the content of m_input is deleted
  // after reading in order to free memory
  std::string *m_input = nullptr; // old way to save LWTNN
  // New way to save and load the network.
  // The network itself has a steamer and can be saved and loaded automatically.
  // if freemem() is true, deleteAllButNet() caneb called on this
  // to reduce memory usage.
  std::unique_ptr<VNetworkBase> m_nn = nullptr;
  std::vector<int> *m_relevantLayers = nullptr;
  // The member object is a generic network type, that may implement
  // ONNX, LWTNN or something else
  std::vector<int> *m_normLayers =
      nullptr; // vector of index layers (-1 corresponds to truth energy)
  std::vector<float> *m_normMeans =
      nullptr; // vector of mean values for normalizing energy fraction per
               // layer, last index is for total energy
  std::vector<float> *m_normStdDevs =
      nullptr; // vector of std dev values for normalizing energy fraction per
               // layer, last index is for total energy

  ClassDefOverride(TFCSPredictExtrapWeights, 2) // TFCSPredictExtrapWeights
};

#endif
