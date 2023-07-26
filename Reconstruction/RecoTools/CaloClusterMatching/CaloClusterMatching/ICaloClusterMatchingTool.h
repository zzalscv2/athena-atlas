/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


///////////////////////// -*- C++ -*- /////////////////////////////
// ICaloClusterMatchingTool.h 
// Header file for class ICaloClusterMatchingTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef CALOCLUSTERMATCHING_ICALOCLUSTERMATCHINGTOOL_H
#define CALOCLUSTERMATCHING_ICALOCLUSTERMATCHINGTOOL_H 1

// STL includes

// FrameWork includes
#include "AsgTools/IAsgTool.h"
#include "StoreGate/WriteDecorHandle.h"

// Forward declaration

// xAOD includes
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloClusterMatching/TopoClusterMap.h"

namespace ClusterMatching {
  typedef std::pair<const xAOD::CaloCluster*,float> tcmatch_pair;

  inline static bool gtrClusterE(const tcmatch_pair& a, const tcmatch_pair& b) {return a.first->e()>b.first->e();}
  inline static bool gtrMatchedE(const tcmatch_pair& a, const tcmatch_pair& b) {return a.second*a.first->e()>b.second*b.first->e();}
  inline static bool gtrMatchedEFrac(const tcmatch_pair& a, const tcmatch_pair& b) {return a.second>b.second;}

  enum ClusterSortMethod{
    ClusterE,
    MatchedE,
    MatchedEFrac
  };
}

class ICaloClusterMatchingTool
  : virtual public asg::IAsgTool
{ 
  ASG_TOOL_INTERFACE(ICaloClusterMatchingTool)


public: 
  using elementLinkDecorHandle_t = SG::WriteDecorHandle<xAOD::CaloClusterContainer,std::vector<ElementLink<xAOD::CaloClusterContainer> > >;


  // Prime the map before starting to match
  virtual StatusCode fillClusterMap(const EventContext& ctx, TopoClusterMap& tcmap) const = 0;

  // return shared fraction of clustered cell energy (wrt testCluster)
  virtual float getClusterSharedEfrac(const xAOD::CaloCluster& refCluster,
				      const xAOD::CaloCluster& testCluster) const = 0;

  // return true if clusters share a given fraction of their cell energy
  virtual bool clustersAreMatched(const xAOD::CaloCluster& refCluster,
				  const xAOD::CaloCluster& testCluster) const = 0;

  // fill a list of clusters from the testClusters container that match the reference cluster
  // match criteria determined by calling clustersAreMatched
  // return true if matchedClusters list is non-empty
  virtual bool getMatchedClusters(const xAOD::CaloCluster& refCluster,
				  const std::vector<const xAOD::CaloCluster*>& testClusters,
				  std::vector<const xAOD::CaloCluster*>& matchedClusters) const = 0;

  // fill a list of clusters from the configured cluster container that match the reference cluster
  // match criteria determined by calling clustersAreMatched
  // return true if matchedClusters list is non-empty
  virtual bool getMatchedClusters(const xAOD::CaloCluster& refCluster,
				  std::vector<const xAOD::CaloCluster*>& matchedClusters,
				  const TopoClusterMap& tcmap,
				  bool useLeadingCellEtaPhi=false) const = 0;

  // fill a list of clusters from the testClusters container that match the reference cluster
  // match criteria determined by calling clustersAreMatched
  // return true if matchedClusters list is non-empty
  virtual bool getMatchedClusters(const xAOD::CaloCluster& refCluster,
				  const std::vector<const xAOD::CaloCluster*>& testClusters,
				  std::vector<std::pair<const xAOD::CaloCluster*, float> >& matchedClustersAndE) const = 0;

  // fill a list of clusters from the configured cluster container that match the reference cluster
  // match criteria determined by calling clustersAreMatched
  // return true if matchedClusters list is non-empty
  virtual bool getMatchedClusters(const xAOD::CaloCluster& refCluster,
				  std::vector<std::pair<const xAOD::CaloCluster*, float> >& matchedClustersAndE,
				  const TopoClusterMap& tcmap,
				  bool useLeadingCellEtaPhi=false) const = 0;

  // set ElementLinks to clusters from the configured cluster container that match the reference cluster
  // works via getMatchedClusters
  // return true if matchedClusters list is non-empty
  virtual StatusCode linkMatchedClusters(elementLinkDecorHandle_t& elementLinkDec,
                                         const xAOD::CaloCluster& refCluster,
					 const std::vector<const xAOD::CaloCluster*>& testClusters,
					 bool (*gtrthan)(const std::pair<const xAOD::CaloCluster*,float>& pair1,
							 const std::pair<const xAOD::CaloCluster*,float>& pair2) = ClusterMatching::gtrMatchedE) const = 0;

  // set ElementLinks to clusters from the configured cluster container that match the reference cluster
  // works via getMatchedClusters
  // return true if matchedClusters list is non-empty
  virtual StatusCode linkMatchedClusters(elementLinkDecorHandle_t& elementLinkDec,
                                         const xAOD::CaloCluster& refCluster,
					 const TopoClusterMap& tcmap,
					 bool useLeadingCellEtaPhi=false,
					 bool (*gtrthan)(const std::pair<const xAOD::CaloCluster*,float>& pair1,
							 const std::pair<const xAOD::CaloCluster*,float>& pair2) = ClusterMatching::gtrMatchedE) const = 0;
}; 

#endif //> !CALOCLUSTERMATCHING_ICALOCLUSTERMATCHINGTOOL_H
