/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTRIGUTILS_H
#define BTRIGUTILS_H

#include <vector>

#include "GaudiKernel/MsgStream.h"

#include "TrigInDetEvent/TrigInDetTrack.h"
#include "TrigInDetEvent/TrigInDetTrackFitPar.h"
//#include "TrigMuonEvent/TrigMuonEFInfoContainer.h"
#include "TrigSteeringEvent/Enums.h"

#include "TrkTrack/Track.h"

// xAOD edm
#include "xAODMuon/MuonContainer.h"


double InvMass(const std::vector<const TrigInDetTrack*>  &    , const std::vector<double>&);
double InvMass(const std::vector<const TrigInDetTrackFitPar*>&, const std::vector<double>&);
double InvMass(const std::vector<const Trk::TrackParameters*>&, const std::vector<double>&);
double InvMass(const std::vector<const Trk::Track*>          &, const std::vector<double>&);
double InvMass(const std::vector<const Trk::Perigee*>        &, const std::vector<double>&);


//HLT::ErrorCode GetTrigMuonEFInfoTracks(const TrigMuonEFInfoContainer*, std::vector<const Trk::Track*>&, MsgStream&);
HLT::ErrorCode GetxAODMuonTracks(const xAOD::MuonContainer*, std::vector<const Trk::Track*>&, MsgStream&);

// JW new methods to help simplify the algorithms

double fabsDeltaPhi(double phi1, double phi2); //! absolute delta phi - correcting for 2pi
double fabsDeltaEta(double eta1, double eta2); //! absolute delta eta

#endif // BTRIGUTILS_H
