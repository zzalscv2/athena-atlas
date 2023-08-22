/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkFitterUtils/TrackFitInputPreparator.h"
#include "TrkGlobalChi2Fitter/GlobalChi2Fitter.h"
#include "TrkTrack/GXFMaterialEffects.h"

#include "TrkParameters/TrackParameters.h"

#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/TrapezoidBounds.h"

#include "TrkGeometry/Layer.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/MaterialLayer.h"
#include "TrkGeometry/TrackingVolume.h"

#include "TrkVolumes/Volume.h"
#include "TrkVolumes/CylinderVolumeBounds.h"

#include "TrkEventPrimitives/TransportJacobian.h"

#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkMaterialOnTrack/EstimatedBremOnTrack.h"

#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkEventPrimitives/FitQuality.h"

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkVertexOnTrack/VertexOnTrack.h"
#include "TrkSegment/TrackSegment.h"

#include "TrkExInterfaces/IPropagator.h"

#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetPrepRawData/PixelCluster.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictDetDescr/IdDictManager.h"

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "TrkEventPrimitives/unique_clone.h"

#include "LayerSort.h"
#include "TrkVolumes/VolumeBounds.h"
#include "cmath"
#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <exception>
#include <memory>

using CLHEP::MeV;
using CLHEP::mm;

namespace {
  double getDistance(const Trk::DistanceSolution& distsol) {
    if (distsol.numberOfSolutions() == 1) {
      return distsol.first();
    } if (distsol.numberOfSolutions() == 2) {
      return (
        std::abs(distsol.first()) < std::abs(distsol.second()) ? 
        distsol.first() : 
        distsol.second()
      );
    } else {
      return 0;
    }
  }
  //This function used to avoid FPE divide by zero or overflow by limiting the q/p values to a 
  //more limited range
  double
  limitInversePValue(double qOverP){
    const double magnitude = std::abs(qOverP);
    //limits found empirically to leave the 25-event q431 digest unchanged
    constexpr double maxP{100.*10e6*MeV};
    constexpr double minP{1.e-3*MeV};
    constexpr double lo {1./maxP};
    constexpr double hi {1./minP};
    double limited = std::clamp(magnitude, lo, hi);
    return std::copysign(limited, qOverP);
  }
 

  std::pair<const Trk::TrackParameters *, const Trk::TrackParameters *> getFirstLastIdPar(const Trk::Track & track) {
    const Trk::TrackParameters *firstidpar = nullptr;
    const Trk::TrackParameters *lastidpar = nullptr;

    DataVector<const Trk::TrackParameters>::const_iterator parit = track.trackParameters()->begin();

    while ((firstidpar == nullptr) && parit != track.trackParameters()->end()) {
      if (
        ((**parit).covariance() != nullptr) &&
        (**parit).associatedSurface().type() == Trk::SurfaceType::Perigee) 
      {
        firstidpar = *parit;
      }
      
      ++parit;
    }

    parit = track.trackParameters()->end();
    do {
      --parit;
      if (
        ((**parit).covariance() != nullptr) && 
        (**parit).associatedSurface().type() == Trk::SurfaceType::Perigee) 
      {
        lastidpar = *parit;
      }
    } while ((lastidpar == nullptr) && parit != track.trackParameters()->begin());
  
    return std::make_pair(firstidpar, lastidpar);
  }

  Trk::PropDirection invertPropdir(Trk::PropDirection i) {
    if (i == Trk::alongMomentum) {
      return Trk::oppositeMomentum;
    } else if (i == Trk::oppositeMomentum) {
      return Trk::alongMomentum;
    } else {
      return Trk::anyDirection;
    }
  }

 

  bool trackParametersClose(const Trk::TrackParameters & a, const Trk::TrackParameters & b, double e) {
    return (
      std::abs(a.parameters()[0] - b.parameters()[0]) < e &&
      std::abs(a.parameters()[1] - b.parameters()[1]) < e &&
      std::abs(a.parameters()[2] - b.parameters()[2]) < e
    );
  }
    
  //coerce the phi coordinate to the range -pi -> pi
  void
  coercePhiCoordinateRange(double & phi){
    const auto absPhi{std::abs(phi)};
    constexpr double twoPi{2.*M_PI};
    if (std::abs(phi - twoPi) < absPhi) {
      phi -= twoPi;
    }
    if (std::abs(phi + twoPi) < absPhi) {
      phi += twoPi;
    }
  }

#if defined(__GNUC__)
// We compile this package with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
  void
  calculateJac(Eigen::Matrix<double, 5, 5> &jac,
               Eigen::Matrix<double, 5, 5> &out,
               int jmin, int jmax) {
    out = (jac * out);

    if (jmin > 0) {
      out.block(0, 0, 4, jmin).setZero();
    }

    if (jmax < 4) {
      out.block(0, jmax + 1, 4, 5 - (jmax + 1)).setZero();
    }

    out(4, 4) = jac(4, 4);
  }

} //end of anonymous namespace 

namespace Trk {
  GlobalChi2Fitter::GlobalChi2Fitter(
    const std::string & t,
    const std::string & n,
    const IInterface * p
  ):
    base_class(t, n, p),
    m_idVolume(nullptr, std::make_unique<Trk::CylinderVolumeBounds>(560, 2750).release())
  {
  }

  StatusCode GlobalChi2Fitter::initialize() {
    ATH_CHECK(m_field_cache_key.initialize());

    if (!m_ROTcreator.name().empty()) {
      ATH_CHECK(m_ROTcreator.retrieve());
    }

    if (!m_broadROTcreator.name().empty()) { 
      ATH_CHECK(m_broadROTcreator.retrieve());
    }

    ATH_CHECK(m_updator.retrieve());
    ATH_CHECK(m_extrapolator.retrieve());
    ATH_CHECK(m_navigator.retrieve());
    ATH_CHECK(m_residualPullCalculator.retrieve());
    ATH_CHECK(m_propagator.retrieve());

    if (!m_boundaryCheckTool.name().empty()) {
      ATH_CHECK(m_boundaryCheckTool.retrieve());
    } else if (m_holeSearch.value()) {
      ATH_MSG_ERROR("Hole search requested but no boundary check tool provided.");
      return StatusCode::FAILURE;
    }

    if (m_calomat) {
      ATH_CHECK(m_calotool.retrieve());
      
      if (!m_calotoolparam.empty()) {
        ATH_CHECK(m_calotoolparam.retrieve());
      }
    } else{
      m_calotool.disable();
      m_calotoolparam.disable();
    }

    ATH_CHECK(m_scattool.retrieve());
    ATH_CHECK(m_elosstool.retrieve());
    
    if (!m_matupdator.name().empty()) {
      ATH_CHECK(m_matupdator.retrieve());
    }

    // need an Atlas id-helper to identify sub-detectors, take the one from detStore
    ATH_CHECK(detStore()->retrieve(m_DetID, "AtlasID"));

    if (m_fillderivmatrix && m_acceleration) {
      ATH_MSG_WARNING("FillDerivativeMatrix option selected, switching off acceleration!");
      m_acceleration = false;
    }

    if (!m_trackingGeometryReadKey.key().empty()){
      ATH_CHECK( m_trackingGeometryReadKey.initialize());
    }
    if (m_useCaloTG) {
      ATH_CHECK(m_caloMaterialProvider.retrieve());
      ATH_MSG_INFO(m_caloMaterialProvider << " retrieved ");
    }
    else{
      m_caloMaterialProvider.disable();
    }

    ATH_CHECK(m_clusterSplitProbContainer.initialize( !m_clusterSplitProbContainer.empty() ));

    /*
     * Doing a hole search only makes sense if we are also creating a track
     * summary, because the track summary is the only way for us to export the
     * hole search information out of the fitter. For this reason, we disable
     * the hole search in the case that track summaries are disabled.
     */
    if (m_holeSearch.value() && !m_createSummary.value()) {
      ATH_MSG_ERROR("Hole search requested but track summaries are disabled.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("fixed momentum: " << m_p);

    return StatusCode::SUCCESS;
  }

  StatusCode GlobalChi2Fitter::finalize() {

    ATH_MSG_INFO(m_fit_status[S_FITS] << " attempted track fits");
    if (m_fit_status[S_FITS] > 0) {
      ATH_MSG_INFO(m_fit_status[S_SUCCESSFUL_FITS] << " successful track fits");
      ATH_MSG_INFO(m_fit_status[S_MAT_INV_FAIL]
                   << " track fits failed because of a matrix inversion failure");
      ATH_MSG_INFO(m_fit_status[S_NOT_ENOUGH_MEAS]
                   << " tracks were rejected by the outlier logic");
      ATH_MSG_INFO(m_fit_status[S_PROPAGATION_FAIL]
                   << " track fits failed because of a propagation failure");
      ATH_MSG_INFO(m_fit_status[S_INVALID_ANGLES]
                   << " track fits failed because of an invalid angle (theta/phi)");
      ATH_MSG_INFO(m_fit_status[S_NOT_CONVERGENT]
                   << " track fits failed because the fit did not converge");
      ATH_MSG_INFO(m_fit_status[S_HIGH_CHI2]
                   << " tracks did not pass the chi^2 cut");
      ATH_MSG_INFO(m_fit_status[S_LOW_MOMENTUM]
                   << " tracks were killed by the energy loss update");
    }

    return StatusCode::SUCCESS;
  }

  // combined fit of two tracks
  // --------------------------------
  std::unique_ptr<Track>
  GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const Track& intrk1,
    const Track& intrk2,
    const RunOutlierRemoval,
    const ParticleHypothesis) const
  {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Track,Track,)");

    Cache cache(this);
    initFieldCache(ctx,cache);

    GXFTrajectory trajectory;
    if (!m_straightlineprop) {
      trajectory.m_straightline = (
        !cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn()
      );
    }
    
    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;

    bool firstismuon = isMuonTrack(intrk1);
    const Track *indettrack = firstismuon ? &intrk2 : &intrk1;
    const Track *muontrack = firstismuon ? &intrk1 : &intrk2;
    bool muonisstraight = muontrack->info().trackProperties(TrackInfo::StraightTrack);
    bool measphi = false;

    for (const auto *i : *(muontrack->measurementsOnTrack())) {
      const RIO_OnTrack *rot = nullptr;
      
      if (i->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack)) {
        const auto *const crot = static_cast<const CompetingRIOsOnTrack *>(i);
        rot = &crot->rioOnTrack(0);
      } else {
        if (i->type(Trk::MeasurementBaseType::RIO_OnTrack)){
          rot =static_cast<const RIO_OnTrack *>(i);
        }
      }
      if ((rot != nullptr) && !m_DetID->is_mdt(rot->identify())) {
        const Surface *surf = &rot->associatedSurface();
        Amg::Vector3D measdir = surf->transform().rotation().col(0);
        double dotprod1 = measdir.dot(Amg::Vector3D(0, 0, 1));
        double dotprod2 = measdir.dot(
          Amg::Vector3D(surf->center().x(), surf->center().y(), 0) /
          surf->center().perp());
        if (std::abs(dotprod1) < .5 && std::abs(dotprod2) < .5) {
          measphi = true;
          break;
        }
      }
    }

    const IPropagator *prop = &*m_propagator;
    auto [firstidpar, lastidpar] = getFirstLastIdPar(*indettrack);

    if ((firstidpar == nullptr) || (lastidpar == nullptr)) {
      return nullptr;
    }

    std::unique_ptr<const TrackParameters> parforcalo = unique_clone(firstismuon ? firstidpar : lastidpar);
    
    if (!cache.m_field_cache.solenoidOn()) {
      const AmgVector(5) & newpars = parforcalo->parameters();
      
      parforcalo=parforcalo->associatedSurface().createUniqueTrackParameters(
        newpars[0], newpars[1], newpars[2], newpars[3], 1 / 5000., std::nullopt
      );
    }

    std::vector < MaterialEffectsOnTrack > calomeots;
    if (!m_useCaloTG) {
      if (!m_calotool.empty()) {
        calomeots = m_calotool->extrapolationSurfacesAndEffects(
          *m_navigator->highestVolume(ctx), 
          *prop,
          *parforcalo,
          parforcalo->associatedSurface(),
          Trk::anyDirection,
          Trk::muon
        );
      }
    } else {
      m_caloMaterialProvider->getCaloMEOT(*indettrack, *muontrack, calomeots);
    }

    if (firstismuon) {
      std::reverse(calomeots.begin(), calomeots.end());
    }
    
    if (calomeots.empty()) {
      ATH_MSG_WARNING("No calorimeter material collected, failing fit");
      return nullptr;
    }

    std::unique_ptr<Track> track;

    bool tmp = m_calomat;
    cache.m_calomat = false;
    bool tmp2 = cache.m_extmat;
    bool tmp4 = cache.m_idmat;

    const TrackParameters *measperid = indettrack->perigeeParameters();
    const TrackParameters *measpermuon = muontrack->perigeeParameters();
    
    double qoverpid = measperid != nullptr ? measperid->parameters()[Trk::qOverP] : 0;
    double qoverpmuon = measpermuon != nullptr ? measpermuon->parameters()[Trk::qOverP] : 0;
    
    const AmgSymMatrix(5) * errmatid = measperid != nullptr ? measperid->covariance() : nullptr;
    const AmgSymMatrix(5) * errmatmuon = measpermuon != nullptr ? measpermuon->covariance() : nullptr;

    if (
      !firstismuon && 
      (errmatid != nullptr) && 
      (errmatmuon != nullptr) && 
      qoverpmuon != 0 && 
      !m_calotoolparam.empty() && 
      !m_useCaloTG
    ) {
      double piderror = std::sqrt((*errmatid) (4, 4)) / (qoverpid * qoverpid);
      double pmuonerror = std::sqrt((*errmatmuon) (4, 4)) / (qoverpmuon * qoverpmuon);
      double energyerror = std::sqrt(
        calomeots[1].energyLoss()->sigmaDeltaE() *
        calomeots[1].energyLoss()->sigmaDeltaE() + piderror * piderror +
        pmuonerror * pmuonerror
      );
      
      if (
        (std::abs(calomeots[1].energyLoss()->deltaE()) -
         std::abs(1 / qoverpid) + std::abs(1 / qoverpmuon)
        ) / energyerror > 5
      ) {
        ATH_MSG_DEBUG("Changing from measured to parametrized energy loss");
        calomeots = m_calotoolparam->extrapolationSurfacesAndEffects(
          *m_navigator->highestVolume(ctx),
          *prop, 
          *parforcalo,
          parforcalo->associatedSurface(),
          Trk::anyDirection,
          Trk::muon
        );
        
        if (calomeots.empty()) {
          ATH_MSG_WARNING("No calorimeter material collected, failing fit");
          return nullptr;
        }
      }
    }

    int nfits = cache.m_fit_status[S_FITS];
    bool firstfitwasattempted = false;

    if (cache.m_caloEntrance == nullptr) {
       const TrackingGeometry *geometry = trackingGeometry(cache,ctx);
      
      if (geometry != nullptr) {
        cache.m_caloEntrance = geometry->trackingVolume("InDet::Containers::InnerDetector");
      }
      
      if (cache.m_caloEntrance == nullptr) {
        ATH_MSG_ERROR("calo entrance not available");
        return nullptr;
      }
    }

    if (
      (!cache.m_field_cache.toroidOn() && !cache.m_field_cache.solenoidOn()) ||
      (
        cache.m_getmaterialfromtrack && 
        !muonisstraight && 
        measphi &&
        muontrack->info().trackFitter() != Trk::TrackInfo::Unknown && 
        qoverpid * qoverpmuon > 0
      )
    ) {
      track.reset(mainCombinationStrategy(ctx,cache, intrk1, intrk2, trajectory, calomeots));
      
      if (cache.m_fit_status[S_FITS] == (unsigned int) (nfits + 1)) {
        firstfitwasattempted = true;
      }
    }

    if (
      (track == nullptr) && 
      !firstfitwasattempted && 
      (cache.m_field_cache.toroidOn() || cache.m_field_cache.solenoidOn())
    ) {
      // Reset the trajectory
      GXFTrajectory trajectory2;
      trajectory2.m_straightline = trajectory.m_straightline;
      trajectory2.m_fieldprop = trajectory.m_fieldprop;
      trajectory = trajectory2;
      track.reset(backupCombinationStrategy(ctx,cache, intrk1, intrk2, trajectory, calomeots));
    }

    bool pseudoupdated = false;
    
    if (track != nullptr) {
      for (std::unique_ptr<GXFTrackState> & pseudostate : trajectory.trackStates()) { 
        if (pseudostate == nullptr) {
          continue;
        }
        
        if (
          pseudostate->measurementType() != TrackState::Pseudo ||
          !pseudostate->getStateType(TrackStateOnSurface::Measurement)
        ) {
          continue;
        }
        
        if ((pseudostate == nullptr) || pseudostate->fitQuality().chiSquared() < 10) {
          continue;
        }
        
        const TrackParameters *pseudopar = pseudostate->trackParameters();
        std::unique_ptr<const TrackParameters> updpar(m_updator->removeFromState(
          *pseudopar,
          pseudostate->measurement()->localParameters(),
          pseudostate->measurement()->localCovariance()
        ));
        
        if (updpar == nullptr) {
          continue;
        }
        
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;
        
        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(
            DefinedParameter(updpar->parameters()[Trk::locY], Trk::locY)
          ),
          covMatrix, 
          pseudopar->associatedSurface()
        );
        
        pseudostate->setMeasurement(std::move(newpseudo));
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        pseudostate->setMeasurementErrors(errors);
        pseudoupdated = true;
      }
      
