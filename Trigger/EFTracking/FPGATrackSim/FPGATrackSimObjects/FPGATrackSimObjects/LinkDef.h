/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <vector>
#include <map>
#include <utility>


#ifdef __CINT__
#ifndef FPGATrackSimOBJECTS_FPGATrackSim_STANDALONE
#define FPGATrackSimOBJECTS_FPGATrackSim_STANDALONE
#endif
// unimplemented in rel 22
// #pragma link off globals;
// #pragma link off classes;
// #pragma link off functions;


#pragma link C++ class FPGATrackSimEventInfo+;
#pragma link C++ class FPGATrackSimOptionalEventInfo+;
#pragma link C++ class FPGATrackSimEventInputHeader+;
#pragma link C++ class FPGATrackSimLogicalEventInputHeader+;
#pragma link C++ class FPGATrackSimLogicalEventOutputHeader+;
#pragma link C++ class FPGATrackSimTowerInputHeader+;
#pragma link C++ class std::vector<FPGATrackSimTowerInputHeader>+;
#pragma link C++ class FPGATrackSimDataFlowInfo+;

#pragma link C++ class FPGATrackSimOfflineTrack+;
#pragma link C++ class std::vector<FPGATrackSimOfflineTrack>+;
#pragma link C++ class FPGATrackSimOfflineHit+;
#pragma link C++ class std::vector<FPGATrackSimOfflineHit>+;

#pragma link C++ class FPGATrackSimCluster+;
#pragma link C++ class std::vector<FPGATrackSimCluster*>+;
#pragma link C++ class std::vector<FPGATrackSimCluster>+;

#pragma link C++ class FPGATrackSimHit+;
#pragma link C++ class std::vector<FPGATrackSimHit>+;
#pragma link C++ class std::vector<FPGATrackSimHit*>+;
#pragma link C++ class std::vector<std::vector<const FPGATrackSimHit*>>+;

#pragma link C++ class FPGATrackSimRoad+;
#pragma link C++ class FPGATrackSimMultiTruth+;
#pragma link C++ class FPGATrackSimMatchInfo+;

#pragma link C++ class FPGATrackSimTrack+;
#pragma link C++ class std::vector<FPGATrackSimTrack>+;

#pragma link C++ class FPGATrackSimTruthTrack+;
#pragma link C++ class std::vector<FPGATrackSimTruthTrack>+;

#pragma link C++ class FPGATrackSimTrackStream+;
#endif
