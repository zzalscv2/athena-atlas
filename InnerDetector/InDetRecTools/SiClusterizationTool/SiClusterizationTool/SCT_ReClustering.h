/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/**
 * @file SCT_ReClustering.h
 * header file for SCT_ReClustering class
 * @author Paul J. Bell
 * @date Sept 2004
 */


#ifndef SiClusterizationTool_SCT_ReClustering_H
#define SiClusterizationTool_SCT_ReClustering_H

class SCT_ID;
class Identifier;
#include <vector>

///This performs reclustering after identifying dead or noisy channels 
class SCT_ReClustering
{
 public:
  /** called by SCT_ClusteringTool. If some bad channel has broken the 
   * cluster (provided as a list of RDOs) in non-consecutive fragments, just split it.
   */ 
  static std::vector<std::vector<Identifier> > recluster(std::vector<std::vector<Identifier>>&, const SCT_ID&);
    
 private:
  typedef std::vector<Identifier> ID_Vector; 
  typedef std::vector<Identifier>::iterator Discont; 
};

#endif // SiClusterizationTool_SCT_ReClustering_H
