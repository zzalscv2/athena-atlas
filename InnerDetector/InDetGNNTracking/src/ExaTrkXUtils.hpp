/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// **********************************
// Utils for the ExaTrkX algorithm. 
// @author xiangyang.ju@cern.ch
//***********************************

#pragma once 

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


void buildEdges(
    std::vector<float>& embedFeatures,
    std::vector<int64_t>& edgeList,
    int64_t numSpacepoints,
    int embeddingDim,    // dimension of embedding space
    float rVal, // radius of the ball
    int kVal    // number of nearest neighbors
);

template <typename vertex_t, typename weight_t, typename label_t>
void weaklyConnectedComponents(
vertex_t numNodes,
std::vector<vertex_t>& rowIndices,
std::vector<vertex_t>& colIndices,
std::vector<weight_t>& edgeWeights,
std::vector<label_t>& trackLabels) 
{
    typedef boost::adjacency_list<
      boost::vecS,            // edge list
      boost::vecS,            // vertex list
      boost::undirectedS,     // directedness
      boost::no_property,     // property associated with vertices
      float                  // property associated with edges
    > Graph; 

    Graph g(numNodes);
    float edge_cut = 0.75;
    for(size_t idx=0; idx < rowIndices.size(); ++idx) {
        if (edgeWeights[idx] > edge_cut) {
            boost::add_edge(rowIndices[idx], colIndices[idx], edgeWeights[idx], g);
        }
    }
    boost::connected_components(g, &trackLabels[0]);
}
