// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimCLUSTERINGTOOLI_H
#define FPGATrackSimCLUSTERINGTOOLI_H

/**
 * @file FPGATrackSimClusteringToolI.h
 * @author Alex Martyniuk - martyniu@cern.ch
 * @date 04/12/19
 * @brief Declares an abstract class that implements an interface for pixel clustering.
 * This class is implemented in
 *      FPGATrackSimClusteringTool.h
 *      FPGATrackSimClusteringFTKTool.h
 *      FPGATrackSimClusteringOfflineTool.h
 */

#include "GaudiKernel/IAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"

class FPGATrackSimClusteringToolI : virtual public ::IAlgTool
{
    public:
	DeclareInterfaceID( FPGATrackSimClusteringToolI, 1, 0);
	virtual ~FPGATrackSimClusteringToolI() = default;

	virtual StatusCode DoClustering(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) const = 0;
};

#endif //FPGATrackSimCLUSTERINGTOOLI_H
