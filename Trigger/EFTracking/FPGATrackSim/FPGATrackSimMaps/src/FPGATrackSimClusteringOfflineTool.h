/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimCLUSTERINGOFFLINETOOL_H
#define FPGATrackSimCLUSTERINGOFFLINETOOL_H

/*
 * httClustering
 * ---------------
 *
 * Routines to perform clustering in the pixels, based on FPGATrackSim
 *
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stack>
#include <queue>

class FPGATrackSimClusteringOfflineTool : public extends <AthAlgTool,FPGATrackSimClusteringToolI> {
public:

  FPGATrackSimClusteringOfflineTool(const std::string&, const std::string&, const IInterface*);

  virtual ~FPGATrackSimClusteringOfflineTool() = default;

  virtual StatusCode DoClustering(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) const override;

 private:

};

#endif // FPGATrackSimCLUSTERINGOFFLINETOOL_H
