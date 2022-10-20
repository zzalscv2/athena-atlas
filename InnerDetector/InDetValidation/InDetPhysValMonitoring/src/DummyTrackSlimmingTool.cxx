/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "DummyTrackSlimmingTool.h"
#include "TrkTrack/Track.h"

DummyTrackSlimmingTool::DummyTrackSlimmingTool(const std::string& type, const std::string& name, const IInterface* p)
  : AthAlgTool(type, name, p) {
  declareInterface<Trk::ITrackSlimmingTool>(this);
}

void
DummyTrackSlimmingTool::slimTrack(Trk::Track&) const{
}

void
DummyTrackSlimmingTool::slimConstTrack(const Trk::Track&) const{
}




