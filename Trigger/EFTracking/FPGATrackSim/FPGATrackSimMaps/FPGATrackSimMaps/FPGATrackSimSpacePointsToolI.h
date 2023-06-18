// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimSPACEPOINTSTOOLI_H
#define FPGATrackSimSPACEPOINTSTOOLI_H

/**
 * @file FPGATrackSimSpacePointsToolI.h
 * @author Julian Wollrath - wollrath@cern.ch
 * @date 2021
 * @brief Declares an abstract class that implements an interface for spacepoint formation.
 * This class is implemented in
 *      FPGATrackSimSpacePointsTool.h
 */

#include "GaudiKernel/IAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"

const InterfaceID IID_FPGATrackSimSpacePointsToolI("FPGATrackSimSpacePointsToolI", 1, 0);

class FPGATrackSimSpacePointsToolI : virtual public ::IAlgTool {
 public:
    DeclareInterfaceID(FPGATrackSimSpacePointsToolI, 1, 0);
    virtual ~FPGATrackSimSpacePointsToolI() = default;

    virtual StatusCode DoSpacePoints(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) = 0;
};

#endif // FPGATrackSimSPACEPOINTSTOOLI_H
