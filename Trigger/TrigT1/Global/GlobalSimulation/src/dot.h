//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * This file contains functions that produce dot files (graph visualisation).
 */

#ifndef GLOBALSIM_DOT_H
#define GLOBALSIM_DOT_H


#include <memory>
#include <string>

// A utitilty to create dot files to visulise digraphs.

namespace GlobalSim {
  class Digraph;
  void dot(const Digraph& G, const std::string& fn);
  void dot(const std::unique_ptr<Digraph>& G, const std::string& fn);
}
#endif
