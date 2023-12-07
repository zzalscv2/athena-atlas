/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// TODO does m_inputs ever get filled in??
#include "ISF_FastCaloSimEvent/TFCSPredictExtrapWeights.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimEvent/TFCSCenterPositionCalculation.h"
#include "CxxUtils/as_const_ptr.h"

#include "TFile.h"
#include "TClass.h"

#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandFlat.h"

#if defined(__FastCaloSimStandAlone__)
#include "CLHEP/Random/TRandomEngine.h"
#else
#include <CLHEP/Random/RanluxEngine.h>
#endif

#include <iostream>
#include <fstream>

// Neural networks
#include "ISF_FastCaloSimEvent/TFCSNetworkFactory.h"
#include "ISF_FastCaloSimEvent/VNetworkBase.h"

// XML reader
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

//=============================================
//======= TFCSPredictExtrapWeights =========
//=============================================

TFCSPredictExtrapWeights::TFCSPredictExtrapWeights(const char *name,
                                                   const char *title)
    : TFCSLateralShapeParametrizationHitBase(name, title) {
  set_freemem();
  set_UseHardcodedWeight();
}

// Destructor
TFCSPredictExtrapWeights::~TFCSPredictExtrapWeights() {
  if (m_input != nullptr) {
    delete m_input;
  }
  if (m_relevantLayers != nullptr) {
    delete m_relevantLayers;
  }
  if (m_normLayers != nullptr) {
    delete m_normLayers;
  }
  if (m_normMeans != nullptr) {
    delete m_normMeans;
  }
  if (m_normStdDevs != nullptr) {
    delete m_normStdDevs;
  }
}

bool TFCSPredictExtrapWeights::operator==(
    const TFCSParametrizationBase &ref) const {
  if (IsA() != ref.IsA()) {
    ATH_MSG_DEBUG("operator==: different class types "
                  << IsA()->GetName() << " != " << ref.IsA()->GetName());
    return false;
  }
  const TFCSPredictExtrapWeights &ref_typed =
      static_cast<const TFCSPredictExtrapWeights &>(ref);

  if (TFCSParametrizationBase::compare(ref))
    return true;
  if (!TFCSParametrization::compare(ref))
    return false;
  if (!TFCSLateralShapeParametrization::compare(ref))
    return false;

  return (m_input->compare(*ref_typed.m_input) == 0);
}

// getNormInputs()
// Get values needed to normalize inputs
bool TFCSPredictExtrapWeights::getNormInputs(
    const std::string &etaBin, const std::string &FastCaloTXTInputFolderName) {
  ATH_MSG_DEBUG(" Getting normalization inputs... ");

  // Open corresponding TXT file and extract mean/std dev for each variable
  if (m_normLayers != nullptr) {
    m_normLayers->clear();
  } else {
    m_normLayers = new std::vector<int>();
  }
  if (m_normMeans != nullptr) {
    m_normMeans->clear();
  } else {
    m_normMeans = new std::vector<float>();
  }
  if (m_normStdDevs != nullptr) {
    m_normStdDevs->clear();
  } else {
    m_normStdDevs = new std::vector<float>();
  }
  const std::string inputFileName = FastCaloTXTInputFolderName +
                                    "MeanStdDevEnergyFractions_eta_" + etaBin +
                                    ".txt";
  ATH_MSG_DEBUG(" Opening " << inputFileName);
  std::ifstream inputTXT(inputFileName);
  int index;
  if (inputTXT.is_open()) {
    std::string line;
    while (getline(inputTXT, line)) {
      std::stringstream ss(line);
      unsigned int counter = 0;
      while (ss.good()) {
        std::string substr;
        getline(ss, substr, ' ');
        if (counter == 0) { // Get index (#layer or -1 if var == etrue)
          if (substr != "etrue") {
            index = std::stoi(substr.substr(substr.find('_') + 1));
          } else { // etrue
            index = -1;
          }
          m_normLayers->push_back(index);
          ATH_MSG_VERBOSE("Filling m_normLayers; Got the index "
                          << index << " from the line " << line);
        } else if (counter == 1) {
          m_normMeans->push_back(std::stof(substr));
        } else if (counter == 2) {
          m_normStdDevs->push_back(std::stof(substr));
        }
        counter++;
      }
    }
    inputTXT.close();
  } else {
    ATH_MSG_ERROR(" Unable to open file ");
    return false;
  }

  return true;
}