      if (pseudoupdated) {
        trajectory.setConverged(false);
        cache.m_matfilled = true;
        
        track.reset(myfit(
          ctx,
          cache, 
          trajectory, 
          *track->perigeeParameters(), 
          false,
          (cache.m_field_cache.toroidOn() || cache.m_field_cache.solenoidOn()) ? muon : nonInteracting
        ));
        
        cache.m_matfilled = false;
      }
    }

    cache.m_fit_status[S_FITS] = nfits + 1;
    
    if (track != nullptr) {
      track->info().addPatternReco(intrk1.info());
      track->info().addPatternReco(intrk2.info());
      cache.incrementFitStatus(S_SUCCESSFUL_FITS);
    }
    
    cache.m_calomat = tmp;
    cache.m_extmat = tmp2;
    cache.m_idmat = tmp4;
    return track;
  }

  Track *GlobalChi2Fitter::mainCombinationStrategy(
    const EventContext& ctx,
    Cache & cache,
    const Track & intrk1,
    const Track & intrk2,
    GXFTrajectory & trajectory,
    std::vector<MaterialEffectsOnTrack> & calomeots
  ) const {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::mainCombinationStrategy");

    double mass = Trk::ParticleMasses::mass[muon];

    bool firstismuon = isMuonTrack(intrk1);
    const Track *indettrack = firstismuon ? &intrk2 : &intrk1;
    const Track *muontrack = firstismuon ? &intrk1 : &intrk2;
    
    auto [tmpfirstidpar, tmplastidpar] = getFirstLastIdPar(*indettrack);
    std::unique_ptr<const TrackParameters> firstidpar = unique_clone(tmpfirstidpar);
    std::unique_ptr<const TrackParameters> lastidpar = unique_clone(tmplastidpar);

    if ((firstidpar == nullptr) || (lastidpar == nullptr)) {
      return nullptr;
    }

    DataVector<const TrackStateOnSurface>::const_iterator tsosit =
      firstismuon ? 
      muontrack->trackStateOnSurfaces()->end() : 
      muontrack->trackStateOnSurfaces()->begin();
      
    if (firstismuon) {
     -- tsosit;
    }
    
    const MeasurementBase *closestmuonmeas = nullptr;
    std::unique_ptr<const TrackParameters> tp_closestmuon = nullptr;

    while (closestmuonmeas == nullptr) {
      closestmuonmeas = nullptr;
      const TrackParameters *thispar = (**tsosit).trackParameters();
      
      if ((**tsosit).measurementOnTrack() != nullptr) {
        closestmuonmeas = (**tsosit).measurementOnTrack();
        
        if (thispar != nullptr) {
          const AmgVector(5) & parvec = thispar->parameters();
          tp_closestmuon=thispar->associatedSurface().createUniqueTrackParameters(
            parvec[0], parvec[1], parvec[2], parvec[3], parvec[4], std::nullopt
          );
        }
        break;
      }
      
      if (firstismuon) {
        --tsosit;
      } else {
        ++tsosit;
      }
    }

    PropDirection propdir = firstismuon ? Trk::alongMomentum : oppositeMomentum;
    std::unique_ptr<const TrackParameters> tmppar;

    if (cache.m_msEntrance == nullptr) {
      const TrackingGeometry *geometry = trackingGeometry(cache,ctx);
      
      if (geometry != nullptr) {
        cache.m_msEntrance = geometry->trackingVolume("MuonSpectrometerEntrance");
      }
      
      if (cache.m_msEntrance == nullptr) {
        ATH_MSG_ERROR("MS entrance not available");
      }
    }

    if ((tp_closestmuon != nullptr) && (cache.m_msEntrance != nullptr)) {
      tmppar = m_extrapolator->extrapolateToVolume(
        ctx, *tp_closestmuon, *cache.m_msEntrance, propdir, nonInteracting);
    }

    std::unique_ptr<const std::vector<const TrackStateOnSurface *>> matvec;
    
    if (tmppar != nullptr) {
      const Surface & associatedSurface = tmppar->associatedSurface();
      std::unique_ptr<Surface> muonsurf = nullptr;
      
      if (associatedSurface.type() == Trk::SurfaceType::Cylinder) {
        if (associatedSurface.bounds().type() == Trk::SurfaceBounds::Cylinder) {
          const CylinderBounds *cylbounds = static_cast <const CylinderBounds * >(&associatedSurface.bounds());
          Amg::Transform3D trans = Amg::Transform3D(associatedSurface.transform());
          double radius = cylbounds->r();
          double hlength = cylbounds->halflengthZ();
          muonsurf = std::make_unique<CylinderSurface>(trans, radius + 1, hlength);
        }
      } else if (associatedSurface.type() == Trk::SurfaceType::Disc) {
        if (associatedSurface.bounds().type() == Trk::SurfaceBounds::Disc) {
          double newz = (
            associatedSurface.center().z() > 0 ? 
            associatedSurface.center().z() + 1 : 
            associatedSurface.center().z() - 1
          );
          
          Amg::Vector3D newpos(
            associatedSurface.center().x(), 
            associatedSurface.center().y(), 
            newz
          );
          Amg::Transform3D trans = associatedSurface.transform();
          trans.translation() << newpos;
          
          const DiscBounds *discbounds = static_cast<const DiscBounds *>(&associatedSurface.bounds());
          double rmin = discbounds->rMin();
          double rmax = discbounds->rMax();
          muonsurf = std::make_unique<DiscSurface>(trans, rmin, rmax);
        }
      }
      
      if (muonsurf != nullptr) {
        matvec.reset(m_extrapolator->extrapolateM(
          ctx,
          *tp_closestmuon, 
          *muonsurf, 
          propdir,
          false, 
          muon
        ));
      }
    }

    std::vector<const TrackStateOnSurface *> tmp_matvec;

    if ((matvec != nullptr) && !matvec->empty()) {
      tmp_matvec = *matvec;
      delete tmp_matvec.back();
      tmp_matvec.pop_back();
      
      for (auto & i : tmp_matvec) {
        propdir = firstismuon ? Trk::alongMomentum : oppositeMomentum;
        if (i->materialEffectsOnTrack()->derivedType() != MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK){
          continue;
        }
        const MaterialEffectsOnTrack *meff = static_cast<const MaterialEffectsOnTrack *>(i->materialEffectsOnTrack());
        
        const Surface *matsurf = &meff->associatedSurface();
        tmppar = m_propagator->propagateParameters(
            ctx,
            *tp_closestmuon,
            *matsurf,
            propdir,
            false,
            trajectory.m_fieldprop,
            Trk::nonInteracting
          );
        
        
        if (tmppar == nullptr) {
          propdir = !firstismuon ? Trk::alongMomentum : oppositeMomentum;
          tmppar=m_propagator->propagateParameters(
              ctx,
              *tp_closestmuon,
              *matsurf,
              propdir,
              false,
              trajectory.m_fieldprop,
              Trk::nonInteracting
            );
          
        }
        
        if (tmppar == nullptr) {
          return nullptr;
        }
        
        AmgVector(5) newpars = tmppar->parameters();
        
        if (newpars[Trk::qOverP] != 0) {
          double sign = (newpars[Trk::qOverP] > 0) ? 1 : -1;
          double de = std::abs(meff->energyLoss()->deltaE());
          double oldp = std::abs(1 / newpars[Trk::qOverP]);
          double newp2 = oldp * oldp + (!firstismuon ? 2 : -2) * de * std::sqrt(mass * mass + oldp * oldp) + de * de;
          
          if (newp2 > 0) {
            newpars[Trk::qOverP] = sign / std::sqrt(newp2);
          }
        }
        
        tp_closestmuon=tmppar->associatedSurface().createUniqueTrackParameters(
          newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
        );
      }
      
      if (!firstismuon) {
        std::reverse(tmp_matvec.begin(), tmp_matvec.end());
      }
    }

    DataVector<const TrackStateOnSurface>::const_iterator beginStates = intrk1.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator itStates = beginStates;
    DataVector<const TrackStateOnSurface>::const_iterator endState = firstismuon ? tsosit + 1 : intrk1.trackStateOnSurfaces()->end();
    DataVector<const TrackStateOnSurface>::const_iterator beginStates2 = !firstismuon ? tsosit : intrk2.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator itStates2 = beginStates2;
    DataVector<const TrackStateOnSurface>::const_iterator endState2 = intrk2.trackStateOnSurfaces()->end();

    for (; itStates != endState; ++itStates) {
      if (firstismuon && (*itStates)->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
        continue;
      }
      
      bool tmpgetmat = cache.m_getmaterialfromtrack;
      
      if ((*itStates)->materialEffectsOnTrack() != nullptr) {
        if (firstismuon) {
          cache.m_extmat = false;
        } else {
          cache.m_idmat = false;
        }
        
        const auto *const pBaseMEOT = (*itStates)->materialEffectsOnTrack();
        const bool itsAnMEOT = (pBaseMEOT->derivedType() == MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK);
        
        if (itsAnMEOT ){
          const auto *const pMEOT =static_cast<const MaterialEffectsOnTrack *>((*itStates)->materialEffectsOnTrack());
          if ((pMEOT->scatteringAngles() == nullptr) or (pMEOT->energyLoss() == nullptr)) {
            cache.m_getmaterialfromtrack = true;  // always take calorimeter layers
          }
        }
      }
      
      makeProtoState(cache, trajectory, *itStates);
      cache.m_getmaterialfromtrack = tmpgetmat;
    }
    
    if (
      !firstismuon && 
      intrk1.info().trackProperties(TrackInfo::SlimmedTrack)
    ) {
      trajectory.trackStates().back()->setTrackParameters(nullptr);
    }
    
    std::unique_ptr<const TrackParameters> firstscatpar;
    std::unique_ptr<const TrackParameters> lastscatpar;
    const TrackParameters *origlastidpar = unique_clone(lastidpar).release();
    
    double newqoverpid = 0;

    if (!firstismuon) {
      double de = std::abs(calomeots[1].energyLoss()->deltaE());
      double sigmade = std::abs(calomeots[1].energyLoss()->sigmaDeltaE());

      double pbefore = std::abs(1 / firstidpar->parameters()[Trk::qOverP]);
      double pafter = std::abs(1 / tp_closestmuon->parameters()[Trk::qOverP]);
      double elosspull = (pbefore - pafter - de) / sigmade;
      
      if (std::abs(elosspull) > 10) {
        if (elosspull > 10) {
          newqoverpid = 1 / (de + pafter + 10 * sigmade);
        } else {
          newqoverpid = 1 / (de + pafter - 10 * sigmade);
        }

        if (tp_closestmuon->parameters()[Trk::qOverP] * newqoverpid < 0) {
          newqoverpid *= -1;
        }
        
        const AmgVector(5) & newpar = firstidpar->parameters();
        firstidpar=firstidpar->associatedSurface().createUniqueTrackParameters(
          newpar[0], newpar[1], newpar[2], newpar[3], newqoverpid, std::nullopt
        );
      }

      lastidpar = m_extrapolator->extrapolateToVolume(
        ctx, *firstidpar, *cache.m_caloEntrance, alongMomentum, Trk::muon);
    }

    if (lastidpar == nullptr) {
      lastidpar = unique_clone(origlastidpar);
    }
    
    firstscatpar= m_propagator->propagateParameters(
      ctx,
      *(firstismuon ? tp_closestmuon.get() : lastidpar.get()),
      calomeots[0].associatedSurface(),
      Trk::alongMomentum, 
      false, 
      trajectory.m_fieldprop,
      Trk::nonInteracting
    );

    if (firstscatpar == nullptr) {
      return nullptr;
    }
    
    lastscatpar = m_propagator->propagateParameters(
      ctx,
      *(firstismuon ? firstidpar : tp_closestmuon),
      calomeots[2].associatedSurface(),
      Trk::oppositeMomentum, 
      false,
      trajectory.m_fieldprop,
      Trk::nonInteracting
    );

    if (lastscatpar == nullptr) {
      return nullptr;
    }

    std::unique_ptr<TransportJacobian> jac1, jac2;
    std::unique_ptr<const TrackParameters> elosspar;
    
    double firstscatphi = 0;
    double secondscatphi = 0;
    double firstscattheta = 0;
    double secondscattheta = 0;
    double muonscatphi = 0;
    double muonscattheta = 0;
    
    const TrackParameters *idscatpar = !firstismuon ? firstscatpar.get() : lastscatpar.get();
    const TrackParameters *muonscatpar = firstismuon ? firstscatpar.get() : lastscatpar.get();
    
    newqoverpid = idscatpar->parameters()[Trk::qOverP];
    
    Amg::Vector3D calosegment = lastscatpar->position() - firstscatpar->position();
    muonscatphi = calosegment.phi() - muonscatpar->parameters()[Trk::phi];
    
    if (std::abs(std::abs(muonscatphi) - 2 * M_PI) < std::abs(muonscatphi)) {
      muonscatphi += (muonscatphi < 0 ? 2 * M_PI : -2 * M_PI);
    }
    
    muonscattheta = calosegment.theta() - muonscatpar->parameters()[Trk::theta];
    std::unique_ptr<const TrackParameters> startPar = unique_clone(cache.m_idmat ? lastidpar.get() : indettrack->perigeeParameters());
   
    for (int i = 0; i < 2; i++) {
      std::unique_ptr<const TrackParameters> tmpelosspar;
      AmgVector(5) params1 = muonscatpar->parameters();
      params1[Trk::phi] += muonscatphi;
      params1[Trk::theta] += muonscattheta;

      if (!correctAngles(params1[Trk::phi], params1[Trk::theta])) {
        return nullptr;
      }
      
      std::unique_ptr<const TrackParameters> tmppar1(muonscatpar->associatedSurface().createUniqueTrackParameters(
        params1[0], params1[1], params1[2], params1[3], params1[4], std::nullopt
      ));
      
      PropDirection propdir = !firstismuon ? oppositeMomentum : alongMomentum;
      
      TransportJacobian *tmp_jac1 = jac1.get();
      tmpelosspar = m_propagator->propagateParameters(
        ctx,
        *tmppar1,
        calomeots[1].
        associatedSurface(),
        propdir, 
        false,
        trajectory.m_fieldprop,
        tmp_jac1,
        Trk::nonInteracting
      );
      if (jac1.get() != tmp_jac1) jac1.reset(tmp_jac1);
      
      if (m_numderiv) {
        jac1 = numericalDerivatives(
          ctx,
          firstscatpar.get(),
          calomeots[1].associatedSurface(), 
          propdir,
          trajectory.m_fieldprop
        );
      }

      if ((tmpelosspar == nullptr) || (jac1 == nullptr)) {
        // @TODO
        // according to coverity elosspar cannot be non NULL here
        // because elosspar is initially NULL and only set in the last loop iteration  (i==1)
        // is this intended ?
        // delete elosspar;
        return nullptr;
      }

      const AmgVector(5) & newpars = tmpelosspar->parameters();
      std::unique_ptr<const TrackParameters> elosspar2(tmpelosspar->associatedSurface().createUniqueTrackParameters(
        newpars[0], newpars[1], newpars[2], newpars[3], newqoverpid, std::nullopt
      ));
      
      if (i == 1) {
        elosspar = std::move(tmpelosspar);
      }
      
      TransportJacobian * tmp_jac2 = jac2.get();
      std::unique_ptr<const TrackParameters> scat2(m_propagator->propagateParameters(
        ctx,
        *elosspar2,
        !firstismuon ? 
        calomeots[0].associatedSurface() : 
        calomeots[2].associatedSurface(), 
        propdir, 
        false,
        trajectory.m_fieldprop,
        tmp_jac2,
        Trk::nonInteracting
      ));
      if (jac2.get() != tmp_jac2) jac2.reset(tmp_jac2);
      
      if (m_numderiv) {
        jac2 = numericalDerivatives(
          ctx,
          elosspar2.get(),
          !firstismuon ? 
          calomeots[0].associatedSurface() : 
          calomeots[2].associatedSurface(),
          !firstismuon ? 
          Trk::oppositeMomentum : 
          Trk::alongMomentum,
          trajectory.m_fieldprop
        );
      }
      
      if ((scat2 == nullptr) || (jac2 == nullptr)) {
        return nullptr;
      }
      
      double jac3[5][5];
      for (int j = 0; j < 5; j++) {
        for (int k = 0; k < 5; k++) {
          jac3[j][k] = 0;
          for (int l = 0; l < 5; l++) {
            jac3[j][k] += (*jac2) (j, l) * (*jac1) (l, k);
          }
        }
      }
      
      jac1.reset(nullptr);
      jac2.reset(nullptr);
      Amg::MatrixX jac4(2, 2);
      
      jac4(0, 0) = jac3[0][2];
      jac4(1, 1) = jac3[1][3];
      jac4(0, 1) = jac3[0][3];
      jac4(1, 0) = jac3[1][2];
      
      jac4 = jac4.inverse();
      
      double dloc1 = idscatpar->parameters()[Trk::loc1] - scat2->parameters()[Trk::loc1];
      double dloc2 = idscatpar->parameters()[Trk::loc2] - scat2->parameters()[Trk::loc2];
      const Trk::CylinderSurface * cylsurf = nullptr;
      
      if (scat2->associatedSurface().type() == Trk::SurfaceType::Cylinder) 
        cylsurf = static_cast<const Trk::CylinderSurface *>(&scat2->associatedSurface());
        
      const Trk::DiscSurface * discsurf = nullptr;
      
      if (scat2->associatedSurface().type() == Trk::SurfaceType::Cylinder)
        discsurf = static_cast<const Trk::DiscSurface *>(&scat2->associatedSurface());

      if (cylsurf != nullptr) {
        double length = 2 * M_PI * cylsurf->bounds().r();
        if (std::abs(std::abs(dloc1) - length) < std::abs(dloc1)) {
          if (dloc1 > 0) {
            dloc1 -= length;
          } else {
            dloc1 += length;
          }
        }
      }
      
      if (discsurf != nullptr) {
        if (std::abs(std::abs(dloc2) - 2 * M_PI) < std::abs(dloc2)) {
          if (dloc2 > 0) {
            dloc2 -= 2 * M_PI;
          } else {
            dloc2 += 2 * M_PI;
          }
        }
      }
      
      double dphi = jac4(0, 0) * dloc1 + jac4(0, 1) * dloc2;
      double dtheta = jac4(1, 0) * dloc1 + jac4(1, 1) * dloc2;

      if (i == 1) {
        dphi = dtheta = 0;
      }
      
      muonscatphi += dphi;
      muonscattheta += dtheta;
      
      double idscatphi = idscatpar->parameters()[Trk::phi] - (scat2->parameters()[Trk::phi] + dphi);
      
      if (std::abs(std::abs(idscatphi) - 2 * M_PI) < std::abs(idscatphi)) {
        idscatphi += ((idscatphi < 0) ? 2 * M_PI : -2 * M_PI);
      }
      
      double idscattheta = idscatpar->parameters()[Trk::theta] - (scat2->parameters()[Trk::theta] + dtheta);

      if (firstismuon) {
        firstscatphi = muonscatphi;
        secondscatphi = idscatphi;
        firstscattheta = muonscattheta;
        secondscattheta = idscattheta;
      } else {
        firstscatphi = -idscatphi;
        secondscatphi = -muonscatphi;
        firstscattheta = -idscattheta;
        secondscattheta = -muonscattheta;
      }
      
      if (i == 1 && cache.m_field_cache.toroidOn() && !firstismuon) {
        AmgVector(5) params2 = scat2->parameters();
        params2[Trk::phi] += idscatphi;
        params2[Trk::theta] += idscattheta;

        if (!correctAngles(params2[Trk::phi], params2[Trk::theta])) {
          return nullptr;
        }

        firstscatpar=scat2->associatedSurface().createUniqueTrackParameters(
          params2[0], params2[1], params2[2], params2[3], params2[4], std::nullopt
        );
        idscatpar = firstscatpar.get();

        startPar = m_extrapolator->extrapolateToVolume(ctx,
                                                       *idscatpar,
                                                       *cache.m_caloEntrance,
                                                       oppositeMomentum,
                                                       Trk::nonInteracting);

        if (startPar != nullptr) {
          Amg::Vector3D trackdir = startPar->momentum().unit();
          Amg::Vector3D curvZcrossT = -(trackdir.cross(Amg::Vector3D(0, 0, 1)));
          Amg::Vector3D curvU = curvZcrossT.unit();
          Amg::Vector3D curvV = trackdir.cross(curvU);
          Amg::RotationMatrix3D rot = Amg::RotationMatrix3D::Identity();
          
          rot.col(0) = curvU;
          rot.col(1) = curvV;
          rot.col(2) = trackdir;
          
          Amg::Transform3D trans;
          trans.linear().matrix() << rot;
          trans.translation() << startPar->position() - .1 * trackdir;
          PlaneSurface curvlinsurf(trans);
          
          const TrackParameters *curvlinpar = m_extrapolator->extrapolateDirectly(
            ctx,
            *startPar, 
            curvlinsurf,
            Trk::oppositeMomentum,
            Trk::nonInteracting != 0u
          ).release();
          
          if (curvlinpar != nullptr) {
            startPar.reset(curvlinpar);
          }
        }
        
        firstscatpar = std::move(scat2);
      }
    }

    std::unique_ptr<GXFMaterialEffects> firstscatmeff = std::make_unique<GXFMaterialEffects>(calomeots[0]);
    std::unique_ptr<GXFMaterialEffects> elossmeff = std::make_unique<GXFMaterialEffects>(calomeots[1]);
    std::unique_ptr<GXFMaterialEffects> secondscatmeff = std::make_unique<GXFMaterialEffects>(calomeots[2]);

    double pull1 = std::abs(firstscatphi / firstscatmeff->sigmaDeltaPhi());
    double pull2 = std::abs(secondscatphi / secondscatmeff->sigmaDeltaPhi());

    if (firstismuon) {
      for (auto & i : tmp_matvec) {
        makeProtoState(cache, trajectory, i, -1);
      }
    }

    firstscatmeff->setScatteringAngles(firstscatphi, firstscattheta);
    secondscatmeff->setScatteringAngles(secondscatphi, secondscattheta);

    if (!firstismuon) {
      elossmeff->setdelta_p(1000 * (lastscatpar->parameters()[Trk::qOverP] - newqoverpid));
    } else {
      elossmeff->setdelta_p(1000 * (newqoverpid - firstscatpar->parameters()[Trk::qOverP]));
    }

    elossmeff->setSigmaDeltaE(calomeots[1].energyLoss()->sigmaDeltaE());

    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(firstscatmeff), std::move(firstscatpar)), -1);
    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(elossmeff), std::move(elosspar)), -1);
    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(secondscatmeff), std::move(lastscatpar)), -1);

    if (!firstismuon) {
      for (auto & i : tmp_matvec) {
        makeProtoState(cache, trajectory, i, -1);
      }
    }

    ATH_MSG_DEBUG("pull1: " << pull1 << " pull2: " << pull2);
    
    if (startPar == nullptr) {
      return nullptr;
    }
    
    if (
      (pull1 > 5 || pull2 > 5) &&
      (pull1 > 25 || pull2 > 25 || closestmuonmeas->associatedSurface().type() == Trk::SurfaceType::Line)
    ) {
      return nullptr;
    }

    bool largegap = false;
    double previousz = 0;
    
    for (itStates2 = beginStates2; itStates2 != endState2; ++itStates2) {
      const MaterialEffectsBase *meff = (*itStates2)->materialEffectsOnTrack();
      const TrackParameters *tpar = (*itStates2)->trackParameters();
      const MeasurementBase *meas = (*itStates2)->measurementOnTrack();
      
      if (meff != nullptr) {
        if (!firstismuon) {
          const MaterialEffectsOnTrack *mefot{};
          if (meff->derivedType()  == MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK){
            mefot = static_cast<const MaterialEffectsOnTrack *>(meff);
          }
          if ( mefot and mefot->energyLoss() and 
            std::abs(mefot->energyLoss()->deltaE()) > 250 && 
            mefot->energyLoss()->sigmaDeltaE() < 1.e-9
          ) {
            return nullptr;
          }
          
          cache.m_extmat = false;
        } else {
          cache.m_idmat = false;
        }
      }
      bool ispseudo = meas->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack);
      if (
        ispseudo && 
        !(itStates2 == beginStates2 || itStates2 == beginStates2 + 1) && 
        !largegap
      ) {
        continue;
      }
      
      makeProtoState(cache, trajectory, *itStates2);

      if (
        itStates2 == endState2 - 1 && 
        tpar->associatedSurface().type() == Trk::SurfaceType::Line && 
        tpar->position().perp() > 9000 && 
        std::abs(tpar->position().z()) < 13000
      ) {
        std::unique_ptr<const TrackParameters> pseudopar(tpar->clone());
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;
        
        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(pseudopar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          pseudopar->associatedSurface()
        );
        
        std::unique_ptr<GXFTrackState> pseudostate = std::make_unique<GXFTrackState>(std::move(newpseudo), std::move(pseudopar));
        pseudostate->setMeasurementType(TrackState::Pseudo);
        
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        
        pseudostate->setMeasurementErrors(errors);
        trajectory.addMeasurementState(std::move(pseudostate));
        ispseudo = true;
        ATH_MSG_DEBUG("Adding pseudomeasurement");
      }
      
      if (
        std::abs(trajectory.trackStates().back()->position().z()) > 20000 && 
        std::abs(previousz) < 12000
      ) {
        largegap = true;
      }
      
      if (trajectory.trackStates().back()->getStateType(TrackStateOnSurface::Measurement)) {
        previousz = trajectory.trackStates().back()->position().z();
      }
    }

    Track *track = myfit(
      ctx,
      cache, 
      trajectory, 
      *startPar, 
      false,
      (cache.m_field_cache.toroidOn() || cache.m_field_cache.solenoidOn()) ?  muon : nonInteracting
    );
    
    return track;
  }

  Track *
  GlobalChi2Fitter::backupCombinationStrategy(
    const EventContext& ctx,
    Cache & cache,
    const Track & intrk1,
    const Track & intrk2,
    GXFTrajectory & trajectory,
    std::vector<MaterialEffectsOnTrack> & calomeots
  ) const {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::backupCombinationStrategy");

    bool firstismuon = false;
    const Track *indettrack = &intrk1;
    if(isMuonTrack(intrk1)) {
      firstismuon = true;
      indettrack = &intrk2;
    }

    DataVector<const TrackStateOnSurface>::const_iterator beginStates = intrk1.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator itStates = beginStates;
    DataVector<const TrackStateOnSurface>::const_iterator endState = intrk1.trackStateOnSurfaces()->end();
    DataVector<const TrackStateOnSurface>::const_iterator beginStates2 = intrk2.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator itStates2 = beginStates2;
    DataVector<const TrackStateOnSurface>::const_iterator endState2 = intrk2.trackStateOnSurfaces()->end();

    const TrackParameters *firstidpar = nullptr;
    const auto *const pParametersVector = indettrack->trackParameters();
    // Dont understand why the second track parameters are taken
    // Is it assumed the ID track is slimmed?
    if (pParametersVector->size() > 1)
      firstidpar = (*pParametersVector)[1];
    else
      firstidpar = pParametersVector->back();

    std::unique_ptr<const TrackParameters> lastidpar = nullptr;
    if ((firstidpar != nullptr) && (cache.m_caloEntrance != nullptr))
      lastidpar = m_extrapolator->extrapolateToVolume(
        ctx, *firstidpar, *cache.m_caloEntrance, alongMomentum, Trk::muon);

    if (lastidpar == nullptr) {
      lastidpar.reset(pParametersVector->back()->clone());
    }
    
    std::unique_ptr < const TrackParameters > firstscatpar;
    std::unique_ptr < const TrackParameters > lastscatpar;
    std::unique_ptr < const TrackParameters > elosspar;

    double charge = (lastidpar->parameters()[Trk::qOverP] < 0) ? -1 : 1;

    Perigee startper(
      lastidpar->position(), 
      lastidpar->momentum(), 
      charge,
      lastidpar->position()
    );

    if (!firstismuon) {
      firstscatpar = m_propagator->propagateParameters(
        ctx,
        *lastidpar,
        calomeots[0].associatedSurface(),
        Trk::alongMomentum,
        false,
        trajectory.m_fieldprop,
        Trk::nonInteracting);

      if (!firstscatpar) {
        return nullptr;
      }

      std::unique_ptr<const TrackParameters> tmppar(
        m_propagator->propagateParameters(
          ctx,
          *firstscatpar,
          calomeots[1].associatedSurface(),
          Trk::alongMomentum,
          false,
          trajectory.m_fieldprop,
          Trk::nonInteracting));

      if (!tmppar) {
        return nullptr;
      }
      
      double sign = (tmppar->parameters()[Trk::qOverP] < 0) ? -1 : 1;
      double mass = Trk::ParticleMasses::mass[muon];

      double oldp = std::abs(1 / tmppar->parameters()[Trk::qOverP]);
      double de = std::abs(calomeots[1].energyLoss()->deltaE());

      double newp2 = oldp * oldp - 2 * de * std::sqrt(mass * mass + oldp * oldp) + de * de;
      
      if (newp2 < 4.e6) {
        newp2 = 4.e6;
      }
      
      double newqoverp = sign / std::sqrt(newp2);

      const AmgVector(5) & pars = tmppar->parameters();
      
      elosspar=
        tmppar->associatedSurface().createUniqueTrackParameters(
          pars[0], pars[1], pars[2], pars[3], newqoverp, std::nullopt
        );

      lastscatpar = m_propagator->propagateParameters(
        ctx,
        *elosspar,
        calomeots[2].associatedSurface(),
        Trk::alongMomentum,
        false,
        trajectory.m_fieldprop,
        Trk::nonInteracting);

      if (!lastscatpar) {
        return nullptr;
      }
    } else {
      lastscatpar = m_propagator->propagateParameters(
          ctx,
          *firstidpar,
          calomeots[2].associatedSurface(),
          Trk::oppositeMomentum, 
          false,
          trajectory.m_fieldprop,
          Trk::nonInteracting
      );
      
      if (!lastscatpar) {
        return nullptr;
      }
      
      elosspar=  m_propagator->propagateParameters(
          ctx,
          *lastscatpar,
          calomeots[1].associatedSurface(),
          Trk::oppositeMomentum, 
          false,
          trajectory.m_fieldprop,
          Trk::nonInteracting
      );
      
      if (!elosspar) {
        return nullptr;
      }
      
      double sign = (elosspar->parameters()[Trk::qOverP] < 0) ? -1 : 1;
      double newqoverp = sign /
        (1. / std::abs(elosspar->parameters()[Trk::qOverP]) +
         std::abs(calomeots[1].energyLoss()->deltaE()));

      const AmgVector(5) & pars = elosspar->parameters();

      std::unique_ptr<const TrackParameters>tmppar(
        elosspar->associatedSurface().createUniqueTrackParameters(
          pars[0], pars[1], pars[2], pars[3], newqoverp, std::nullopt
        )
      );
      
      firstscatpar = m_propagator->propagateParameters(
          ctx,
          *tmppar,
          calomeots[0].associatedSurface(),
          Trk::oppositeMomentum, 
          false,
          trajectory.m_fieldprop,
          Trk::nonInteracting
      );
      
      if (!firstscatpar) {
        return nullptr;
      }
    }

    for (; itStates != endState; ++itStates) {
      if (
        firstismuon && 
        (*itStates)->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack)
      ) {
        continue;
      }

      if ((*itStates)->materialEffectsOnTrack() != nullptr) {
        if (!firstismuon) {
          cache.m_idmat = false;
        } else {
          continue;
        }
      }
      
      if (firstismuon) {
        makeProtoState(cache, trajectory, *itStates);
      }
    }

    std::unique_ptr<GXFMaterialEffects> firstscatmeff = std::make_unique<GXFMaterialEffects>(calomeots[0]);
    std::unique_ptr<GXFMaterialEffects> elossmeff = std::make_unique<GXFMaterialEffects>(calomeots[1]);
    std::unique_ptr<GXFMaterialEffects> secondscatmeff = std::make_unique<GXFMaterialEffects>(calomeots[2]);

    double dp = 0;
    double sigmadp = 0;
    sigmadp = calomeots[1].energyLoss()->sigmaDeltaE();
    elossmeff->setSigmaDeltaE(sigmadp);

    dp = 1000 * (lastscatpar->parameters()[Trk::qOverP] - firstscatpar->parameters()[Trk::qOverP]);
    elossmeff->setdelta_p(dp);
    
    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(firstscatmeff), std::move(firstscatpar)), -1);
    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(elossmeff), std::move(elosspar)), -1);
    trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(secondscatmeff), std::move(lastscatpar)), -1);
    
    GXFTrackState *secondscatstate = trajectory.trackStates().back().get();
    const Surface *triggersurf1 = nullptr;
    const Surface *triggersurf2 = nullptr;
    Amg::Vector3D triggerpos1(0, 0, 0);
    Amg::Vector3D triggerpos2(0, 0, 0);

    bool seenmdt = false;
    bool mdtbetweenphihits = false;
    int nphi = 0;
    
    for (
      itStates2 = (!firstismuon ? beginStates2 : endState - 1);
      itStates2 != (!firstismuon ? endState2 : beginStates - 1);
      (!firstismuon ? ++itStates2 : --itStates2)
    ) {
      if (
        ((*itStates2)->measurementOnTrack() == nullptr) || 
        (*itStates2)->type(TrackStateOnSurface::Outlier)
      ) {
        continue;
      }
      const auto *const pMeasurement = (*itStates2)->measurementOnTrack();
      const Surface *surf = &pMeasurement->associatedSurface();
      bool isCompetingRIOsOnTrack = pMeasurement->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack);
      const RIO_OnTrack *rot = nullptr;
      
      if (isCompetingRIOsOnTrack) {
        const auto *const crot =  static_cast<const CompetingRIOsOnTrack *>(pMeasurement);
        rot = &crot->rioOnTrack(0);
      } else {
        if (pMeasurement->type(Trk::MeasurementBaseType::RIO_OnTrack)){
          rot = static_cast<const RIO_OnTrack *>(pMeasurement);
        }
      }
      if ((rot != nullptr) && m_DetID->is_mdt(rot->identify()) && (triggersurf1 != nullptr)) {
        seenmdt = true;
      }
      if (
        (rot != nullptr) && (
          m_DetID->is_tgc(rot->identify()) || 
          m_DetID->is_rpc(rot->identify()) ||
          m_DetID->is_stgc(rot->identify())
        )
      ) {
        bool measphi = true;
        Amg::Vector3D measdir = surf->transform().rotation().col(0);
        double dotprod1 = measdir.dot(Amg::Vector3D(0, 0, 1));
        double dotprod2 = measdir.dot(Amg::Vector3D(surf->center().x(), surf->center().y(), 0) / surf->center().perp());
        if (std::abs(dotprod1) > .5 || std::abs(dotprod2) > .5) {
          measphi = false;
        }
        if (measphi) {
          nphi++;
          Amg::Vector3D thispos =
            (*itStates2)->trackParameters() != nullptr ? 
            (*itStates2)->trackParameters()->position() : 
            rot->globalPosition();
          if (triggersurf1 != nullptr) {
            triggerpos2 = thispos;
            triggersurf2 = surf;
            if (seenmdt) {
              mdtbetweenphihits = true;
            }
          } else {
            triggerpos1 = thispos;
            triggersurf1 = surf;
          }
        }
      }
    }

    double mdttrig1 = 999999;
    double mdttrig2 = 999999;
    const Surface *mdtsurf1 = nullptr;
    const Surface *mdtsurf2 = nullptr;

    for (
      itStates2 = (!firstismuon ? beginStates2 : endState - 1);
      itStates2 != (!firstismuon ? endState2 : beginStates - 1);
      (!firstismuon ? ++itStates2 : --itStates2)
    ) {
      const Surface *surf = nullptr;
      if (
        ((*itStates2)->measurementOnTrack() != nullptr) && 
        !(*itStates2)->type(TrackStateOnSurface::Outlier)
      ) {
        surf = &(*itStates2)->measurementOnTrack()->associatedSurface();
      }
      
      if (surf == nullptr) {
        continue;
      }
      const auto *const pThisMeasurement = (*itStates2)->measurementOnTrack();
      bool isCompetingRioOnTrack = pThisMeasurement->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack);
      const RIO_OnTrack *rot = nullptr;
      
      if (isCompetingRioOnTrack) {
        const auto *crot = static_cast<const CompetingRIOsOnTrack *>(pThisMeasurement);
        rot = &crot->rioOnTrack(0);
      } else {
        if (pThisMeasurement->type(Trk::MeasurementBaseType::RIO_OnTrack)){
          rot = static_cast<const RIO_OnTrack *>(pThisMeasurement);
        }
      }
      const bool thisismdt = rot and m_DetID->is_mdt(rot->identify());
      if (thisismdt) {
        Amg::Vector3D globpos =
          (*itStates2)->trackParameters() != nullptr ? 
          (*itStates2)->trackParameters()->position() : 
          pThisMeasurement->globalPosition();
        if (triggerpos1.mag() > 1 && (globpos - triggerpos1).mag() < mdttrig1) {
          mdttrig1 = (globpos - triggerpos1).mag();
          mdtsurf1 = surf;
        }
        if (triggerpos2.mag() > 1 && (globpos - triggerpos2).mag() < mdttrig2) {
          mdttrig2 = (globpos - triggerpos2).mag();
          mdtsurf2 = surf;
        }
      }
    }

    GXFTrackState * firstpseudostate = nullptr;
    std::vector<GXFTrackState *> outlierstates;
    std::vector<GXFTrackState *> outlierstates2;
    
    outlierstates.reserve(10);
    outlierstates2.reserve(10);
    
    std::unique_ptr<PseudoMeasurementOnTrack> newpseudo;
    
    for (itStates2 = beginStates2; itStates2 != endState2; ++itStates2) {
      const auto *const pMeasurement{(*itStates2)->measurementOnTrack()};
      bool isPseudo = pMeasurement->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack);
      bool isStraightLine = 
        pMeasurement != nullptr ? 
        pMeasurement->associatedSurface().type() == Trk::SurfaceType::Line : 
        false;

      if (
        isStraightLine && 
        !firstismuon && 
        (newpseudo == nullptr) && (
          (itStates2 == beginStates2 || itStates2 == beginStates2 + 1) &&
          std::abs(pMeasurement->globalPosition().z()) < 10000
        )
      ) {
        std::unique_ptr<const TrackParameters> par2;
        if (((*itStates2)->trackParameters() != nullptr) && nphi > 99) {
          par2.reset((*itStates2)->trackParameters()->clone());
        } else {
          par2 = m_propagator->propagateParameters(
              ctx,
              *secondscatstate->trackParameters(),
              pMeasurement->associatedSurface(),
              alongMomentum, 
              false,
              trajectory.m_fieldprop,
              Trk::nonInteracting
          );
        }
        if (par2 == nullptr) {
          continue;
        }
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;
        
        newpseudo = std::make_unique<PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(par2->parameters()[Trk::locY], Trk::locY)), 
          covMatrix, 
          par2->associatedSurface()
        );
        
        std::unique_ptr<GXFTrackState> firstpseudo = std::make_unique<GXFTrackState>(std::move(newpseudo), std::move(par2));
        firstpseudo->setMeasurementType(TrackState::Pseudo);
        
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        
        firstpseudo->setMeasurementErrors(errors);
        firstpseudostate = firstpseudo.get();
        trajectory.addMeasurementState(std::move(firstpseudo));
        ATH_MSG_DEBUG("Adding PseudoMeasurement");
        continue;
      }
      
      if (isPseudo && !firstismuon) {
        continue;
      }
      
      if ((**itStates2).materialEffectsOnTrack() != nullptr) {
        if (firstismuon) {
          cache.m_idmat = false;
        } else {
          continue;
        }
      }

      if (!firstismuon) {
        if (
          ((**itStates2).measurementOnTrack() != nullptr) &&
          &(**itStates2).measurementOnTrack()->associatedSurface() == triggersurf1 &&
          (mdtsurf1 != nullptr)
        ) {
          std::unique_ptr<Amg::Transform3D> transf = std::make_unique<Amg::Transform3D>(mdtsurf1->transform());
          
          transf->translation() << triggerpos1;
          StraightLineSurface slsurf(*transf);
          Amg::MatrixX covMatrix(1, 1);
          covMatrix(0, 0) = 100;

          std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
            LocalParameters(DefinedParameter(0, Trk::locY)), covMatrix, slsurf
          );
          
          std::unique_ptr<GXFTrackState> pseudostate1 = std::make_unique<GXFTrackState>(std::move(newpseudo), nullptr);
          pseudostate1->setMeasurementType(TrackState::Pseudo);
          
          double errors[5];
          errors[0] = errors[2] = errors[3] = errors[4] = -1;
          errors[1] = 10;
          
          pseudostate1->setMeasurementErrors(errors);
          outlierstates2.push_back(pseudostate1.get());
          trajectory.addMeasurementState(std::move(pseudostate1));
        }
        
        if (
          ((**itStates2).measurementOnTrack() != nullptr) &&
          &(**itStates2).measurementOnTrack()->associatedSurface() == triggersurf2 && 
          mdtbetweenphihits && 
          (mdtsurf2 != nullptr)
        ) {
          std::unique_ptr<Amg::Transform3D> transf = std::make_unique<Amg::Transform3D>(mdtsurf2->transform());
          transf->translation() << triggerpos2;
          StraightLineSurface slsurf(*transf);
          Amg::MatrixX covMatrix(1, 1);
          covMatrix(0, 0) = 100;

          std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
            LocalParameters(DefinedParameter(0, Trk::locY)), covMatrix, slsurf
          );
          
          std::unique_ptr<GXFTrackState> pseudostate2 = std::make_unique<GXFTrackState>(std::move(newpseudo), nullptr);
          pseudostate2->setMeasurementType(TrackState::Pseudo);
          
          double errors[5];
          errors[0] = errors[2] = errors[3] = errors[4] = -1;
          errors[1] = 10;
          
          pseudostate2->setMeasurementErrors(errors);
          // cppcheck-suppress invalidLifetime; false positive
          outlierstates2.push_back(pseudostate2.get());
          trajectory.addMeasurementState(std::move(pseudostate2));
        }
        
        makeProtoState(cache, trajectory, *itStates2);

        if (
          (
            trajectory.trackStates().back()->measurementType() == TrackState::TGC ||
            (
              trajectory.trackStates().back()->measurementType() == TrackState::RPC && 
              trajectory.trackStates().back()->measuresPhi()
            )
          ) && 
          trajectory.trackStates().back()->getStateType(TrackStateOnSurface::Measurement)
        ) {
          outlierstates.push_back(trajectory.trackStates().back().get());
          trajectory.setOutlier((int) trajectory.trackStates().size() - 1, true);
        }
      }
    }

    trajectory.setNumberOfPerigeeParameters(0);

    Track *track = nullptr;

    trajectory.setPrefit(2);
    const TrackParameters *startpar2 = &startper;
    cache.m_matfilled = true;
    bool tmpacc = cache.m_acceleration;
    cache.m_acceleration = false;
    // @TODO eventually track created but not used why ?
    std::unique_ptr<Trk::Track> tmp_track(
      myfit(ctx, cache, trajectory, *startpar2, false, muon));
    cache.m_acceleration = tmpacc;

    cache.m_matfilled = false;
    if (
      !firstismuon && 
      trajectory.converged() &&
      std::abs(trajectory.residuals().tail<1>()(0) / trajectory.errors().tail<1>()(0)) > 10
    ) {
      return nullptr;
    }

    if (trajectory.converged()) {
      if (firstpseudostate != nullptr) {
        const TrackParameters *par2 = firstpseudostate->trackParameters();
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;

        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(par2->parameters()[Trk::locY], Trk::locY)), 
          covMatrix, 
          par2->associatedSurface()
        );
        firstpseudostate->setMeasurement(std::move(newpseudo));
        firstpseudostate->setRecalibrated(false);
      }

      for (int j = 0; j < (int) trajectory.trackStates().size(); j++) {
        for (const auto & i : outlierstates2) {
          if (trajectory.trackStates()[j].get() == i) {
            trajectory.setOutlier(j, true);
          }
        }
        
        for (const auto & i : outlierstates) {
          if (trajectory.trackStates()[j].get() == i) {
            trajectory.setOutlier(j, false);
          }
        }
      }
      
      for (
        itStates = (firstismuon ? beginStates2 : endState - 1);
        itStates != (firstismuon ? endState2 : beginStates - 1);
        (firstismuon ? ++itStates : --itStates)
      ) {
        if ((*itStates)->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
          continue;
        }
        
        makeProtoState(cache, trajectory, *itStates, (firstismuon ? -1 : 0));
      }
      
      trajectory.reset();
      trajectory.setPrefit(0);
      trajectory.setNumberOfPerigeeParameters(5);
      track = myfit(ctx, cache, trajectory, *firstidpar, false, muon);
      cache.m_matfilled = false;
    }
    
    return track;
  }

  std::unique_ptr<Track>
  GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const Track& inputTrack,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects) const
  {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Track,)");
    
    Cache cache(this);
    initFieldCache(ctx,cache);

    GXFTrajectory trajectory;
    
    if (!m_straightlineprop) {
      trajectory.m_straightline = (!cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn());
    }

    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;

    return std::unique_ptr<Track>(
      fitIm(ctx, cache, inputTrack, runOutlier, matEffects));
  }

  Track *
  GlobalChi2Fitter::alignmentFit(AlignmentCache& alignCache,
                        const Track &inputTrack,
                        const RunOutlierRemoval runOutlier,
                        const ParticleHypothesis matEffects) const {


    const EventContext& ctx = Gaudi::Hive::currentContext();
    Cache cache(this);
    initFieldCache(ctx, cache);

    delete alignCache.m_derivMatrix;
    alignCache.m_derivMatrix = nullptr;

    delete alignCache.m_fullCovarianceMatrix;
    alignCache.m_fullCovarianceMatrix = nullptr;
    alignCache.m_iterationsOfLastFit = 0;

    Trk::Track* newTrack =
      fitIm(ctx, cache, inputTrack, runOutlier, matEffects);
    if(newTrack != nullptr){
      if(cache.m_derivmat.size() != 0)
        alignCache.m_derivMatrix = new Amg::MatrixX(cache.m_derivmat);
      if(cache.m_fullcovmat.size() != 0)
        alignCache.m_fullCovarianceMatrix = new Amg::MatrixX(cache.m_fullcovmat);
      alignCache.m_iterationsOfLastFit = cache.m_lastiter;
    }
    return newTrack;
  }

  Track*
  GlobalChi2Fitter::fitIm(
    const EventContext& ctx,
    Cache& cache,
    const Track& inputTrack,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects) const
  {

    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Track,,)");

    GXFTrajectory trajectory;
    
    if (!m_straightlineprop) {
      trajectory.m_straightline = (!cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn());
    }
    
    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;

    if (inputTrack.trackStateOnSurfaces()->empty()) {
      ATH_MSG_WARNING("Track with zero track states, cannot perform fit");
      return nullptr;
    }
    
    if (inputTrack.trackParameters()->empty()) {
      ATH_MSG_WARNING("Track without track parameters, cannot perform fit");
      return nullptr;
    }
    
    std::unique_ptr<const TrackParameters> minpar = unique_clone(inputTrack.perigeeParameters());
    const TrackParameters *firstidpar = nullptr;
    const TrackParameters *lastidpar = nullptr;
    
    if (minpar == nullptr) {
      minpar = unique_clone(*(inputTrack.trackParameters()->begin()));
    }
    
    bool tmpgetmat = cache.m_getmaterialfromtrack;
    
    if (
      matEffects == Trk::nonInteracting || 
      inputTrack.info().trackFitter() == TrackInfo::Unknown
    ) {
      cache.m_getmaterialfromtrack = false;
    }
    
    DataVector<const TrackStateOnSurface>::const_iterator itStates = inputTrack.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator endState = inputTrack.trackStateOnSurfaces()->end();

    trajectory.trackStates().reserve(inputTrack.trackStateOnSurfaces()->size());
                                     
    const Surface *firsthitsurf = nullptr;
    const Surface *lasthitsurf = nullptr;
    bool hasid = false;
    bool hasmuon = false;
    bool iscombined = false;
    bool seenphimeas = false;
    bool phiem = false;
    bool phibo = false;
    
    for (; itStates != endState; ++itStates) {
      const auto *const pMeasurement = (**itStates).measurementOnTrack();
      if (
        (pMeasurement == nullptr) && 
        ((**itStates).materialEffectsOnTrack() != nullptr) && 
        matEffects != inputTrack.info().particleHypothesis()
      ) {
        continue;
      }

      if (pMeasurement != nullptr) {
        const Surface *surf = &pMeasurement->associatedSurface();
        Identifier hitid = surf->associatedDetectorElementIdentifier();
        if (!hitid.is_valid()) {
          if (pMeasurement->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack)) {
            const CompetingRIOsOnTrack *crot = static_cast<const CompetingRIOsOnTrack *>(pMeasurement);
            hitid = crot->rioOnTrack(0).identify();
          }
        }
        if (hitid.is_valid()) {
          if (firsthitsurf == nullptr) {
            firsthitsurf = surf;
          }
          lasthitsurf = surf;
          if (m_DetID->is_indet(hitid)) {
            hasid = true;
            if (hasmuon) {
              iscombined = true;
            }
            if ((**itStates).trackParameters() != nullptr) {
              lastidpar = (**itStates).trackParameters();
              if (firstidpar == nullptr) {
                firstidpar = lastidpar;
              }
            }
          } else {
            if (!(**itStates).type(TrackStateOnSurface::Outlier)) {
              Amg::Vector3D measdir = surf->transform().rotation().col(0);
              double dotprod1 = measdir.dot(Amg::Vector3D(0, 0, 1));
              double dotprod2 = measdir.dot(Amg::Vector3D(surf->center().x(), surf->center().y(), 0) / surf->center().perp());
              if (std::abs(dotprod1) < .5 && std::abs(dotprod2) < .5) {
                seenphimeas = true;
                if (std::abs(surf->center().z()) > 13000) {
                  phiem = true;
                }
                if (surf->center().perp() > 9000 && std::abs(surf->center().z()) < 13000) {
                  phibo = true;
                }
              }
            }
            hasmuon = true;
            if (hasid) {
              iscombined = true;
            }
          }
        }
        
        if (iscombined && seenphimeas && (phiem || phibo)) {
          if (pMeasurement->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
            continue;
          }
        }
      }
      makeProtoState(cache, trajectory, *itStates);
    }

    if (
      cache.m_getmaterialfromtrack && trajectory.numberOfScatterers() != 0 && 
      (hasmuon || cache.m_acceleration)
    ) {
      cache.m_matfilled = true;
    }

    if (firstidpar == lastidpar) {
      firstidpar = lastidpar = nullptr;
    }
    
    if (
      iscombined && 
      !cache.m_matfilled &&
      (
        m_DetID->is_indet(firsthitsurf->associatedDetectorElementIdentifier()) !=
        m_DetID->is_indet(lasthitsurf->associatedDetectorElementIdentifier())
      ) && 
      (firstidpar != nullptr)
    ) {
      if (m_DetID->is_indet(firsthitsurf->associatedDetectorElementIdentifier())) {
        minpar = unique_clone(lastidpar);
      } else {
        minpar = unique_clone(firstidpar);
      }
    }
    
    bool tmpacc = cache.m_acceleration;
    bool tmpfiteloss = m_fiteloss;
    bool tmpsirecal = cache.m_sirecal;
    std::unique_ptr<Track> tmptrack = nullptr;
    
    if (matEffects == Trk::proton || matEffects == Trk::kaon || matEffects == Trk::electron) {
      ATH_MSG_DEBUG("call myfit(GXFTrajectory,TP,,)");
      cache.m_fiteloss = true;
      cache.m_sirecal = false;
      
      if (matEffects == Trk::electron) {
        cache.m_asymeloss = true;
      }

      tmptrack.reset(myfit(ctx, cache, trajectory, *minpar, false, matEffects));
      cache.m_sirecal = tmpsirecal;
      
      if (tmptrack == nullptr) {
        cache.m_matfilled = false;
        cache.m_getmaterialfromtrack = tmpgetmat;
        cache.m_acceleration = tmpacc;
        cache.m_fiteloss = tmpfiteloss;
        return nullptr;
      }
      
      int nscats = 0;
      bool isbrem = false;
      double bremdp = 0;
      unsigned int n_brem=0;
      
      for (std::unique_ptr<GXFTrackState> & state : trajectory.trackStates()) {
        GXFMaterialEffects *meff = state->materialEffects();
        
        if (meff != nullptr) {
          nscats++;
          
          const TrackParameters *layerpars = state->trackParameters();
          const MaterialProperties *matprop = meff->materialProperties();
          
          double p = 1. / std::abs(layerpars->parameters()[Trk::qOverP] - .0005 * meff->delta_p());
          
          std::optional<Amg::Vector2D> locpos(state->associatedSurface().globalToLocal(layerpars->position()));
          const Amg::Vector3D layerNormal(state->associatedSurface().normal(*locpos));
          double costracksurf = 1.;
          
          costracksurf = std::abs(layerNormal.dot(layerpars->momentum().unit()));
          
          double oldde = meff->deltaE();
          
          std::unique_ptr<EnergyLoss> eloss;
          double sigmascat = 0;
          
          if (matprop != nullptr) {
            eloss = std::make_unique<EnergyLoss>(
                m_elosstool->energyLoss(*matprop, p, 1. / costracksurf,
                                        Trk::alongMomentum, matEffects));
            sigmascat = std::sqrt(m_scattool->sigmaSquare(*matprop, p, 1. / costracksurf, matEffects));
            
            if (eloss != nullptr) {
              meff->setDeltaE(eloss->deltaE());
            }
          } else {
            MaterialProperties tmpprop(1., meff->x0(), 0., 0., 0., 0.);
            sigmascat = std::sqrt(m_scattool->sigmaSquare(tmpprop, p, 1. / costracksurf, matEffects));
          }
          
          meff->setScatteringSigmas(
            sigmascat / std::sin(layerpars->parameters()[Trk::theta]),
            sigmascat
          );


          if (matEffects == electron) {
            state->resetStateType(TrackStateOnSurface::Scatterer);
            meff->setDeltaE(oldde);
            if (!meff->isKink()) {
              meff->setSigmaDeltaE(0);
            } else {
              isbrem = true;
              bremdp = meff->delta_p();
            }
          } else if (eloss != nullptr) {
            meff->setSigmaDeltaE(eloss->sigmaDeltaE());
          }
          if (meff->sigmaDeltaE() > 0) {
             ++n_brem;
          }
        }
      }
      
      const AmgVector(5) & refpars = trajectory.referenceParameters()->parameters();
      minpar=trajectory.referenceParameters()->associatedSurface().createUniqueTrackParameters(
        refpars[0], refpars[1], refpars[2], refpars[3], refpars[4], std::nullopt
      );
      
      trajectory.reset();
      cache.m_matfilled = true;
      
      if (matEffects == Trk::electron) {
        if (!isbrem) {
          trajectory.brems().clear();
        } else {
          trajectory.brems().resize(1);
          trajectory.brems()[0] = bremdp;
        }

        cache.m_asymeloss = false;
        trajectory.setNumberOfScatterers(nscats);
        // @TODO fillResiduals assumes that numberOfBrems == number of states with material effects and sigmaDeltaE() > 0
        //       not clear whether fillResiduals has to be adjusted for electrons rather than this
        trajectory.setNumberOfBrems(n_brem);
      }
    }

    std::unique_ptr<Track> track(myfit(ctx, cache, trajectory, *minpar, runOutlier, matEffects));
    
    bool pseudoupdated = false;
    
    if ((track != nullptr) && hasid && hasmuon) {
      for (std::unique_ptr<GXFTrackState> & pseudostate : trajectory.trackStates()) {
        if (
          (pseudostate == nullptr) || 
          pseudostate->measurementType() != TrackState::Pseudo || 
          pseudostate->fitQuality().chiSquared() < 10
        ) {
          continue;
        }
        
        const TrackParameters *pseudopar = pseudostate->trackParameters();
        std::unique_ptr<const TrackParameters> updpar(m_updator->removeFromState(
          *pseudopar,
          pseudostate->measurement()->localParameters(),
          pseudostate->measurement()->localCovariance()
        ));
        
        if (updpar == nullptr) {
          continue;
        }
        
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;
        
        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(updpar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          pseudopar->associatedSurface()
        );
        
        pseudostate->setMeasurement(std::move(newpseudo));
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        pseudostate->setMeasurementErrors(errors);
        pseudoupdated = true;
      }
      
      if (pseudoupdated) {
        trajectory.setConverged(false);
        cache.m_matfilled = true;
        track.reset(myfit(ctx, cache, trajectory, *track->perigeeParameters(), false, muon));
        cache.m_matfilled = false;
      }
    }

    cache.m_matfilled = false;
    cache.m_getmaterialfromtrack = tmpgetmat;
    cache.m_acceleration = tmpacc;
    cache.m_fiteloss = tmpfiteloss;

    if (track != nullptr) {
      cache.incrementFitStatus(S_SUCCESSFUL_FITS);
      const TrackInfo& old_info = inputTrack.info();
      track->info().addPatternReco(old_info);
    }

    return track.release();
  }

  std::unique_ptr<Track>
  GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const PrepRawDataSet& prds,
    const TrackParameters& param,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects) const
  {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(PRDS,TP,)");
    MeasurementSet rots;

    for (const auto *prd : prds) {
      const Surface & prdsurf = (*prd).detectorElement()->surface((*prd).identify());
      const RIO_OnTrack *rot = nullptr;
      const PlaneSurface *plsurf = nullptr;
      
      if (prdsurf.type() == Trk::SurfaceType::Plane)
        plsurf = static_cast < const PlaneSurface *>(&prdsurf);

      const StraightLineSurface *slsurf = nullptr;
      
      if (prdsurf.type() == Trk::SurfaceType::Line)
        slsurf = static_cast < const StraightLineSurface *>(&prdsurf);

      if ((slsurf == nullptr) && (plsurf == nullptr)) {
        ATH_MSG_ERROR("Surface is neither PlaneSurface nor StraightLineSurface!");
      }
      
      if (!m_broadROTcreator.empty() && (slsurf != nullptr)) {
        rot = m_broadROTcreator->correct(*prd, param);
      } else if (slsurf != nullptr) {
        AtaStraightLine atasl(
          slsurf->center(), 
          param.parameters()[Trk::phi],
          param.parameters()[Trk::theta],
          param.parameters()[Trk::qOverP], 
          *slsurf
        );
        rot = m_ROTcreator->correct(*prd, atasl);
      } else if (plsurf != nullptr) {
        if (param.covariance() != nullptr) {
          AtaPlane atapl(
            plsurf->center(), 
            param.parameters()[Trk::phi],
            param.parameters()[Trk::theta],
            param.parameters()[Trk::qOverP], 
            *plsurf,
            AmgSymMatrix(5)(*param.covariance())
          );
          rot = m_ROTcreator->correct(*prd, atapl);
        } else {
          AtaPlane atapl(
            plsurf->center(), 
            param.parameters()[Trk::phi],
            param.parameters()[Trk::theta],
            param.parameters()[Trk::qOverP], 
            *plsurf
          );
          rot = m_ROTcreator->correct(*prd, atapl);
        }
      }
      
      if (rot != nullptr) {
        rots.push_back(rot);
      }
    }

    std::unique_ptr<Track> track =
      fit(ctx, rots, param, runOutlier, matEffects);

    for (const auto *rot : rots) {
      delete rot;
    }
    
    return track;
  }

  std::unique_ptr<Track>
  GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const Track& inputTrack,
    const MeasurementSet& addMeasColl,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects) const
  {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Track,Meas'BaseSet,,)");

    Cache cache(this);
    initFieldCache(ctx,cache);

    GXFTrajectory trajectory;
    
    if (!m_straightlineprop) {
      trajectory.m_straightline = (!cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn());
    }
    
    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;

    const TrackParameters *minpar = inputTrack.perigeeParameters();
    
    if (minpar == nullptr) {
      minpar = *(inputTrack.trackParameters()->begin());
    }

    MeasurementSet hitColl;

    // collect MBs from Track (speed: assume this method is used for extending track at end)
    DataVector<const TrackStateOnSurface>::const_iterator itStates = inputTrack.trackStateOnSurfaces()->begin();
    DataVector<const TrackStateOnSurface>::const_iterator endState = inputTrack.trackStateOnSurfaces()->end();

    bool old_reintoutl = cache.m_reintoutl;
    cache.m_reintoutl = false;
    bool tmpasymeloss = cache.m_asymeloss;
    
    if (matEffects == electron) {
      cache.m_asymeloss = true;
    }
    
    for (; itStates != endState; ++itStates) {
      makeProtoState(cache, trajectory, *itStates);
      if (
        matEffects == electron && 
        !trajectory.trackStates().empty() &&
        (trajectory.trackStates().back()->materialEffects() != nullptr) &&
        trajectory.trackStates().back()->materialEffects()->sigmaDeltaE() > 50.001
      ) {
        trajectory.trackStates().back()->materialEffects()->setKink(true);
      }
    }
    
    cache.m_reintoutl = old_reintoutl;
    MeasurementSet::const_iterator itSet = addMeasColl.begin();
    MeasurementSet::const_iterator itSetEnd = addMeasColl.end();

    for (; itSet != itSetEnd; ++itSet) {
      if ((*itSet) == nullptr) {
        ATH_MSG_WARNING("There is an empty MeasurementBase object in the track! Skip this object..");
      } else {
        makeProtoStateFromMeasurement(cache, trajectory, *itSet);
      }
    }

    // fit set of MeasurementBase using main method, start with first TrkParameter in inputTrack
    std::unique_ptr<Track> track(myfit(ctx, cache, trajectory, *minpar, runOutlier, matEffects));
    cache.m_asymeloss = tmpasymeloss;
    
    if (track != nullptr) {
      double oldqual = 
        inputTrack.fitQuality()->numberDoF() != 0 ? 
        inputTrack.fitQuality()->chiSquared() / inputTrack.fitQuality()->numberDoF() : 
        -999;
        
      double newqual = 
        track->fitQuality()->numberDoF() != 0 ? 
        track->fitQuality()->chiSquared() / track->fitQuality()->numberDoF() : 
        -999;

      if (m_extensioncuts && runOutlier && newqual > 2 && newqual > 2 * oldqual) {
        ATH_MSG_DEBUG("Extension rejected");

        cache.incrementFitStatus(S_HIGH_CHI2);
        track.reset(nullptr);
      }
    }

    if (track != nullptr) {
      cache.incrementFitStatus(S_SUCCESSFUL_FITS);
    }
    
    return track;
  }

  // extend a track fit to include an additional set of PrepRawData objects
  // --------------------------------
  std::unique_ptr<Track>
  GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const Track& intrk,
    const PrepRawDataSet& prds,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects) const
  {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Track,PRDS,)");
    MeasurementSet rots;
    const TrackParameters *hitparam = intrk.trackParameters()->back();

    for (const auto *prd : prds) {
      const Surface & prdsurf = (*prd).detectorElement()->surface((*prd).identify());

      Amg::VectorX parameterVector = hitparam->parameters();
      std::unique_ptr<const TrackParameters>trackparForCorrect(
        hitparam->associatedSurface().createUniqueTrackParameters(
          parameterVector[Trk::loc1],
          parameterVector[Trk::loc2],
          parameterVector[Trk::phi],
          parameterVector[Trk::theta],
          parameterVector[Trk::qOverP],
          AmgSymMatrix(5)(*hitparam->covariance())
        )
      );
      
      const RIO_OnTrack *rot = nullptr;
      
      if (!m_broadROTcreator.empty() && prdsurf.type() == Trk::SurfaceType::Line) {
        rot = m_broadROTcreator->correct(*prd, *hitparam);
      } else {
        rot = m_ROTcreator->correct(*prd, *trackparForCorrect);
      }
      
      if (rot != nullptr) {
        rots.push_back(rot);
      }
    }
    
    std::unique_ptr<Track> track = fit(ctx,intrk, rots, runOutlier, matEffects);
    
    for (const auto *rot : rots) {
      delete rot;
    }
    
    return track;
  }

  std::unique_ptr<Track> GlobalChi2Fitter::fit(
    const EventContext& ctx,
    const MeasurementSet & rots,
    const TrackParameters & param,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects
  ) const {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::fit(Meas'BaseSet,,)");

    Cache cache(this);
    initFieldCache(ctx,cache);

    GXFTrajectory trajectory;
    
    if (!m_straightlineprop) {
      trajectory.m_straightline = (!cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn());
    }
    
    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;

    for (const auto *itSet : rots) {
      if (itSet == nullptr) {
        ATH_MSG_WARNING("There is an empty MeasurementBase object in the track! Skip this object..");
      } else {
        makeProtoStateFromMeasurement(cache, trajectory, itSet);
      }
    } 
    
    std::unique_ptr<const TrackParameters> startpar(param.clone());
    
    if (
      matEffects == muon && 
      trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() == 0
    ) {
      cache.m_matfilled = true;
      trajectory.setPrefit(2);
      
      myfit(ctx,cache, trajectory, *startpar, runOutlier, matEffects);
      
      cache.m_matfilled = false;
      
      if (!trajectory.converged()) {
        return nullptr;
      }

      trajectory.setConverged(false);
      const TrackParameters *firstpar = trajectory.trackStates()[0]->trackParameters();
      const TrackParameters *lastpar = trajectory.trackStates().back()->trackParameters();
      
      PerigeeSurface persurf(firstpar->position() - 10 * firstpar->momentum().unit());
      
      if (trajectory.trackStates().front()->measurementType() == TrackState::Pseudo) {
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;

        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(firstpar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          firstpar->associatedSurface()
        );
        
        trajectory.trackStates().front()->setMeasurement(std::move(newpseudo));
      }
      
      if (trajectory.trackStates().back()->measurementType() == TrackState::Pseudo) {
        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;

        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(lastpar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          lastpar->associatedSurface()
        );
        
        trajectory.trackStates().back()->setMeasurement(std::move(newpseudo));
      }
      
      if (!trajectory.m_straightline) {
        trajectory.setPrefit(3);
        const AmgVector(5) & refpars = trajectory.referenceParameters()->parameters();
        startpar = trajectory.referenceParameters()->associatedSurface().createUniqueTrackParameters(
          refpars[0], refpars[1], refpars[2], refpars[3], refpars[4], std::nullopt
        );

        trajectory.reset();
        
        myfit(ctx,cache, trajectory, *startpar, runOutlier, matEffects);
        
        cache.m_matfilled = true;
        
        if (!trajectory.converged()) {
          return nullptr;
        }
      }
      
      const AmgVector(5) & refpars = trajectory.referenceParameters()->parameters();
      startpar = trajectory.referenceParameters()->associatedSurface().createUniqueTrackParameters(
        refpars[0], refpars[1], refpars[2], refpars[3], refpars[4], std::nullopt
      );

      trajectory.reset();
      trajectory.setPrefit(0);
      
      if (trajectory.trackStates().front()->measurementType() == TrackState::Pseudo) {
        firstpar = trajectory.trackStates().front()->trackParameters();

        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;

        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(firstpar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          firstpar->associatedSurface()
        );
        
        trajectory.trackStates().front()->setMeasurement(std::move(newpseudo));
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        trajectory.trackStates().front()->setMeasurementErrors(errors);
      }
      
      if (trajectory.trackStates().back()->measurementType() == TrackState::Pseudo) {
        lastpar = trajectory.trackStates().back()->trackParameters();

        Amg::MatrixX covMatrix(1, 1);
        covMatrix(0, 0) = 100;

        std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
          LocalParameters(DefinedParameter(lastpar->parameters()[Trk::locY], Trk::locY)),
          covMatrix, 
          lastpar->associatedSurface()
        );
        
        trajectory.trackStates().back()->setMeasurement(std::move(newpseudo));
        double errors[5];
        errors[0] = errors[2] = errors[3] = errors[4] = -1;
        errors[1] = 10;
        trajectory.trackStates().back()->setMeasurementErrors(errors);
      }
    }
    
    std::unique_ptr<Track> track;
    
    if (startpar != nullptr) {
      track.reset(myfit(ctx,cache, trajectory, *startpar, runOutlier, matEffects));
    }
    
    if (track != nullptr) {
      cache.incrementFitStatus(S_SUCCESSFUL_FITS);
    }
    cache.m_matfilled = false;
    
    return track;
  }

  void GlobalChi2Fitter::makeProtoState(
    Cache & cache,
    GXFTrajectory & trajectory,
    const TrackStateOnSurface * tsos,
    int index
  ) const {
    if (
      (
        tsos->type(TrackStateOnSurface::Scatterer) || 
        tsos->type(TrackStateOnSurface::BremPoint) || 
        tsos->type(TrackStateOnSurface::CaloDeposit) || 
        tsos->type(TrackStateOnSurface::InertMaterial)
      ) && cache.m_getmaterialfromtrack
    ) {
      if (cache.m_acceleration && trajectory.numberOfHits() == 0) {
        return;
      }
      if (tsos->materialEffectsOnTrack()->derivedType() != MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK){
        return;
      }
      const MaterialEffectsOnTrack *meff = static_cast<const MaterialEffectsOnTrack *>(tsos->materialEffectsOnTrack());
     
      std::unique_ptr<GXFMaterialEffects> newmeff;

      if (
        meff->scatteringAngles() or 
        meff->energyLoss() or
        !tsos->type(TrackStateOnSurface::Scatterer) or 
        (tsos->trackParameters() == nullptr)
      ) {
        newmeff = std::make_unique<GXFMaterialEffects>(*meff);
      } else {
        Trk::MaterialProperties matprop(meff->thicknessInX0(), 1., 0., 0., 0., 0.);
        
        double sigmascat = std::sqrt(m_scattool->sigmaSquare(
          matprop, 
          std::abs(1. / tsos->trackParameters()->parameters()[Trk::qOverP]), 
          1., 
          Trk::muon)
        );
        
        auto newsa = Trk::ScatteringAngles(
          0, 
          0,
          sigmascat / std::sin(tsos->trackParameters()->parameters()[Trk::theta]), 
          sigmascat
        );
        
        Trk::MaterialEffectsOnTrack newmeot(
          meff->thicknessInX0(), 
          newsa, 
          nullptr,
          tsos->surface()
        );
        
        newmeff = std::make_unique<GXFMaterialEffects>(newmeot);
      }
      
      if (
        (meff->energyLoss() != nullptr) && 
        meff->energyLoss()->sigmaDeltaE() > 0 &&
        (
          (tsos->type(TrackStateOnSurface::BremPoint) && (meff->scatteringAngles() != nullptr)) || 
          ((meff->scatteringAngles() == nullptr) || meff->thicknessInX0() == 0)
        )
      ) {
        newmeff->setSigmaDeltaE(meff->energyLoss()->sigmaDeltaE());
        
        if (
          (tsos->trackParameters() != nullptr) && 
          !trajectory.trackStates().empty() &&
          ((**trajectory.trackStates().rbegin()).trackParameters() != nullptr)
        ) {
          double delta_p = 1000 * (
            tsos->trackParameters()->parameters()[Trk::qOverP] -
            (**trajectory.trackStates().rbegin()).trackParameters()->
            parameters()[Trk::qOverP]
          );
          
          newmeff->setdelta_p(delta_p);
        }
      }
      
      trajectory.addMaterialState(std::make_unique<GXFTrackState>(std::move(newmeff), unique_clone(tsos->trackParameters())), index);
    }
    
    if (
      tsos->type(TrackStateOnSurface::Measurement) || 
      tsos->type(TrackStateOnSurface::Outlier)
    ) {
      bool isoutlier = false;
      
      if (tsos->type(TrackStateOnSurface::Outlier) && !cache.m_reintoutl) {
        isoutlier = true;
      }
      
      makeProtoStateFromMeasurement(
        cache, 
        trajectory,
        tsos->measurementOnTrack(),
        tsos->trackParameters(), 
        isoutlier,
        index
      );
    }
  }

  void GlobalChi2Fitter::makeProtoStateFromMeasurement(
    Cache & cache,
    GXFTrajectory & trajectory,
    const MeasurementBase * measbase,
    const TrackParameters * trackpar, 
    bool isoutlier,
    int index
  ) const {
    const Segment *seg = nullptr;

    if (!measbase->associatedSurface().associatedDetectorElementIdentifier().is_valid()) {
      if (measbase->type(Trk::MeasurementBaseType::Segment)){
        seg = static_cast<const Segment *>(measbase);
      }
    }

    int imax = 1;

    if ((seg != nullptr) && m_decomposesegments) {
      imax = (int) seg->numberOfMeasurementBases();
    }

    for (int i = 0; i < imax; i++) {
      const MeasurementBase *measbase2 = ((seg != nullptr) && m_decomposesegments) ? seg->measurement(i) : measbase;
      const TrackParameters *newtrackpar = ((seg != nullptr) && m_decomposesegments) ? nullptr : trackpar;
      std::unique_ptr<GXFTrackState> ptsos = std::make_unique<GXFTrackState>(
        std::unique_ptr<const MeasurementBase>(measbase2->clone()), 
        std::unique_ptr<const TrackParameters>(newtrackpar != nullptr ? newtrackpar->clone() : nullptr)
      );
      const Amg::MatrixX & covmat = measbase2->localCovariance();
      double sinstereo = 0;
      double errors[5];
      errors[0] = errors[1] = errors[2] = errors[3] = errors[4] = -1;
      TrackState::MeasurementType hittype = TrackState::unidentified;
      Identifier hitid = measbase2->associatedSurface().associatedDetectorElementIdentifier();
      //const CompetingRIOsOnTrack *crot = nullptr;
      if (!hitid.is_valid()) {
        if (measbase2->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack )){
          const CompetingRIOsOnTrack *crot = static_cast<const CompetingRIOsOnTrack *>(measbase2);
          hitid = crot->rioOnTrack(0).identify();
        }
      }
      
      bool measphi = false;
      
      if (hitid.is_valid() && measbase2->localParameters().contains(Trk::locX)) {
        bool rotated = false;

        if (m_DetID->is_indet(hitid) && !m_DetID->is_muon(hitid)) {
          if (m_DetID->is_pixel(hitid)) {
            hittype = TrackState::Pixel;
          } else if (m_DetID->is_sct(hitid)) {
            if (covmat.cols() != 1 && covmat(1, 0) != 0) {
              rotated = true;
            }
            hittype = TrackState::SCT;
          } else if (m_DetID->is_trt(hitid)) {
            hittype = TrackState::TRT;
          }
        } else {                // Muon hit
          if (m_DetID->is_rpc(hitid)) {
            hittype = TrackState::RPC;
            if (measbase->localParameters().parameterKey() != 1) {
              ATH_MSG_WARNING("Corrupt RPC hit, skipping it");
              continue;
            }
          } else if (m_DetID->is_mdt(hitid)) {
            hittype = TrackState::MDT;
          } else if (m_DetID->is_tgc(hitid)) {
            if (measbase2->associatedSurface().bounds().type() == Trk::SurfaceBounds::Trapezoid) {
              rotated = true;
            }
            hittype = TrackState::TGC;
          } else if (m_DetID->is_csc(hitid)) {
            hittype = TrackState::CSC;
          } else if (m_DetID->is_mm(hitid)) {
            hittype = TrackState::MM;
          } else if (m_DetID->is_stgc(hitid)) {
            hittype = TrackState::STGC;
          }
        }
        
        if (rotated) {
          const double traceCov = covmat(0, 0) + covmat(1, 1);
          const double diagonalProduct = covmat(0, 0) * covmat(1, 1);
          const double element01Sq = covmat(0, 1) * covmat(0, 1);
          const double sqrtTerm = std::sqrt(
              (traceCov) * (traceCov) - 4. * (diagonalProduct - element01Sq)
            );
           
          double v0 = 0.5 * (
            traceCov - sqrtTerm
          );
          sinstereo = std::sin(0.5 * std::asin(2 * covmat(0, 1) / (-sqrtTerm)));
          errors[0] = std::sqrt(v0);
        } else {
          errors[0] = std::sqrt(covmat(0, 0));
          if (hittype == TrackState::Pixel) {
            errors[1] = std::sqrt(covmat(1, 1));
          }
        }
        if (
          hittype == TrackState::RPC || 
          hittype == TrackState::TGC || 
          hittype == TrackState::CSC || 
          hittype == TrackState::STGC
        ) {
          const Surface *surf = &measbase2->associatedSurface();
          Amg::Vector3D measdir = surf->transform().rotation().col(0);
          double dotprod1 = measdir.dot(Amg::Vector3D(0, 0, 1));
          double dotprod2 = measdir.dot(Amg::Vector3D(surf->center().x(), surf->center().y(), 0) / surf->center().perp());
          if (std::abs(dotprod1) < .5 && std::abs(dotprod2) < .5) {
            measphi = true;
          }
        }
      } else {
        const Trk::LocalParameters & psmpar = measbase2->localParameters();
        // @TODO coverity complains about index shadowing the method argument index
        // this is solved by renaming index in this block by param_index
        int param_index = 0;
        if (psmpar.contains(Trk::locRPhi)) {
          errors[0] = std::sqrt(covmat(0, 0));
          param_index++;
        }

        if (psmpar.contains(Trk::locZ)) {
          errors[1] = std::sqrt(covmat(param_index, param_index));
          param_index++;
        }
        
        if (psmpar.contains(Trk::phi)) {
          errors[2] = std::sqrt(covmat(param_index, param_index));
          param_index++;
        }
        
        if (psmpar.contains(Trk::theta)) {
          errors[3] = std::sqrt(covmat(param_index, param_index));
          param_index++;
        }
        
        if (psmpar.contains(Trk::qOverP)) {
          errors[4] = std::sqrt(covmat(param_index, param_index));
          param_index++;
        }
        if (measbase2->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
          hittype = TrackState::Pseudo;
          ATH_MSG_DEBUG("PseudoMeasurement, pos=" << measbase2->globalPosition());
        } else if (measbase2->type(Trk::MeasurementBaseType::VertexOnTrack )) {
          hittype = TrackState::Vertex;
          ATH_MSG_DEBUG("VertexOnTrack, pos=" << measbase2->globalPosition());  // print out the hit type
        } else if (measbase2->type(Trk::MeasurementBaseType::Segment )) {
          hittype = TrackState::Segment;
          ATH_MSG_DEBUG("Segment, pos=" << measbase2->globalPosition());        // print out the hit type
        }
      }
      if (
        errors[0] > 0 || 
        errors[1] > 0 || 
        errors[2] > 0 || 
        errors[3] > 0 || 
        errors[4] > 0
      ) {
        ptsos->setMeasurementErrors(errors);
        ptsos->setSinStereo(sinstereo);
        ptsos->setMeasurementType(hittype);
        ptsos->setMeasuresPhi(measphi);
        
        if (isoutlier && !cache.m_reintoutl) {
          ptsos->resetStateType(TrackStateOnSurface::Outlier);
        }
        
        // @TODO here index really is supposed to refer to the method argument index ?
        bool ok = trajectory.addMeasurementState(std::move(ptsos), index);
        if (!ok) {
          ATH_MSG_WARNING("Duplicate hit on track");
        }
      } else {
        ATH_MSG_WARNING("Measurement error is zero or negative, drop hit");
      }
    }
  }

  bool GlobalChi2Fitter::processTrkVolume(
    Cache & cache,
    const Trk::TrackingVolume *tvol
  ) const {
    if (tvol == nullptr) {
      return false;
    }
    
    const Trk::BinnedArray < Trk::Layer > *confinedLayers = tvol->confinedLayers();
      
    // loop over confined layers
    if (confinedLayers != nullptr) {
      Trk::BinnedArraySpan<Trk::Layer const * const >layerVector = confinedLayers->arrayObjects();
      Trk::BinnedArraySpan<Trk::Layer const * const >::const_iterator layerIter = layerVector.begin();
      
      // loop over layers
      for (; layerIter != layerVector.end(); ++layerIter) {
        // push_back the layer
        if (*layerIter != nullptr) {
          // get the layerIndex
          const Trk::LayerIndex & layIndex = (*layerIter)->layerIndex();
          // skip navigaion layers for the moment

          if ((layIndex.value() == 0) || ((*layerIter)->layerMaterialProperties() == nullptr)) {
            continue;
          }
          
          const CylinderLayer *cyllay = nullptr;
          if ((*layerIter)->surfaceRepresentation().type() == Trk::SurfaceType::Cylinder)
            cyllay = static_cast<const CylinderLayer *>((*layerIter));
            
          const DiscLayer *disclay = nullptr;
          
          if ((*layerIter)->surfaceRepresentation().type() == Trk::SurfaceType::Disc)
            disclay = static_cast<const DiscLayer *>((*layerIter));
            
          if (disclay != nullptr) {
            if (disclay->center().z() < 0) {
              cache.m_negdiscs.push_back(disclay);
            } else {
              cache.m_posdiscs.push_back(disclay);
            }
          } else if (cyllay != nullptr) {
            cache.m_barrelcylinders.push_back(cyllay);
          } else {
            return false;
          }
        }
      }
    }

    const auto & bsurf = tvol->boundarySurfaces();
    
    for (size_t ib = 0 ; ib < bsurf.size(); ++ib) {
      const Layer *layer = bsurf[ib]->surfaceRepresentation().materialLayer();
      
      if (layer == nullptr) continue;
      
      const Trk::LayerIndex & layIndex = layer->layerIndex();
      
      if ((layIndex.value() == 0) || (layer->layerMaterialProperties() == nullptr)) {
        continue;
      }
      
      const CylinderSurface *cylsurf = nullptr;
      
      if (layer->surfaceRepresentation().type() == Trk::SurfaceType::Cylinder)
        cylsurf = static_cast<const CylinderSurface *>(&layer->surfaceRepresentation());
          
      const DiscSurface *discsurf = nullptr;
      
      if (layer->surfaceRepresentation().type() == Trk::SurfaceType::Disc)
        discsurf = static_cast<const DiscSurface *>(&layer->surfaceRepresentation());

      if (discsurf != nullptr) {
        if (
          discsurf->center().z() < 0 &&
          std::find(cache.m_negdiscs.begin(), cache.m_negdiscs.end(), layer) == cache.m_negdiscs.end()
        ) {
          cache.m_negdiscs.push_back(layer);
        } else if (
          discsurf->center().z() > 0 &&
          std::find(cache.m_posdiscs.begin(), cache.m_posdiscs.end(), layer) == cache.m_posdiscs.end()
        ) {
          cache.m_posdiscs.push_back(layer);
        }
      } else if (
        (cylsurf != nullptr) &&
        std::find(cache.m_barrelcylinders.begin(), cache.m_barrelcylinders.end(), layer) == cache.m_barrelcylinders.end()
      ) {
        cache.m_barrelcylinders.push_back(layer);
      }
      
      if ((cylsurf == nullptr) && (discsurf == nullptr)) {
        return false;
      }
    }
    
    const TrackingVolumeArray* confinedVolumes = tvol->confinedVolumes();
    // get the confined volumes and loop over it -> call recursively
    if (confinedVolumes != nullptr) {
      Trk::BinnedArraySpan<Trk::TrackingVolume const * const> volumes = confinedVolumes->arrayObjects();
      
      Trk::BinnedArraySpan<Trk::TrackingVolume const * const >::const_iterator volIter = volumes.begin();
      Trk::BinnedArraySpan<Trk::TrackingVolume const * const>::const_iterator volIterEnd = volumes.end();
      
      for (; volIter != volIterEnd; ++volIter) {
        if (*volIter != nullptr) {
          bool ok = processTrkVolume(cache, *volIter);
          if (!ok) {
            return false;
          }
        }
      }
    }
    
    return true;
  }

  

 
  
  std::optional<std::pair<Amg::Vector3D, double>> 
  GlobalChi2Fitter::addMaterialFindIntersectionDisc(
    Cache & cache,
    const DiscSurface &surf,
    const TrackParameters &parforextrap,
    const TrackParameters &refpar2,
    const ParticleHypothesis matEffects
  ) {
    /*
     * Please refer to external sources on how to find the intersection between
     * a line and a disc.
     */
    double field[3];
    double * pos = parforextrap.position().data();
    double currentqoverp = (matEffects != Trk::electron) ? parforextrap.parameters()[Trk::qOverP] : refpar2.parameters()[Trk::qOverP];
    cache.m_field_cache.getFieldZR(pos, field);
    double sinphi = std::sin(parforextrap.parameters()[Trk::phi0]);
    double cosphi = std::cos(parforextrap.parameters()[Trk::phi0]);
    double sintheta = std::sin(parforextrap.parameters()[Trk::theta]);
    double costheta = std::cos(parforextrap.parameters()[Trk::theta]);
    //magnetic field deviation from straight line
    //https://cds.cern.ch/record/1281363/files/ATLAS-CONF-2010-072.pdf, equation 1
    //converted to MeV and kT
    double r = (std::abs(currentqoverp) > 1e-10) ? -sintheta / (currentqoverp * 300. * field[2]) : 1e6;
    double xc = parforextrap.position().x() - r * sinphi;
    double yc = parforextrap.position().y() + r * cosphi;
    double phi0 = std::atan2(parforextrap.position().y() - yc, parforextrap.position().x() - xc);
    double z0 = parforextrap.position().z();
    double delta_s = (surf.center().z() - z0) / costheta;
    double delta_phi = delta_s * sintheta / r;
    double x = xc + std::abs(r) * std::cos(phi0 + delta_phi);
    double y = yc + std::abs(r) * std::sin(phi0 + delta_phi);
    Amg::Vector3D intersect = Amg::Vector3D(x, y, surf.center().z());
    double perp = intersect.perp();
    const DiscBounds *discbounds = (const DiscBounds *) (&surf.bounds());
    
    if (perp > discbounds->rMax() || perp < discbounds->rMin()) {
      return {};
    }
    
    double costracksurf = std::abs(costheta);
    
    return std::make_pair(intersect, costracksurf);
  }
  
  std::optional<std::pair<Amg::Vector3D, double>> 
  GlobalChi2Fitter::addMaterialFindIntersectionCyl(
    Cache & cache,
    const CylinderSurface &surf,
    const TrackParameters &parforextrap,
    const TrackParameters &refpar2,
    const ParticleHypothesis matEffects
  ) {
    /*
     * I hope you like trigonometry!
     *
     * For more information, please find a source elsewhere on finding
     * intersections with cylinders.
     */
    double field[3];
    double * pos = parforextrap.position().data();
    double currentqoverp = (matEffects != Trk::electron) ? parforextrap.parameters()[Trk::qOverP] : refpar2.parameters()[Trk::qOverP];
    cache.m_field_cache.getFieldZR(pos, field);
    double sinphi = std::sin(parforextrap.parameters()[Trk::phi0]);
    double cosphi = std::cos(parforextrap.parameters()[Trk::phi0]);
    double sintheta = std::sin(parforextrap.parameters()[Trk::theta]);
    double costheta = std::cos(parforextrap.parameters()[Trk::theta]);
    double tantheta = std::tan(parforextrap.parameters()[Trk::theta]);
    double r = (std::abs(currentqoverp) > 1e-10) ? -sintheta / (currentqoverp * 300. * field[2]) : 1e6;
    double xc = parforextrap.position().x() - r * sinphi;
    double yc = parforextrap.position().y() + r * cosphi;
    double phi0 = std::atan2(parforextrap.position().y() - yc, parforextrap.position().x() - xc);
    double z0 = parforextrap.position().z();
    double d = xc * xc + yc * yc;
    double rcyl = surf.bounds().r();
    double mysqrt = ((r + rcyl) * (r + rcyl) - d) * (d - (r - rcyl) * (r - rcyl));
    
    if (mysqrt < 0) {
      return {};
    }
    
    mysqrt = std::sqrt(mysqrt);
    double firstterm = xc / 2 + (xc * (rcyl * rcyl - r * r)) / (2 * d);
    double secondterm = (mysqrt * yc) / (2 * d);
    double x1 = firstterm + secondterm;
    double x2 = firstterm - secondterm;
    firstterm = yc / 2 + (yc * (rcyl * rcyl - r * r)) / (2 * d);
    secondterm = (mysqrt * xc) / (2 * d);
    double y1 = firstterm - secondterm;
    double y2 = firstterm + secondterm;
    double x = parforextrap.position().x();
    double y = parforextrap.position().y();
    double dist1 = (x - x1) * (x - x1) + (y - y1) * (y - y1);
    double dist2 = (x - x2) * (x - x2) + (y - y2) * (y - y2);
    
    if (dist1 < dist2) {
      x = x1;
      y = y1;
    } else {
      x = x2;
      y = y2;
    }
    
    double phi1 = std::atan2(y - yc, x - xc);
    double deltaphi = phi1 - phi0;
    
    coercePhiCoordinateRange(deltaphi);
    
    double delta_z = r * deltaphi / tantheta;
    double z = z0 + delta_z;

    Amg::Vector3D intersect = Amg::Vector3D(x, y, z);

    if (std::abs(z - surf.center().z()) > surf.bounds().halflengthZ()) {
      return {};
    }
    
    Amg::Vector3D normal(x, y, 0);
    double phidir = parforextrap.parameters()[Trk::phi] + deltaphi;
    coercePhiCoordinateRange(phidir);
    
    Amg::Vector3D trackdir(cos(phidir) * sintheta, std::sin(phidir) * sintheta, costheta);
    
    double costracksurf = std::abs(normal.unit().dot(trackdir));
    
    return std::make_pair(intersect, costracksurf);
  }

  void GlobalChi2Fitter::addMaterialUpdateTrajectory(
    Cache &cache,
    GXFTrajectory &trajectory,
    int indexoffset,
    std::vector<std::pair<const Layer *, const Layer *>> &layers,
    const TrackParameters *refpar,
    const TrackParameters *refpar2,
    ParticleHypothesis matEffects
  ) const {
    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    std::vector<std::unique_ptr<GXFTrackState>> oldstates = std::move(states);

    states.clear();
    states.reserve(oldstates.size() + layers.size());

    int layerindex = 0;
    
    /*
     * First, simply copy any upstream states. We do not need to anything with
     * them as they are presumably already fit.
     */
    for (int i = 0; i <= indexoffset; i++) {
      trajectory.addBasicState(std::move(oldstates[i]));
    }
    
    const TrackParameters *parforextrap = refpar;

    /*
     * For non-upstream layers, that is to say layers after the last existing
     * material, the logic is not so simple.
     */
    for (int i = indexoffset + 1; i < (int) oldstates.size(); i++) {
      double rmeas = oldstates[i]->position().perp();
      double zmeas = oldstates[i]->position().z();

      /*
       * Iterate over layers. Note that this is shared between different track
       * states! This makes sense, because the track states are sorted and so
       * are the layers. If that sorting is consistent between the two, which
       * it should be, this works.
       */
      while (layerindex < (int) layers.size()) {
        Amg::Vector3D intersect;
        double costracksurf = 0.0;
        const Layer *layer = nullptr;
        
        /*
         * Remember how we distinguish between disc and cylinder surfaces: if
         * the first element of the pair is not null, then it points to a
         * cylinder. If the second element of the pais is not null, then it's a
         * disc surface. That is the logic being applied here. Separate
         * handling of cylinders and discs.
         */
        if (layers[layerindex].first != nullptr) {
          /*
           * First, convert the pointer to a real CylinderSurface pointer.
           */
          layer = layers[layerindex].first;
          const CylinderSurface *cylsurf = (const CylinderSurface *) (&layer->surfaceRepresentation());
          
          /*
           * Check if we have a different set of parameters that make more
           * sense. If not, reuse the ones we already had.
           */
          if (oldstates[i]->trackParameters() != nullptr) {
            double rlayer = cylsurf->bounds().r();
            if (std::abs(rmeas - rlayer) < std::abs(parforextrap->position().perp() - rlayer)) {
              parforextrap = oldstates[i]->trackParameters();
            }
          }
          
          /*
           * Check if we have an intersection with this layer. If so, break out
           * of this loop, we have what we need. Otherwise, go to the next
           * layer and try again.
           */
          if (auto res = addMaterialFindIntersectionCyl(cache, *cylsurf, *parforextrap, *refpar2, matEffects)) {
            std::tie(intersect, costracksurf) = res.value();
          } else {
            layerindex++;
            continue;
          }
          
          if (cylsurf->bounds().r() > rmeas) break;
        } else if (layers[layerindex].second != nullptr) {
          /*
           * The logic for disc surfaces is essentially identical to the logic
           * for cylinder surfaces. You'll find comments for that just a dozen
           * lines up.
           */
          layer = layers[layerindex].second;
          const DiscSurface *discsurf = (const DiscSurface *) (&layer->surfaceRepresentation());
          
          if (oldstates[i]->trackParameters() != nullptr) {
            double zlayer = discsurf->center().z();
            if (std::abs(zmeas - zlayer) < std::abs(parforextrap->position().z() - zlayer)) {
              parforextrap = oldstates[i]->trackParameters();
            }
          }
          
          if (auto res = addMaterialFindIntersectionDisc(cache, *discsurf, *parforextrap, *refpar2, matEffects)) {
            std::tie(intersect, costracksurf) = res.value();
          } else {
            layerindex++;
            continue;
          }
          
          if (std::abs(discsurf->center().z()) > std::abs(zmeas)) break;
        } else {
          throw std::logic_error("Unhandled surface.");
        }
        
        /*
         * Grab the material properties from our layer. If there are none, just
         * go to the next layer.
         */
        const MaterialProperties *matprop = layer->layerMaterialProperties()->fullMaterial(intersect);
        if (matprop == nullptr) {
          layerindex++;
          continue;
        }
        
        /*
         * Convert the material properties into the internal representation of
         * material effects.
         */
        double X0 = matprop->thicknessInX0();
        double currentqoverp = (matEffects != Trk::electron)
                                 ? parforextrap->parameters()[Trk::qOverP]
                                 : refpar2->parameters()[Trk::qOverP];
        double actualx0 = X0 / costracksurf;
        double de = -std::abs(
          (matprop->thickness() / costracksurf) *
          m_elosstool->dEdX(
            *matprop,
            (m_p != 0.0 ? std::abs(m_p) : std::abs(1. / currentqoverp)),
            matEffects));
        double sintheta = std::sin(parforextrap->parameters()[Trk::theta]);
        double sigmascat = std::sqrt(m_scattool->sigmaSquare(
          *matprop,
          (m_p != 0.0 ? std::abs(m_p) : std::abs(1. / currentqoverp)),
          1. / costracksurf,
          matEffects));

        std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>();
        meff->setDeltaE(de);
        meff->setScatteringSigmas(sigmascat / sintheta, sigmascat);
        meff->setX0(actualx0);
        meff->setSurface(&layer->surfaceRepresentation());
        meff->setMaterialProperties(matprop);

        /*
         * If we have an electron, or if so configured, calculate energy loss
         * as well.
         */
        std::unique_ptr<EnergyLoss> eloss;
        
        if (cache.m_fiteloss || (matEffects == electron && cache.m_asymeloss)) {
          eloss = std::make_unique<EnergyLoss>(m_elosstool->energyLoss(
            *matprop,
            (m_p != 0.0 ? std::abs(m_p) : std::abs(1. / currentqoverp)),
            1. / costracksurf, 
            alongMomentum,
            matEffects
          ));
          if (eloss != nullptr) {
            meff->setSigmaDeltaE(eloss->sigmaDeltaE());
          }
        }
        
        if (matEffects == electron && cache.m_asymeloss) {
          meff->setDeltaE(-5);
          if (trajectory.numberOfTRTHits() == 0) {
            meff->setScatteringSigmas(0, 0);
          }
          
          meff->setSigmaDeltaE(50);
          if (eloss != nullptr) {
            meff->setSigmaDeltaEPos(eloss->sigmaPlusDeltaE());
            meff->setSigmaDeltaENeg(eloss->sigmaMinusDeltaE());
          }
        }

        ATH_MSG_DEBUG(
          "X0: " << meff->x0() << " qoverp: " << currentqoverp << 
          " sigmascat " << meff->sigmaDeltaTheta() <<" eloss: " << meff->deltaE() << 
          " sigma eloss: " << meff->sigmaDeltaE()
        );

        /*
         * Create a new track state in the internal representation and load it
         * with any and all information we might have.
         */
        std::unique_ptr<GXFTrackState> matstate = std::make_unique<GXFTrackState>(
          std::move(meff),
          std::unique_ptr<const TrackParameters>()
        );
        matstate->setPosition(intersect);
        trajectory.addMaterialState(std::move(matstate));

        /*
         * We're done on this layer, so the next state will go to the next
         * layer.
         */
        layerindex++;
      }

      trajectory.addBasicState(std::move(oldstates[i]));
    }
  }

  void GlobalChi2Fitter::addMaterialGetLayers(
    Cache & cache,
    std::vector<std::pair<const Layer *, const Layer *>> & layers,
    std::vector<std::pair<const Layer *, const Layer *>> & upstreamlayers,
    const std::vector<std::unique_ptr<GXFTrackState>> & oldstates,
    GXFTrackState & firstsistate,
    GXFTrackState & lastsistate,
    const TrackParameters *refpar,
    bool hasmat
  ) {
    /*
     * Reserve some arbitrary number of layers in the output vectors.
     */
    upstreamlayers.reserve(5);
    layers.reserve(30);
    
    /*
     * Gather a bunch of numbers from the parameters. Someties we need to grab
     * them from the first silicon state, sometimes from the last.
     */
    double firstz = firstsistate.trackParameters()->position().z();
    double firstr = firstsistate.trackParameters()->position().perp();
    double firstz2 = hasmat ? lastsistate.trackParameters()->position().z() : firstsistate.trackParameters()->position().z();
    double firstr2 = hasmat ? lastsistate.trackParameters()->position().perp() : firstsistate.trackParameters()->position().perp();
    
    GXFTrackState *firststate = oldstates.front().get();
    GXFTrackState *laststate = oldstates.back().get();
    
    /*
     * This number is particularly interesting, as it determines which side we
     * need to look at in regards to the disc layers.
     */
    double lastz = laststate->position().z();
    double lastr = laststate->position().perp();
    
    const Layer *startlayer = firststate->associatedSurface().associatedLayer();
    const Layer *startlayer2 = hasmat ? lastsistate.associatedSurface().associatedLayer() : nullptr;
    const Layer *endlayer = laststate->associatedSurface().associatedLayer();
    
    double tantheta = std::tan(refpar->parameters()[Trk::theta]);
    double slope = (tantheta != 0) ? 1 / tantheta : 0;  // (lastz-firstz)/(lastr-firstr);
    
    /*
     * First, we will grab our disc layers.
     */
    if (slope != 0) {
      std::vector < const Layer *>::const_iterator it;
      std::vector < const Layer *>::const_iterator itend;
      
      /*
       * If we're on the positive z-side of the detector, we will iterate over
       * the positive discs. Otherwise, we will need to iterate over the
       * negative discs.
       */
      if (lastz > 0) {
        it = cache.m_posdiscs.begin();
        itend = cache.m_posdiscs.end();
      } else {
        it = cache.m_negdiscs.begin();
        itend = cache.m_negdiscs.end();
      }
      
      /*
       * Iterate over our disc layers.
       */
      for (; it != itend; ++it) {
        /*
         * If we've overshot the last hit in our track, we don't need to look
         * at any further layers. We're done!
         */
        if (std::abs((*it)->surfaceRepresentation().center().z()) > std::abs(lastz)) {
          break;
        }
        
        /*
         * Grab the bounds from the layer, which is a more useful kind of
         * object that allows us to do some geometric calculations.
         */
        const DiscBounds *discbounds = (const DiscBounds *) (&(*it)->surfaceRepresentation().bounds());
        
        /*
         * Ensure that we've actually hit the layer!
         */
        if (discbounds->rMax() < firstr || discbounds->rMin() > lastr) {
          continue;
        }
        
        double rintersect = firstr + ((*it)->surfaceRepresentation().center().z() - firstz) / slope;
        
        if (
          rintersect < discbounds->rMin() - 50 || 
          rintersect > discbounds->rMax() + 50
        ) {
          continue;
        }
        
        /*
         * We also do not need to consider the last layer. If all goes well,
         * the next loop will immediately break because it will be an
         * overshoot.
         */
        if ((*it) == endlayer) {
          continue;
        }
        
        /*
         * If this layer lies before the first hit, it's an upstream hit and we
         * add it to the upstream layer vector.
         *
         * Notice how we add this layer on the right side of the pair, that's
         * the convention. Discs to right, cylinders go left.
         */
        if (
          std::abs((*it)->surfaceRepresentation().center().z()) < std::abs(firstz) || 
          (*it) == startlayer
        ) {
          upstreamlayers.emplace_back((Layer *) nullptr, (*it));
        }
        
        /*
         * Otherwise, it's a normal layer. Add it.
         */
        if (
          (*it) != startlayer &&
          (std::abs((*it)->surfaceRepresentation().center().z()) > std::abs(firstz2) || 
          (*it) == startlayer2)
        ) {
          layers.emplace_back((Layer *) nullptr, (*it));
        }
      }
    }
    
    /*
     * Now, we add the barrel cylinder layers.
     */
    for (const auto *barrelcylinder : cache.m_barrelcylinders) {
      /*
       * Check for overshoots and reject them.
       */
      if (barrelcylinder->surfaceRepresentation().bounds().r() > lastr) {
        break;
      }
      
      /*
       * Confirm intersection with the layer.
       */
      double zintersect = firstz + (barrelcylinder->surfaceRepresentation().bounds().r() - firstr) * slope;

      if (std::abs(zintersect - barrelcylinder->surfaceRepresentation().center().z()) >
          ((const CylinderSurface*)(&barrelcylinder->surfaceRepresentation()))->bounds().halflengthZ() + 50) {
        continue;
      }

      if (barrelcylinder == endlayer) {
        continue;
      }
      
      /*
       * Same as with the discs, add the layers to the output vectors.
       */
      if (barrelcylinder->surfaceRepresentation().bounds().r() < firstr ||
          barrelcylinder == startlayer) {
        upstreamlayers.emplace_back(barrelcylinder, (Layer*)nullptr);
      }

      if (barrelcylinder != startlayer &&
          (barrelcylinder->surfaceRepresentation().bounds().r() > firstr2 ||
           barrelcylinder == startlayer2)) {
        layers.emplace_back(barrelcylinder, (Layer*)nullptr);
      }
    }

    /*
     * Sort the layers such that they are in the right order, from close to far
     * in respect to the experiment center.
     */
    std::sort(layers.begin(), layers.end(), GXF::LayerSort());
    std::sort(upstreamlayers.begin(), upstreamlayers.end(), GXF::LayerSort());
  }

  void GlobalChi2Fitter::addIDMaterialFast(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory & trajectory,
    const TrackParameters * refpar2,
    ParticleHypothesis matEffects
  ) const {
    /*
     * Ensure that the cache contains a valid tracking geometry that we can
     * use.
     */
    if (cache.m_caloEntrance == nullptr) {
      const TrackingGeometry *geometry = trackingGeometry(cache,ctx);
      
      if (geometry != nullptr) {
        cache.m_caloEntrance = geometry->trackingVolume("InDet::Containers::InnerDetector");
      } else {
        ATH_MSG_ERROR("Tracking Geometry not available");
      }
      
      if (cache.m_caloEntrance == nullptr) {
        ATH_MSG_ERROR("calo entrance not available");
        return;
      }
    }

    /*
     * If we have not yet set the discs on either side of the detector as well
     * as the barrel layers, do so now.
     */
    if (
      cache.m_negdiscs.empty() && 
      cache.m_posdiscs.empty() && 
      cache.m_barrelcylinders.empty()
    ) {
      /*
       * Attempt to add the layer information to the cache using the previously
       * selected tracking volume.
       */
      bool ok = processTrkVolume(cache, cache.m_caloEntrance);

      /*
       * If this process somehow fails, we cannot use the fast material adding
       * algorithm and we must fall back to the slow version. As far as I know
       * this doesn't really happen.
       */
      if (!ok) {
        ATH_MSG_DEBUG("Falling back to slow material collection");
        cache.m_fastmat = false;
        addMaterial(ctx, cache, trajectory, refpar2, matEffects);
        return;
      }
      
      /*
       * Sort the discs and barrel layers such that they are in the right
       * order. What the right order is in this case is defined a bit above
       * this code, in the GXF::LayerSort2 class. Should be in increasing order
       * of distance from the detector center.
       */
      std::stable_sort(cache.m_negdiscs.begin(), cache.m_negdiscs.end(), GXF::LayerSort2());
      std::stable_sort(cache.m_posdiscs.begin(), cache.m_posdiscs.end(), GXF::LayerSort2());
      std::stable_sort(cache.m_barrelcylinders.begin(), cache.m_barrelcylinders.end(), GXF::LayerSort2());
    }
    
    const TrackParameters *refpar = refpar2;
    bool hasmat = false;
    int indexoffset = 0, lastmatindex = 0;
    std::vector<std::unique_ptr<GXFTrackState>> & oldstates = trajectory.trackStates();
    
    GXFTrackState *firstsistate = nullptr;
    GXFTrackState *lastsistate = nullptr;
   
    /*
     * This loop serves several purposes in one, because it's very efficient:
     *
     * 1. It detects whether there are already any materials on this track, and
     *    if so where they are.
     * 2. It determines what the first and last silicon hits are.
     * 3. It calculates trackparameters for any states that might not have them
     *    for whatever reason.
     */
    for (int i = 0; i < (int) oldstates.size(); i++) {
      if (oldstates[i]->materialEffects() != nullptr) {
        hasmat = true;
        lastmatindex = i;
      }
      
      if (
        oldstates[i]->measurementType() == TrackState::Pixel || 
        oldstates[i]->measurementType() == TrackState::SCT
      ) {
        if (firstsistate == nullptr) {
          if (oldstates[i]->trackParameters() == nullptr) {
            std::unique_ptr<const TrackParameters> tmppar(m_propagator->propagateParameters(
              ctx,
              *refpar, 
              oldstates[i]->associatedSurface(), 
              alongMomentum, 
              false, 
              trajectory.m_fieldprop,
              Trk::nonInteracting
            ));
            
            if (tmppar == nullptr) return;
            
            oldstates[i]->setTrackParameters(std::move(tmppar));
          }
          firstsistate = oldstates[i].get();
        }
        lastsistate = oldstates[i].get();
      }
    }
    
    /*
     * Only happens when there are no tracks, and that shouldn't happen in the
     * first place.
     */
    if (lastsistate == nullptr) {
      throw std::logic_error("No track state");
    }

    /*
     * Also try to generate a set of track parameters for the last silicon hit
     * if it doesn't have any. I don't really know when that would happen, but
     * I suppose it's possible. Anything is possible, if you believe hard
     * enough.
     */
    if (lastsistate->trackParameters() == nullptr) {
      std::unique_ptr<const TrackParameters> tmppar(m_propagator->propagateParameters(
        ctx,
        *refpar,
        lastsistate->associatedSurface(),
        alongMomentum, false,
        trajectory.m_fieldprop,
        Trk::nonInteracting
      ));
      
      if (tmppar == nullptr) return;
      
      lastsistate->setTrackParameters(std::move(tmppar));
    }

    /*
     * If we have found any materials on the track, we've presumably already
     * done a fit for that part of the track, so the reference parameters are
     * either the first or last silicon state's parameters.
     */
    if (hasmat) {
      refpar = lastsistate->trackParameters();
      indexoffset = lastmatindex;
    } else {
      refpar = firstsistate->trackParameters();
    }
    
    /*
     * These vectors will hold the layers. The types here are a little bit
     * strange, but the idea is that the right member is a disc surface and the
     * left member is a cylindrical surface. Could be more elegantly done using
     * polymorphism.
     *
     * The upstream layers may already be filled due to previous fits.
     *
     * TODO: Use polymorphism to get rid of these strange types.
     */
    std::vector<std::pair<const Layer *, const Layer *>> layers;
    std::vector<std::pair<const Layer *, const Layer *>> & upstreamlayers = trajectory.upstreamMaterialLayers();
    
    /*
     * Fill the aforementioned layer vectors with layers.
     */
    addMaterialGetLayers(cache, layers, upstreamlayers, oldstates, *firstsistate, *lastsistate, refpar, hasmat);

    /*
     * Finally, use that layer information to actually add states to the track.
     */
    addMaterialUpdateTrajectory(cache, trajectory, indexoffset, layers, refpar, refpar2, matEffects);
  }

  void GlobalChi2Fitter::addMaterial(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory & trajectory,
    const TrackParameters * refpar2,
    ParticleHypothesis matEffects
  ) const {
    if (refpar2 == nullptr) {
      return;
    }
    const MeasurementBase *firstmuonhit = nullptr;
    const MeasurementBase *lastmuonhit = nullptr;
    const MeasurementBase *firstidhit =
      nullptr;
    const MeasurementBase *lastidhit = nullptr;
    const MeasurementBase *firsthit = nullptr;
    const MeasurementBase *lasthit = nullptr;
    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    std::vector<std::unique_ptr<GXFTrackState>> matstates;
    std::unique_ptr< const std::vector < const TrackStateOnSurface *>,
                     void (*)(const std::vector<const TrackStateOnSurface *> *) >
      matvec(nullptr,&Trk::GlobalChi2Fitter::Cache::objVectorDeleter<TrackStateOnSurface>);
    bool matvec_used=false;
    std::unique_ptr<TrackParameters> startmatpar1;
    std::unique_ptr<TrackParameters> startmatpar2;
    const TrackParameters *firstidpar = nullptr;
    const TrackParameters *lastidpar = nullptr;
    const TrackParameters *firstsiliconpar = nullptr;
    const TrackParameters *lastsiliconpar = nullptr;
    const TrackParameters *firstmatpar = nullptr;
    const TrackParameters *firstcalopar = nullptr;
    const TrackParameters *lastcalopar = nullptr;
    const TrackParameters *firstmuonpar = nullptr;
    const TrackParameters *lastmuonpar = nullptr;

    int npseudomuon1 = 0;
    int npseudomuon2 = 0;

    for (auto & state : states) {
      TrackState::MeasurementType meastype = state->measurementType();
      const TrackParameters *tp = state->trackParameters();
      GXFMaterialEffects *meff = state->materialEffects();
      
      if (meastype == TrackState::Pseudo) {
        if (firstidhit == nullptr) {
          npseudomuon1++;
        } else {
          npseudomuon2++;
        }
        continue;
      }
      
      if (state->getStateType(TrackStateOnSurface::Measurement) || state->getStateType(TrackStateOnSurface::Outlier)) {
        if (firsthit == nullptr) {
          firsthit = state->measurement();
          if (cache.m_acceleration) {
            if (tp == nullptr) {
              tp = m_extrapolator->extrapolate(
                ctx,
                *refpar2, 
                state->associatedSurface(),
                alongMomentum, 
                false, 
                matEffects
              ).release();

              if (tp == nullptr) {
                return;
              }

              state->setTrackParameters(std::unique_ptr<const TrackParameters>(tp));
            }
            // When acceleration is enabled, material collection starts from first hit
            refpar2 = tp;
          }
        }
        lasthit = state->measurement();
        if (
          meastype == TrackState::Pixel || 
          meastype == TrackState::SCT || 
          meastype == TrackState::TRT
        ) {
          if (firstidhit == nullptr) {
            firstidhit = state->measurement();
          }
        
          if ((firstidpar == nullptr) && (tp != nullptr)) {
            firstidpar = tp;
          }

          lastidhit = state->measurement();
          if (tp != nullptr) {
            lastidpar = tp;
          }
          
          if ((tp != nullptr) && meastype != TrackState::TRT) {
            if (firstsiliconpar == nullptr) {
              firstsiliconpar = tp;
            }
            lastsiliconpar = tp;
          }
        }
        
        if (
          meastype == TrackState::RPC || 
          meastype == TrackState::CSC || 
          meastype == TrackState::TGC || 
          meastype == TrackState::MDT || 
          meastype == TrackState::MM || 
          meastype == TrackState::STGC
        ) {
          if (firstmuonhit == nullptr) {
            firstmuonhit = state->measurement();
            if (tp != nullptr) {
              firstmuonpar = tp;
            }
          }
          lastmuonhit = state->measurement();
          if (tp != nullptr) {
            lastmuonpar = tp;
          }
        }
      }
      if (state->getStateType(TrackStateOnSurface::Scatterer) || state->getStateType(TrackStateOnSurface::BremPoint)) {
        if (meff->deltaE() == 0) {
          if (firstcalopar == nullptr) {
            firstcalopar = state->trackParameters();
          }
          lastcalopar = state->trackParameters();
        }
        if (firstmatpar == nullptr) {
          firstmatpar = state->trackParameters();
        }
      }
    }

    std::unique_ptr<TrackParameters> refpar;
    AmgVector(5) newpars = refpar2->parameters();
    
    if (trajectory.m_straightline && m_p != 0) {
      newpars[Trk::qOverP] = 1 / m_p;
    }
    
    refpar = refpar2->associatedSurface().createUniqueTrackParameters(
      newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
    );

    if (firstmatpar != nullptr) {
      startmatpar1 = unique_clone(firstsiliconpar);
      startmatpar2 = unique_clone(lastsiliconpar);
    }
    
    if ((startmatpar1 == nullptr) || ((firstidhit != nullptr) && (firstmuonhit != nullptr))) {
      startmatpar1 = unique_clone(refpar);
      startmatpar2 = unique_clone(refpar);

      double mass = trajectory.mass();
      if (mass > 200 * MeV) {
        const AmgVector(5) & newpars = startmatpar2->parameters();
        double oldp = std::abs(1 / newpars[Trk::qOverP]);
        double sign = (newpars[Trk::qOverP] < 0) ? -1 : 1;
        
        startmatpar2 = startmatpar2->associatedSurface().createUniqueTrackParameters(
          newpars[0], newpars[1], newpars[2], newpars[3],
          sign / std::sqrt(oldp * oldp + 2 * 100 * MeV * std::sqrt(oldp * oldp + mass * mass) + 100 * MeV * 100 * MeV), 
          std::nullopt
        );
      }
    } else if (trajectory.m_straightline && m_p != 0) {
      AmgVector(5) newpars = startmatpar1->parameters();
      newpars[Trk::qOverP] = 1 / m_p;
      
      startmatpar1 = startmatpar1->associatedSurface().createUniqueTrackParameters(
        newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
      );
      
      newpars = startmatpar2->parameters();
      newpars[Trk::qOverP] = 1 / m_p;
      
      startmatpar2 = startmatpar2->associatedSurface().createUniqueTrackParameters(
        newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
      );
    }

    if ((firstidhit != nullptr) && trajectory.numberOfSiliconHits() > 0 && cache.m_idmat) {
      
      DistanceSolution distsol = firstidhit->associatedSurface().straightLineDistanceEstimate(
        refpar->position(),
        refpar->momentum().unit()
      );
      
      double distance = getDistance(distsol);

      if (distance < 0 && distsol.numberOfSolutions() > 0 && !cache.m_acceleration) {
        ATH_MSG_DEBUG("Obtaining upstream layers from Extrapolator");

        const Surface *destsurf = &firstidhit->associatedSurface();
        std::unique_ptr<const TrackParameters> tmppar;
        
        if (firstmuonhit != nullptr) {
          if (cache.m_caloEntrance == nullptr) {
            const TrackingGeometry *geometry = trackingGeometry(cache,ctx);
            
            if (geometry != nullptr) {
              cache.m_caloEntrance = geometry->trackingVolume("InDet::Containers::InnerDetector");
            } else {
              ATH_MSG_ERROR("Tracking Geometry not available");
            }
          }

          if (cache.m_caloEntrance == nullptr) {
            ATH_MSG_ERROR("calo entrance not available");
          } else {
            tmppar = m_extrapolator->extrapolateToVolume(ctx,
                                                         *startmatpar1,
                                                         *cache.m_caloEntrance,
                                                         oppositeMomentum,
                                                         Trk::nonInteracting);

            if (tmppar != nullptr) {
              destsurf = &tmppar->associatedSurface();
            }
          }
        }

        if (matvec_used) cache.m_matTempStore.push_back( std::move(matvec) );
        matvec.reset( m_extrapolator->extrapolateM(ctx,
                                                   *startmatpar1, 
                                                   *destsurf, 
                                                   oppositeMomentum, 
                                                   false, matEffects) );
        matvec_used=false;

        if (matvec && !matvec->empty()) {
          for (int i = (int)matvec->size() - 1; i > -1; i--) {
            const MaterialEffectsBase *meb = (*matvec)[i]->materialEffectsOnTrack();
            if (meb) {
              if (meb->derivedType() == MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK) {
                const MaterialEffectsOnTrack *meot = static_cast < const MaterialEffectsOnTrack * >(meb);
                std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(*meot);
                const TrackParameters * newpars = (*matvec)[i]->trackParameters() != nullptr ? (*matvec)[i]->trackParameters()->clone() : nullptr;
                meff->setSigmaDeltaE(0);
                matstates.push_back(std::make_unique<GXFTrackState>(
                  std::move(meff),
                  std::unique_ptr<const TrackParameters>(newpars)
                ));
                matvec_used=true;
              }
            }
          }
        }
      }
    }

    if ((lastidhit != nullptr) && trajectory.numberOfSiliconHits() > 0 && cache.m_idmat) {
      DistanceSolution distsol = lastidhit->associatedSurface().straightLineDistanceEstimate(
        refpar->position(),
        refpar->momentum().unit()
      );

      double distance = getDistance(distsol);

      if (distance > 0 && distsol.numberOfSolutions() > 0) {
        ATH_MSG_DEBUG("Obtaining downstream ID layers from Extrapolator");
        const Surface *destsurf = &lastidhit->associatedSurface();
        std::unique_ptr<const TrackParameters> tmppar;
        std::unique_ptr<Surface> calosurf;
        if (firstmuonhit != nullptr) {
          if (cache.m_caloEntrance == nullptr) {
            const TrackingGeometry *geometry = trackingGeometry(cache,ctx);

            if (geometry != nullptr) {
              cache.m_caloEntrance = geometry->trackingVolume("InDet::Containers::InnerDetector");
            } else {
              ATH_MSG_ERROR("Tracking Geometry not available");
            }
          }

          if (cache.m_caloEntrance == nullptr) {
            ATH_MSG_ERROR("calo entrance not available");
          } else {
            tmppar = m_extrapolator->extrapolateToVolume(ctx,
                                                         *startmatpar2,
                                                         *cache.m_caloEntrance,
                                                         Trk::alongMomentum,
                                                         Trk::nonInteracting);
          }

          if (tmppar != nullptr) {
            const CylinderSurface *cylcalosurf = nullptr;
            
            if (tmppar->associatedSurface().type() == Trk::SurfaceType::Cylinder)
              cylcalosurf = static_cast<const CylinderSurface *>(&tmppar->associatedSurface());
            
            const DiscSurface *disccalosurf = nullptr;
            
            if (tmppar->associatedSurface().type() == Trk::SurfaceType::Disc)
              disccalosurf = static_cast<const DiscSurface *>(&tmppar->associatedSurface());
            
            if (cylcalosurf != nullptr) {
              Amg::Transform3D trans = Amg::Transform3D(cylcalosurf->transform());
              const CylinderBounds & cylbounds = cylcalosurf->bounds();
              double radius = cylbounds.r();
              double hlength = cylbounds.halflengthZ();
              calosurf = std::make_unique<CylinderSurface>(trans, radius - 1, hlength);
            } else if (disccalosurf != nullptr) {
              double newz = (
                disccalosurf->center().z() > 0 ? 
                disccalosurf->center().z() - 1 : 
                disccalosurf->center().z() + 1
              );

              Amg::Vector3D newpos(
                disccalosurf->center().x(),
                disccalosurf->center().y(), 
                newz
              );
              
              Amg::Transform3D trans = (disccalosurf->transform());
              trans.translation() << newpos;
              
              const DiscBounds *discbounds = static_cast<const DiscBounds *>(&disccalosurf->bounds());
              double rmin = discbounds->rMin();
              double rmax = discbounds->rMax();
              calosurf = std::make_unique<DiscSurface>(trans, rmin, rmax);
            }
            destsurf = calosurf.release();
          }
        }

        if (matvec_used) cache.m_matTempStore.push_back( std::move(matvec) );
        matvec.reset(m_extrapolator->extrapolateM(
          ctx, *startmatpar2, *destsurf, alongMomentum, false, matEffects));
        matvec_used = false;

        if (matvec && !matvec->empty()) {
          for (const auto & i : *matvec) {
            const Trk::MaterialEffectsBase * meb = i->materialEffectsOnTrack();
            
            if (meb) {
              if (meb->derivedType() == MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK) {
                const MaterialEffectsOnTrack *meot = static_cast<const MaterialEffectsOnTrack *>(meb);
                std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(*meot);
                if (cache.m_fiteloss && (meot->energyLoss() != nullptr)) {
                  meff->setSigmaDeltaE(meot->energyLoss()->sigmaDeltaE());
                }

                if (matEffects == electron && cache.m_asymeloss) {
                  meff->setDeltaE(-5);
                  
                  if (trajectory.numberOfTRTHits() == 0) {
                    meff->setScatteringSigmas(0, 0);
                  }
                  
                  meff->setSigmaDeltaE(50);
                }

                const TrackParameters * newparams = i->trackParameters() != nullptr ? i->trackParameters()->clone() : nullptr;

                matstates.push_back(std::make_unique<GXFTrackState>(
                  std::move(meff),
                  std::unique_ptr<const TrackParameters>(newparams)
                ));
                matvec_used=true;
              }
            }
          }
        } else {
          ATH_MSG_WARNING("No material layers collected from Extrapolator");
        }
      }
    }

    if (cache.m_calomat && (firstmuonhit != nullptr) && (firstidhit != nullptr)) {
      const IPropagator *prop = &*m_propagator;

      std::vector<MaterialEffectsOnTrack> calomeots = m_calotool->extrapolationSurfacesAndEffects(
        *m_navigator->highestVolume(ctx), 
        *prop,
        *lastidpar,
        firstmuonhit->associatedSurface(),
        alongMomentum, 
        muon
      );

      if (calomeots.empty()) {
        ATH_MSG_WARNING("No material layers collected in calorimeter");
        return;
      }

      std::unique_ptr<const TrackParameters> prevtrackpars = unique_clone(lastidpar);
      if (lasthit == lastmuonhit) {
        for (int i = 0; i < (int) calomeots.size(); i++) {
          PropDirection propdir = alongMomentum;
    
          std::unique_ptr<const TrackParameters> layerpar(m_propagator->propagateParameters(
            ctx,
            *prevtrackpars,
            calomeots[i].associatedSurface(), 
            propdir,
            false,
            trajectory.m_fieldprop,
            nonInteracting
          ));

          if (layerpar == nullptr) {
            cache.incrementFitStatus(S_PROPAGATION_FAIL);
            return;
          }

          std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(calomeots[i]);
          
          if (i == 2) {
            lastcalopar = layerpar.get();
          }
          
          if (i == 1) {
            double qoverp = layerpar->parameters()[Trk::qOverP];
            double qoverpbrem = 0;

            if (
              npseudomuon2 < 2 && 
              (firstmuonpar != nullptr) && 
              std::abs(firstmuonpar->parameters()[Trk::qOverP]) > 1.e-9
            ) {
              qoverpbrem = firstmuonpar->parameters()[Trk::qOverP];
            } else {
              double sign = (qoverp > 0) ? 1 : -1;
              qoverpbrem = sign / (1 / std::abs(qoverp) - std::abs(calomeots[i].energyLoss()->deltaE()));
            }

            const AmgVector(5) & newpar = layerpar->parameters();

            layerpar = layerpar->associatedSurface().createUniqueTrackParameters(
              newpar[0], newpar[1], newpar[2], newpar[3], qoverpbrem, std::nullopt
            );
            meff->setdelta_p(1000 * (qoverpbrem - qoverp));
          }

          matstates.push_back(std::make_unique<GXFTrackState>(
            std::move(meff),
            std::unique_ptr<const TrackParameters>(layerpar != nullptr ? layerpar->clone() : nullptr)
          ));
          prevtrackpars = std::move(layerpar);
        }
      }

      if (
        firsthit == firstmuonhit && 
        (!cache.m_getmaterialfromtrack || lasthit == lastidhit)
      ) {
        prevtrackpars = unique_clone(firstidpar);
        for (int i = 0; i < (int) calomeots.size(); i++) {
          PropDirection propdir = oppositeMomentum;
          std::unique_ptr<const TrackParameters> layerpar(m_propagator->propagateParameters(
            ctx,
            *prevtrackpars,
            calomeots[i].associatedSurface(), 
            propdir,
            false,
            trajectory.m_fieldprop,
            nonInteracting
          ));
          
          if (layerpar == nullptr) {
            cache.incrementFitStatus(S_PROPAGATION_FAIL);
            return;
          }
          
          std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(calomeots[i]);
          
          if (i == 2) {
            firstcalopar = unique_clone(layerpar.get()).release();
          }

          prevtrackpars = unique_clone(layerpar.get());
          
          if (i == 1) {
            double qoverpbrem = layerpar->parameters()[Trk::qOverP];
            double qoverp = 0;
          
            if (
              npseudomuon1 < 2 && 
              (lastmuonpar != nullptr) && 
              std::abs(lastmuonpar->parameters()[Trk::qOverP]) > 1.e-9
            ) {
              qoverp = lastmuonpar->parameters()[Trk::qOverP];
            } else {
              double sign = (qoverpbrem > 0) ? 1 : -1;
              qoverp = sign / (1 / std::abs(qoverpbrem) + std::abs(calomeots[i].energyLoss()->deltaE()));
            }

            meff->setdelta_p(1000 * (qoverpbrem - qoverp));
            const AmgVector(5) & newpar = layerpar->parameters();

            prevtrackpars = layerpar->associatedSurface().createUniqueTrackParameters(
              newpar[0], newpar[1], newpar[2], newpar[3], qoverp, std::nullopt
            );
          }

          matstates.insert(matstates.begin(), std::make_unique<GXFTrackState>(std::move(meff), std::move(layerpar)));
        }
      }
    }

    if (lasthit == lastmuonhit && cache.m_extmat) {
      std::unique_ptr<const Trk::TrackParameters> muonpar1;
      
      if (lastcalopar != nullptr) {
        if (cache.m_msEntrance == nullptr) {
          const TrackingGeometry *geometry = trackingGeometry(cache,ctx);

          if (geometry != nullptr) {
            cache.m_msEntrance = geometry->trackingVolume("MuonSpectrometerEntrance");
          } else {
            ATH_MSG_ERROR("Tracking Geometry not available");
          }
        }

        if (cache.m_msEntrance == nullptr) {
          ATH_MSG_ERROR("MS entrance not available");
        } else if (cache.m_msEntrance->inside(lastcalopar->position())) {
          muonpar1 = m_extrapolator->extrapolateToVolume(ctx,
                                                         *lastcalopar,
                                                         *cache.m_msEntrance,
                                                         Trk::alongMomentum,
                                                         Trk::nonInteracting);

          if (muonpar1 != nullptr) {
            Amg::Vector3D trackdir = muonpar1->momentum().unit();
            Amg::Vector3D curvZcrossT = -(trackdir.cross(Amg::Vector3D(0, 0, 1)));
            Amg::Vector3D curvU = curvZcrossT.unit();
            Amg::Vector3D curvV = trackdir.cross(curvU);
            Amg::RotationMatrix3D rot = Amg::RotationMatrix3D::Identity();
            rot.col(0) = curvU;
            rot.col(1) = curvV;
            rot.col(2) = trackdir;
            Amg::Transform3D trans;
            trans.linear().matrix() << rot;
            trans.translation() << muonpar1->position() - .1 * trackdir;
            PlaneSurface curvlinsurf(trans);

            std::unique_ptr<const TrackParameters> curvlinpar(m_extrapolator->extrapolateDirectly(
              ctx,
              *muonpar1, 
              curvlinsurf,
              Trk::alongMomentum,
              Trk::nonInteracting != 0u
            ));

            if (curvlinpar != nullptr) {
              muonpar1 = std::move(curvlinpar);
            }
          }
        } else {
          muonpar1 = std::unique_ptr<TrackParameters>(lastcalopar->clone());
        }
      } else {
        muonpar1 = std::unique_ptr<TrackParameters>(refpar->clone());
      }

      DistanceSolution distsol;
      
      if (muonpar1 != nullptr) {
        distsol = lastmuonhit->associatedSurface().straightLineDistanceEstimate(
          muonpar1->position(), 
          muonpar1->momentum().unit()
        );
      }

      double distance = getDistance(distsol);

      if ((distance > 0) and(distsol.numberOfSolutions() >
                             0) and (firstmuonhit != nullptr)) {
        distsol = firstmuonhit->associatedSurface().straightLineDistanceEstimate(
          muonpar1->position(),
          muonpar1->momentum().unit()
        );

        distance = 0;
        
        if (distsol.numberOfSolutions() == 1) {
          distance = distsol.first();
        } else if (distsol.numberOfSolutions() == 2) {
          distance = (
            std::abs(distsol.first()) < std::abs(distsol.second()) ? 
            distsol.first() : 
            distsol.second()
          );
        }

        if (distance < 0 && distsol.numberOfSolutions() > 0 && (firstidhit == nullptr)) {
          if (firstmuonpar != nullptr) {
            AmgVector(5) newpars = firstmuonpar->parameters();
            
            if (trajectory.m_straightline && m_p != 0) {
              newpars[Trk::qOverP] = 1 / m_p;
            }
            
            muonpar1 = firstmuonpar->associatedSurface().createUniqueTrackParameters(
              newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
            );
          } else {
            std::unique_ptr<const TrackParameters> tmppar(m_propagator->propagateParameters(
              ctx,
              *muonpar1,
              firstmuonhit->associatedSurface(),
              oppositeMomentum, 
              false,
              trajectory.m_fieldprop,
              nonInteracting
            ));

            if (tmppar != nullptr) {
              muonpar1 = std::move(tmppar);
            }
          }
        }

        const TrackParameters *prevtp = muonpar1.get();
        ATH_MSG_DEBUG("Obtaining downstream layers from Extrapolator");
        if (matvec_used) cache.m_matTempStore.push_back( std::move(matvec) );
        matvec.reset(m_extrapolator->extrapolateM(ctx,
                                                  *prevtp,
                                                  states.back()->associatedSurface(),
                                                  alongMomentum,
                                                  false,
                                                  Trk::nonInteractingMuon));
        matvec_used = false;

        if (matvec && matvec->size() > 1000 && m_rejectLargeNScat) {
          ATH_MSG_DEBUG("too many scatterers: " << matvec->size());
          return;
        }
        
        if (matvec && !matvec->empty()) {
          for (int j = 0; j < (int) matvec->size(); j++) {
            const MaterialEffectsBase *meb = (*matvec)[j]->materialEffectsOnTrack();
            
            if (meb) {
              if ((meb->derivedType() ==  MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK) and (j < (int) matvec->size() - 1)) {
                const MaterialEffectsOnTrack *meot = static_cast<const MaterialEffectsOnTrack *>(meb);
                std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(*meot);
              
                if (
                  !trajectory.m_straightline && 
                  (meot->energyLoss() != nullptr) && 
                  std::abs(meff->deltaE()) > 25 && 
                  std::abs((*matvec)[j]->trackParameters()->position().z()) < 13000
                ) {
                  meff->setSigmaDeltaE(meot->energyLoss()->sigmaDeltaE());
                }

                const TrackParameters * newparams = (*matvec)[j]->trackParameters() != nullptr ? (*matvec)[j]->trackParameters()->clone() : nullptr;

                matstates.push_back(std::make_unique<GXFTrackState>(
                  std::move(meff),
                  std::unique_ptr<const TrackParameters>(newparams)
                ));
                matvec_used=true;
              }
            }
          }
        }
      }
    }

    if (firsthit == firstmuonhit && cache.m_extmat && (firstcalopar != nullptr)) {
      std::unique_ptr<const Trk::TrackParameters> muonpar1;
    
      if (cache.m_msEntrance == nullptr) {
        const TrackingGeometry *geometry = trackingGeometry(cache,ctx);

        if (geometry != nullptr) {
          cache.m_msEntrance = geometry->trackingVolume("MuonSpectrometerEntrance");
        } else {
          ATH_MSG_ERROR("Tracking Geometry not available");
        }
      }

      if (cache.m_msEntrance == nullptr) {
        ATH_MSG_ERROR("MS entrance not available");
      } else if (cache.m_msEntrance->inside(firstcalopar->position())) {
        muonpar1 = m_extrapolator->extrapolateToVolume(ctx,
                                                       *firstcalopar,
                                                       *cache.m_msEntrance,
                                                       Trk::oppositeMomentum,
                                                       Trk::nonInteracting);

        if (muonpar1 != nullptr) {
          Amg::Vector3D trackdir = muonpar1->momentum().unit();
          Amg::Vector3D curvZcrossT = -(trackdir.cross(Amg::Vector3D(0, 0, 1)));
          Amg::Vector3D curvU = curvZcrossT.unit();
          Amg::Vector3D curvV = trackdir.cross(curvU);
          Amg::RotationMatrix3D rot = Amg::RotationMatrix3D::Identity();
          rot.col(0) = curvU;
          rot.col(1) = curvV;
          rot.col(2) = trackdir;
          Amg::Transform3D trans;
          trans.linear().matrix() << rot;
          trans.translation() << muonpar1->position() - .1 * trackdir;
          PlaneSurface curvlinsurf(trans);

          std::unique_ptr<const TrackParameters> curvlinpar(m_extrapolator->extrapolateDirectly(
            ctx,
            *muonpar1, 
            curvlinsurf,
            Trk::oppositeMomentum,
            Trk::nonInteracting != 0u
          ));
          
          if (curvlinpar != nullptr) {
            muonpar1 = std::move(curvlinpar);
          }
        }
      } else {
        muonpar1 = std::unique_ptr<const TrackParameters>(firstcalopar->clone());
      }
      

      DistanceSolution distsol;
      
      if (muonpar1 != nullptr) {
        distsol = firstmuonhit->associatedSurface().straightLineDistanceEstimate(
          muonpar1->position(),
          muonpar1->momentum().unit()
        );
      }

      double distance = getDistance(distsol);

      if (distance < 0 && distsol.numberOfSolutions() > 0) {
        const TrackParameters *prevtp = muonpar1.get();
        ATH_MSG_DEBUG("Collecting upstream muon material from extrapolator");
        if (matvec_used) cache.m_matTempStore.push_back( std::move(matvec) );
        matvec.reset(m_extrapolator->extrapolateM(ctx,
                                                  *prevtp,
                                                  states[0]->associatedSurface(),
                                                  oppositeMomentum,
                                                  false,
                                                  Trk::nonInteractingMuon));
        matvec_used = false;

        if (matvec && !matvec->empty()) {
          ATH_MSG_DEBUG("Retrieved " << matvec->size() << " material states");
          
          for (int j = 0; j < (int) matvec->size(); j++) {
            const MaterialEffectsBase *meb = (*matvec)[j]->materialEffectsOnTrack();
            
            if (meb != nullptr) {
              

              if ((meb->derivedType() == MaterialEffectsBase::MATERIAL_EFFECTS_ON_TRACK) && j < (int) matvec->size() - 1) {
                const MaterialEffectsOnTrack *meot = static_cast<const MaterialEffectsOnTrack *>(meb);
                std::unique_ptr<GXFMaterialEffects> meff = std::make_unique<GXFMaterialEffects>(*meot);
                
                if (
                  !trajectory.m_straightline && 
                  (meot->energyLoss() != nullptr) && 
                  std::abs(meff->deltaE()) > 25 && 
                  std::abs((*matvec)[j]->trackParameters()->position().z()) < 13000
                ) {
                  meff->setSigmaDeltaE(meot->energyLoss()->sigmaDeltaE());
                }

                const TrackParameters* tmpparams =
                  (*matvec)[j]->trackParameters() != nullptr
                    ? (*matvec)[j]->trackParameters()->clone()
                    : nullptr;

                matstates.insert(matstates.begin(), std::make_unique<GXFTrackState>(
                  std::move(meff),
                  std::unique_ptr<const TrackParameters>(tmpparams)
                ));
                matvec_used=true;
              }
            }
          }
        }
      }
    }

    ATH_MSG_DEBUG("Number of layers: " << matstates.size());

    // Now insert the material states into the trajectory
    std::vector<std::unique_ptr<GXFTrackState>> & newstates = states;
    std::vector<std::unique_ptr<GXFTrackState>> oldstates = std::move(newstates);

    newstates.clear();
    newstates.reserve(oldstates.size() + matstates.size());

    int layerno = 0;
    int firstlayerno = -1;
    
    if (cache.m_acceleration) {
      trajectory.addBasicState(std::move(oldstates[0]));
    }
    
    double cosphi = std::cos(refpar->parameters()[Trk::phi0]);
    double sinphi = std::sin(refpar->parameters()[Trk::phi0]);
    
    for (int i = cache.m_acceleration ? 1 : 0; i < (int) oldstates.size(); i++) {
      bool addlayer = true;

      while (addlayer && layerno < (int) matstates.size()) {
        addlayer = false;
        const TrackParameters *layerpar = matstates[layerno]->trackParameters();

        DistanceSolution distsol = oldstates[i]->associatedSurface().straightLineDistanceEstimate(
          layerpar->position(),
          layerpar->momentum().unit()
        );

        double distance = getDistance(distsol);

        if (distance > 0 && distsol.numberOfSolutions() > 0) {
          addlayer = true;
        }

        if (layerpar->associatedSurface().type() == Trk::SurfaceType::Cylinder) {
          double cylinderradius = layerpar->associatedSurface().bounds().r();
          double trackimpact = std::abs(-refpar->position().x() * sinphi + refpar->position().y() * cosphi);
          
          if (trackimpact > cylinderradius - 5 * mm) {
            layerno++;
            continue;
          }
        }

        if (i == (int) oldstates.size() - 1) {
          addlayer = true;
        }
        
        if (addlayer) {
          GXFMaterialEffects *meff = matstates[layerno]->materialEffects();
         
          if (meff->sigmaDeltaPhi() > .4 || (meff->sigmaDeltaPhi() == 0 && meff->sigmaDeltaE() <= 0)) {
            if (meff->sigmaDeltaPhi() > .4) {
              ATH_MSG_DEBUG("Material state with excessive scattering, skipping it");
            }

            if (meff->sigmaDeltaPhi() == 0) {
              ATH_MSG_WARNING("Material state with zero scattering, skipping it");
            }
            
            layerno++;
            continue;
          }

          if (firstlayerno < 0) {
            firstlayerno = layerno;
          }

          trajectory.addMaterialState(std::move(matstates[layerno]));
          
          if ((layerpar != nullptr) && matEffects != pion && matEffects != muon) {
            const TrackingVolume *tvol = m_navigator->volume(ctx,layerpar->position());
            const Layer *lay = nullptr;

            if (tvol != nullptr) {
              lay = (tvol->closestMaterialLayer(layerpar->position(),layerpar->momentum().normalized())).object;
            }

            const MaterialProperties *matprop = lay != nullptr ? lay->fullUpdateMaterialProperties(*layerpar) : nullptr;
            meff->setMaterialProperties(matprop);
          }

          layerno++;
        }
      }
      trajectory.addBasicState(std::move(oldstates[i]));
    }

    ATH_MSG_DEBUG("Total X0: " << trajectory.totalX0() << " total eloss: " << trajectory.totalEnergyLoss());

    if (matvec_used) cache.m_matTempStore.push_back( std::move(matvec) );
 }

  std::unique_ptr<const TrackParameters> GlobalChi2Fitter::makePerigee(
    Cache & cache,
    const TrackParameters & param,
    ParticleHypothesis matEffects
  ) const {
    const PerigeeSurface *persurf = nullptr;
    
    if (param.associatedSurface().type() == Trk::SurfaceType::Perigee)
      persurf = static_cast<const PerigeeSurface *>(&param.associatedSurface());

    if ((persurf != nullptr) && (!cache.m_acceleration || persurf->center().perp() > 5)) {
      const AmgVector(5) & pars = param.parameters();
      return param.associatedSurface().createUniqueTrackParameters(
        pars[0], pars[1], pars[2], pars[3], pars[4], std::nullopt
      );
    }
    
    if (cache.m_acceleration) {
      return nullptr;
    }
    
    PerigeeSurface tmppersf;
    std::unique_ptr<const TrackParameters> per(m_extrapolator->extrapolate(
      Gaudi::Hive::currentContext(),param, tmppersf, oppositeMomentum, false, matEffects));

    if (per == nullptr) {
      ATH_MSG_DEBUG("Cannot make Perigee with starting parameters");
      return nullptr;
    }
    
    return per;
  }

  Track *GlobalChi2Fitter::myfit(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory & trajectory,
    const TrackParameters & param,
    const RunOutlierRemoval runOutlier,
    const ParticleHypothesis matEffects
  ) const {
    ATH_MSG_DEBUG("--> entering GlobalChi2Fitter::myfit_helper");
    cache.m_fittercode = FitterStatusCode::Success;
    trajectory.m_straightline = m_straightlineprop;
    
    if (!trajectory.m_straightline) {
      if (trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() == trajectory.numberOfHits()) {
        trajectory.m_straightline = !cache.m_field_cache.solenoidOn();
      } else if ((trajectory.prefit() == 0) && trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() == 0) {
        trajectory.m_straightline = !cache.m_field_cache.toroidOn();
      } else {
        trajectory.m_straightline = (!cache.m_field_cache.solenoidOn() && !cache.m_field_cache.toroidOn());
      }
    }
    
    trajectory.m_fieldprop = trajectory.m_straightline ? Trk::NoField : Trk::FullField;
    cache.m_lastiter = 0;

    Amg::SymMatrixX lu;

    if (trajectory.numberOfPerigeeParameters() == -1) {
      cache.incrementFitStatus(S_FITS);
      if (trajectory.m_straightline) {
        trajectory.setNumberOfPerigeeParameters(4);
      } else {
        trajectory.setNumberOfPerigeeParameters(5);
      }
    }
    
    if (trajectory.nDOF() < 0) {
      ATH_MSG_DEBUG("Not enough measurements, reject track");
      return nullptr;
    }

    cache.m_phiweight.clear();
    cache.m_firstmeasurement.clear();
    cache.m_lastmeasurement.clear();

    if (matEffects != nonInteracting && param.parameters()[Trk::qOverP] == 0 && m_p == 0) {
      ATH_MSG_WARNING("Attempt to apply material corrections with q/p=0, reject track");
      return nullptr;
    }

    if (matEffects == Trk::electron && trajectory.m_straightline) {
      ATH_MSG_WARNING("Electron fit requires helix track model");
      return nullptr;
    }

    double mass = Trk::ParticleMasses::mass[matEffects];
    trajectory.setMass(mass);

    ATH_MSG_DEBUG("start param: " << param << " pos: " << param.position() << " pt: " << param.pT());

    std::unique_ptr<const TrackParameters> per = makePerigee(cache, param, matEffects);

    if (!cache.m_acceleration && (per == nullptr)) {
      cache.m_fittercode = FitterStatusCode::ExtrapolationFailure;
      cache.incrementFitStatus(S_PROPAGATION_FAIL);
      ATH_MSG_DEBUG("Propagation to perigee failed 1");
      return nullptr;
    }
    
    if (matEffects != Trk::nonInteracting && !cache.m_matfilled) {
      if (
        cache.m_fastmat && 
        cache.m_acceleration &&
        trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() == trajectory.numberOfHits() && 
        (m_matupdator.empty() || (m_trackingGeometryReadKey.key().empty()))
      ) {
        ATH_MSG_WARNING("Tracking Geometry Service and/or Material Updator Tool not configured");
        ATH_MSG_WARNING("Falling back to slow material collection");
        
        cache.m_fastmat = false;
      }
      
      if (
        !cache.m_fastmat || 
        !cache.m_acceleration ||
        trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() != trajectory.numberOfHits()
      ) {
        addMaterial(ctx, cache, trajectory, per != nullptr ? per.get() : &param, matEffects);
      } else {
        addIDMaterialFast(
          ctx, cache, trajectory, per != nullptr ? per.get() : &param, matEffects);
      }
    }

    if (cache.m_acceleration && (trajectory.referenceParameters() == nullptr) && (per == nullptr)) {
      Amg::Vector3D vertex;
      
      if (trajectory.numberOfScatterers() >= 2) {
        GXFTrackState *scatstate = nullptr;
        GXFTrackState *scatstate2 = nullptr;
        int scatindex = 0;

        for (std::vector<std::unique_ptr<GXFTrackState>>::iterator it =
               trajectory.trackStates().begin();
             it != trajectory.trackStates().end();
             ++it) {
          if ((**it).getStateType(TrackStateOnSurface::Scatterer)) {
            if (
              scatindex == trajectory.numberOfScatterers() / 2 || 
              (**it).materialEffects()->deltaE() == 0
            ) {
              scatstate2 = (*it).get();
              break;
            }
            
            scatindex++;
            scatstate = (*it).get();
          }
        }

        // @TODO coverity complains about a possible null pointer dereferencing in scatstate->... or scatstate2->...
        // it seems to me that if (**it).materialEffects()->deltaE()==0 of the first scatterer
        // than scatstate will be NULL.
        if ((scatstate == nullptr) || (scatstate2 == nullptr)) {
          throw std::logic_error("Invalid scatterer");
        }
        
        vertex = .49 * (scatstate->position() + scatstate2->position());
      } else {
        int nstates = (int) trajectory.trackStates().size();
        vertex = .49 * (
          trajectory.trackStates()[nstates / 2 - 1]->position() +
          trajectory.trackStates()[nstates / 2]->position()
        );
      }

      PerigeeSurface persurf(vertex);
      std::unique_ptr<const TrackParameters> nearestpar;
      double mindist = 99999;
      std::vector < GXFTrackState * >mymatvec;

      for (auto & it : trajectory.trackStates()) {
        if ((*it).trackParameters() == nullptr) {
          continue;
        }

        double distance = persurf
                            .straightLineDistanceEstimate(
                              (*it).trackParameters()->position(),
                              (*it).trackParameters()->momentum().unit())
                            .first();

        bool insideid = (
          (cache.m_caloEntrance == nullptr) || 
          cache.m_caloEntrance->inside((*it).trackParameters()->position())
        );

        if (
          (((*it).measurement() != nullptr) && insideid) || (
            ((*it).materialEffects() != nullptr) && 
            distance > 0 && (
              (*it).materialEffects()->deltaE() == 0 || 
              ((*it).materialEffects()->sigmaDeltaPhi() == 0 && 
              !insideid) || 
              (*it).materialEffects()->deltaPhi() != 0
            )
          )
        ) {
          double dist = ((*it).trackParameters()->position() - vertex).perp();
          if (dist < mindist) {
            mindist = dist;
            nearestpar = unique_clone((*it).trackParameters());
            mymatvec.clear();
            continue;
          }
        }
        
        if (((*it).materialEffects() != nullptr) && distance > 0) {
          mymatvec.push_back(it.get());
        }
      }
      
      if (nearestpar == nullptr) {
        nearestpar = unique_clone(&param);
      }

      for (auto & state : mymatvec) {
        Trk::PropDirection propdir = Trk::alongMomentum;
        const Surface &matsurf = state->associatedSurface();
        DistanceSolution distsol = matsurf.straightLineDistanceEstimate(
          nearestpar->position(), nearestpar->momentum().unit());

        double distance = getDistance(distsol);
        
        if (distance < 0 && distsol.numberOfSolutions() > 0) {
          propdir = oppositeMomentum;
        }
        
        std::unique_ptr<const TrackParameters> tmppar(m_propagator->propagateParameters(
          ctx,
          *nearestpar, 
          matsurf, 
          propdir,
          false,
          trajectory.m_fieldprop,
          Trk::nonInteracting
        ));
        
        if (tmppar == nullptr) {
          propdir = (propdir == oppositeMomentum) ? alongMomentum : oppositeMomentum;
          tmppar = m_propagator->propagateParameters(
            ctx,
            *nearestpar, 
            matsurf, 
            propdir,
            false, 
            trajectory.m_fieldprop,
            Trk::nonInteracting
          );
          
          if (tmppar == nullptr) {
            cache.m_fittercode = FitterStatusCode::ExtrapolationFailure;
            cache.incrementFitStatus(S_PROPAGATION_FAIL);

            ATH_MSG_DEBUG("Propagation to perigee failed 2");

            return nullptr;
          }
        }

        AmgVector(5) newpars = tmppar->parameters();
        
        if (state->materialEffects()->sigmaDeltaE() > 0) {
          newpars[Trk::qOverP] += .001 * state->materialEffects()->delta_p();
        } else if (newpars[Trk::qOverP] != 0) {
          double sign = (newpars[Trk::qOverP] > 0) ? 1 : -1;
          double de = std::abs(state->materialEffects()->deltaE());
          double oldp = std::abs(1 / newpars[Trk::qOverP]);
          double newp2 = oldp * oldp - 2 * de * std::sqrt(mass * mass + oldp * oldp) + de * de;
          if (newp2 > 0) {
            newpars[Trk::qOverP] = sign / std::sqrt(newp2);
          }
        }
        
        nearestpar = tmppar->associatedSurface().createUniqueTrackParameters(
          newpars[0], newpars[1], newpars[2], newpars[3], newpars[4], std::nullopt
        );
      }
      
      std::unique_ptr<Trk::TrackParameters> tmpPars(m_propagator->propagateParameters(
        ctx,
        *nearestpar, 
        persurf,
        Trk::anyDirection, 
        false,
        trajectory.m_fieldprop,
        Trk::nonInteracting
      ));

      // Parameters are at a Perigee surface so they are perigee parameters
      if (tmpPars != nullptr) {
        per.reset(static_cast < const Perigee *>(tmpPars.release()));
      }
      
      if ((per != nullptr) && (matEffects == Trk::proton || matEffects == Trk::kaon)) {
        double sign = (per->parameters()[Trk::qOverP] < 0) ? -1. : 1.;
        double oldp = 1. / std::abs(per->parameters()[Trk::qOverP]);
        double toteloss = std::abs(trajectory.totalEnergyLoss());
        double newp = std::sqrt(oldp * oldp + 2 * toteloss * std::sqrt(oldp * oldp + mass * mass) + toteloss * toteloss);
        AmgVector(5) params = per->parameters();
        params[Trk::qOverP] = sign / newp;
        
        per = per->associatedSurface().createUniqueTrackParameters(
          params[0], params[1], params[2], params[3], params[4], std::nullopt
        );
      }
      
      if (per == nullptr) {
        cache.m_fittercode = FitterStatusCode::ExtrapolationFailure;
        cache.incrementFitStatus(S_PROPAGATION_FAIL);
        ATH_MSG_DEBUG("Propagation to perigee failed 3");

        return nullptr;
      }
      
      PerigeeSurface persurf2(per->position());
      per = persurf2.createUniqueTrackParameters(
        0, 
        0, 
        per->parameters()[Trk::phi],
        per->parameters()[Trk::theta],
        per->parameters()[Trk::qOverP], 
        std::nullopt
      );
    } else if (per == nullptr) {
      per = makePerigee(cache, param, matEffects);
    }
    
    if ((per == nullptr) && (trajectory.referenceParameters() == nullptr)) {
      cache.m_fittercode = FitterStatusCode::ExtrapolationFailure;
      cache.incrementFitStatus(S_PROPAGATION_FAIL);
      ATH_MSG_DEBUG("Propagation to perigee failed 4");

      return nullptr;
    }
    
    if (trajectory.m_straightline && (per != nullptr)) {
      if (trajectory.numberOfPerigeeParameters() == -1) {
        trajectory.setNumberOfPerigeeParameters(4);
      }
      
      const AmgVector(5) & pars = per->parameters();
      per = per->associatedSurface().createUniqueTrackParameters(
        pars[0], pars[1], pars[2], pars[3], 0, std::nullopt
      );
    } else if (trajectory.numberOfPerigeeParameters() == -1) {
      trajectory.setNumberOfPerigeeParameters(5);
    }

    if (per != nullptr) {
      trajectory.setReferenceParameters(std::move(per));
    }

    int nfitpar = trajectory.numberOfFitParameters();
    int nperpars = trajectory.numberOfPerigeeParameters();
    int nscat = trajectory.numberOfScatterers();
    int nbrem = trajectory.numberOfBrems();

    Eigen::MatrixXd a;
    Eigen::MatrixXd a_inv;
    a.resize(nfitpar, nfitpar);
    
    Amg::VectorX b(nfitpar);

    Amg::MatrixX derivPool(5, nfitpar);
    derivPool.setZero();

    for (std::unique_ptr<GXFTrackState> & state : trajectory.trackStates()) {
      if (state->materialEffects() != nullptr) {
        continue;
      }
      state->setDerivatives(derivPool);
    }
    
    bool doderiv = true;
    int it = 0;
    int tmpminiter = cache.m_miniter;
    
    for (; it < m_maxit; ++it) {
      cache.m_lastiter = it;
      
      if (it >= m_maxit - 1) {
        ATH_MSG_DEBUG("Fit did not converge");
        cache.m_fittercode = FitterStatusCode::NoConvergence;
        cache.incrementFitStatus(S_NOT_CONVERGENT);
        cache.m_miniter = tmpminiter;
        return nullptr;
      }
      
      if (!trajectory.converged()) {
        cache.m_fittercode =
          runIteration(ctx, cache, trajectory, it, a, b, lu, doderiv);
        if (cache.m_fittercode != FitterStatusCode::Success) {
          if (cache.m_fittercode == FitterStatusCode::ExtrapolationFailure) {
            cache.incrementFitStatus(S_PROPAGATION_FAIL);
          } else if (cache.m_fittercode == FitterStatusCode::InvalidAngles) {
            cache.incrementFitStatus(S_INVALID_ANGLES);
          } else if (cache.m_fittercode == FitterStatusCode::ExtrapolationFailureDueToSmallMomentum) {
            cache.incrementFitStatus(S_LOW_MOMENTUM);
          }
          cache.m_miniter = tmpminiter;
          return nullptr;
        }
        
        int nhits = trajectory.numberOfHits();
        int ntrthits = trajectory.numberOfTRTHits();
        int nsihits = trajectory.numberOfSiliconHits();
        double redchi2 =  (trajectory.nDOF() > 0) ? trajectory.chi2() / trajectory.nDOF() : 0;
        double prevredchi2 = (trajectory.nDOF() > 0) ? trajectory.prevchi2() / trajectory.nDOF() : 0;
        
        
        if( nsihits > 0 && it > 0 && it < m_maxitPixelROT )
          updatePixelROTs( trajectory, a, b );

        if (
          it > 0 && 
          it < 4 && (
            (redchi2 < prevredchi2 && 
            (redchi2 > prevredchi2 - 1 || redchi2 < 2)) || 
            nsihits + ntrthits == nhits
          ) && 
          (runOutlier || m_trtrecal) && 
          ntrthits > 0
        ) {
          if (it != 1 || nsihits != 0 || trajectory.nDOF() <= 0 || trajectory.chi2() / trajectory.nDOF() <= 3) {
            ATH_MSG_DEBUG("Running TRT cleaner");
            runTrackCleanerTRT(cache, trajectory, a, b, lu, runOutlier, m_trtrecal, it);
            if (cache.m_fittercode != FitterStatusCode::Success) {
              ATH_MSG_DEBUG("TRT cleaner failed, returning null...");
              cache.m_miniter = tmpminiter;
              return nullptr;
            }
          }
        }

        // PHF cut at iteration 3 (to save CPU time)
        int ntrtprechits = trajectory.numberOfTRTPrecHits();
        int ntrttubehits = trajectory.numberOfTRTTubeHits();
        float phf = 1.;
        if (ntrtprechits+ntrttubehits) {
          phf = float(ntrtprechits)/float(ntrtprechits+ntrttubehits);
        }
        if (phf<m_minphfcut && it>=3) {
          if ((ntrtprechits+ntrttubehits)>=15) {
            return nullptr;
          }
        }
        ATH_MSG_DEBUG("Iter = " << it << " | nTRTStates = " << ntrthits
                                << " | nTRTPrecHits = " << ntrtprechits
                                << " | nTRTTubeHits = " << ntrttubehits
                                << " | nOutliers = "
                                << trajectory.numberOfOutliers());

        if (!trajectory.converged()) {
          cache.m_fittercode = updateFitParameters(trajectory, b, lu);
          if (cache.m_fittercode != FitterStatusCode::Success) {
            if (cache.m_fittercode == FitterStatusCode::InvalidAngles) {
              cache.incrementFitStatus(S_INVALID_ANGLES);
            }
            cache.m_miniter = tmpminiter;
            return nullptr;
          }
        }
      } else {
        break;
      }
    }

    cache.m_miniter = tmpminiter;

    if (trajectory.prefit() == 0) {
      // Solve assuming the matrix is SPD.
      // Cholesky Decomposition is used --  could use LDLT

      Eigen::LLT < Eigen::MatrixXd > lltOfW(a);
      if (lltOfW.info() == Eigen::Success) {
        // Solve for x  where Wx = I
        // this is cheaper than invert as invert makes no assumptions about the
        // matrix being symmetric
        int ncols = a.cols();
        Amg::MatrixX weightInvAMG = Amg::MatrixX::Identity(ncols, ncols);
        a_inv = lltOfW.solve(weightInvAMG);
      } else {
        ATH_MSG_DEBUG("matrix inversion failed!");
        cache.incrementFitStatus(S_MAT_INV_FAIL);
        cache.m_fittercode = FitterStatusCode::MatrixInversionFailure;
        return nullptr;
      }
    }
    
    GXFTrajectory *finaltrajectory = &trajectory;
    if (
      (runOutlier || cache.m_sirecal) && 
      trajectory.numberOfSiliconHits() == trajectory.numberOfHits()
    ) {
      calculateTrackErrors(trajectory, a_inv, true);
      finaltrajectory = runTrackCleanerSilicon(ctx,cache, trajectory, a, a_inv, b, runOutlier);
    }

    if (cache.m_fittercode != FitterStatusCode::Success) {
      ATH_MSG_DEBUG("Silicon cleaner failed, returning null...");
      if (finaltrajectory != &trajectory) {
        delete finaltrajectory;
      }
      return nullptr;
    }
    
    if (m_domeastrackpar && (finaltrajectory->prefit() == 0)) {
      calculateTrackErrors(*finaltrajectory, a_inv, false);
    }
    
    if (!cache.m_acceleration && (finaltrajectory->prefit() == 0)) {
      if (nperpars == 5) {
        for (int i = 0; i < a.cols(); i++) {
          a_inv(4, i) *= .001;
          a_inv(i, 4) *= .001;
        }
      }
      
      int scatterPos = nperpars + 2 * nscat;
      for (int bremno = 0; bremno < nbrem; bremno++, scatterPos++) {
        for (int i = 0; i < a.cols(); i++) {
          a_inv(scatterPos, i) *= .001;
          a_inv(i, scatterPos) *= .001;
        }
      }

      AmgSymMatrix(5) errmat;
      errmat.setZero();
      int nperparams = finaltrajectory->numberOfPerigeeParameters();
      for (int i = 0; i < nperparams; i++) {
        for (int j = 0; j < nperparams; j++) {
          (errmat) (j, i) = a_inv(j, i);
        }
      }
      
      if (trajectory.m_straightline) {
        (errmat) (4, 4) = 1e-20;
      }

      const AmgVector(5) & perpars = finaltrajectory->referenceParameters()->parameters();
      std::unique_ptr<const TrackParameters> measper(
        finaltrajectory->referenceParameters()->associatedSurface().createUniqueTrackParameters(
          perpars[0], perpars[1], perpars[2], perpars[3], perpars[4], std::move(errmat)
        )
      );
      
      finaltrajectory->setReferenceParameters(std::move(measper));
      if (m_fillderivmatrix) {
        cache.m_fullcovmat = a_inv;
      }
    }
    
    std::unique_ptr<Track> track = nullptr;

    if (finaltrajectory->prefit() > 0) {
      if (finaltrajectory != &trajectory) {
        delete finaltrajectory;
      }
      return nullptr;
    }
    
    if (finaltrajectory->numberOfOutliers() <= m_maxoutliers || !runOutlier) {
      track = makeTrack(ctx,cache, *finaltrajectory, matEffects);
    } else {
      cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
      cache.m_fittercode = FitterStatusCode::OutlierLogicFailure;
    }

    double cut = (finaltrajectory->numberOfSiliconHits() ==
                  finaltrajectory->numberOfHits())
                   ? 999.0
                   : m_chi2cut.value();

    if (
      runOutlier && 
      (track != nullptr) && (
        track->fitQuality()->numberDoF() != 0 && 
        track->fitQuality()->chiSquared() / track->fitQuality()->numberDoF() > cut
      )
    ) {
      track.reset(nullptr);
      cache.incrementFitStatus(S_HIGH_CHI2);
    }
    
    if (track == nullptr) {
      ATH_MSG_DEBUG("Track rejected");
    }
    
    if (finaltrajectory != &trajectory) {
      delete finaltrajectory;
    }
    
    return track.release();
  }

  void GlobalChi2Fitter::fillResiduals(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory & trajectory, 
    int it,
    Amg::SymMatrixX & a,
    Amg::VectorX & b,
    Amg::SymMatrixX & lu_m, 
    bool &doderiv
  ) const {
    ATH_MSG_DEBUG("fillResiduals");

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    double chi2 = 0;
    int scatno = 0;
    int bremno = 0;
    int measno = 0;
    int nbrem = trajectory.numberOfBrems();
    int nperpars = trajectory.numberOfPerigeeParameters();
    int nfitpars = trajectory.numberOfFitParameters();

    Amg::VectorX & res = trajectory.residuals();
    Amg::MatrixX & weightderiv = trajectory.weightedResidualDerivatives();
    int nidhits = trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits();
    int nsihits = trajectory.numberOfSiliconHits();
    int ntrthits = trajectory.numberOfTRTHits();
    int nhits = trajectory.numberOfHits();
    int nmeas = (int) res.size();
    Amg::VectorX & error = trajectory.errors();
    ParamDefsAccessor paraccessor;
    bool scatwasupdated = false;

    GXFTrackState *state_maxbrempull = nullptr;
    int bremno_maxbrempull = 0;
    double maxbrempull = 0;

    for (int hitno = 0; hitno < (int) states.size(); hitno++) {
      std::unique_ptr<GXFTrackState> & state = states[hitno];
      const TrackParameters *currenttrackpar = state->trackParameters();
      TrackState::MeasurementType hittype = state->measurementType();
      const MeasurementBase *measbase = state->measurement();

      if (state->getStateType(TrackStateOnSurface::Measurement)) {
        if (
          hittype == TrackState::Pseudo && 
          it <= 100 && 
          it > 1 && 
          trajectory.nDOF() != 0 && 
          std::abs((trajectory.prevchi2() - trajectory.chi2()) / trajectory.nDOF()) < 15 && 
          !state->associatedSurface().isFree() && 
          nidhits < trajectory.numberOfHits() && 
          (nperpars == 0 || nidhits > 0) && 
          (!state->isRecalibrated())
        ) {
          Amg::MatrixX covMatrix(1, 1);
          covMatrix(0, 0) = 100;

          std::unique_ptr<const PseudoMeasurementOnTrack> newpseudo = std::make_unique<const PseudoMeasurementOnTrack>(
            LocalParameters(DefinedParameter(currenttrackpar->parameters()[Trk::locY], Trk::locY)),
            covMatrix,
            currenttrackpar->associatedSurface()
          );
          
          state->setMeasurement(std::move(newpseudo));
          measbase = state->measurement();
        }
        
        double *errors = state->measurementErrors();

        std::array<double,5> residuals = m_residualPullCalculator->residuals(measbase, currenttrackpar, ResidualPull::Biased, hittype);
        
        for (int i = 0; i < 5; i++) {
          if (
            !measbase->localParameters().contains(paraccessor.pardef[i]) ||
            (i > 0 && (hittype == TrackState::SCT || hittype == TrackState::TGC))
          ) {
            continue;
          }
          
          error[measno] = 
            (trajectory.prefit() > 0 && (hittype == TrackState::MDT || (hittype == TrackState::CSC && !state->measuresPhi()))) ? 
            2 : 
            errors[i];

          res[measno] = residuals[i];
          
          if (i == 2 && std::abs(std::abs(res[measno]) - 2 * M_PI) < std::abs(res[measno])) {
            if (res[measno] < 0) {
              res[measno] += 2 * M_PI;
            } else {
              res[measno] -= 2 * M_PI;
            }
          }
          measno++;
        }
      } else if (state->getStateType(TrackStateOnSurface::Outlier)) {
        double *errors = state->measurementErrors();
        for (int i = 0; i < 5; i++) {
          if (errors[i] > 0) {
            error[measno] = errors[i];
            measno++;
          }
        }
      }
      
      if (
        state->getStateType(TrackStateOnSurface::Scatterer) && 
        ((trajectory.prefit() == 0) || state->materialEffects()->deltaE() == 0)
      ) {
        double deltaphi = state->materialEffects()->deltaPhi();
        double measdeltaphi = state->materialEffects()->measuredDeltaPhi();
        double sigmadeltaphi = state->materialEffects()->sigmaDeltaPhi();
        double deltatheta = state->materialEffects()->deltaTheta();
        double sigmadeltatheta = state->materialEffects()->sigmaDeltaTheta();
        
        if (trajectory.prefit() != 1) {
          b[nperpars + 2 * scatno] -= (deltaphi - measdeltaphi) / (sigmadeltaphi * sigmadeltaphi);
          b[nperpars + 2 * scatno + 1] -= deltatheta / (sigmadeltatheta * sigmadeltatheta);
        } else {
          b[nperpars + scatno] -= deltatheta / (sigmadeltatheta * sigmadeltatheta);
        }
        
        chi2 += (
          deltaphi * deltaphi / (sigmadeltaphi * sigmadeltaphi) +
          deltatheta * deltatheta / (sigmadeltatheta * sigmadeltatheta)
        );

        scatno++;
      }

      if ((state->materialEffects() != nullptr) && state->materialEffects()->sigmaDeltaE() > 0) {
        double averagenergyloss = std::abs(state->materialEffects()->deltaE());
        const double qoverpbrem = limitInversePValue(1000 * states[hitno]->trackParameters()->parameters()[Trk::qOverP]);
        const double qoverp = limitInversePValue(qoverpbrem - state->materialEffects()->delta_p());
        const double pbrem = 1. / std::abs(qoverpbrem);
        const double p = 1. / std::abs(qoverp);
        const double mass = .001 * trajectory.mass();
        const double energy = std::sqrt(p * p + mass * mass);
        const double bremEnergy = std::sqrt(pbrem * pbrem + mass * mass);

        res[nmeas - nbrem + bremno] = .001 * averagenergyloss - energy + bremEnergy;

        double sigde = state->materialEffects()->sigmaDeltaE();
        double sigdepos = state->materialEffects()->sigmaDeltaEPos();
        double sigdeneg = state->materialEffects()->sigmaDeltaENeg();
        
        error[nmeas - nbrem + bremno] = .001 * state->materialEffects()->sigmaDeltaE();
        
        if (state->materialEffects()->isKink()) {
          maxbrempull = -999999999;
          state_maxbrempull = nullptr;
        }
        
        if (
          cache.m_asymeloss && 
          it > 0 && 
          (trajectory.prefit() == 0) && 
          sigde > 0 && 
          sigde != sigdepos && 
          sigde != sigdeneg
        ) {
          double elosspull = res[nmeas - nbrem + bremno] / (.001 * sigde);

          if (trajectory.mass() > 100) {
            if (elosspull < -1) {
              state->materialEffects()->setSigmaDeltaE(sigdepos);
            } else if (elosspull > 1) {
              state->materialEffects()->setSigmaDeltaE(sigdeneg);
            }
            
            error[nmeas - nbrem + bremno] = .001 * state->materialEffects()->sigmaDeltaE();
          } else if ((trajectory.numberOfTRTHits() == 0) || it >= 3) {
            if (
              !state->materialEffects()->isKink() && (
                (elosspull < -.2 && m_fixbrem == -1 && elosspull < maxbrempull) || 
                (m_fixbrem >= 0 && bremno == m_fixbrem)
              )
            ) {
              bremno_maxbrempull = bremno;
              state_maxbrempull = state.get();
              maxbrempull = elosspull;
            }
          }
        }
        
        if (
          it > 0 && 
          hitno >= 2 && 
          !m_calotoolparam.empty() && 
          (trajectory.prefit() == 0) && 
          state->materialEffects()->sigmaDeltaPhi() == 0 && 
          state->materialEffects()->isMeasuredEloss() && 
          res[nmeas - nbrem + bremno] / (.001 * state->materialEffects()->sigmaDeltaEAve()) > 2.5
        ) {
          const TrackParameters* parforcalo =
            trajectory.prefit() != 0 ? trajectory.referenceParameters()
                                     : states[hitno - 2]->trackParameters();
          const IPropagator* prop = &*m_propagator;

          std::vector<MaterialEffectsOnTrack> calomeots =
            m_calotoolparam->extrapolationSurfacesAndEffects(
              *m_navigator->highestVolume(ctx),
              *prop,
              *parforcalo,
              parforcalo->associatedSurface(),
              Trk::anyDirection,
              Trk::muon);

          if (calomeots.size() == 3) {
            averagenergyloss = std::abs(calomeots[1].energyLoss()->deltaE());
            double newres = .001 * averagenergyloss - energy + bremEnergy;
            double newerr = .001 * calomeots[1].energyLoss()->sigmaDeltaE();
            
            if (std::abs(newres / newerr) < std::abs(res[nmeas - nbrem + bremno] / error[nmeas - nbrem + bremno])) {
              ATH_MSG_DEBUG("Changing from measured to parametrized energy loss");
              
              state->materialEffects()->setEloss(std::unique_ptr<EnergyLoss>(calomeots[1].energyLoss()->clone()));
              state->materialEffects()->setSigmaDeltaE(calomeots[1].energyLoss()->sigmaDeltaE());
              res[nmeas - nbrem + bremno] = newres;
              error[nmeas - nbrem + bremno] = newerr;
            }
          }
          
          state->materialEffects()->setMeasuredEloss(false);
        }
        bremno++;
      }
    }
    
    measno = 0;
    
    for (; measno < nmeas; measno++) {
      if (error[measno] == 0) {
        continue;
      }
      
      chi2 += res[measno] * (1. / (error[measno] * error[measno])) * res[measno];

    }
    if (!doderiv && (scatwasupdated)) {
      lu_m = a;
    }

    double oldchi2 = trajectory.chi2();
    trajectory.setPrevChi2(oldchi2);
    trajectory.setChi2(chi2);
    
    double oldredchi2 = (trajectory.nDOF() > 0) ? oldchi2 / trajectory.nDOF() : 0;
    double newredchi2 = (trajectory.nDOF() > 0) ? chi2 / trajectory.nDOF() : 0;
    
    if (
      trajectory.prefit() > 0 && (
        (newredchi2 < 2 && it != 0) || 
        (newredchi2 < oldredchi2 + .1 && std::abs(newredchi2 - oldredchi2) < 1 && it != 1)
      )
    ) {
      trajectory.setConverged(true);
    }

    double maxdiff = (nsihits != 0 && nsihits + ntrthits == nhits && chi2 < oldchi2) ? 200 : 1.;
    maxdiff = 1;
    int miniter = (nsihits != 0 && nsihits + ntrthits == nhits) ? 1 : 2;
    
    if (miniter < cache.m_miniter) {
      miniter = cache.m_miniter;
    }
    
    if (it >= miniter && std::abs(oldchi2 - chi2) < maxdiff) {
      trajectory.setConverged(true);
    }

    if ((state_maxbrempull != nullptr) && trajectory.converged()) {
      state_maxbrempull->materialEffects()->setSigmaDeltaE(
        10 * state_maxbrempull->materialEffects()->sigmaDeltaEPos()
      );
      
      state_maxbrempull->materialEffects()->setKink(true);
      trajectory.setConverged(false);
      
      double olderror = error[nmeas - nbrem + bremno_maxbrempull];
      double newerror = .001 * state_maxbrempull->materialEffects()->sigmaDeltaE();
      error[nmeas - nbrem + bremno_maxbrempull] = .001 * state_maxbrempull->materialEffects()->sigmaDeltaE();

      if (a.cols() != nfitpars) {
        ATH_MSG_ERROR("Your assumption is wrong!!!!");
      }

      for (int i = 0; i < nfitpars; i++) {
        if (weightderiv(nmeas - nbrem + bremno_maxbrempull, i) == 0) {
          continue;
        }

        for (int j = i; j < nfitpars; j++) {
          a.fillSymmetric(
            i, j,
            a(i, j) - (
              weightderiv(nmeas - nbrem + bremno_maxbrempull, i) *
              weightderiv(nmeas - nbrem + bremno_maxbrempull, j) * 
              (1 - olderror * olderror / (newerror * newerror))
            )
          );
        }
        weightderiv(nmeas - nbrem + bremno_maxbrempull, i) *= olderror / newerror;
      }
      lu_m = a;
      trajectory.setChi2(1e15);
      doderiv = true;
    }
  }

  void GlobalChi2Fitter::fillDerivatives(
    GXFTrajectory & trajectory,
    bool onlybrem
  ) const {
    ATH_MSG_DEBUG("fillDerivatives");

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    int scatno = 0;
    int bremno = 0;
    int measno = 0;
    int nscatupstream = trajectory.numberOfUpstreamScatterers();
    int nbremupstream = trajectory.numberOfUpstreamBrems();
    int nscat = trajectory.numberOfScatterers();
    int nbrem = trajectory.numberOfBrems();
    int nperparams = trajectory.numberOfPerigeeParameters();

    Amg::MatrixX & weightderiv = trajectory.weightedResidualDerivatives();
    Amg::VectorX & error = trajectory.errors();

    int nmeas = (int) weightderiv.rows();

    ParamDefsAccessor paraccessor;
    
    for (std::unique_ptr<GXFTrackState> & state : states) {
      if (
        onlybrem && 
        ((state->materialEffects() == nullptr) || state->materialEffects()->sigmaDeltaE() <= 0)
      ) {
        continue;
      } 
      
      TrackState::MeasurementType hittype = state->measurementType();
      const MeasurementBase *measbase = state->measurement();
      const auto [scatmin, scatmax] = std::minmax(scatno, nscatupstream);
      const auto [bremmin, bremmax] = std::minmax(bremno, nbremupstream);
      if (state->getStateType(TrackStateOnSurface::Measurement)) {
        Amg::MatrixX & derivatives = state->derivatives();
        double sinstereo = 0;
        
        if (hittype == TrackState::SCT || hittype == TrackState::TGC) {
          sinstereo = state->sinStereo();
        }
        
        double cosstereo = (sinstereo == 0) ? 1. : std::sqrt(1 - sinstereo * sinstereo);
        
        for (int i = 0; i < 5; i++) {
          if (
            !measbase->localParameters().contains(paraccessor.pardef[i]) ||
            (i > 0 && (hittype == TrackState::SCT || hittype == TrackState::TGC))
          ) {
            continue;
          }
          
          if (trajectory.numberOfPerigeeParameters() > 0) {
            int cols = trajectory.m_straightline ? 4 : 5;

            if (i == 0) {
              weightderiv.row(measno).head(cols) =
                (derivatives.row(0).head(cols) * cosstereo +
                 sinstereo * derivatives.row(1).head(cols)) /
                error[measno];
            } else {
              weightderiv.row(measno).head(cols) = derivatives.row(i).head(cols) / error[measno];
            }
          }
          
          for (int j = scatmin; j < scatmax; j++) {
            int index = nperparams + ((trajectory.prefit() != 1) ? 2 * j : j);
            double thisderiv = 0;
            double sign = 1;
            //
            
            if (i == 0 && sinstereo != 0) {
              thisderiv = sign * (derivatives(0, index) * cosstereo + sinstereo * derivatives(1, index));
            } else {
              thisderiv = sign * derivatives(i, index);
            }
            
            weightderiv(measno, index) = thisderiv / error[measno];
            
            if (trajectory.prefit() != 1) {
              index++;
              
              if (i == 0 && sinstereo != 0) {
                thisderiv = sign * (derivatives(0, index) * cosstereo + sinstereo * derivatives(1, index));
              } else {
                thisderiv = sign * derivatives(i, index);
              }
              
              weightderiv(measno, index) = thisderiv / error[measno];
            }
          }
          
          for (int j = bremmin; j < bremmax; j++) {
            double thisderiv = 0;
            int index = j + nperparams + 2 * nscat;
            if (i == 0 && sinstereo != 0) {
              thisderiv = derivatives(0, index) * cosstereo + sinstereo * derivatives(1, index);
            } else {
              thisderiv = derivatives(i, index);
            }
            weightderiv(measno, index) = thisderiv / error[measno];
          }
          
          measno++;
        }
      } else if (state->getStateType(TrackStateOnSurface::Outlier)) {
        double *errors = state->measurementErrors();
        for (int i = 0; i < 5; i++) {
          if (errors[i] > 0) {
            measno++;
          }
        }
      } else if (
        state->getStateType(TrackStateOnSurface::Scatterer) &&
        ((trajectory.prefit() == 0) || state->materialEffects()->deltaE() == 0)
      ) {
        scatno++;
      }

      if ((state->materialEffects() != nullptr) && state->materialEffects()->sigmaDeltaE() > 0) {
        //limit values to avoid FPE overflow or div by zero
        double qoverpbrem = limitInversePValue(1000 * state->trackParameters()->parameters()[Trk::qOverP]);
        double qoverp = limitInversePValue(qoverpbrem - state->materialEffects()->delta_p());
        
        double mass = .001 * trajectory.mass();
        
        const auto thisMeasurementIdx{nmeas - nbrem + bremno};
        //references (courtesy of Christos Anastopoulos):
        //https://inspirehep.net/files/a810ad0047a22af32fbff587c6c98ce5 
        //https://its.cern.ch/jira/browse/ATLASRECTS-6213
        auto multiplier = [] (double mass, double qOverP){
          return std::copysign(1./(qOverP * qOverP * std::sqrt(1. + mass * mass * qOverP * qOverP)), qOverP);
        };
        const auto qoverpTerm {multiplier(mass, qoverp) / error[thisMeasurementIdx]};
        const auto qoverpBremTerm {multiplier(mass, qoverpbrem) / error[thisMeasurementIdx]};
        
        if (trajectory.numberOfPerigeeParameters() > 0) {
          weightderiv(thisMeasurementIdx, 4) = qoverpBremTerm - qoverpTerm;
        }
        //
        const auto bremNoBase =  nperparams + 2 * nscat;
        if (bremno < nbremupstream) {
          weightderiv(thisMeasurementIdx, bremNoBase + bremno) = qoverpTerm;
          for (int bremno2 = bremno + 1; bremno2 < nbremupstream; bremno2++) {
            weightderiv(thisMeasurementIdx, bremNoBase + bremno2) = qoverpTerm - qoverpBremTerm;
          }
        } else {
          weightderiv(thisMeasurementIdx, bremNoBase + bremno) = qoverpBremTerm;
          for (int bremno2 = nbremupstream; bremno2 < bremno; bremno2++) {
            weightderiv(thisMeasurementIdx, bremNoBase + bremno2) = qoverpBremTerm - qoverpTerm;
          }
        }
        bremno++;
      }
    }
  }

  FitterStatusCode GlobalChi2Fitter::runIteration(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory & trajectory, 
    int it,
    Amg::SymMatrixX & a, 
    Amg::VectorX & b,
    Amg::SymMatrixX & lu, 
    bool &doderiv
  ) const {
    int measno = 0;
    int nfitpars = trajectory.numberOfFitParameters();
    int nperpars = trajectory.numberOfPerigeeParameters();
    int scatpars = 2 * trajectory.numberOfScatterers();
    int nupstreamstates = trajectory.numberOfUpstreamStates();
    int nbrem = trajectory.numberOfBrems();
    double oldchi2 = trajectory.chi2();
    double oldredchi2 = (trajectory.nDOF() > 0) ? oldchi2 / trajectory.nDOF() : 0;
    int nsihits = trajectory.numberOfSiliconHits();
    int ntrthits = trajectory.numberOfTRTHits();
    int nhits = trajectory.numberOfHits();

    if (cache.m_phiweight.empty()) {
      cache.m_phiweight.assign(trajectory.trackStates().size(), 1);
    }
    
    FitterStatusCode fsc = calculateTrackParameters(ctx,trajectory, doderiv);
    
    if (fsc != FitterStatusCode::Success) {
      return fsc;
    }
    
    b.setZero();

    fillResiduals(ctx, cache, trajectory, it, a, b, lu, doderiv);

    double newredchi2 = (trajectory.nDOF() > 0) ? trajectory.chi2() / trajectory.nDOF() : 0;

    ATH_MSG_DEBUG("old chi2: " << oldchi2 << "/" << trajectory.nDOF() << 
    "=" << oldredchi2 << " new chi2: " << trajectory.chi2() << "/" << 
    trajectory.nDOF() << "=" << newredchi2);

    if (trajectory.prefit() > 0 && trajectory.converged()) {
      return FitterStatusCode::Success;
    }

    Amg::VectorX & res = trajectory.residuals();
    Amg::VectorX & error = trajectory.errors();
    std::vector < std::pair < double, double >>&scatsigmas = trajectory.scatteringSigmas();

    int nmeas = (int) res.size();

    const Amg::MatrixX & weight_deriv = trajectory.weightedResidualDerivatives();

    if (doderiv) {
      calculateDerivatives(trajectory);
      fillDerivatives(trajectory, !doderiv);
    }

    if (cache.m_firstmeasurement.empty()) {
      cache.m_firstmeasurement.resize(nfitpars);
      cache.m_lastmeasurement.resize(nfitpars);
      for (int i = 0; i < nperpars; i++) {
        cache.m_firstmeasurement[i] = 0;
        cache.m_lastmeasurement[i] = nmeas - nbrem;
      }
      measno = 0;
      int scatno = 0;
      int bremno = 0;
      for (int i = 0; i < (int) trajectory.trackStates().size(); i++) {
        std::unique_ptr<GXFTrackState> & state = trajectory.trackStates()[i];
        GXFMaterialEffects *meff = state->materialEffects();
        if (meff == nullptr) {
          measno += state->numberOfMeasuredParameters();
        }
        if (meff != nullptr) {
          if (meff->sigmaDeltaTheta() != 0
              && ((trajectory.prefit() == 0) || meff->deltaE() == 0)) {
            int scatterPos = nperpars + 2 * scatno;
            if (i < nupstreamstates) {
              cache.m_lastmeasurement[scatterPos] =
                cache.m_lastmeasurement[scatterPos + 1] = measno;
              cache.m_firstmeasurement[scatterPos] =
                cache.m_firstmeasurement[scatterPos + 1] = 0;
            } else {
              cache.m_lastmeasurement[scatterPos] =
                cache.m_lastmeasurement[scatterPos + 1] = nmeas - nbrem;
              cache.m_firstmeasurement[scatterPos] =
                cache.m_firstmeasurement[scatterPos + 1] = measno;
            }
            scatno++;
          }
          if (meff->sigmaDeltaE() > 0) {
            if (i < nupstreamstates) {
              cache.m_firstmeasurement[nperpars + scatpars + bremno] = 0;
              cache.m_lastmeasurement[nperpars + scatpars + bremno] = measno;
            } else {
              cache.m_firstmeasurement[nperpars + scatpars + bremno] = measno;
              cache.m_lastmeasurement[nperpars + scatpars + bremno] =
                nmeas - nbrem;
            }

            bremno++;
          }
        }
      }
    }

    if (a.cols() != nfitpars) {
      ATH_MSG_ERROR("Your assumption is wrong!!!!");
    }

    for (int k = 0; k < nfitpars; k++) {
      int minmeas = 0;
      int maxmeas = nmeas - nbrem;
      maxmeas = cache.m_lastmeasurement[k];
      minmeas = cache.m_firstmeasurement[k];

      for (measno = minmeas; measno < maxmeas; measno++) {
        double tmp =
          res[measno] * (1. / error[measno]) * weight_deriv(measno, k);
        b[k] += tmp;
      }

      if (k == 4 || k >= nperpars + scatpars) {
        for (measno = nmeas - nbrem; measno < nmeas; measno++) {
          b[k] += res[measno] * (1. / error[measno]) * weight_deriv(measno, k);
        }
      }
      
      if (doderiv) {
        for (int l = k; l < nfitpars; l++) {
          maxmeas =
            std::min(cache.m_lastmeasurement[k], cache.m_lastmeasurement[l]);
          minmeas =
            std::max(cache.m_firstmeasurement[k],
                     cache.m_firstmeasurement[l]);
          double tmp = 0;
          for (measno = minmeas; measno < maxmeas; measno++) {
            tmp += weight_deriv(measno, k) * weight_deriv(measno, l);
          }
          a.fillSymmetric(l, k, tmp);
        }
      }
    }
    
    if (doderiv) {
      int scatno = 0;
      
      for (int k = nperpars; k < nperpars + scatpars; k += 2) {
        a(k, k) += 1. / (scatsigmas[scatno].first * scatsigmas[scatno].first);
        a(k + 1, k + 1) += 1. / (scatsigmas[scatno].second * scatsigmas[scatno].second);
        scatno++;
      }

      for (int measno = nmeas - nbrem; measno < nmeas; measno++) {
        for (int k = 4; k < nfitpars; k++) {
          if (k == 5) {
            k = nperpars + scatpars;
          }
          
          for (int l = k; l < nfitpars; l++) {
            if (l == 5) {
              l = nperpars + scatpars;
            }
            double tmp = a(l, k) + weight_deriv(measno, k) * weight_deriv(measno, l);
            a.fillSymmetric(l, k, tmp);
          }
        }
      }
    }
    
    unsigned int scatno = 0;
    bool weightchanged = false;
    
    for (std::unique_ptr<GXFTrackState> & thisstate : trajectory.trackStates()) {
      GXFMaterialEffects *meff = thisstate->materialEffects();
      
      if (meff != nullptr) {
        const PlaneSurface *plsurf = nullptr;
        
        if (thisstate->associatedSurface().type() == Trk::SurfaceType::Plane)
          plsurf = static_cast < const PlaneSurface *>(&thisstate->associatedSurface());
        if (meff->deltaE() == 0 || ((trajectory.prefit() == 0) && (plsurf != nullptr))) {
          weightchanged = true;

          if (a.cols() != nfitpars) {
            ATH_MSG_ERROR("Your assumption is wrong!!!!");
          }
          
          int scatNoIndex = 2 * scatno + nperpars;

          if (trajectory.prefit() == 0) {
            if (thisstate->materialEffects()->sigmaDeltaPhi() != 0) {
              if (scatno >= cache.m_phiweight.size()) {
                std::stringstream message;
                message << "scatno is out of range " << scatno << " !< " << cache.m_phiweight.size();
                throw std::range_error(message.str());
              }

              if (!doderiv) {
                a(scatNoIndex, scatNoIndex) /= cache.m_phiweight[scatno];
              }
              
              if (it == 0) {
                cache.m_phiweight[scatno] = 1.00000001;
              } else if (it == 1) {
                cache.m_phiweight[scatno] = 1.0000001;
              } else if (it <= 3) {
                cache.m_phiweight[scatno] = 1.0001;
              } else if (it <= 6) {
                cache.m_phiweight[scatno] = 1.01;
              } else {
                cache.m_phiweight[scatno] = 1.1;
              }
              
              a(scatNoIndex, scatNoIndex) *= cache.m_phiweight[scatno];
            }
          }

          else if (trajectory.prefit() >= 2) {
            if (newredchi2 > oldredchi2 - 1 && newredchi2 < oldredchi2) {
              a(scatNoIndex, scatNoIndex) *= 1.0001;
              a(scatNoIndex + 1, scatNoIndex + 1) *= 1.0001;
            } else if (newredchi2 > oldredchi2 - 25 && newredchi2 < oldredchi2) {
              a(scatNoIndex, scatNoIndex) *= 1.001;
              a(scatNoIndex + 1, scatNoIndex + 1) *= 1.0001;
            } else {
              a(scatNoIndex, scatNoIndex) *= 1.1;
              a(scatNoIndex + 1, scatNoIndex + 1) *= 1.001;
            }
          }
        }
        
        if (
          thisstate->materialEffects()->sigmaDeltaPhi() != 0 &&
          ((trajectory.prefit() == 0) || thisstate->materialEffects()->deltaE() == 0)
        ) {
          scatno++;
        }
      }
    }
    
    if (
      (trajectory.prefit() == 2) && 
      doderiv && 
      trajectory.numberOfBrems() > 0 && 
      (newredchi2 < oldredchi2 - 25 || newredchi2 > oldredchi2)
    ) {
      a(4, 4) *= 1.001;
    }

    if (doderiv || weightchanged) {
      lu = a;
    }

    if (trajectory.converged()) {
      if ((trajectory.prefit() == 0) && nsihits + ntrthits != nhits) {
        unsigned int scatno = 0;
        
        if (a.cols() != nfitpars) {
          ATH_MSG_ERROR("Your assumption is wrong!!!!");
        }

        for (std::unique_ptr<GXFTrackState> & thisstate : trajectory.trackStates()) {
          if ((thisstate->materialEffects() != nullptr) && thisstate->materialEffects()->sigmaDeltaPhi() != 0) {
            if (scatno >= cache.m_phiweight.size()) {
              std::stringstream message;
              message << "scatno is out of range " << scatno << " !< " << cache.m_phiweight.size();
              throw std::range_error(message.str());
            }
            
            const PlaneSurface *plsurf = nullptr;
            
            if (thisstate->associatedSurface().type() == Trk::SurfaceType::Plane)
              plsurf = static_cast<const PlaneSurface *>(&thisstate->associatedSurface());
              
            if (thisstate->materialEffects()->deltaE() == 0 || (plsurf != nullptr)) {
              int scatNoIndex = 2 * scatno + nperpars;
              a(scatNoIndex, scatNoIndex) /= cache.m_phiweight[scatno];
              cache.m_phiweight[scatno] = 1;
            }
            
            if (thisstate->materialEffects()->sigmaDeltaPhi() != 0) {
              scatno++;
            }
          }
        }
        lu = a;
      }
      return FitterStatusCode::Success;
    }

    if (
      !m_redoderivs && 
      it < 5 && 
      (newredchi2 < 2 || (newredchi2 < oldredchi2 && newredchi2 > oldredchi2 - .5)) && 
      (trajectory.prefit() == 0)
    ) {
      doderiv = false;
    }

    return FitterStatusCode::Success;
  }

  FitterStatusCode GlobalChi2Fitter::updateFitParameters(
    GXFTrajectory & trajectory,
    Amg::VectorX & b,
    const Amg::SymMatrixX & lu_m
  ) const {
    ATH_MSG_DEBUG("UpdateFitParameters");

    const TrackParameters *refpar = trajectory.referenceParameters();
    double d0 = refpar->parameters()[Trk::d0];
    double z0 = refpar->parameters()[Trk::z0];
    double phi = refpar->parameters()[Trk::phi0];
    double theta = refpar->parameters()[Trk::theta];
    double qoverp = refpar->parameters()[Trk::qOverP];
    int nscat = trajectory.numberOfScatterers();
    int nbrem = trajectory.numberOfBrems();
    int nperparams = trajectory.numberOfPerigeeParameters();

    Eigen::LLT<Eigen::MatrixXd> llt(lu_m);
    Amg::VectorX result;

    if (llt.info() == Eigen::Success) {
      result = llt.solve(b);
    } else {
      result = Eigen::VectorXd::Zero(b.size());
    }

    if (trajectory.numberOfPerigeeParameters() > 0) {
      d0 += result[0];
      z0 += result[1];
      phi += result[2];
      theta += result[3];
      qoverp = (trajectory.m_straightline) ? 0 : .001 * result[4] + qoverp;
    } 
    
    if (!correctAngles(phi, theta)) {
      ATH_MSG_DEBUG("angles out of range: " << theta << " " << phi);
      ATH_MSG_DEBUG("Fit failed");
      return FitterStatusCode::InvalidAngles;
    }

    std::vector < std::pair < double, double >>&scatangles = trajectory.scatteringAngles();
    std::vector < double >&delta_ps = trajectory.brems();
    
    for (int i = 0; i < nscat; i++) {
      scatangles[i].first += result[2 * i + nperparams];
      scatangles[i].second += result[2 * i + nperparams + 1];
    }

    for (int i = 0; i < nbrem; i++) {
      delta_ps[i] += result[nperparams + 2 * nscat + i];
    }

    std::unique_ptr<const TrackParameters> newper(
      trajectory.referenceParameters()->associatedSurface().createUniqueTrackParameters(
        d0, z0, phi, theta, qoverp, std::nullopt
      )
    );
    
    trajectory.setReferenceParameters(std::move(newper));
    trajectory.setScatteringAngles(scatangles);
    trajectory.setBrems(delta_ps);
    
    return FitterStatusCode::Success;
  }

  void GlobalChi2Fitter::updatePixelROTs(
    GXFTrajectory & trajectory,
    Amg::SymMatrixX & a,
    Amg::VectorX & b 
  ) const{
    if ( trajectory.numberOfSiliconHits() == 0) {
      return;
    }
    
    if ( m_clusterSplitProbContainer.empty() ){
      return;
    } 

    const EventContext &evtctx = Gaudi::Hive::currentContext();

    SG::ReadHandle<Trk::ClusterSplitProbabilityContainer> splitProbContainer(m_clusterSplitProbContainer, evtctx);
    if (!splitProbContainer.isValid()) {
      ATH_MSG_FATAL("Failed to get cluster splitting probability container " << m_clusterSplitProbContainer);
    }

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    Amg::VectorX & res = trajectory.residuals();
    Amg::VectorX & err = trajectory.errors();
    Amg::MatrixX & weightderiv = trajectory.weightedResidualDerivatives();
    int nfitpars = trajectory.numberOfFitParameters();

    int measno = 0;
    for (size_t stateno = 0; stateno < states.size(); stateno++) {
      
      // Increment the measurement counter everytime we have crossed a measurement/outlier surface
      if ( stateno > 0 && ( states[stateno-1]->getStateType(TrackStateOnSurface::Measurement) ||
                            states[stateno-1]->getStateType(TrackStateOnSurface::Outlier) ) ) {
        measno += states[stateno-1]->numberOfMeasuredParameters();
      }
      
      std::unique_ptr<GXFTrackState> & state = states[stateno];
      if (!state->getStateType(TrackStateOnSurface::Measurement)) { 
        continue;
      }

      TrackState::MeasurementType hittype = state->measurementType();
      if (hittype != TrackState::Pixel) {
        continue;
      }

      const PrepRawData *prd{};
      if (const auto *const pMeas = state->measurement(); pMeas->type(Trk::MeasurementBaseType::RIO_OnTrack)){
        const auto *const rot = static_cast<const RIO_OnTrack *>(pMeas);
        prd = rot->prepRawData();
      }

      if(!prd)
        continue;

      if(!prd->type(Trk::PrepRawDataType::PixelCluster)){
        continue;
      }
      const InDet::PixelCluster* pixelCluster = static_cast<const InDet::PixelCluster*> ( prd );
      const auto &splitProb = splitProbContainer->splitProbability(pixelCluster);
      if (!splitProb.isSplit()) {
        ATH_MSG_DEBUG( "Pixel cluster is not split so no need to update" );
        continue;
      }

      std::unique_ptr < const RIO_OnTrack > newrot;
      double *olderror = state->measurementErrors();
      const TrackParameters *trackpars = state->trackParameters();

      double newerror[5] = {-1,-1,-1,-1,-1};
      double newres[2] = {-1,-1};

      newrot.reset(m_ROTcreator->correct(*prd, *trackpars));
      
      if(!newrot)
        continue;

      const Amg::MatrixX & covmat = newrot->localCovariance();
      
      newerror[0] = std::sqrt(covmat(0, 0));
      newres[0] = newrot->localParameters()[Trk::locX] - trackpars->parameters()[Trk::locX];  
      newerror[1] = std::sqrt(covmat(1, 1));
      newres[1] = newrot->localParameters()[Trk::locY] - trackpars->parameters()[Trk::locY];
      
      if (a.cols() != nfitpars) {
        ATH_MSG_ERROR("Your assumption is wrong!!!!");
      }

      //loop over both measurements  --  treated as uncorrelated 
      for( int k =0; k<2; k++ ){
        double oldres = res[measno+k];
        res[measno+k] = newres[k];
        err[measno+k] = newerror[k];
        
        for (int i = 0; i < nfitpars; i++) {
          if (weightderiv(measno+k, i) == 0) {
            continue;
          }

          b[i] -= weightderiv(measno+k, i) * (oldres / olderror[k] - (newres[k] * olderror[k]) / (newerror[k] * newerror[k]));
          
          for (int j = i; j < nfitpars; j++) {
            a.fillSymmetric(
              i, j,
              a(i, j) + (
                weightderiv(measno+k, i) *
                weightderiv(measno+k, j) *
                ((olderror[k] * olderror[k]) / (newerror[k] * newerror[k]) - 1)
              )
            );
          }
          weightderiv(measno+k, i) *= olderror[k] / newerror[k];
        }
      }

      state->setMeasurement(std::move(newrot));
      state->setMeasurementErrors(newerror);

    }// end for
  }


  void GlobalChi2Fitter::runTrackCleanerTRT(
    Cache & cache,
    GXFTrajectory & trajectory,
    Amg::SymMatrixX & a,
    Amg::VectorX & b, 
    Amg::SymMatrixX & lu_m,
    bool runOutlier, 
    bool trtrecal,
    int it
  ) const {
    double scalefactor = m_scalefactor;

    if (it == 1 && trajectory.numberOfSiliconHits() + trajectory.numberOfTRTHits() == trajectory.numberOfHits()) {
      scalefactor *= 2;
    }
    
    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    Amg::VectorX & res = trajectory.residuals();
    Amg::VectorX & err = trajectory.errors();
    Amg::MatrixX & weightderiv = trajectory.weightedResidualDerivatives();
    int nfitpars = trajectory.numberOfFitParameters();
    
    if (a.cols() != nfitpars) {
      ATH_MSG_ERROR("Your assumption is wrong!!!!");
    }

    int nperpars = trajectory.numberOfPerigeeParameters();
    int nscats = trajectory.numberOfScatterers();
    int hitno = 0;
    int measno = 0;
    bool outlierremoved = false;
    bool hitrecalibrated = false;
    
    for (int stateno = 0; stateno < (int) states.size(); stateno++) {
      std::unique_ptr<GXFTrackState> & state = states[stateno];
      
      if (state->getStateType(TrackStateOnSurface::Measurement)) {  // Hit is not (yet) an outlier
        TrackState::MeasurementType hittype = state->measurementType();

        if (hittype == TrackState::TRT) {
          if (
            runOutlier &&
            std::abs(state->trackParameters()->parameters()[Trk::driftRadius]) > 1.05 * state->associatedSurface().bounds().r()
          ) {
            ATH_MSG_DEBUG("Removing TRT hit #" << hitno);
            
            trajectory.setOutlier(stateno);
            outlierremoved = true;

            double *errors = state->measurementErrors();
            double olderror = errors[0];

            trajectory.updateTRTHitCount(stateno, olderror);
            
            for (int i = 0; i < nfitpars; i++) {
              if (weightderiv(measno, i) == 0) {
                continue;
              }
              
              b[i] -= res[measno] * weightderiv(measno, i) / olderror;
              
              for (int j = i; j < nfitpars; j++) {
                a.fillSymmetric(
                  i, j,
                  a(i, j) - weightderiv(measno, i) * weightderiv(measno, j)
                );
              }
              weightderiv(measno, i) = 0;
            }
            
            res[measno] = 0;
          } else if (trtrecal) {
            double *errors = state->measurementErrors();
            double olderror = errors[0];
            const Trk::RIO_OnTrack * oldrot{};
            const auto *const thisMeasurement{state->measurement()};
            if ( not thisMeasurement->type(Trk::MeasurementBaseType::RIO_OnTrack)){
              continue;
            } 
            oldrot = static_cast<const Trk::RIO_OnTrack *>(thisMeasurement);
            double oldradius = oldrot->localParameters()[Trk::driftRadius];
            if (oldrot->prepRawData() != nullptr) {
              double dcradius = oldrot->prepRawData()->localPosition()[Trk::driftRadius];
              double dcerror = std::sqrt(oldrot->prepRawData()->localCovariance()(Trk::driftRadius, Trk::driftRadius));
              double trackradius = state->trackParameters()->parameters()[Trk::driftRadius];

              std::unique_ptr<const Trk::RIO_OnTrack> newrot = nullptr;
              double distance = std::abs(std::abs(trackradius) - dcradius);
              
              if (distance < scalefactor * dcerror && (olderror > 1. || trackradius * oldradius < 0)) {
                newrot.reset(m_ROTcreator->correct(*oldrot->prepRawData(), *state->trackParameters()));
              } else if (distance > scalefactor * dcerror && olderror < 1.) {
                newrot.reset(m_broadROTcreator->correct(*oldrot->prepRawData(), *state->trackParameters()));
              }
              
              if (newrot != nullptr) {
                ATH_MSG_DEBUG("Recalibrating TRT hit #" << hitno);
                hitrecalibrated = true;
                double newradius = newrot->localParameters()[Trk::driftRadius];
                double newerror = std::sqrt(newrot->localCovariance()(Trk::driftRadius, Trk::driftRadius));
                
                if ((measno < 0) or (measno >= (int) res.size())) {
                  throw std::runtime_error(
                    "'res' array index out of range in TrkGlobalChi2Fitter/src/GlobalChi2Fitter.cxx:" + std::to_string(__LINE__)
                  );
                }
                
                double oldres = res[measno];
                double newres = newradius - state->trackParameters()->parameters()[Trk::driftRadius];
                errors[0] = newerror;
                state->setMeasurement(std::move(newrot));

                trajectory.updateTRTHitCount(stateno, olderror);

                for (int i = 0; i < nfitpars; i++) {
                  if (weightderiv(measno, i) == 0) {
                    continue;
                  }

                  b[i] -= weightderiv(measno, i) * (oldres / olderror - (newres * olderror) / (newerror * newerror));
                  
                  for (int j = i; j < nfitpars; j++) {
                    double weight = 1;
                    
                    if (
                      !cache.m_phiweight.empty() && 
                      i == j && 
                      i >= nperpars && 
                      i < nperpars + 2 * nscats && 
                      (i - nperpars) % 2 == 0
                    ) {
                      weight = cache.m_phiweight[(i - nperpars) / 2];
                    }
                    
                    a.fillSymmetric(
                      i, j,
                      a(i, j) + weightderiv(measno, i) * weightderiv(measno, j) * ((olderror * olderror) / (newerror * newerror) - 1) * weight
                    );
                  }
                  weightderiv(measno, i) *= olderror / newerror;
                }
                
                res[measno] = newres;
                err[measno] = newerror;
              }
            }
          }
        }
      }
      
      if (state->getStateType(TrackStateOnSurface::Measurement) || state->getStateType(TrackStateOnSurface::Outlier)) {
        hitno++;
        measno += state->numberOfMeasuredParameters();
      }
    }
    
    if (trajectory.nDOF() < 0) {
      cache.m_fittercode = FitterStatusCode::OutlierLogicFailure;
      cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
    }
    
    if (outlierremoved || hitrecalibrated) {
      lu_m = a;
      trajectory.setConverged(false);

      cache.m_miniter = it + 2;
    }
  }

  GXFTrajectory *GlobalChi2Fitter::runTrackCleanerSilicon(
    const EventContext& ctx,
    Cache & cache,
    GXFTrajectory &trajectory,
    Amg::SymMatrixX & a,
    Amg::SymMatrixX &fullcov,
    Amg::VectorX & b,
    bool runoutlier
  ) const {
    bool trackok = false;
    GXFTrajectory *oldtrajectory = &trajectory;
    std::unique_ptr < GXFTrajectory > cleanup_oldtrajectory;
    GXFTrajectory *newtrajectory = nullptr;
    std::unique_ptr < GXFTrajectory > cleanup_newtrajectory;

    // the oldtracjectory will be returned, so in case newtrajectory==oldtrajectory
    // the cleanup_newtrajectory == NULL and cleanup_oldtrajectory = oldtrajectory, otherwise
    // cleanup_newtrajectory will destroy the object oldtrajectory is pointing to.

    while (!trackok && oldtrajectory->nDOF() > 0) {
      trackok = true;
      std::vector<std::unique_ptr<GXFTrackState>> & states = oldtrajectory->trackStates();
      Amg::VectorX & res = oldtrajectory->residuals();
      Amg::VectorX & err = oldtrajectory->errors();
      Amg::MatrixX & weightderiv = oldtrajectory->weightedResidualDerivatives();
      int nfitpars = oldtrajectory->numberOfFitParameters();
      int nhits = oldtrajectory->numberOfHits();
      int nsihits = oldtrajectory->numberOfSiliconHits();
      
      if (nhits != nsihits) {
        return &trajectory;
      }
      
      double maxsipull = -1;
      int hitno = 0;
      int hitno_maxsipull = -1;
      int measno_maxsipull = -1;
      int stateno_maxsipull = 0;
      GXFTrackState *state_maxsipull = nullptr;
      int measno = 0;
      int n3sigma = 0;
      double cut = m_outlcut;
      double cut2 = m_outlcut - 1.;
      int noutl = oldtrajectory->numberOfOutliers();

      if (noutl > 0) {
        cut2 = cut - 1.25;
      }
      
      for (int stateno = 0; stateno < (int) states.size(); stateno++) {
        std::unique_ptr<GXFTrackState> & state = states[stateno];
        
        if (state->getStateType(TrackStateOnSurface::Measurement)) {
          TrackState::MeasurementType hittype = state->measurementType();

          if ((hittype == TrackState::Pixel || hittype == TrackState::SCT) && state->hasTrackCovariance()) {
            double *errors = state->measurementErrors();
            AmgSymMatrix(5) & trackcov = state->trackCovariance();
            const Amg::MatrixX & hitcov = state->measurement()->localCovariance();
            double sinstereo = state->sinStereo();
            double cosstereo = (sinstereo == 0) ? 1 : std::sqrt(1 - sinstereo * sinstereo);
            double weight1 = -1;
            
            if (hitcov(0, 0) > trackcov(0, 0)) {
              if (sinstereo == 0) {
                weight1 = errors[0] * errors[0] - trackcov(0, 0);
              } else {
                weight1 = errors[0] * errors[0] - (
                  trackcov(0, 0) * cosstereo * cosstereo + 2 *
                  trackcov(1, 0) * cosstereo * sinstereo + trackcov(1, 1) * sinstereo * sinstereo
                );
              }
            }
            
            double weight2 = (
              hittype == TrackState::Pixel && hitcov(1, 1) > trackcov(1, 1) ? 
              errors[1] * errors[1] - trackcov(1, 1) : 
              -1
            );

            double sipull1 = weight1 > 0 ? std::abs(res[measno] / std::sqrt(weight1)) : -1;
            double sipull2 = (
              hittype == TrackState::Pixel && weight2 > 0 ? 
              std::abs(res[measno + 1] / std::sqrt(weight2)) : 
              -1
            );
            sipull1 = std::max(sipull1, sipull2);

            if (sipull1 > maxsipull) {
              maxsipull = sipull1;
              measno_maxsipull = measno;
              state_maxsipull = state.get();
              stateno_maxsipull = stateno;
              hitno_maxsipull = hitno;
            }
            
            if (hittype == TrackState::Pixel && sipull1 > cut2) {
              n3sigma++;
            }
          }
        }

        if (state->getStateType(TrackStateOnSurface::Measurement) || state->getStateType(TrackStateOnSurface::Outlier)) {
          hitno++;
          measno += state->numberOfMeasuredParameters();
        }
      }
      
      double maxpull = maxsipull;

      ATH_MSG_DEBUG(" maxsipull: " << maxsipull << " hitno_maxsipull: " <<
        hitno_maxsipull << " n3sigma: " << n3sigma << " cut: " << cut << " cut2: " << cut2);

      Amg::SymMatrixX * newap = &a;
      Amg::VectorX * newbp = &b;
      Amg::SymMatrixX newa(nfitpars, nfitpars);
      Amg::VectorX newb(nfitpars);

      if (
        maxpull > 2 && 
        oldtrajectory->chi2() / oldtrajectory->nDOF() > .25 * m_chi2cut
      ) {
        state_maxsipull = oldtrajectory->trackStates()[stateno_maxsipull].get();
        const PrepRawData *prd{};
        if (const auto *const pMeas = state_maxsipull->measurement(); pMeas->type(Trk::MeasurementBaseType::RIO_OnTrack)){
          const auto *const rot = static_cast<const RIO_OnTrack *>(pMeas);
          prd = rot->prepRawData();
        }
        std::unique_ptr < const RIO_OnTrack > broadrot;
        double *olderror = state_maxsipull->measurementErrors();
        TrackState::MeasurementType hittype_maxsipull = state_maxsipull->measurementType();
        const TrackParameters *trackpar_maxsipull = state_maxsipull->trackParameters();

        Amg::VectorX parameterVector = trackpar_maxsipull->parameters();
        std::unique_ptr<const TrackParameters> trackparForCorrect(
          trackpar_maxsipull->associatedSurface().createUniqueTrackParameters(
            parameterVector[Trk::loc1],
            parameterVector[Trk::loc2],
            parameterVector[Trk::phi],
            parameterVector[Trk::theta],
            parameterVector[Trk::qOverP],
            state_maxsipull->hasTrackCovariance()
              ? std::optional<AmgSymMatrix(5)>(
                  state_maxsipull->trackCovariance())
              : std::nullopt));

        double newerror[5];
        newerror[0] = newerror[1] = newerror[2] = newerror[3] = newerror[4] = -1;
        double newpull = -1;
        double newpull1 = -1;
        double newpull2 = -1;
        double newres1 = -1;
        double newres2 = -1;
        double newsinstereo = 0;

        if (
          (prd != nullptr) && 
          !state_maxsipull->isRecalibrated() && 
          maxpull > 2.5 &&
          oldtrajectory->chi2() / trajectory.nDOF() > .3 * m_chi2cut && 
          cache.m_sirecal
        ) {
          broadrot.reset(m_broadROTcreator->correct(*prd, *trackparForCorrect));
        }
        
        if (broadrot) {
          const Amg::MatrixX & covmat = broadrot->localCovariance();
          newerror[0] = std::sqrt(covmat(0, 0));
          
          if (state_maxsipull->sinStereo() != 0) {
            double v0 = 0.5 * (
              covmat(0, 0) + covmat(1, 1) -
              std::sqrt(
                (covmat(0, 0) + covmat(1, 1)) * (covmat(0, 0) + covmat(1, 1)) - 
                4 * (covmat(0, 0) * covmat(1, 1) - covmat(0, 1) * covmat(0, 1))
              )
            );
            
            double v1 = 0.5 * (
              covmat(0, 0) + covmat(1, 1) + 
              std::sqrt(
                (covmat(0, 0) + covmat(1, 1)) * (covmat(0, 0) + covmat(1, 1)) -
                4 * (covmat(0, 0) * covmat(1, 1) - covmat(0, 1) * covmat(0, 1))
              )
            );
            
            newsinstereo = std::sin(0.5 * std::asin(2 * covmat(0, 1) / (v0 - v1)));
            newerror[0] = std::sqrt(v0);
          }
          
          double cosstereo = (newsinstereo == 0) ? 1. : std::sqrt(1 - newsinstereo * newsinstereo);
          
          if (cosstereo != 1.) {
            newres1 = (
              cosstereo * (broadrot->localParameters()[Trk::locX] - trackpar_maxsipull->parameters()[Trk::locX]) +
              newsinstereo * (broadrot->localParameters()[Trk::locY] - trackpar_maxsipull->parameters()[Trk::locY])
            );
          } else {
            newres1 = broadrot->localParameters()[Trk::locX] - trackpar_maxsipull->parameters()[Trk::locX];
          }
          
          if (newerror[0] == 0.0) {
            ATH_MSG_WARNING("Measurement error is zero or negative, treating as outlier");
            newpull1 = 9999.;
          }
          else {
            newpull1 = std::abs(newres1 / newerror[0]);
          }
          
          if (hittype_maxsipull == TrackState::Pixel) {
            newerror[1] = std::sqrt(covmat(1, 1));
            newres2 = broadrot->localParameters()[Trk::locY] - trackpar_maxsipull->parameters()[Trk::locY];
            newpull2 = std::abs(newres2 / newerror[1]);
          }
          
          newpull = std::max(newpull1, newpull2);
        }

        if (
          broadrot && 
          newpull < m_outlcut &&
          (newerror[0] > 1.5 * olderror[0] || newerror[1] > 1.5 * std::abs(olderror[1]))
        ) {
          if ((measno_maxsipull < 0) or(measno_maxsipull >= (int) res.size())) {
            throw std::runtime_error(
              "'res' array index out of range in TrkGlobalChi2Fitter/src/GlobalChi2Fitter.cxx:" + std::to_string(__LINE__)
            );
          }
          
          trackok = false;
          newtrajectory = oldtrajectory;

          if (a.cols() != nfitpars) {
            ATH_MSG_ERROR("Your assumption is wrong!!!!");
          }

          double oldres1 = res[measno_maxsipull];
          res[measno_maxsipull] = newres1;
          err[measno_maxsipull] = newerror[0];
          
          for (int i = 0; i < nfitpars; i++) {
            if (weightderiv(measno_maxsipull, i) == 0) {
              continue;
            }
            
            b[i] -= weightderiv(measno_maxsipull, i) * (oldres1 / olderror[0] - (newres1 * olderror[0]) / (newerror[0] * newerror[0]));
            
            for (int j = i; j < nfitpars; j++) {
              a.fillSymmetric(
                i, j,
                a(i, j) + (
                  weightderiv(measno_maxsipull, i) *
                  weightderiv(measno_maxsipull, j) *
                  ((olderror[0] * olderror[0]) / (newerror[0] * newerror[0]) - 1)
                )
              );
            }
            weightderiv(measno_maxsipull, i) *= olderror[0] / newerror[0];
          }
          
          if (hittype_maxsipull == TrackState::Pixel) {
            double oldres2 = res[measno_maxsipull + 1];
            res[measno_maxsipull + 1] = newres2;
            err[measno_maxsipull + 1] = newerror[1];
            
            for (int i = 0; i < nfitpars; i++) {
              if (weightderiv(measno_maxsipull + 1, i) == 0) {
                continue;
              }
              
              b[i] -= weightderiv(measno_maxsipull + 1, i) * (oldres2 / olderror[1] - (newres2 * olderror[1]) / (newerror[1] * newerror[1]));
              
              for (int j = i; j < nfitpars; j++) {
                a.fillSymmetric(
                  i, j,
                  a(i, j) + (
                    weightderiv(measno_maxsipull + 1, i) *
                    weightderiv(measno_maxsipull + 1, j) * 
                    ((olderror[1] * olderror[1]) / (newerror[1] * newerror[1]) - 1)
                  )
                );
              }
              
              weightderiv(measno_maxsipull + 1, i) *= olderror[1] / newerror[1];
            }
          }
          
          ATH_MSG_DEBUG(
            "Recovering outlier, hitno=" << hitno_maxsipull << " measno=" << 
            measno_maxsipull << " pull=" << maxsipull << " olderror_0=" << 
            olderror[0] << " newerror_0=" << newerror[0] << " olderror_1=" <<
            olderror[1] << " newerror_1=" << newerror[1]
          );

          state_maxsipull->setMeasurement(std::move(broadrot));
          state_maxsipull->setSinStereo(newsinstereo);
          state_maxsipull->setMeasurementErrors(newerror);
        } else if (
          (
            (
              ((n3sigma < 2 && maxsipull > cut2 && maxsipull < cut) || n3sigma > 1) && 
              (oldtrajectory->chi2() / oldtrajectory->nDOF() > .3 * m_chi2cut || noutl > 1)
            ) || 
            maxsipull > cut
          ) && 
          (oldtrajectory->nDOF() > 1 || hittype_maxsipull == TrackState::SCT) && 
          runoutlier
        ) {
          trackok = false;
          ATH_MSG_DEBUG(
            "Removing outlier, hitno=" << hitno_maxsipull << ", measno=" << 
            measno_maxsipull << " pull=" << maxsipull
          );
          
          newa = a;
          newb = b;
          newap = &newa;
          newbp = &newb;
          cleanup_newtrajectory = std::make_unique<GXFTrajectory>(*oldtrajectory);
          newtrajectory = cleanup_newtrajectory.get();
          
          if (newa.cols() != nfitpars) {
            ATH_MSG_ERROR("Your assumption is wrong!!!!");
          }

          Amg::VectorX & newres = newtrajectory->residuals();
          Amg::MatrixX & newweightderiv = newtrajectory->weightedResidualDerivatives();
          if ((measno_maxsipull < 0) or(measno_maxsipull >= (int) res.size())) {
            throw std::runtime_error(
              "'res' array index out of range in TrkGlobalChi2Fitter/src/GlobalChi2Fitter.cxx:" + std::to_string(__LINE__)
            );
          }
          
          double oldres1 = res[measno_maxsipull];
          newres[measno_maxsipull] = 0;
          
          for (int i = 0; i < nfitpars; i++) {
            if (weightderiv(measno_maxsipull, i) == 0) {
              continue;
            }
            
            newb[i] -= weightderiv(measno_maxsipull, i) * oldres1 / olderror[0];
            
            for (int j = i; j < nfitpars; j++) {
              newa.fillSymmetric(
                i, j,
                newa(i, j) - (
                  weightderiv(measno_maxsipull, i) *
                  weightderiv(measno_maxsipull, j)
                )
              );
            }
            newweightderiv(measno_maxsipull, i) = 0;
          }
          
          if (hittype_maxsipull == TrackState::Pixel) {
            double oldres2 = res[measno_maxsipull + 1];
            newres[measno_maxsipull + 1] = 0;
            
            for (int i = 0; i < nfitpars; i++) {
              if (weightderiv(measno_maxsipull + 1, i) == 0) {
                continue;
              }
              
              newb[i] -= weightderiv(measno_maxsipull + 1, i) * oldres2 / olderror[1];
              
              for (int j = i; j < nfitpars; j++) {
                if (weightderiv(measno_maxsipull + 1, j) == 0) {
                  continue;
                }
                
                newa.fillSymmetric(
                  i, j,
                  newa(i, j) - (
                    weightderiv(measno_maxsipull + 1, i) *
                    weightderiv(measno_maxsipull + 1, j)
                  )
                );
              }
              newweightderiv(measno_maxsipull + 1, i) = 0;
            }
          }
          
          newtrajectory->setOutlier(stateno_maxsipull);
        }
      }
      
      if (!trackok) {
        Amg::SymMatrixX lu_m = *newap;
        newtrajectory->setConverged(false);
        bool doderiv = m_redoderivs;
        cache.m_fittercode = updateFitParameters(*newtrajectory, *newbp, lu_m);
        if (cache.m_fittercode != FitterStatusCode::Success) {
          cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
          return nullptr;
        }

        for (int it = 0; it < m_maxit; ++it) {
          if (it == m_maxit - 1) {
            ATH_MSG_DEBUG("Fit did not converge");
            cache.m_fittercode = FitterStatusCode::NoConvergence;
            cache.incrementFitStatus(S_NOT_CONVERGENT);
            return nullptr;
          }
          
          if (!newtrajectory->converged()) {
            cache.m_fittercode = runIteration(
              ctx, cache, *newtrajectory, it, *newap, *newbp, lu_m, doderiv);

            if (cache.m_fittercode != FitterStatusCode::Success) {
              cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
              return nullptr;
            }
            
            if (!newtrajectory->converged()) {
              cache.m_fittercode = updateFitParameters(*newtrajectory, *newbp, lu_m);
              if (cache.m_fittercode != FitterStatusCode::Success) {
                cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);

                return nullptr;
              }
            }
          } else {
            double oldchi2 = oldtrajectory->chi2() / oldtrajectory->nDOF();
            double newchi2 = (newtrajectory->nDOF() > 0) ? newtrajectory->chi2() / newtrajectory->nDOF() : 0;
            double mindiff = 0;
            
            if (newtrajectory->nDOF() != oldtrajectory->nDOF() && maxsipull > cut2) {
              mindiff = (oldchi2 > .33 * m_chi2cut || noutl > 0) ? .8 : 1.;
              
              if (noutl == 0 && maxsipull < cut - .5 && oldchi2 < .5 * m_chi2cut) {
                mindiff = 2.;
              }
            }
            
            if (newchi2 > oldchi2 || (newchi2 > oldchi2 - mindiff && newchi2 > .33 * oldchi2)) {
              ATH_MSG_DEBUG("Outlier not confirmed, keeping old trajectory");
              
              if (oldchi2 > m_chi2cut) {
                cache.m_fittercode = FitterStatusCode::OutlierLogicFailure;
                cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
                return nullptr;
              }
              
              cleanup_oldtrajectory.release();
              return oldtrajectory;
            }
            if (oldtrajectory != newtrajectory) {
              cleanup_oldtrajectory = std::move(cleanup_newtrajectory);
              oldtrajectory = newtrajectory;
              a = newa;
              b = newb;
            }

            // Solve assuming the matrix is SPD.
            // Cholesky Decomposition is used
            Eigen::LLT < Eigen::MatrixXd > lltOfW(a);
            if (lltOfW.info() == Eigen::Success) {
              // Solve for x  where Wx = I
              // this is cheaper than invert as invert makes no assumptions about the
              // matrix being symmetric
              int ncols = a.cols();
              Amg::MatrixX weightInvAMG = Amg::MatrixX::Identity(ncols, ncols);
              fullcov = lltOfW.solve(weightInvAMG);
            } else {
              ATH_MSG_DEBUG("matrix inversion failed!");
              cache.incrementFitStatus(S_MAT_INV_FAIL);
              cache.m_fittercode = FitterStatusCode::MatrixInversionFailure;
              return nullptr;
            }
            break;
          }
        }
      }
      
      if (!trackok) {
        calculateTrackErrors(*oldtrajectory, fullcov, true);
      }
    }
    
    if (
      oldtrajectory->nDOF() > 0 && 
      oldtrajectory->chi2() / oldtrajectory->nDOF() > m_chi2cut && 
      runoutlier
    ) {
      cache.m_fittercode = FitterStatusCode::OutlierLogicFailure;
      cache.incrementFitStatus(S_NOT_ENOUGH_MEAS);
      return nullptr;
    }
    
    cleanup_oldtrajectory.release();
    return oldtrajectory;
  }

  void GlobalChi2Fitter::makeTrackFillDerivativeMatrix(
    Cache &cache,
    GXFTrajectory &oldtrajectory
  ) {
    Amg::MatrixX & derivs = oldtrajectory.weightedResidualDerivatives();
    Amg::VectorX & errors = oldtrajectory.errors();
    int nrealmeas = 0;
    
    for (auto & hit : oldtrajectory.trackStates()) {
      if (const auto *pMeas{hit->measurement()};
        hit->getStateType(TrackStateOnSurface::Measurement) and (
          pMeas->type(Trk::MeasurementBaseType::RIO_OnTrack) or  
          pMeas->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack)
        )
      ) {
        nrealmeas += hit->numberOfMeasuredParameters();
      }
    } 
    cache.m_derivmat.resize(nrealmeas, oldtrajectory.numberOfFitParameters());
    cache.m_derivmat.setZero();
    int measindex = 0;
    int measindex2 = 0;
    int nperpars = oldtrajectory.numberOfPerigeeParameters();
    int nscat = oldtrajectory.numberOfScatterers();
    for (auto & hit : oldtrajectory.trackStates()) {
      if (const auto *pMeas{hit->measurement()};
        hit->getStateType(TrackStateOnSurface::Measurement) and (
          pMeas->type(Trk::MeasurementBaseType::RIO_OnTrack) or 
          pMeas->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack)
        )
      ) {
        for (int i = measindex; i < measindex + hit->numberOfMeasuredParameters(); i++) {
          for (int j = 0; j < oldtrajectory.numberOfFitParameters(); j++) {
            cache.m_derivmat(i, j) = derivs(measindex2, j) * errors[measindex2];
            if ((j == 4 && !oldtrajectory.m_straightline) || j >= nperpars + 2 * nscat) {
              cache.m_derivmat(i, j) *= 1000;
            }
          }
          
          measindex2++;
        }
        
        measindex += hit->numberOfMeasuredParameters();
      } else if (hit->materialEffects() == nullptr) {
        measindex2 += hit->numberOfMeasuredParameters();
      }
    }
  }

  std::unique_ptr<const TrackParameters> GlobalChi2Fitter::makeTrackFindPerigeeParameters(
    const EventContext & ctx,
    Cache &cache,
    GXFTrajectory &oldtrajectory,
    const ParticleHypothesis matEffects
  ) const {
    GXFTrackState *firstmeasstate = nullptr, *lastmeasstate = nullptr;
    std::tie(firstmeasstate, lastmeasstate) = oldtrajectory.findFirstLastMeasurement();
    std::unique_ptr<const TrackParameters> per(nullptr);

    if (cache.m_acceleration && !m_matupdator.empty()) {
      std::unique_ptr<const TrackParameters> prevpar(
        firstmeasstate->trackParameters() != nullptr ?
        firstmeasstate->trackParameters()->clone() :
        nullptr
      );
      std::vector<std::pair<const Layer *, const Layer *>> & upstreamlayers = oldtrajectory.upstreamMaterialLayers();
      bool first = true;
      
      for (int i = (int)upstreamlayers.size() - 1; i >= 0; i--) {
        if (prevpar == nullptr) {
          break;
        }
        
        PropDirection propdir = oppositeMomentum;
        const Layer *layer = upstreamlayers[i].first;
        
        if (layer == nullptr) {
          layer = upstreamlayers[i].second;
        }
        
        DistanceSolution distsol = layer->surfaceRepresentation().straightLineDistanceEstimate(
          prevpar->position(), prevpar->momentum().unit()
        );
        double distance = getDistance(distsol);

        if (distsol.numberOfSolutions() == 2) {
          if (std::abs(distance) < 0.01) {
            continue;
          }
          
          if (distsol.first() * distsol.second() < 0 && !first) {
            continue;
          }
        }
        
        if (first && distance > 0) {
          propdir = alongMomentum;
        }

        std::unique_ptr<const TrackParameters> layerpar(
          m_propagator->propagate(
            ctx,
            *prevpar,
            layer->surfaceRepresentation(),
            propdir,
            true,
            oldtrajectory.m_fieldprop,
            nonInteracting
          )
        );
        
        if (layerpar == nullptr) {
          continue;
        }
        
        if (layer->surfaceRepresentation().bounds().inside(layerpar->localPosition())) {
          layerpar = m_matupdator->update(layerpar.get(), *layer, oppositeMomentum, matEffects);
        }

        prevpar = std::move(layerpar);
        first = false;
      }
      
      const Layer *startlayer = firstmeasstate->trackParameters()->associatedSurface().associatedLayer();
      
      if ((startlayer != nullptr) && (startlayer->layerMaterialProperties() != nullptr)) {
        double startfactor = startlayer->layerMaterialProperties()->alongPostFactor();
        const Surface & discsurf = startlayer->surfaceRepresentation();
        
        if (discsurf.type() == Trk::SurfaceType::Disc && discsurf.center().z() * discsurf.normal().z() < 0) {
          startfactor = startlayer->layerMaterialProperties()->oppositePostFactor();
        }
        if (startfactor > 0.5) {
          std::unique_ptr<const TrackParameters> updatedpar = m_matupdator->update(
            firstmeasstate->trackParameters(), *startlayer, oppositeMomentum, matEffects
          );

          if (updatedpar != nullptr) {
            firstmeasstate->setTrackParameters(std::move(updatedpar));
          }
        }
      }
      
      // @TODO Coverity complains about a possible NULL pointer dereferencing in lastmeasstate->...
      // Now an exception is thrown if there is no firstmeastate. Thus if the code here is
      // reached then there should be a firstmeasstate and a lastmeasstate

      const Layer *endlayer = lastmeasstate->trackParameters()->associatedSurface().associatedLayer();
      
      if ((endlayer != nullptr) && (endlayer->layerMaterialProperties() != nullptr)) {
        double endfactor = endlayer->layerMaterialProperties()->alongPreFactor();
        const Surface & discsurf = endlayer->surfaceRepresentation();
        
        if (discsurf.type() == Trk::SurfaceType::Disc && discsurf.center().z() * discsurf.normal().z() < 0) {
          endfactor = endlayer->layerMaterialProperties()->oppositePreFactor();
        }
        
        if (endfactor > 0.5) {
          std::unique_ptr<const TrackParameters> updatedpar = m_matupdator->update(
            lastmeasstate->trackParameters(), *endlayer, alongMomentum, matEffects
          );

          if (updatedpar != nullptr) {
            lastmeasstate->setTrackParameters(std::move(updatedpar));
          }
        }
      }
      
      if (prevpar != nullptr) {
        per = m_propagator->propagate(
            ctx,
            *prevpar,
            PerigeeSurface(Amg::Vector3D(0, 0, 0)),
            oppositeMomentum,
            false,
            oldtrajectory.m_fieldprop,
            nonInteracting
        );
      }
      
      if (per == nullptr) {
        ATH_MSG_DEBUG("Failed to extrapolate to perigee, returning 0");
        cache.incrementFitStatus(S_PROPAGATION_FAIL);
        cache.m_fittercode = FitterStatusCode::ExtrapolationFailure;
        return nullptr;
      }
    } else if (cache.m_acceleration && (firstmeasstate->trackParameters() != nullptr)) {
      per = m_extrapolator->extrapolate(ctx,
                                        *firstmeasstate->trackParameters(),
                                        PerigeeSurface(Amg::Vector3D(0, 0, 0)),
                                        oppositeMomentum,
                                        false,
                                        matEffects);
    } else {
      per.reset(oldtrajectory.referenceParameters()->clone());
    }

    return per;
  }

  std::unique_ptr<GXFTrackState> 
  GlobalChi2Fitter::makeTrackFindPerigee(
    const EventContext & ctx,
    Cache &cache,
    GXFTrajectory &oldtrajectory,
    const ParticleHypothesis matEffects
  ) const {
    std::unique_ptr<const TrackParameters> per = makeTrackFindPerigeeParameters(ctx, cache, oldtrajectory, matEffects);

    if (per == nullptr) {
      return nullptr;
    }

    ATH_MSG_DEBUG("Final perigee: " << *per << " pos: " << per->position() << " pT: " << per->pT());

    return std::make_unique<GXFTrackState>(std::move(per), TrackStateOnSurface::Perigee);
  }

  void GlobalChi2Fitter::holeSearchHelper(
    const std::vector<std::unique_ptr<TrackParameters>> & hc,
    std::set<Identifier> & id_set,
    std::set<Identifier> & sct_set,
    TrackHoleCount & rv,
    bool count_holes,
    bool count_dead
  ) const {
    /*
     * Our input is a list of track states, which we are iterating over. We
     * need to examine each one and update the values in our track hole count
     * accordingly.
     */
    for (const std::unique_ptr<TrackParameters> & tp : hc) {
      /*
       * It is possible, expected even, for some of these pointers to be null.
       * In those cases, it would be dangerous to continue, so we need to make
       * sure we skip them.
       */
      if (tp == nullptr) {
        continue;
      }

      /*
       * Extract the detector element of the track parameter surface for
       * examination. If for whatever reason there is none (i.e. the surface
       * is not a detector at all), we can skip it and continue.
       */
      const TrkDetElementBase * de = tp->associatedSurface().associatedDetectorElement();

      if (de == nullptr) {
        continue;
      }

      Identifier id = de->identify();

      /*
       * If, for whatever reason, we have already visited this detector, we do
       * not want to visit it again. Otherwise we might end up with modules
       * counted twice, and that would be very bad.
       */
      if (id_set.find(id) != id_set.end()) {
        continue;
      }

      /*
       * This is the meat of the pudding, we use the boundary checking tool
       * to see whether this set of parameters is a hole candidate, a dead
       * module, or not a hole at all.
       */
      BoundaryCheckResult bc = m_boundaryCheckTool->boundaryCheck(*tp);

      if (bc == BoundaryCheckResult::DeadElement && count_dead) {
        /*
         * If the module is dead, our job is very simple. We just check
         * whether it is a Pixel or an SCT and increment the appropriate
         * counter. We also insert the module into our set of visited elements.
         */
        if (m_DetID->is_pixel(id)) {
          ++rv.m_pixel_dead;
        } else if (m_DetID->is_sct(id)) {
          ++rv.m_sct_dead;
        }
        id_set.insert(id);
      } else if (bc == BoundaryCheckResult::Candidate && count_holes) {
        /*
         * If the module is a candidate, it's much the same, but we also need
         * to handle double SCT holes.
         */
        if (m_DetID->is_pixel(id)) {
          ++rv.m_pixel_hole;
        } else if (m_DetID->is_sct(id)) {
          ++rv.m_sct_hole;

          /*
           * To check for SCT double holes, we need to first fetch the other
           * side of the current SCT. Thankfully, the detector description
           * makes this very easy.
           */
          const InDetDD::SiDetectorElement* e = dynamic_cast<const InDetDD::SiDetectorElement *>(de);
          const Identifier os = e->otherSide()->identify();

          /*
           * We keep a special set containing only SCT hole IDs. We simply
           * check whether the ID of the other side of the SCT is in this set
           * to confirm that we have a double hole. Note that the first side
           * in a double hole will be counted as a SCT hole only, and the
           * second side will count as another hole as well as a double hole,
           * which is exactly the behaviour we would expect to see.
           */
          if (sct_set.find(os) != sct_set.end()) {
            ++rv.m_sct_double_hole;
          }

          /*
           * We need to add our SCT to the SCT identifier set if it is a
           * candidate hit, otherwise known as a hole in this context.
           */
          sct_set.insert(id);
        }

        /*
         * SCTs are also added to the set of all identifiers to avoid double
         * counting them.
         */
        id_set.insert(id);
      }
    }
  }

  std::vector<std::reference_wrapper<GXFTrackState>> GlobalChi2Fitter::holeSearchStates(
    GXFTrajectory & trajectory
  ) const {
    /*
     * Firstly, we will need to find the last measurement state on our track.
     * This will allow us to break the main loop later once we are done with
     * our work.
     */
    GXFTrackState * lastmeas = nullptr;

    for (const std::unique_ptr<GXFTrackState> & s : trajectory.trackStates()) {
      if (s->getStateType(TrackStateOnSurface::Measurement)) {
        lastmeas = s.get();
      }
    }

    /*
     * We create a vector of reference wrappers and reserve at least enough
     * space to contain the entire trajectory. This is perhaps a little
     * wasteful since we will never need this much space, but it may be more
     * efficient than taking the resizing pentalty on the chin.
     */
    std::vector<std::reference_wrapper<GXFTrackState>> rv;
    rv.reserve(trajectory.trackStates().size());

    /*
     * The main body of our method now. We iterate over all track states in
     * the track, at least until we find the last measurement state as found
     * above.
     */
    for (const std::unique_ptr<GXFTrackState> & s : trajectory.trackStates()) {
      /*
       * We are only interested in collecting measurements, perigees, and any
       * outlier states.
       */
      if (
        s->getStateType(TrackStateOnSurface::Measurement) ||
        s->getStateType(TrackStateOnSurface::Perigee) ||
        s->getStateType(TrackStateOnSurface::Outlier)
      ) {
        /*
         * We store a reference to the current track state in our return value
         * vector.
         */
        rv.emplace_back(*s);

        /*
         * We want to make sure we do not collect any TRT results or other
         * non-SCT and non-Pixel detector types. For that, we need to access
         * the details of the detector element and determine the detector type.
         */
        const TrkDetElementBase * de = s->trackParameters()->associatedSurface().associatedDetectorElement();

        if (de != nullptr) {
          Identifier id = de->identify();

          if (!m_DetID->is_pixel(id) && !m_DetID->is_sct(id)) {
            break;
          }
        }

        /*
         * We also have no interest in going past the final measurement, so we
         * break out of the loop if we find it.
         */
        //cppcheck-suppress iterators3 
        if (s.get() == lastmeas) {
          break;
        }
      }
    }

    return rv;
  }

  std::optional<GlobalChi2Fitter::TrackHoleCount> GlobalChi2Fitter::holeSearchProcess(
    const EventContext & ctx,
    const std::vector<std::reference_wrapper<GXFTrackState>> & states
  ) const {
    /*
     * Firstly, we need to guard against tracks having too few measurement
     * states to perform a good hole search. This is a mechanism that we
     * inherit from the reference hole search. If we have too few states, we
     * return a non-extant result to indicate an error state.
     *
     * TODO: The minimum value of 3 is also borrowed from the reference
     * implementation. It's hardcoded for now, but could be a parameter in the
     * future.
     */
    constexpr uint min_meas = 3;
    if (std::count_if(states.begin(), states.end(), [](const GXFTrackState & s){ return s.getStateType(TrackStateOnSurface::Measurement); }) < min_meas) {
      return {};
    }

    bool seen_meas = false;
    TrackHoleCount rv;
    std::set<Identifier> id_set;
    std::set<Identifier> sct_set;

    /*
     * Using an old-school integer-based for loop because we need to iterate
     * over successive pairs of states to do an extrapolation between.
     */
    for (std::size_t i = 0; i < states.size() - 1; i++) {
      /*
       * Gather references to the state at the beginning of the extrapolation,
       * named beg, and the end, named end.
       */
      GXFTrackState & beg = states[i];
      GXFTrackState & end = states[i + 1];

      /*
       * Update the boolean keeping track of whether we have seen a measurement
       * or outlier yet. Once we see one, this will remain true forever, but
       * it helps us make sure we don't collect holes before the first
       * measurement.
       */
      seen_meas |= beg.getStateType(TrackStateOnSurface::Measurement) || beg.getStateType(TrackStateOnSurface::Outlier);

      /*
       * Calculate the distance between the position of the starting parameters
       * and the end parameters. If this distance is sufficiently small, there
       * can be no elements between them (for example, between two SCTs), and
       * we don't need to do an extrapolation. This can easily save us a few
       * microseconds.
       */
      double dist = (beg.trackParameters()->position() - end.trackParameters()->position()).norm();

      /*
       * Only proceed to count holes if we have seen a measurement before (this
       * may include the starting track state, if it is a measurement) and the
       * distance between start and end is at least 2.5 millimeters.
       */
      if (seen_meas && dist >= 2.5) {
        /*
         * First, we retrieve the hole data stored in the beginning state. Note
         * that this may very well be non-extant, but it is possible for the
         * fitter to have deposited some hole information into the track state
         * earlier on in the fitting process.
         */
        std::optional<std::vector<std::unique_ptr<TrackParameters>>> & hc = beg.getHoles();
        std::vector<std::unique_ptr<TrackParameters>> states;

        /*
         * Gather the track states between the start and end of the
         * extrapolation. If the track state contained hole search information,
         * we simply move that out and use it. If there was no information, we
         * do a fresh extrapolation. This can be a CPU hog!
         */
        if (hc.has_value()) {
          states = std::move(*hc);
        } else {
          states = holesearchExtrapolation(ctx, *beg.trackParameters(), end, alongMomentum);
        }

        /*
         * Finally, we process the collected hole candidate states, checking
         * them for liveness and other properties. This helper function will
         * increment the values in rv accordingly.
         */
        holeSearchHelper(states, id_set, sct_set, rv, true, true);
      }
    }

    /*
     * Once we are done processing our measurements, we also need to do a
     * final blind extrapolation to collect and dead modules (but not holes)
     * behind the last measurement. For this, we do a blind extrapolation
     * from the final state.
     */
    GXFTrackState & last = states.back();

    /*
     * To do the blind extrapolation, we need to have a set of track parameters
     * for our last measurement state. We also check whether the position of
     * the last measurement is still inside the inner detector. If it is not,
     * we don't need to blindly extrapolate because we're only interested in
     * collecting inner detector dead modules. This check saves us a few tens
     * of microseconds.
     */
    if (
      last.trackParameters() != nullptr &&
      m_idVolume.inside(last.trackParameters()->position())
    ) {
      /*
       * Simply conduct the blind extrapolation, and then use the helper tool
       * to ensure that the hole counts are updated.
       */
      std::vector<std::unique_ptr<Trk::TrackParameters>> bl = m_extrapolator->extrapolateBlindly(
        ctx,
        *last.trackParameters(),
        Trk::alongMomentum,
        false,
        Trk::pion,
        &m_idVolume
      );

      /*
       * Note that we have flipped one of the boolean parameters of the helper
       * method here to make sure it only collects dead modules, not hole
       * candidates.
       */
      holeSearchHelper(bl, id_set, sct_set, rv, false, true);
    }

    return rv;
  }

  std::unique_ptr<Track> GlobalChi2Fitter::makeTrack(
    const EventContext & ctx,
    Cache & cache,
    GXFTrajectory & oldtrajectory,
    ParticleHypothesis matEffects
  ) const {
    // Convert internal trajectory into track
    auto  trajectory = std::make_unique<DataVector<const TrackStateOnSurface>>();
    
    if (m_fillderivmatrix) {
      makeTrackFillDerivativeMatrix(cache, oldtrajectory);
    }

    GXFTrajectory tmptrajectory(oldtrajectory);

    std::unique_ptr<GXFTrackState> perigee_ts = makeTrackFindPerigee(ctx, cache, oldtrajectory, matEffects);

    if (perigee_ts == nullptr) {
      return nullptr;
    }

    tmptrajectory.addBasicState(std::move(perigee_ts), cache.m_acceleration ? 0 : tmptrajectory.numberOfUpstreamStates());
    //reserve the ouput size
    trajectory->reserve(tmptrajectory.trackStates().size());
    for (auto & hit : tmptrajectory.trackStates()) {
      if (
        hit->measurementType() == TrackState::Pseudo &&
        hit->getStateType(TrackStateOnSurface::Outlier)
      ) {
        hit->resetTrackCovariance();
        continue;
      }
      //should check hit->isSane() here with better equality check(other than ptr comparison)
      auto trackState = hit->trackStateOnSurface();
      hit->resetTrackCovariance();
      trajectory->emplace_back(trackState.release());
    }

    auto qual = std::make_unique<FitQuality>(tmptrajectory.chi2(), tmptrajectory.nDOF());

    
    TrackInfo info;
    
    if (matEffects != electron) {
      info = TrackInfo(TrackInfo::GlobalChi2Fitter, matEffects);
    } else {
      info = TrackInfo(TrackInfo::GlobalChi2Fitter, Trk::electron);
      info.setTrackProperties(TrackInfo::BremFit);

      if (matEffects == electron && tmptrajectory.hasKink()) {
        info.setTrackProperties(TrackInfo::BremFitSuccessful);
      }
    }
    
    if (tmptrajectory.m_straightline) {
      info.setTrackProperties(TrackInfo::StraightTrack);
    }

    std::unique_ptr<Track> rv = std::make_unique<Track>(info, std::move(trajectory), std::move(qual));

    /*
     * Here, we create a track summary and attach it to our newly created
     * track. Note that this code only runs if the m_createSummary Gaudi
     * property is set. In cases where having a track summary on the track is
     * not desired, such as for compatibility with other tools, this can be
     * turned off.
     */
    if (m_createSummary.value()) {
      std::unique_ptr<TrackSummary> ts = std::make_unique<TrackSummary>();

      /*
       * This segment determines the hole search behaviour of the track fitter.
       * It is only invoked if the DoHoleSearch parameter is set, but it can
       * take a significant amount of CPU time, since the hole search is rather
       * expensive. Beware of that!
       */
      if (m_holeSearch.value()) {
        std::optional<TrackHoleCount> hole_count;

        /*
         * First, we collect a list of states that will act as our hole search
         * extrapolation states. This will serve as our source of truth in
         * regards to which track states we need to extrapolate between.
         */
        std::vector<std::reference_wrapper<GXFTrackState>> states = holeSearchStates(tmptrajectory);

        /*
         * Then, collect the actual hole search infomation using our state list
         * from before. This is the expensive operation, as it will invoke a
         * series of extrapolations if not all states have existing hole
         * information! It will also check all the hole candidates to see if
         * they are actually holes or not.
         */
        hole_count = holeSearchProcess(ctx, states);

        /*
         * Note that the hole search is not guaranteed to return a useful set
         * of values. It can, for example, reach an error state if the number
         * of measurements on a track is below a certain threshold. In that
         * case, a non-extant result will be returned, which we must guard
         * against. In that case, the hole counts will remain unset.
         */
        if (hole_count.has_value()) {
          /*
           * If the hole search did return good results, we can proceed to
           * simply copy the numerical values in the track summary.
           */
          ts->update(Trk::numberOfPixelHoles, hole_count->m_pixel_hole);
          ts->update(Trk::numberOfSCTHoles, hole_count->m_sct_hole);
          ts->update(Trk::numberOfSCTDoubleHoles, hole_count->m_sct_double_hole);
          ts->update(Trk::numberOfPixelDeadSensors, hole_count->m_pixel_dead);
          ts->update(Trk::numberOfSCTDeadSensors, hole_count->m_sct_dead);
        }
      }

      rv->setTrackSummary(std::move(ts));
    }

    return rv;
  }

  GlobalChi2Fitter::~GlobalChi2Fitter() = default;

  std::vector<std::unique_ptr<TrackParameters>> GlobalChi2Fitter::holesearchExtrapolation(
    const EventContext & ctx,
    const TrackParameters & src,
    const GXFTrackState & dst,
    PropDirection propdir
  ) const {
    /*
     * First, we conduct a bog standard stepwise extrapolation. This will
     * yield some unwanted results, but we will filter those later.
     */
    std::vector<std::unique_ptr<TrackParameters>> rv = m_extrapolator->extrapolateStepwise(
      ctx, src, dst.associatedSurface(), propdir, false
    );

    /*
     * It is possible for the first returned track parameter to be on the same
     * surface as we started on. That's probably due to some rounding errors.
     * We check for this possibility, and set the pointer to null if it
     * occurs. Note that this leaves some null pointers in the returned vector
     * but this is more performant compared to removing them properly.
     */
    if (
      !rv.empty() && (
        &rv.front()->associatedSurface() == &dst.associatedSurface() ||
        &rv.front()->associatedSurface() == &src.associatedSurface() ||
        trackParametersClose(*rv.front(), src, 0.001) ||
        trackParametersClose(*rv.front(), *dst.trackParameters(), 0.001)
      )
    ) {
      rv.front().reset(nullptr);
    }

    /*
     * Same logic, but for the last returned element. In that case, we get a
     * set of parameters on the destination surface, which we also do not
     * want.
     */
    if (
      rv.size() > 1 && (
        &rv.back()->associatedSurface() == &dst.associatedSurface() ||
        &rv.back()->associatedSurface() == &src.associatedSurface() ||
        trackParametersClose(*rv.back(), src, 0.001) ||
        trackParametersClose(*rv.back(), *dst.trackParameters(), 0.001)
      )
    ) {
      rv.back().reset(nullptr);
    }

    return rv;
  }

  GlobalChi2Fitter::PropagationResult GlobalChi2Fitter::calculateTrackParametersPropagateHelper(
    const EventContext & ctx,
    const TrackParameters & prev,
    const GXFTrackState & ts,
    PropDirection propdir,
    const MagneticFieldProperties& bf,
    bool calcderiv,
    bool holesearch
  ) const {
    std::unique_ptr<const TrackParameters> rv;
    TransportJacobian * jac = nullptr;

    if (calcderiv && !m_numderiv) {
      rv = m_propagator->propagateParameters(
          ctx, prev, ts.associatedSurface(), propdir, false, bf, jac, Trk::nonInteracting, false
      );
    } else {
      rv = m_propagator->propagateParameters(
          ctx, prev, ts.associatedSurface(), propdir, false, bf, Trk::nonInteracting, false
      );

      if (rv != nullptr && calcderiv) {
        jac = numericalDerivatives(ctx, &prev, ts.associatedSurface(), propdir, bf).release();
      }
    }

    std::optional<std::vector<std::unique_ptr<TrackParameters>>> extrapolation;

    if (holesearch) {
      extrapolation = holesearchExtrapolation(ctx, prev, ts, propdir);
    }

    return PropagationResult {
      std::move(rv),
      std::unique_ptr<TransportJacobian>(jac),
      std::move(extrapolation)
    };
  }

  GlobalChi2Fitter::PropagationResult GlobalChi2Fitter::calculateTrackParametersPropagate(
    const EventContext & ctx,
    const TrackParameters & prev,
    const GXFTrackState & ts,
    PropDirection propdir,
    const MagneticFieldProperties& bf,
    bool calcderiv,
    bool holesearch
  ) const {
    PropagationResult rv;

    rv = calculateTrackParametersPropagateHelper(
      ctx, prev, ts, propdir, bf, calcderiv, holesearch
    );

    if (rv.m_parameters == nullptr) {
      propdir = invertPropdir(propdir);

      rv = calculateTrackParametersPropagateHelper(
        ctx, prev, ts, propdir, bf, calcderiv, holesearch
      );
    }

    return rv;
  }

  FitterStatusCode GlobalChi2Fitter::calculateTrackParameters(
    const EventContext& ctx,
    GXFTrajectory & trajectory,
    bool calcderiv
  ) const {
    // Loop over states, calculate track parameters and (optionally) jacobian at each state
    ATH_MSG_DEBUG("CalculateTrackParameters");

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    int nstatesupstream = trajectory.numberOfUpstreamStates();
    const TrackParameters *prevtrackpar = trajectory.referenceParameters();
    std::unique_ptr<const TrackParameters> tmptrackpar;
    
    for (int hitno = nstatesupstream - 1; hitno >= 0; hitno--) {
      const Surface &surf1 = states[hitno]->associatedSurface();
      Trk::PropDirection propdir = Trk::oppositeMomentum;

      DistanceSolution distsol = surf1.straightLineDistanceEstimate(
        prevtrackpar->position(), prevtrackpar->momentum().unit()
      );
      
      double distance = getDistance(distsol);
      
      if (
        distance > 0 && 
        distsol.numberOfSolutions() > 0 && 
        prevtrackpar != trajectory.referenceParameters()
      ) {
        propdir = Trk::alongMomentum;
      }

      GlobalChi2Fitter::PropagationResult rv = calculateTrackParametersPropagate(
        ctx,
        *prevtrackpar,
        *states[hitno],
        propdir,
        trajectory.m_fieldprop,
        calcderiv,
        false
      );

      if (
        propdir == Trk::alongMomentum && 
        (rv.m_parameters != nullptr) && 
        (prevtrackpar->position() - rv.m_parameters->position()).mag() > 5 * mm
      ) {
        ATH_MSG_DEBUG("Propagation in wrong direction");
        
      }
      
      if (rv.m_parameters == nullptr) {
        ATH_MSG_DEBUG("propagation failed, prev par: " << *prevtrackpar <<
          " pos: " << prevtrackpar->position() << " destination surface: " << surf1);
        return FitterStatusCode::ExtrapolationFailure;
      }

      states[hitno]->setTrackParameters(std::move(rv.m_parameters));
      const TrackParameters *currenttrackpar = states[hitno]->trackParameters();
      const Surface &surf = states[hitno]->associatedSurface();

      if (rv.m_jacobian != nullptr) {
        if (
          states[hitno]->materialEffects() != nullptr &&
          states[hitno]->materialEffects()->deltaE() != 0 &&
          states[hitno]->materialEffects()->sigmaDeltaE() <= 0 &&
          !trajectory.m_straightline
        ) {
          double p = 1. / std::abs(currenttrackpar->parameters()[Trk::qOverP]);
          double de = std::abs(states[hitno]->materialEffects()->deltaE());
          double mass = trajectory.mass();
          double newp = std::sqrt(p * p + 2 * de * std::sqrt(mass * mass + p * p) + de * de);
          (*rv.m_jacobian) (4, 4) = ((p + p * de / std::sqrt(p * p + mass * mass)) / newp) * p * p / (newp * newp);
        }

        states[hitno]->setJacobian(*rv.m_jacobian);
      } else if (calcderiv) {
        ATH_MSG_WARNING("Jacobian is null");
        return FitterStatusCode::ExtrapolationFailure;
      }
      
      GXFMaterialEffects *meff = states[hitno]->materialEffects();

      if (meff != nullptr && hitno != 0) {
        std::variant<std::unique_ptr<const TrackParameters>, FitterStatusCode> r = updateEnergyLoss(
          surf, *meff, *states[hitno]->trackParameters(), trajectory.mass(), -1
        );

        if (std::holds_alternative<FitterStatusCode>(r)) {
          return std::get<FitterStatusCode>(r);
        }

        tmptrackpar = std::move(std::get<std::unique_ptr<const TrackParameters>>(r));
        prevtrackpar = tmptrackpar.get();
      } else {
        prevtrackpar = currenttrackpar;
      }
    }

    prevtrackpar = trajectory.referenceParameters();
    
    for (int hitno = nstatesupstream; hitno < (int) states.size(); hitno++) {
      const Surface &surf = states[hitno]->associatedSurface();
      Trk::PropDirection propdir = Trk::alongMomentum;
      DistanceSolution distsol = surf.straightLineDistanceEstimate(prevtrackpar->position(),  prevtrackpar->momentum().unit());
      
      double distance = getDistance(distsol);
      
      if (distance < 0 && distsol.numberOfSolutions() > 0 && prevtrackpar != trajectory.referenceParameters()) {
        propdir = Trk::oppositeMomentum;
      }

      GlobalChi2Fitter::PropagationResult rv = calculateTrackParametersPropagate(
        ctx,
        *prevtrackpar,
        *states[hitno],
        propdir,
        trajectory.m_fieldprop,
        calcderiv,
        false
      );

      if (
        (rv.m_parameters != nullptr) && 
        propdir == Trk::oppositeMomentum && 
        (prevtrackpar->position() - rv.m_parameters->position()).mag() > 5 * mm
      ) {
        ATH_MSG_DEBUG("Propagation in wrong direction");
      }
      
      if (rv.m_parameters == nullptr) {
        ATH_MSG_DEBUG("propagation failed, prev par: " << *prevtrackpar <<
          " pos: " << prevtrackpar->
          position() << " destination surface: " << surf);
        return FitterStatusCode::ExtrapolationFailure;
      }

      if (rv.m_jacobian != nullptr) {
        if (
          states[hitno]->materialEffects() != nullptr &&
          states[hitno]->materialEffects()->deltaE() != 0 &&
          states[hitno]->materialEffects()->sigmaDeltaE() <= 0 &&
          !trajectory.m_straightline
        ) {
          double p = 1 / std::abs(rv.m_parameters->parameters()[Trk::qOverP]);
          double de = std::abs(states[hitno]->materialEffects()->deltaE());
          double mass = trajectory.mass();
          double newp = p * p - 2 * de * std::sqrt(mass * mass + p * p) + de * de;
          
          if (newp > 0) {
            newp = std::sqrt(newp);
          }
          
          (*rv.m_jacobian) (4, 4) = ((p - p * de / std::sqrt(p * p + mass * mass)) / newp) * p * p / (newp * newp);
        }
        
        states[hitno]->setJacobian(*rv.m_jacobian);
      } else if (calcderiv) {
        ATH_MSG_WARNING("Jacobian is null");
        return FitterStatusCode::ExtrapolationFailure;
      }
      
      GXFMaterialEffects *meff = states[hitno]->materialEffects();

      if (meff != nullptr) {
        std::variant<std::unique_ptr<const TrackParameters>, FitterStatusCode> r = updateEnergyLoss(
          surf, *meff, *rv.m_parameters, trajectory.mass(), +1
        );

        if (std::holds_alternative<FitterStatusCode>(r)) {
          return std::get<FitterStatusCode>(r);
        }

        rv.m_parameters = std::move(std::get<std::unique_ptr<const TrackParameters>>(r));
      }

      states[hitno]->setTrackParameters(std::move(rv.m_parameters));
      prevtrackpar = states[hitno]->trackParameters();
    }
    
    return FitterStatusCode::Success;
  }

  std::variant<std::unique_ptr<const TrackParameters>, FitterStatusCode> GlobalChi2Fitter::updateEnergyLoss(
    const Surface & surf,
    const GXFMaterialEffects & meff,
    const TrackParameters & param,
    double mass,
    int sign
  ) const {
    const AmgVector(5) & old = param.parameters();

    double newphi = old[Trk::phi0] + sign * meff.deltaPhi();
    double newtheta = old[Trk::theta] + sign * meff.deltaTheta();

    if (!correctAngles(newphi, newtheta)) {
      ATH_MSG_DEBUG("Angles out of range, phi: " << newphi << " theta: " << newtheta);
      return FitterStatusCode::InvalidAngles;
    }

    double newqoverp = 0;

    if (meff.sigmaDeltaE() <= 0) {
      if (std::abs(old[Trk::qOverP]) < 1.e-12) {
        newqoverp = 0.;
      } else {
        double oldp = std::abs(1 / old[Trk::qOverP]);
        double newp2 = oldp * oldp - sign * 2 * std::abs(meff.deltaE()) * std::sqrt(mass * mass + oldp * oldp) + meff.deltaE() * meff.deltaE();

        if (newp2 < 0) {
          ATH_MSG_DEBUG("Track killed by energy loss update");
          return FitterStatusCode::ExtrapolationFailureDueToSmallMomentum;
        }

        newqoverp = std::copysign(1 / std::sqrt(newp2), old[Trk::qOverP]);
      }
    } else {
      newqoverp = old[Trk::qOverP] + sign * .001 * meff.delta_p();
    }

    return surf.createUniqueTrackParameters(
        old[0], old[1], newphi, newtheta, newqoverp, std::nullopt
    );
  }

  void GlobalChi2Fitter::calculateDerivatives(GXFTrajectory & trajectory) { 
    int nstatesupstream = trajectory.numberOfUpstreamStates();
    int nscatupstream = trajectory.numberOfUpstreamScatterers();
    int nbremupstream = trajectory.numberOfUpstreamBrems();
    int nscats = trajectory.numberOfScatterers();
    int nperpars = trajectory.numberOfPerigeeParameters(); 
    int nfitpars = trajectory.numberOfFitParameters();

    using Matrix55 = Eigen::Matrix<double, 5, 5>;

    Matrix55 initialjac;
    initialjac.setZero();
    initialjac(4, 4) = 1;
    
    Matrix55 jacvertex(initialjac);
    
    std::vector<Matrix55, Eigen::aligned_allocator<Matrix55>> jacscat(trajectory.numberOfScatterers(), initialjac);
    std::vector<Matrix55, Eigen::aligned_allocator<Matrix55>> jacbrem(trajectory.numberOfBrems(), initialjac);

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    GXFTrackState *prevstate = nullptr, *state = nullptr;

    int hit_begin = 0, hit_end = 0, scatno = 0, bremno = 0;

    for (bool forward : {false, true}) {
      if (forward) {
        hit_begin = nstatesupstream;
        hit_end = (int) states.size();  
        scatno = nscatupstream;
        bremno = nbremupstream;
      } else {
        hit_begin = nstatesupstream - 1;
        hit_end = 0;
        scatno = trajectory.numberOfUpstreamScatterers() - 1;
        bremno = trajectory.numberOfUpstreamBrems() - 1;
      }      
      
      for (
        int hitno = hit_begin;
        forward ? (hitno < hit_end) : (hitno >= hit_end); 
        hitno += (forward ? 1 : -1)
      ) {

        state = states[hitno].get();
        
        bool fillderivmat = (!state->getStateType(TrackStateOnSurface::Scatterer) && !state->getStateType(TrackStateOnSurface::BremPoint));
       
        if (fillderivmat && state->derivatives().cols() != nfitpars) {
          state->derivatives().resize(5, nfitpars);
          state->derivatives().setZero();
        }

        int jminscat = 0, jmaxscat = 4, jminbrem = 0, jmaxbrem = 4;
        
        if (hitno == (forward ? hit_end - 1 : 0)) {
          if (!fillderivmat) {
            break;
          }
          jminscat = 2;
          jmaxscat = 3;
          jminbrem = 4;
        }
        
        Eigen::Matrix<double, 5, 5> & jac = state->jacobian();

        if (hitno == nstatesupstream + (forward ? 0 : -1)) {
          jacvertex.block<4, 5>(0, 0) = jac.block<4, 5>(0, 0);
          jacvertex(4, 4) = jac(4, 4);
        } else {
          int jmin = 0, jmax = 0, jcnt = 0;
          int lp_bgn = 0, lp_end = 0;

          jmin = jminscat;
          jmax = jmaxscat;
          jcnt = jmax - jmin + 1;

          lp_bgn = forward ? nscatupstream : nscatupstream - 1;
          lp_end = scatno;

          for (int i = lp_bgn; forward ? (i < lp_end) : (i > lp_end); i += (forward ? 1 : -1)) {
            if (
              i == scatno + (forward ? -1 : 1) &&
              prevstate != nullptr && 
              prevstate->getStateType(TrackStateOnSurface::Scatterer) &&
              (!trajectory.prefit() || prevstate->materialEffects()->deltaE() == 0)
            ) {
              jacscat[i].block(0, jmin, 4, jcnt) = jac.block(0, jmin, 4, jcnt);
              jacscat[i](4, 4) = jac(4, 4);
            } else {
              calculateJac(jac, jacscat[i], jmin, jmax);
            }

            if (fillderivmat) {
              Eigen::MatrixXd & derivmat = state->derivatives();
              int scatterPos = nperpars + 2 * i;
            
              derivmat.block<4, 2>(0, scatterPos) = (forward ? 1 : -1) * jacscat[i].block<4, 2>(0, 2);
            }
          }

          jmin = jminbrem;
          jmax = jmaxbrem;
          jcnt = jmax - jmin + 1;

          lp_bgn = forward ? nbremupstream : nbremupstream - 1;
          lp_end = bremno;

          for (int i = lp_bgn; forward ? (i < lp_end) : (i > lp_end); i += (forward ? 1 : -1)) {
            if (
              i == bremno + (forward ? -1 : 1) &&
              prevstate && 
              prevstate->materialEffects() && 
              prevstate->materialEffects()->sigmaDeltaE() > 0 
            ) {
              jacbrem[i].block(0, jmin, 4, jcnt) = jac.block(0, jmin, 4, jcnt);
              jacbrem[i](4, 4) = jac(4, 4);
            } else {
              calculateJac(jac, jacbrem[i], jmin, jmax);
            }

            if (fillderivmat) {
              Eigen::MatrixXd & derivmat = state->derivatives();
              int scatterPos = nperpars + 2 * nscats + i;

              derivmat.block<5, 1>(0, scatterPos) = (forward ? .001 : -.001) * jacbrem[i].block<5, 1>(0, 4);
            }
          }

          calculateJac(jac, jacvertex, 0, 4);
        }

        if (fillderivmat) {
          Eigen::MatrixXd & derivmat = state->derivatives();
          derivmat.block(0, 0, 4, nperpars) = jacvertex.block(0, 0, 4, nperpars);

          if (nperpars == 5) {
            derivmat.col(4).segment(0, 4) *= .001;
            derivmat(4, 4) = .001 * jacvertex(4, 4);
          }
        }

        if (
          state->getStateType(TrackStateOnSurface::Scatterer) &&
          (!trajectory.prefit() || states[hitno]->materialEffects()->deltaE() == 0)
        ) {
          scatno += (forward ? 1 : -1);
        }
        
        if (
          states[hitno]->materialEffects() && 
          states[hitno]->materialEffects()->sigmaDeltaE() > 0
        ) {
          bremno += (forward ? 1 : -1);
        }
        
        prevstate = states[hitno].get();
      }
    }
  }

  void
   
    GlobalChi2Fitter::calculateTrackErrors(GXFTrajectory & trajectory,
                                           Amg::SymMatrixX & fullcovmat,
                                           bool onlylocal) const {
    //
    // Calculate track errors at each state, except scatterers and brems
    //
    ATH_MSG_DEBUG("CalculateTrackErrors");

    std::vector<std::unique_ptr<GXFTrackState>> & states = trajectory.trackStates();
    int nstatesupstream = trajectory.numberOfUpstreamStates();
      std::vector < int >indices(states.size());
    GXFTrackState *prevstate = nullptr;
    int i = nstatesupstream;
    for (int j = 0; j < (int) states.size(); j++) {
      if (j < nstatesupstream) {
        i--;
        indices[j] = i;
      } else {
        indices[j] = j;
      }
    }
    for (int stateno = 0; stateno < (int) states.size(); stateno++) {
      if (stateno == 0 || stateno == nstatesupstream) {
        prevstate = nullptr;
      }
      int index = indices[stateno];
      std::unique_ptr<GXFTrackState> & state = states[index];
      if (state->materialEffects() != nullptr) {
        prevstate = state.get();
        continue;
      }

      if (!state->hasTrackCovariance()) {
        state->zeroTrackCovariance();
      }
      AmgMatrix(5, 5) & trackerrmat = state->trackCovariance();

      if ((prevstate != nullptr) &&
          (prevstate->getStateType(TrackStateOnSurface::Measurement) ||
           prevstate->getStateType(TrackStateOnSurface::Outlier))
          && !onlylocal) {
        Eigen::Matrix<double, 5, 5> & jac = state->jacobian();
        AmgMatrix(5, 5) & prevcov = states[indices[stateno - 1]]->trackCovariance();
      
        trackerrmat = jac * prevcov * jac.transpose();
      } else {
        Amg::MatrixX & derivatives = state->derivatives();
        
        trackerrmat = derivatives * fullcovmat * derivatives.transpose();
      }

      if (!onlylocal) {
        const MeasurementBase *measurement = state->measurement();
        const Amg::MatrixX & meascov = measurement->localCovariance();
        int j = 0;
        ParamDefsAccessor paraccessor;
        int indices[5] = {
          -1, -1, -1, -1, -1
        };
        bool errorok = true;
        for (int i = 0; i < 5; i++) {
          if (measurement->localParameters().contains(paraccessor.pardef[i])) {
            if (state->getStateType(TrackStateOnSurface::Measurement)
                && trackerrmat(i, i) > meascov(j, j)) {
              errorok = false;
              double scale = std::sqrt(meascov(j, j) / trackerrmat(i, i));
              trackerrmat(i, i) = meascov(j, j);
              for (int k = 0; k < 5; k++) {
                if (k != i) {
                  trackerrmat(k, i) *= scale;
                }
              }
              indices[i] = j;
            }
            j++;
          }
        }
        for (int i = 0; i < 5; i++) {
          if (indices[i] == -1) {
            continue;
          }
          for (int j = 0; j < 5; j++) {
            if (indices[j] == -1) {
              continue;
            }
            trackerrmat(i, j) = meascov(indices[i], indices[j]);
          }
        }
        if (trajectory.m_straightline) {
          trackerrmat(4, 4) = 1e-20;
        }

        const TrackParameters *tmptrackpar =
          state->trackParameters();

        std::optional<AmgMatrix(5, 5)> trkerrmat;
        
        if (state->hasTrackCovariance()) {
          trkerrmat = (state->trackCovariance());
        } else {
          trkerrmat = std::nullopt;
        }

        const AmgVector(5) & tpars = tmptrackpar->parameters();
        std::unique_ptr<const TrackParameters> trackpar(
          tmptrackpar->associatedSurface().createUniqueTrackParameters(tpars[0],
                                                                       tpars[1],
                                                                       tpars[2],
                                                                       tpars[3],
                                                                       tpars[4],
                                                                       std::move(trkerrmat))
        );
        state->setTrackParameters(std::move(trackpar));
        FitQualityOnSurface fitQual{};
        if (state->getStateType(TrackStateOnSurface::Measurement)) {
          if (errorok && trajectory.nDOF() > 0) {
            fitQual = m_updator->fullStateFitQuality(
              *state->trackParameters(),
              measurement->localParameters(),
              measurement->localCovariance()
            );
          } else {
            fitQual = FitQualityOnSurface(0, state->numberOfMeasuredParameters());
          }
        }
        state->setFitQuality(fitQual);
      }
      prevstate = state.get();
    }
  }

  std::unique_ptr<TransportJacobian>
  GlobalChi2Fitter::numericalDerivatives(
    const EventContext& ctx,
    const TrackParameters* prevpar,
    const Surface & surf,
    PropDirection propdir,
    const MagneticFieldProperties& fieldprop) const
  {
    ParamDefsAccessor paraccessor;
    double J[25] = {
      1, 0, 0, 0, 0,
      0, 1, 0, 0, 0,
      0, 0, 1, 0, 0,
      0, 0, 0, 1, 0,
      0, 0, 0, 0, 1
    };
    std::unique_ptr<TransportJacobian> jac = std::make_unique<TransportJacobian>(J);
    const TrackParameters *tmpprevpar = prevpar;
    double eps[5] = {
      0.01, 0.01, 0.00001, 0.00001, 0.000000001
    };

    const AmgVector(5) & vec = tmpprevpar->parameters();

    bool cylsurf = surf.type() == Trk::SurfaceType::Cylinder;
    bool discsurf = surf.type() == Trk::SurfaceType::Disc;
    const Surface & previousSurface = tmpprevpar->associatedSurface();
    bool thiscylsurf = previousSurface.type() == Trk::SurfaceType::Cylinder;
    bool thisdiscsurf = previousSurface.type() == Trk::SurfaceType::Disc;

    for (int i = 0; i < 5; i++) {
      AmgVector(5) vecpluseps = vec, vecminuseps = vec;

      if (thisdiscsurf && i == 1) {
        eps[i] /= vec[0];
      }

      vecpluseps[paraccessor.pardef[i]] += eps[i];
      vecminuseps[paraccessor.pardef[i]] -= eps[i];
      if (thiscylsurf && i == 0) {
        if (vecpluseps[0] / previousSurface.bounds().r() > M_PI) {
          vecpluseps[0] -= 2 * M_PI * previousSurface.bounds().r();
        }
        if (vecminuseps[0] / previousSurface.bounds().r() < -M_PI) {
          vecminuseps[0] += 2 * M_PI * previousSurface.bounds().r();
        }
      }
      if (thisdiscsurf && i == 1) {
        if (vecpluseps[i] > M_PI) {
          vecpluseps[i] -= 2 * M_PI;
        }
        if (vecminuseps[i] < -M_PI) {
          vecminuseps[i] += 2 * M_PI;
        }
      }
      correctAngles(vecminuseps[Trk::phi], vecminuseps[Trk::theta]);
      correctAngles(vecpluseps[Trk::phi], vecpluseps[Trk::theta]);

      std::unique_ptr<const TrackParameters> parpluseps(
        tmpprevpar->associatedSurface().createUniqueTrackParameters(
          vecpluseps[0],
          vecpluseps[1],
          vecpluseps[2],
          vecpluseps[3],
          vecpluseps[4],
          std::nullopt
        )
      );
      std::unique_ptr<const TrackParameters> parminuseps(
        tmpprevpar->associatedSurface().createUniqueTrackParameters(
          vecminuseps[0],
          vecminuseps[1],
          vecminuseps[2],
          vecminuseps[3],
          vecminuseps[4],
          std::nullopt
        )
      );

      std::unique_ptr<const TrackParameters> newparpluseps(
        m_propagator->propagateParameters(
          ctx,
          *parpluseps,
          surf,
          propdir,
          false,
          fieldprop,
          Trk::nonInteracting
        )
      );
      std::unique_ptr<const TrackParameters> newparminuseps(
        m_propagator->propagateParameters(
          ctx,
          *parminuseps,
          surf,
          propdir,
          false,
          fieldprop,
          Trk::nonInteracting
        )
      );

      PropDirection propdir2 =
        (propdir ==
         Trk::alongMomentum) ? Trk::oppositeMomentum : Trk::alongMomentum;
      if (newparpluseps == nullptr) {
        newparpluseps = 
          m_propagator->propagateParameters(
            ctx,
            *parpluseps,
            surf,
            propdir2,
            false,
            fieldprop,
            Trk::nonInteracting
        );
      }
      if (newparminuseps == nullptr) {
        newparminuseps = 
          m_propagator->propagateParameters(
            ctx,
            *parminuseps,
            surf,
            propdir2,
            false,
            fieldprop,
            Trk::nonInteracting
        );
      }
      if ((newparpluseps == nullptr) || (newparminuseps == nullptr)) {
        return nullptr;
      }

      for (int j = 0; j < 5; j++) {
        double diff = newparpluseps->parameters()[paraccessor.pardef[j]] -
          newparminuseps->parameters()[paraccessor.pardef[j]];
        if (cylsurf && j == 0) {
          double length = 2 * M_PI * surf.bounds().r();
          if (std::abs(std::abs(diff) - length) < std::abs(diff)) {
            if (diff > 0) {
              diff -= length;
            } else {
              diff += length;
            }
          }
        }
        if (discsurf && j == 1) {
          if (std::abs(std::abs(diff) - 2 * M_PI) < std::abs(diff)) {
            if (diff > 0) {
              diff -= 2 * M_PI;
            } else {
              diff += 2 * M_PI;
            }
          }
        }

        (*jac) (j, i) = diff / (2 * eps[i]);
      }

    }
    return jac;
  }

  int
    GlobalChi2Fitter::iterationsOfLastFit() const {
    return 0;
  } void
    GlobalChi2Fitter::setMinIterations(int) {
    ATH_MSG_WARNING
      ("Configure the minimum number of Iterations via jobOptions");
  }

  bool
    GlobalChi2Fitter::correctAngles(double &phi, double &theta) {
    if (theta > M_PI) {
      theta = M_PI - theta;
      phi += M_PI;
    }
    if (theta < 0) {
      theta = -theta;
      phi += M_PI;
    }
    if (phi > M_PI) {
      phi -= 2 * M_PI;
    }
    if (phi < -M_PI) {
      phi += 2 * M_PI;
    }
    return theta >= 0 && theta <= M_PI && phi >= -M_PI && phi <= M_PI;
  }

  bool 
  GlobalChi2Fitter::isMuonTrack(const Track & intrk1) const {
    const auto *pDataVector = intrk1.measurementsOnTrack();
    auto nmeas1 = pDataVector->size();
    const auto *pLastValue = (*pDataVector)[nmeas1 - 1];
    //
    const bool lastMeasIsRIO = pLastValue->type(Trk::MeasurementBaseType::RIO_OnTrack);
    const bool lastMeasIsCompetingRIO = pLastValue->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack);
    //we only need the RIO on track pointer to be valid to identify
    const RIO_OnTrack *testrot{};
    //
    if (lastMeasIsRIO){
     testrot = static_cast<const RIO_OnTrack *>(pLastValue);
    } else {
      if (lastMeasIsCompetingRIO){
        const auto *testcrot = static_cast<const CompetingRIOsOnTrack*>(pLastValue);
        testrot = &testcrot->rioOnTrack(0);
      }
    }
    //still undefined, so try penultimate measurement as well
    if (testrot == nullptr) {
      const auto *pPenultimate = (*pDataVector)[nmeas1 - 2];
      const bool penultimateIsRIO = pPenultimate->type(Trk::MeasurementBaseType::RIO_OnTrack);
      const bool penultimateIsCompetingRIO  = pPenultimate->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack);
      if(penultimateIsRIO){
        testrot = static_cast<const RIO_OnTrack *>(pPenultimate);
      } else {
        if (penultimateIsCompetingRIO){
          const auto *testcrot = static_cast<const CompetingRIOsOnTrack*>(pPenultimate);
          testrot = &testcrot->rioOnTrack(0);
        }
      }
    }
    //check: we've successfully got a valid RIO on track; it's not the inner detector;
    //it's really the muon detector (question: doesn't that make the previous check redundant?)
    return (
      (testrot != nullptr) && 
      !m_DetID->is_indet(testrot->identify()) && 
      m_DetID->is_muon(testrot->identify())
    );
  }

  void
  GlobalChi2Fitter::initFieldCache(const EventContext& ctx, Cache& cache)
    const
  {
    SG::ReadCondHandle<AtlasFieldCacheCondObj> rh(
      m_field_cache_key,
      ctx
    );

    const AtlasFieldCacheCondObj * cond_obj(*rh);

    if (cond_obj == nullptr) {
      ATH_MSG_ERROR("Failed to create AtlasFieldCacheCondObj!");
      return;
    }

    cond_obj->getInitializedCache(cache.m_field_cache);
  }

  void GlobalChi2Fitter::throwFailedToGetTrackingGeomtry() const {
     std::stringstream msg;
     msg << "Failed to get conditions data " << m_trackingGeometryReadKey.key() << ".";
     throw std::runtime_error(msg.str());
  }
}
