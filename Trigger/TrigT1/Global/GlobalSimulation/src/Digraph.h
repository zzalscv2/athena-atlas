//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_DIGRAPH_H
#define GLOBSIM_DIGRAPH_H

#include <vector>
#include <ostream>

/*
 * This class contains a representation of a Digraph
 * The number of nodes is fixed at the time of contruction.
 *
 * The graph is represented by a simple adjancy table of node numbers.
 */
namespace GlobalSim {
  class Digraph {
  public:
    friend std::ostream& operator << (std::ostream&, const Digraph&);
    Digraph(std::size_t V);
    Digraph(const std::string& fn);
    
    std::size_t V() const {return m_V;}
    std::size_t E() const {return m_E;}
    
    void addEdge(std::size_t v, std::size_t w);
    std::vector<std::size_t> adj(std::size_t v) const;
    Digraph reverse() const;
    std::string to_string() const;
    
  private:
    std::size_t m_V{0};
    std::size_t m_E{0};

    std::vector<std::vector<std::size_t>> m_adj;
  };
  
  std::ostream& operator << (std::ostream& os, const Digraph& G);
}
#endif