// prepareInputs()
// Prepare input variables to the Neural Network
VNetworkBase::NetworkInputs
TFCSPredictExtrapWeights::prepareInputs(TFCSSimulationState &simulstate,
                                        const float truthE) const {
  VNetworkBase::NetworkInputs inputs;
  std::map<std::string, double> flat_inputs;
  for (int ilayer = 0; ilayer < CaloCell_ID_FCS::MaxSample; ++ilayer) {
    if (std::find(m_relevantLayers->cbegin(), m_relevantLayers->cend(),
                  ilayer) != m_relevantLayers->cend()) {
      const std::string layer = std::to_string(ilayer);
      // Find index
      auto itr =
          std::find(m_normLayers->cbegin(), m_normLayers->cend(), ilayer);
      if (itr != m_normLayers->cend()) {
        const int index = std::distance(m_normLayers->cbegin(), itr);
        std::map<std::string, double> element;
        flat_inputs["ef_" + layer] =
            (simulstate.Efrac(ilayer) - std::as_const(m_normMeans)->at(index)) /
            std::as_const(m_normStdDevs)->at(index);
      } else {
        ATH_MSG_ERROR("Normalization information not found for layer "
                      << ilayer);
      }
    }
  }
  // Find index for truth energy
  auto itr = std::find(m_normLayers->cbegin(), m_normLayers->cend(), -1);
  const int index = std::distance(m_normLayers->cbegin(), itr);
  flat_inputs["etrue"] = (truthE - std::as_const(m_normMeans)->at(index)) /
                         std::as_const(m_normStdDevs)->at(index);
  if (is_match_pdgid(22)) {
    flat_inputs["pdgId"] = 1; // one hot enconding
  } else if (is_match_pdgid(11) || is_match_pdgid(-11)) {
    flat_inputs["pdgId"] = 0; // one hot enconding
  } else {
    ATH_MSG_ERROR("have no one-hot encoding for pdgId; pottential issue with "
                 "predicted weights.");
  }
  inputs["dense_input"] = flat_inputs;

  return inputs;
}

// simulate()
// get predicted extrapolation weights and save them as AuxInfo in simulstate
FCSReturnCode TFCSPredictExtrapWeights::simulate(
    TFCSSimulationState &simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState * /*extrapol*/) const {

  // Get inputs to Neural Network
  const VNetworkBase::NetworkInputs inputVariables =
      prepareInputs(simulstate, truth->E() * 0.001);

  ATH_MSG_DEBUG("Inputs to predicted weights network "
                << VNetworkBase::representNetworkInputs(inputVariables));
  ATH_MSG_DEBUG("Address of m_nn " << m_nn.get());
  ATH_MSG_DEBUG("Description of m_nn " << *m_nn.get());
  // Get predicted extrapolation weights
  VNetworkBase::NetworkOutputs outputs = m_nn->compute(inputVariables);
  for (int ilayer = 0; ilayer < CaloCell_ID_FCS::MaxSample; ++ilayer) {
    if (std::find(m_relevantLayers->cbegin(), m_relevantLayers->cend(),
                  ilayer) != m_relevantLayers->cend()) {
      ATH_MSG_VERBOSE("TFCSPredictExtrapWeights::simulate: layer: "
                      << ilayer
                      << " weight: " << outputs[std::to_string(ilayer)]);
      float weight = outputs[std::to_string(ilayer)];
      // Protections
      if (weight < 0) {
        weight = 0;
      } else if (weight > 1) {
        weight = 1;
      }
      simulstate.setAuxInfo<float>(ilayer, weight);
    } else { // use weight=0.5 for non-relevant layers
      ATH_MSG_VERBOSE(
          "Setting weight=0.5 for layer = " << std::to_string(ilayer));
      simulstate.setAuxInfo<float>(ilayer, float(0.5));
    }
  }
  return FCSSuccess;
}

