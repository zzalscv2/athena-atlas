/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonStationGeoHelpers/MuonChamber.h>
#include <Acts/Geometry/CuboidVolumeBounds.hpp>

namespace MuonGMR4{

using ReadoutSet = MuonChamber::ReadoutSet;

MuonChamber::MuonChamber(defineArgs&& args):
    m_args{std::move(args)} {}
Muon::MuonStationIndex::ChIndex MuonChamber::chamberIndex() const { return m_args.readoutEles[0]->chamberIndex(); }
int MuonChamber::stationName() const { return m_args.readoutEles[0]->stationName(); }
int MuonChamber::stationPhi() const { return m_args.readoutEles[0]->stationPhi(); }
int MuonChamber::stationEta() const { return m_args.readoutEles[0]->stationEta(); }
const ReadoutSet& MuonChamber::readOutElements() const{ return m_args.readoutEles; }
Amg::Transform3D MuonChamber::localToGlobalTrans(const ActsGeometryContext& gctx) const {
    return m_args.readoutEles[0]->localToGlobalTrans(gctx) * m_args.centerTrans;
}
double MuonChamber::halfX() const{ return m_args.halfX; }
double MuonChamber::halfY() const{ return m_args.halfY; }
double MuonChamber::halfZ() const{ return m_args.halfZ; }

std::shared_ptr<Acts::Volume> MuonChamber::boundingVolume(const ActsGeometryContext& gctx) const {
    Acts::VolumeBoundsPtr bounds = std::make_shared<Acts::CuboidVolumeBounds>(halfX(), halfY(), halfZ());
    return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds);
}


#define CHAMBER_SORTING(a, b)                             \
    if (a.chamberIndex() != b.chamberIndex()) {           \
        return a.chamberIndex() < b.chamberIndex();       \
    }                                                     \
    if (a.stationEta() != b.stationEta()) {               \
        return a.stationEta() < b.stationEta();           \
    }                                                     \
    return a.stationPhi() < b.stationPhi();

bool operator<(const MuonReadoutElement& a, const MuonChamber& b) { CHAMBER_SORTING(a,b) }
bool operator<(const MuonChamber& a, const MuonReadoutElement& b) { CHAMBER_SORTING(a,b) }
bool operator<(const MuonChamber& a, const MuonChamber& b) { CHAMBER_SORTING(a,b) }
 
}

#undef CHAMBER_SORTING
