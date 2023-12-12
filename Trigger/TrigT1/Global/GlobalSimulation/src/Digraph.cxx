//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "Digraph.h"
#include <sstream>
#include <fstream>


using namespace GlobalSim;

Digraph::Digraph(std::size_t V): m_V{V} {
  
  m_adj = std::vector<std::vector<std::size_t>>(V, std::vector<std::size_t>());
}

Digraph::Digraph(const std::string& fn) {
  std::ifstream ifs;
  ifs.open(fn);
  
  std::size_t E{0};
  ifs >> m_V;
  ifs >> E;
  m_adj = std::vector<std::vector<std::size_t>>(m_V, std::vector<std::size_t>());
  std::size_t k, v;
  while (true){
    ifs >> k >> v;
    if (!ifs){break;}
    addEdge(k, v);
  }
}

void Digraph::addEdge(std::size_t v, std::size_t w) {
  m_adj.at(v).push_back(w);
  ++m_E;
}
  
std::vector<std::size_t> Digraph::adj(std::size_t v) const  {
  return m_adj.at(v);
}
  
Digraph Digraph::reverse() const {
  Digraph R = Digraph(m_V);
  for (std::size_t v = 0; v != m_V; ++v) {
    for (std::size_t w : m_adj.at(v)) {
      R.addEdge(w, v);
    }
  }
  return R;
}


std::string Digraph::to_string() const {
    std::stringstream ss;
    ss << m_V << '\n';
    ss << m_E << '\n';
    std::size_t count = 0;
    for (const auto& p : m_adj){
      for (const auto& n : p) {
	ss << count << " " << n << '\n';
      }
      ++count;
    }
    return ss.str();
}

namespace GlobalSim {
  std::ostream& operator << (std::ostream& os, const Digraph& G) {
    os << "Digraph V: " << G.V() << " E: " << G.E() << " adj \n";
    for (std::size_t v = 0; v != G.V(); ++v) {
      os << v << ": ";
      for (const auto& w : G.adj(v)){
	os << w << " ";
      }
      os << '\n';
    }
    return os;
  }
}
