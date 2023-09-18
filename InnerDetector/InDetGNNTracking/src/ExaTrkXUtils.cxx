/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ExaTrkXUtils.hpp"
#include <numeric>     // std::iota
#include <algorithm>   // std::sort


void buildEdges(
    std::vector<float>& embedFeatures,
    std::vector<int64_t>& edgeList,
    int64_t numSpacepoints,
    int embeddingDim,    // dimension of embedding space
    float rVal, // radius of the ball
    int kVal    // number of nearest neighbors
){
    // calculate the distances between two spacepoints in the embedding space
    // and keep the k-nearest neighbours within the radius r
    // the distance is calculated using the L2 norm
    // the edge list is with dimension of [2, number-of-edges]
    // the first row is the source node index
    // the second row is the target node index
    // TODO: use the KDTree to speed up the calculation

    std::vector<float> dists;
    dists.reserve(numSpacepoints);
    std::vector<int> idx(numSpacepoints);
    std::vector<int64_t> senders;
    std::vector<int64_t> receivers;
    for (int64_t i = 0; i < numSpacepoints; i++) {
        dists.clear();
        for (int64_t j = 0; j < numSpacepoints; j++) {
            if (i == j) {
                dists.push_back(0);
                continue;
            }
            float dist = 0;
            for (int k = 0; k < embeddingDim; k++) {
                float dist_k = embedFeatures[i * embeddingDim + k] - embedFeatures[j * embeddingDim + k];
                dist += dist_k * dist_k;
            }
            dists.push_back(sqrt(dist));
        }
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&dists](int i1, int i2) {return dists[i1] < dists[i2];});
        int numFilled = -1;
        for (int j = 0; j < numSpacepoints; j++) {
            if (i == j) continue;
            if (dists[idx[j]] > rVal) break;
            numFilled++;
            senders.push_back(i);
            receivers.push_back(idx[j]);
            if (numFilled >= kVal) break;
        }
    }
    edgeList.resize(2 * senders.size());
    std::copy(senders.begin(), senders.end(), edgeList.begin());
    std::copy(receivers.begin(), receivers.end(), edgeList.begin() + senders.size());
}
