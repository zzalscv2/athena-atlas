/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MMClusterOnTrackCreator.h"

#include "MuonRIO_OnTrack/MMClusterOnTrack.h"

Muon::MMClusterOnTrackCreator::MMClusterOnTrackCreator(const std::string& ty, const std::string& na, const IInterface* pa) :
    AthAlgTool(ty, na, pa) {
    // algtool interface - necessary!
    declareInterface<IMuonClusterOnTrackCreator>(this);
    declareInterface<IRIO_OnTrackCreator>(this);
}

StatusCode Muon::MMClusterOnTrackCreator::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_clusterBuilderTool.retrieve());
    ATH_CHECK(m_calibTool.retrieve());
    return StatusCode::SUCCESS;
}

const Muon::MuonClusterOnTrack* Muon::MMClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO,
                                                                                 const Amg::Vector3D& GP) const {
    MuonClusterOnTrack* MClT = nullptr;

    if (!m_idHelperSvc->isMM(RIO.identify())) {
        ATH_MSG_ERROR("MMClusterOnTrackCreator called with an non MM identifier");
        return MClT;
    }

    // check whether PrepRawData has detector element, if not there print warning
    const Trk::TrkDetElementBase* EL = RIO.detectorElement();
    if (!EL) {
        ATH_MSG_WARNING("RIO does not have associated detectorElement!, cannot produce ROT");
        return nullptr;
    }

    // MuClusterOnTrack production
    //
    // in RIO_OnTrack the local param and cov should have the same dimension
    Trk::LocalParameters locpar(RIO.localPosition());

    if (RIO.localCovariance().cols() != RIO.localCovariance().rows()) { ATH_MSG_FATAL("Rows and colums not equal!"); }

    if (RIO.localCovariance().cols() > 1) {
        ATH_MSG_VERBOSE("Making 2-dim local parameters: " << m_idHelperSvc->toString(RIO.identify()));
    } else {
        Trk::DefinedParameter radiusPar(RIO.localPosition().x(), Trk::locX);
        locpar = Trk::LocalParameters(radiusPar);
        ATH_MSG_VERBOSE("Making 1-dim local parameters: " << m_idHelperSvc->toString(RIO.identify()));
    }

    Amg::Vector2D lp{Amg::Vector2D::Zero()};
    double positionAlongStrip = 0;

    if (!EL->surface(RIO.identify()).globalToLocal(GP, GP, lp)) {
        Amg::Vector3D lpos = RIO.detectorElement()->surface(RIO.identify()).transform().inverse() * GP;
        ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
        lp[Trk::locX] = lpos.x();
        lp[Trk::locY] = lpos.y();
    }
    positionAlongStrip = lp[Trk::locY];

    Amg::MatrixX loce = RIO.localCovariance();
    ATH_MSG_DEBUG("All: new err matrix is " << loce);

    // cast to MMPrepData
    const MMPrepData* MClus = dynamic_cast<const MMPrepData*>(&RIO);
    ATH_MSG_VERBOSE("generating MMClusterOnTrack in MMClusterBuilder");
    MClT = new MMClusterOnTrack(MClus, locpar, loce, positionAlongStrip);

    return MClT;
}

const Muon::MuonClusterOnTrack* Muon::MMClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP,
                                                                                 const Amg::Vector3D&) const {
    return createRIO_OnTrack(RIO, GP);
}

const Muon::MuonClusterOnTrack* Muon::MMClusterOnTrackCreator::correct(const Trk::PrepRawData& RIO, const Trk::TrackParameters& TP) const {
    return createRIO_OnTrack(RIO, TP.position());
}

const Muon::MuonClusterOnTrack* Muon::MMClusterOnTrackCreator::calibratedCluster(const Trk::PrepRawData& RIO,
                                                                                 const Amg::Vector3D& GP,
                                                                                 const Amg::Vector3D& GD) const {
    MuonClusterOnTrack* cluster = nullptr;

    if (!m_idHelperSvc->isMM(RIO.identify())) {
        ATH_MSG_ERROR("MMClusterOnTrackCreator called with an non MM identifier");
        return cluster;
    }

    // check whether PrepRawData has detector element, if not there print warning
    const MuonGM::MMReadoutElement* mmEL = dynamic_cast<const MuonGM::MMReadoutElement*>(RIO.detectorElement());
    if (!mmEL) {
        ATH_MSG_WARNING("RIO does not have associated detectorElement!, cannot produce ROT");
        return nullptr;
    }

    // MuClusterOnTrack production
    //
    Trk::LocalParameters locpar =  RIO.localCovariance().cols() > 1 ? Trk::LocalParameters{RIO.localPosition()} : 
                                  Trk::LocalParameters{Trk::DefinedParameter{RIO.localPosition().x(), Trk::locX}};
    
    Amg::Vector2D lp{Amg::Vector2D::Zero()};
   
    if (!mmEL->surface(RIO.identify()).globalToLocal(GP, GP, lp)) {
        Amg::Vector3D lpos = RIO.detectorElement()->surface(RIO.identify()).transform().inverse() * GP;
        ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
        lp[0] = lpos[0];
        lp[1] = lpos[1];        
    }
    lp[Trk::locX] = locpar[Trk::locX];
    /// correct the local x-coordinate for the stereo angle (stereo strips only),
    /// as-built conditions and b-lines (eta and stereo strips), if enabled.
    /// note: there's no point in correcting the seeded y-coordinate.
    Amg::Vector3D localposition3D{Amg::Vector3D::Zero()};
    if (!mmEL->spacePointPosition(RIO.identify(), lp, localposition3D)){
        ATH_MSG_WARNING("Application of final as-built parameters failed for channel "<<m_idHelperSvc->toString(RIO.identify())<<" local pos = ("<<lp.x()<<"/"<<lp.y()<<").");
    }
    locpar[Trk::locX] = localposition3D.x();
    double offsetZ = localposition3D.z();
   
    /// calibrate the input
    const MMPrepData* MClus = dynamic_cast<const MMPrepData*>(&RIO);
    std::vector<NSWCalib::CalibratedStrip> calibratedStrips;
    StatusCode sc = m_calibTool->calibrateClus(Gaudi::Hive::currentContext(), MClus, GP, calibratedStrips);
    if (sc != StatusCode::SUCCESS) {
        ATH_MSG_WARNING("Could not calibrate the MM Cluster in the RIO on track creator");
        return cluster;
    }
    
     Amg::MatrixX loce = RIO.localCovariance();   
    /// calibrate the cluster position along the precision coordinate
    sc = m_clusterBuilderTool->getCalibratedClusterPosition(MClus, calibratedStrips, GD.theta(), lp, loce);
    if (sc != StatusCode::SUCCESS) {
        ATH_MSG_WARNING("Could not calibrate the MM Cluster in the RIO on track creator");
        return cluster;
    }
    /// set the value of the local parameter after the calibration
    locpar[Trk::locX] = lp[Trk::locX];

    ATH_MSG_VERBOSE("generating MMClusterOnTrack in MMClusterBuilder");
    cluster = new MMClusterOnTrack(MClus, locpar, loce, lp.y());
    cluster->setOffsetNormal(offsetZ);

    return cluster;
}