// simulate_hit()
FCSReturnCode TFCSPredictExtrapWeights::simulate_hit(
    Hit &hit, TFCSSimulationState &simulstate, const TFCSTruthState * /*truth*/,
    const TFCSExtrapolationState *extrapol) {

  const int cs = calosample();

  // Get corresponding predicted extrapolation weight from simulstate
  float extrapWeight;
  if (simulstate.hasAuxInfo(cs)) {
    extrapWeight = simulstate.getAuxInfo<float>(cs);
  } else { // missing AuxInfo
    ATH_MSG_FATAL(
        "Simulstate is not decorated with extrapolation weights for cs = "
        << std::to_string(cs));
    return FCSFatal;
  }

  double eta = (1. - extrapWeight) * extrapol->eta(cs, SUBPOS_ENT) +
               extrapWeight * extrapol->eta(cs, SUBPOS_EXT);
  double phi = (1. - extrapWeight) * extrapol->phi(cs, SUBPOS_ENT) +
               extrapWeight * extrapol->phi(cs, SUBPOS_EXT);
  float extrapWeight_for_r_z = extrapWeight;
  if (UseHardcodedWeight()) {
    extrapWeight_for_r_z = 0.5;
    ATH_MSG_DEBUG(
        "Will use extrapWeight=0.5 for r and z when constructing a hit");
  } else {
    ATH_MSG_DEBUG("Will use predicted extrapWeight also for r and z when "
                  "constructing a hit");
  }
  double r = (1. - extrapWeight_for_r_z) * extrapol->r(cs, SUBPOS_ENT) +
             extrapWeight_for_r_z * extrapol->r(cs, SUBPOS_EXT);
  double z = (1. - extrapWeight_for_r_z) * extrapol->z(cs, SUBPOS_ENT) +
             extrapWeight_for_r_z * extrapol->z(cs, SUBPOS_EXT);

  if (!std::isfinite(r) || !std::isfinite(z) || !std::isfinite(eta) ||
      !std::isfinite(phi)) {
    ATH_MSG_WARNING("Extrapolator contains NaN or infinite number.\nSetting "
                    "center position to calo boundary.");
    ATH_MSG_WARNING("Before fix: center_r: "
                    << r << " center_z: " << z << " center_phi: " << phi
                    << " center_eta: " << eta << " weight: " << extrapWeight
                    << " cs: " << cs);
    // If extrapolator fails we can set position to calo boundary
    r = extrapol->IDCaloBoundary_r();
    z = extrapol->IDCaloBoundary_z();
    eta = extrapol->IDCaloBoundary_eta();
    phi = extrapol->IDCaloBoundary_phi();
    ATH_MSG_WARNING("After fix: center_r: "
                    << r << " center_z: " << z << " center_phi: " << phi
                    << " center_eta: " << eta << " weight: " << extrapWeight
                    << " cs: " << cs);
  }

  hit.setCenter_r(r);
  hit.setCenter_z(z);
  hit.setCenter_eta(eta);
  hit.setCenter_phi(phi);

  ATH_MSG_DEBUG("TFCSPredictExtrapWeights: center_r: "
                << hit.center_r() << " center_z: " << hit.center_z()
                << " center_phi: " << hit.center_phi()
                << " center_eta: " << hit.center_eta()
                << " weight: " << extrapWeight << " cs: " << cs);

  return FCSSuccess;
}

