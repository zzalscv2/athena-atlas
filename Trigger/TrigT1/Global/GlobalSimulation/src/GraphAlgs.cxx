//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GraphAlgs.h"
#include <sstream>


namespace GlobalSim {

  DirectedDFS::DirectedDFS(const Digraph& G, const std::vector<std::size_t>& sources) {
      m_marked = std::vector<bool>(G.V(), false);
      for (const auto& v : sources) {
        dfs(G, v);
      }      
    }

  DirectedDFS::DirectedDFS(const Digraph& G,
			   std::size_t s) :
    DirectedDFS(G, std::vector<std::size_t>{s}) {}

  
  DirectedDFS::DirectedDFS(const std::unique_ptr<Digraph>& G,
			   std::size_t s) :
    DirectedDFS(*G, std::vector<std::size_t>{s}) {}


  
  void DirectedDFS::dfs(const Digraph&G, std::size_t v) {
    m_marked[v] = true;
    for (const auto& w : G.adj(v)) {
      if(!m_marked[w]) dfs(G, w);
    }
  }

  void DirectedCycle::dfs(const Digraph& G, std::size_t v) {
    std::stringstream ss;
 

    m_onStack[v] = true;
    m_marked[v] = true;

    ss <<"0 entering DirectedCycle::dfs onStack set for v: " << v ;
    for (std::size_t w : G.adj(v)) {
      if (!m_cycle.empty()) {return;}
      if(!m_marked[w]) {
	m_edgeTo[w] = v;
	
	dfs(G, w);
      } else if (m_onStack[w]){
	for(std::size_t x = v; x != w; x = m_edgeTo[x]) {
	  m_cycle.push_back(x);
	}
	m_cycle.push_back(w);
	m_cycle.push_back(v);
      }
    }
    m_onStack[v] = false;
  }   
  
  DepthFirstOrder::DepthFirstOrder(const Digraph& G): m_marked(G.V(), false){
    for (std::size_t v = 0; v != G.V(); ++v) {
      if (!m_marked[v]) {dfs(G, v);}
    }
    
    m_reversePost.assign(std::make_reverse_iterator(m_post.end()),
			 std::make_reverse_iterator(m_post.begin()));
  }
  
  void DepthFirstOrder::dfs(const Digraph& G, std::size_t v) {
    
    m_pre.push_back(v);
    m_marked[v] = true;
    
    for (const std::size_t w : G.adj(v)) {
      if (!m_marked[w]) {

	dfs(G, w);
      }
    }
    
    m_post.push_back(v);
  }


  Topological::Topological(const Digraph& G) {

    auto cycleFinder = DirectedCycle(G);
    if (!cycleFinder.hasCycle()){
      auto dfs = DepthFirstOrder(G);
      // if all edges were resevered, this would be
      //  m_order = dfs.reversePost();
      m_order = dfs.post();
    }

  }

  const std::vector<std::size_t>& Topological::order() const {
    return m_order;
  }



  bool Topological::isDag() const {
    return !order().empty();
  }
}    
