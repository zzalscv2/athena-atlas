/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TFCSGANLWTNNHandler.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// class header include
#include "ISF_FastCaloSimEvent/TFCSGANLWTNNHandler.h"

#include "TFile.h" //Needed for TBuffer

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

TFCSGANLWTNNHandler::TFCSGANLWTNNHandler() { m_graph = nullptr; }

TFCSGANLWTNNHandler::~TFCSGANLWTNNHandler() {
  if (m_input != nullptr) {
    delete m_input;
  }
  if (m_graph != nullptr) {
    delete m_graph;
  }
}

bool TFCSGANLWTNNHandler::LoadGAN(const std::string &inputFile) {
  std::ifstream input(inputFile);
  std::stringstream sin;
  sin << input.rdbuf();
  input.close();
  // build the graph
  auto config = lwt::parse_json_graph(sin);
  m_graph = new lwt::LightweightGraph(config);
  if (m_graph == nullptr) {
    return false;
  }
  if (m_input != nullptr) {
    delete m_input;
  }
  m_input = new std::string(sin.str());
  return true;
}

void TFCSGANLWTNNHandler::Streamer(TBuffer &R__b) {
  // Stream an object of class TFCSGANLWTNNHandler
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(TFCSGANLWTNNHandler::Class(), this);
    if (m_graph != nullptr) {
      delete m_graph;
      m_graph = nullptr;
    }
    if (m_input != nullptr) {
      std::stringstream sin;
      sin.str(*m_input);
      auto config = lwt::parse_json_graph(sin);
      m_graph = new lwt::LightweightGraph(config);
    }
#ifndef __FastCaloSimStandAlone__
    // When running inside Athena, delete config to free the memory
    delete m_input;
    m_input = nullptr;
#endif
  } else {
    R__b.WriteClassBuffer(TFCSGANLWTNNHandler::Class(), this);
  }
}
