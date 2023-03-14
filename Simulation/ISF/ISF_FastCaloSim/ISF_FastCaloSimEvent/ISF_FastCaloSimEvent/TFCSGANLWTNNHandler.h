/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TFCSGANLWTNNHandler.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_TFCSGANLWTNNHANDLER_H
#define ISF_TFCSGANLWTNNHANDLER_H 1

#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"

#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"
#include <string>

class TFCSGANLWTNNHandler {
public:
  TFCSGANLWTNNHandler();
  virtual ~TFCSGANLWTNNHandler();

  const lwt::LightweightGraph *GetGraph() const { return m_graph; }

  bool LoadGAN(const std::string &inputFile);

private:
  const lwt::LightweightGraph *m_graph; //! Do not persistify
  std::string *m_input = nullptr;

  ClassDef(TFCSGANLWTNNHandler, 5) // TFCSGANLWTNNHandler
};

#endif //> !ISF_TFCSGANLWTNNHANDLER_H
