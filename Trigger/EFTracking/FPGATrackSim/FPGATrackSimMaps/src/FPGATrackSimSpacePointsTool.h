/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimSPACEPOINTSTOOL_H
#define FPGATrackSimSPACEPOINTSTOOL_H

#include <array>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimMaps/FPGATrackSimSpacePointsToolI.h"

class FPGATrackSimSpacePointsTool : public extends<AthAlgTool, FPGATrackSimSpacePointsToolI> {
 public:
    FPGATrackSimSpacePointsTool(const std::string &, const std::string &, const IInterface *);
    virtual ~FPGATrackSimSpacePointsTool() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode DoSpacePoints(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) override;

 private:
    void SpacePointFinder(const std::vector<FPGATrackSimHit> &, std::vector<std::array<int, 2>> &);


    Gaudi::Property<bool> m_duplicate {this, "Duplication", false, "Duplicate spacepoint to layer on the other side of the stave"};
    Gaudi::Property<bool> m_filter {this, "Filtering", false, "Filter out incomplete spacepoints"};
    Gaudi::Property<bool> m_filterClose {this, "FilteringClosePoints", false, "Filter out single hits close to spacepoints"};
};

#endif // FPGATrackSimSPACEPOINTSTOOL_H
