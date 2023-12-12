//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "dot.h"
#include "Digraph.h"
#include <fstream>

using namespace GlobalSim;

namespace GlobalSim {
  void dot(const Digraph& G, const std::string& fn) {
    std::ofstream ofs (fn, std::ofstream::out);
    std::size_t V = G.V();

    ofs << "digraph G{\n";
    ofs << "layout=twopi ranksep=3 ratio=auto\n";

    for (std::size_t v = 0; v != V; ++v) {
      for (const auto& w : G.adj(v)) {
	ofs  << v << "->" << w << '\n';
      }
    }
    ofs << "}\n";
  }

  void dot(const std::unique_ptr<Digraph>& G, const std::string& fn) {
    dot(*G, fn);
  }
}