// initializeNetwork()
// Initialize network
bool TFCSPredictExtrapWeights::initializeNetwork(
    int pid, const std::string &etaBin,
    const std::string &FastCaloNNInputFolderName) {

  set_pdgid(pid);
  ATH_MSG_DEBUG(
      "Using FastCaloNNInputFolderName: " << FastCaloNNInputFolderName);

  std::string inputFileName = FastCaloNNInputFolderName + "NN_" + etaBin + ".*";
  ATH_MSG_DEBUG("Will read JSON file: " << inputFileName);
  m_nn = TFCSNetworkFactory::create(inputFileName);
  if (m_nn == nullptr) {
    ATH_MSG_ERROR("Could not create network from " << inputFileName);
    return false;
  }
  ATH_MSG_VERBOSE("m_nn points to " << m_nn.get());
  // Extract relevant layers from the outputs
  m_relevantLayers = new std::vector<int>();
  for (std::string name : m_nn->getOutputLayers()) {
    // now pre-strip any common prefix.
    // std::string layer_num = ::split(name, "_")[1];
    const int layer = std::stoi(name.substr(name.find('_') + 1));
    m_relevantLayers->push_back(layer);
  }
  return true;
}

// Streamer()
void TFCSPredictExtrapWeights::Streamer(TBuffer &R__b) {
  // Stream an object of class TFCSPredictExtrapWeights
  ATH_MSG_DEBUG("In Streamer of " << __FILE__);
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(TFCSPredictExtrapWeights::Class(), this);
    ATH_MSG_DEBUG("Reading: m_nn points to " << m_nn.get());
    if (m_input && !m_input->empty()) {
      ATH_MSG_DEBUG("Reading: m_input starts as " << m_input->substr(0, 5));
      m_nn = TFCSNetworkFactory::create(*m_input);
    }
    ATH_MSG_DEBUG("Reading: m_nn points to " << m_nn.get());
#ifndef __FastCaloSimStandAlone__
    // When running inside Athena, delete input/config/normInputs to free the
    // memory
    if (freemem()) {
      delete m_input;
      m_input = nullptr;
    }
#endif
  } else {
    R__b.WriteClassBuffer(TFCSPredictExtrapWeights::Class(), this);
  }
}

// unit_test()
// Function for testing
void TFCSPredictExtrapWeights::unit_test(
    TFCSSimulationState *simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) {
  const std::string this_file = __FILE__;
  const std::string parent_dir = this_file.substr(0, this_file.find("/src/"));
  const std::string norm_path = parent_dir + "/share/NormPredExtrapSample/";
  std::string net_path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/"
                         "FastCaloSim/LWTNNPredExtrapSample/";
  test_path(net_path, norm_path, simulstate, truth, extrapol);
  net_path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastCaloSim/"
             "ONNXPredExtrapSample/";
  test_path(net_path, norm_path, simulstate, truth, extrapol);
}

