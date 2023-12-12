//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_GRAPHALGS_H 
#define GLOBALSIM_GRAPHALGS_H

#include "Digraph.h"
#include <vector>
#include <stack>
#include <queue>
#include <memory>


// See standard text books eg Sedgewick and Wayne, Algorithms, edition 4
// Algothms in C++ Part 5 Graph Algorithms Sedgewick and Van Wyk
// Of particular intersest is the topological ordering of a digraph which
// is used to determine the execute order off Algorithms in a call graph.

namespace GlobalSim {
  class DirectedDFS {
  public:

    DirectedDFS(const Digraph& G, const std::vector<std::size_t>& sources);
    DirectedDFS(const Digraph& G, std::size_t s);
    DirectedDFS(const std::unique_ptr<Digraph>& G, std::size_t s);



    bool reachable(std::size_t v) const {return m_marked.at(v);}

  private:
    std::vector<bool> m_marked;
    void dfs(const Digraph&G, std::size_t v);
  };

  class DirectedCycle {
  public:
    DirectedCycle(const Digraph& G) :
      m_onStack(G.V(), false),
      m_marked(G.V(), false),
      m_edgeTo(G.V(), -1) // large number as unsigned
    {
      auto GV = G.V();
      for (std::size_t v = 0; v != GV; ++v) {
	if (!m_marked.at(v)){dfs(G, v);}
      }
    }

    DirectedCycle(const std::unique_ptr<Digraph>& G) :
      DirectedCycle(*G){}

    bool hasCycle() const {return !m_cycle.empty();}
    const std::vector<std::size_t> cycle() const {return m_cycle;}
  private:
    std::vector<bool> m_onStack;
    std::vector<bool> m_marked;

    std::vector<std::size_t> m_cycle;
    std::vector<std::size_t> m_edgeTo;
    
    void dfs(const Digraph& G, std::size_t v);
    
  };

  class DepthFirstOrder {
  public:
    DepthFirstOrder(const Digraph& G);
    std::vector<std::size_t> pre() const {return m_pre;}
    std::vector<std::size_t> post() const {return m_post;}
    std::vector<std::size_t> reversePost() const {return m_reversePost;}
    
  private:
    std::vector<std::size_t> m_pre;
    std::vector<std::size_t> m_post;
    std::vector<std::size_t> m_reversePost;
    
    std::vector<bool> m_marked;
    
    void dfs(const Digraph& G, std::size_t v);
 
  };

  // Find a topoligical sort of a Digraph, or report if not possible.
  class Topological {
  public:

    Topological(const Digraph& G);
    const std::vector<std::size_t>& order() const;
    bool isDag() const;
    
  private:
    std::vector<std::size_t> m_order;
  };
}

#endif
