/**
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 *
 * Class for a neural network read in the LWTNN format.
 * Derived from the abstract base class VNetworkBase
 * such that it can be used interchangably with it's
 * sibling classes, TFCSSimpleLWTNNHandler, TFCSGANLWTNNHandler,
 * TFCSONNXHandler.
 *
 * Frustratingly, LightweightNeuralNetwork and LightweightGraph
 * from lwtnn do not have a common ancestor,
 * they could be connected with the bridge pattern,
 * but that is more complex that currently required.
 *
 * The LoadNetwork function has VNetworkBase as it's return type
 * so that it can make a run-time decision about which derived class
 * to use, based on the file name presented.
 **/

// Hopefully documentation gets inherited from VNetworkBase

#ifndef TFCSSIMPLELWTNNHANDLER_H
#define TFCSSIMPLELWTNNHANDLER_H

#include "ISF_FastCaloSimEvent/VNetworkLWTNN.h"
#include <iostream>

// Becuase we have a field of type LightweightNeuralNetwork
#include "lwtnn/LightweightNeuralNetwork.hh"

// For writing to a tree
#include "TTree.h"

class TFCSSimpleLWTNNHandler : public VNetworkLWTNN {
public:
  // Don't lose the default constructor
  using VNetworkLWTNN::VNetworkLWTNN;

  /**
   * @brief TFCSSimpleLWTNNHandler constructor.
   *
   * Calls setupPersistedVariables and setupNet.
   *
   * @param inputFile file-path on disk (with file name) of a readable
   *                  lwtnn file containing a json format description
   *                  of the network to be constructed, or the json
   *                  itself as a string.
   **/
  explicit TFCSSimpleLWTNNHandler(const std::string &inputFile);

  /**
   * @brief TFCSSimpleLWTNNHandler copy constructor.
   *
   * Will copy the variables that would be generated by
   * setupPersistedVariables and setupNet.
   *
   * @param copy_from existing network that we are copying
   **/
  TFCSSimpleLWTNNHandler(const TFCSSimpleLWTNNHandler &copy_from);

  /**
   * @brief Function to pass values to the network.
   *
   * This function, hides variations in the formated needed
   * by different network libraries, providing a uniform input
   * and output type.
   *
   * @param inputs  values to be evaluated by the network
   * @return        the output of the network
   * @see VNetworkBase::NetworkInputs
   * @see VNetworkBase::NetworkOutputs
   **/
  NetworkOutputs compute(NetworkInputs const &inputs) const override;

  /**
   * @brief List the names of the outputs.
   *
   * Outputs are stored in an NetworkOutputs object
   * which is indexed by strings. This function
   * returns the list of all strings that will index the outputs.
   *
   **/
  std::vector<std::string> getOutputLayers() const override;

protected:
  /**
   * @brief Perform actions that prepare network for use.
   *
   * Will be called in the streamer or class constructor
   * after the inputs have been set (either automaically by the
   * streamer or by setupPersistedVariables in the constructor).
   * Does not delete any resources used.
   *
   **/
  void setupNet() override;

private:
  // unique ptr deletes the object when it goes out of scope
  /**
   * @brief The network that we are wrapping here.
   **/
  std::unique_ptr<lwt::LightweightNeuralNetwork>
      m_lwtnn_neural; //! Do not persistify
  /**
   * @brief List of names that index the output layer.
   **/
  std::vector<std::string> m_outputLayers; //! Do not persistify

  // Suppling a ClassDef for writing to file.
  ClassDefOverride(TFCSSimpleLWTNNHandler, 1);
};

#endif // TFCSSIMPLELWTNNHANDLER_H