// test_path()
// Function for testing
void TFCSPredictExtrapWeights::test_path(
    std::string &net_path, std::string const &norm_path,
    TFCSSimulationState *simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) {
  ISF_FCS::MLogging logger;
  ATH_MSG_NOCLASS(logger, "Testing net path ..."
                              << net_path.substr(net_path.length() - 20)
                              << " and norm path ..."
                              << norm_path.substr(norm_path.length() - 20));
  if (!simulstate) {
    simulstate = new TFCSSimulationState();
#if defined(__FastCaloSimStandAlone__)
    simulstate->setRandomEngine(new CLHEP::TRandomEngine());
#else
    simulstate->setRandomEngine(new CLHEP::RanluxEngine());
#endif
  }
  if (!truth) {
    TFCSTruthState *t = new TFCSTruthState();
    t->SetPtEtaPhiM(524288000, 0, 0, 130); // 524288 GeV
    t->set_pdgid(22);                      // photon
    truth = t;
  }
  if (!extrapol) {
    TFCSExtrapolationState *e = new TFCSExtrapolationState();
    e->set_CaloSurface_eta(truth->Eta());
    e->set_IDCaloBoundary_eta(truth->Eta());
    for (int i = 0; i < 24; ++i) {
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_ENT, truth->Eta());
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_EXT, truth->Eta());
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_MID, truth->Eta());
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_ENT, 0);
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_EXT, 0);
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_MID, 0);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_ENT, 1500 + i * 10);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_EXT, 1510 + i * 10);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_MID, 1505 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_ENT, 3500 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_EXT, 3510 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_MID, 3505 + i * 10);
    }
    extrapol = e;
  }

  // Set energy in layers which then will be retrieved in simulate_hit()
  simulstate->set_E(0, 1028.77124023);
  simulstate->set_E(1, 68199.0625);
  simulstate->set_E(2, 438270.78125);
  simulstate->set_E(3, 3024.02929688);
  simulstate->set_E(12, 1330.10131836);
  simulstate->set_E(1028.77124023 + 68199.0625 + 438270.78125 + 3024.02929688 +
                    1330.10131836);
  simulstate->set_Efrac(0, simulstate->E(0) / simulstate->E());
  simulstate->set_Efrac(1, simulstate->E(1) / simulstate->E());
  simulstate->set_Efrac(2, simulstate->E(2) / simulstate->E());
  simulstate->set_Efrac(3, simulstate->E(3) / simulstate->E());
  simulstate->set_Efrac(12, simulstate->E(12) / simulstate->E());

  const int pdgId = truth->pdgid();
  const float Ekin = truth->Ekin();
  const float eta = truth->Eta();

  ATH_MSG_NOCLASS(logger, "True energy " << Ekin << " pdgId " << pdgId
                                         << " eta " << eta);

  // Find eta bin
  const int Eta = eta * 10;
  std::string etaBin = "";
  for (int i = 0; i <= 25; ++i) {
    int etaTmp = i * 5;
    if (Eta >= etaTmp && Eta < (etaTmp + 5)) {
      etaBin = std::to_string(i * 5) + "_" + std::to_string((i + 1) * 5);
    }
  }

  ATH_MSG_NOCLASS(logger, "etaBin = " << etaBin);

  TFCSPredictExtrapWeights NN("NN", "NN");
  NN.setLevel(MSG::INFO);
  const int pid = truth->pdgid();
  NN.initializeNetwork(pid, etaBin, net_path);
  NN.getNormInputs(etaBin, norm_path);

  // Get extrapWeights and save them as AuxInfo in simulstate

  // Get inputs to Neural Network
  const int pid2 = *(NN.pdgid()).begin();
  ATH_MSG_NOCLASS(logger, " Have pid2 = " << pid2);
  const VNetworkBase::NetworkInputs inputVariables =
      NN.prepareInputs(*simulstate, truth->E() * 0.001);

  // Get predicted extrapolation weights
  ATH_MSG_NOCLASS(logger, "computing with m_nn");
  VNetworkBase::NetworkOutputs outputs = NN.m_nn->compute(inputVariables);
  const std::vector<int> layers = {0, 1, 2, 3, 12};
  for (int ilayer : layers) {
    simulstate->setAuxInfo<float>(ilayer, outputs[std::to_string(ilayer)]);
  }

  // Simulate
  const int layer = 0;
  NN.set_calosample(layer);
  TFCSLateralShapeParametrizationHitBase::Hit hit;
  NN.simulate_hit(hit, *simulstate, truth, extrapol);

  // Write
  TFile *fNN = new TFile("FCSNNtest.root", "RECREATE");
  NN.Write();
  fNN->ls();
  fNN->Close();
  delete fNN;

  // Open
  fNN = TFile::Open("FCSNNtest.root");
  TFCSPredictExtrapWeights *NN2 = (TFCSPredictExtrapWeights *)(fNN->Get("NN"));

  NN2->setLevel(MSG::INFO);
  NN2->simulate_hit(hit, *simulstate, truth, extrapol);
  simulstate->Print();

  return;
}

void TFCSPredictExtrapWeights::Print(Option_t *option) const {
  TString opt(option);
  const bool shortprint = opt.Index("short") >= 0;
  const bool longprint =
      msgLvl(MSG::DEBUG) || (msgLvl(MSG::INFO) && !shortprint);
  TString optprint = opt;
  optprint.ReplaceAll("short", "");
  TFCSLateralShapeParametrizationHitBase::Print(option);

  if (longprint)
    ATH_MSG_INFO(optprint << "  m_input (TFCSPredictExtrapWeights): "
                          << CxxUtils::as_const_ptr(m_input));
  if (longprint)
    ATH_MSG_INFO(optprint << "  Address of m_nn: " << m_nn.get());
}
