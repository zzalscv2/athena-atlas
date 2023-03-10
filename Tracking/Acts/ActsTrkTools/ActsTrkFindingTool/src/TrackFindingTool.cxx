/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingTool.h"

// Athena
#include "TrkTrackSummary/TrackSummary.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/SourceLinkAccessorConcept.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/EventData/detail/TransformationFreeToBound.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/DiscTrapezoidBounds.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"

// PACKAGE
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"

// Other
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <sstream>
#include <functional>
#include <vector>
#include <tuple>

namespace
{
  /// =========================================================================
  /// Include all sorts of stuff needed to interface with the Acts Core classes.
  /// This is only required by code in this file, so we keep it in the anonymous namespace.
  /// The actual TrackFindingTool class definition comes later.
  /// =========================================================================

  /// Borrowed from Athena Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  /// We could also access them directly from there, but that would pull inline a lot of other stuff we
  /// don't need.

  static Acts::Result<void>
  gainMatrixUpdate(const Acts::GeometryContext &gctx,
                   typename Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type>::TrackStateProxy trackState,
                   Acts::NavigationDirection direction,
                   const Acts::Logger &logger)
  {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<ActsTrk::TrackFindingTool::traj_Type>(gctx, trackState, direction, logger);
  }

  static Acts::Result<void>
  gainMatrixSmoother(const Acts::GeometryContext &gctx,
                     Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type> &trajectory,
                     size_t entryIndex,
                     const Acts::Logger &logger)
  {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<ActsTrk::TrackFindingTool::traj_Type>(gctx, trajectory, entryIndex, logger);
  }

  /// Borrowed from Acts Examples/Framework/include/ActsExamples/EventData/GeometryContainers.hpp

  // extract the geometry identifier from a variety of types
  struct GeometryIdGetter
  {
    // explicit geometry identifier are just forwarded
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier geometryId) const
    {
      return geometryId;
    }
    // encoded geometry ids are converted back to geometry identifiers.
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier::Value encoded) const
    {
      return Acts::GeometryIdentifier(encoded);
    }
    // support elements in map-like structures.
    template <typename T>
    constexpr Acts::GeometryIdentifier operator()(
        const std::pair<Acts::GeometryIdentifier, T> &mapItem) const
    {
      return mapItem.first;
    }
    // support elements that implement `.geometryId()`.
    template <typename T>
    inline auto operator()(const T &thing) const
        -> decltype(thing.geometryId(), Acts::GeometryIdentifier())
    {
      return thing.geometryId();
    }
    // support reference_wrappers around such types as well
    template <typename T>
    inline auto operator()(std::reference_wrapper<T> thing) const
        -> decltype(thing.get().geometryId(), Acts::GeometryIdentifier())
    {
      return thing.get().geometryId();
    }
  };

  struct CompareGeometryId
  {
    // indicate that comparisons between keys and full objects are allowed.
    using is_transparent = void;
    // compare two elements using the automatic key extraction.
    template <typename Left, typename Right>
    constexpr bool operator()(Left &&lhs, Right &&rhs) const
    {
      return GeometryIdGetter()(lhs) < GeometryIdGetter()(rhs);
    }
  };

  /// Store elements that know their detector geometry id, e.g. simulation hits.
  ///
  /// @tparam T type to be stored, must be compatible with `CompareGeometryId`
  ///
  /// The container stores an arbitrary number of elements for any geometry
  /// id. Elements can be retrieved via the geometry id; elements can be selected
  /// for a specific geometry id or for a larger range, e.g. a volume or a layer
  /// within the geometry hierachy using the helper functions below. Elements can
  /// also be accessed by index that uniquely identifies each element regardless
  /// of geometry id.
  template <typename T>
  using GeometryIdMultiset =
      boost::container::flat_multiset<T, CompareGeometryId>;

  /// The accessor for the GeometryIdMultiset container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  template <typename T>
  struct GeometryIdMultisetAccessor
  {
    using Container = GeometryIdMultiset<T>;
    using Key = Acts::GeometryIdentifier;
    using Value = typename GeometryIdMultiset<T>::value_type;
    using Iterator = typename GeometryIdMultiset<T>::const_iterator;

    // pointer to the container
    const Container *container = nullptr;

    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      assert(container != nullptr);
      return container->equal_range(surface.geometryId());
    }
  };

  /// Adapted from Acts Examples/Framework/include/ActsExamples/EventData/IndexSourceLink.hpp

  /// Container of uncalibrated source links.
  ///
  /// Since the source links provide a `.geometryId()` accessor, they can be
  /// stored in an ordered geometry container.
  using UncalibSourceLinkMultiset =
      GeometryIdMultiset<ATLASUncalibSourceLink>;

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  struct UncalibSourceLinkAccessor
      : GeometryIdMultisetAccessor<ATLASUncalibSourceLink>
  {
    using BaseIterator = GeometryIdMultisetAccessor<ATLASUncalibSourceLink>::Iterator;
    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;

    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      assert(container != nullptr);
      auto [begin, end] = container->equal_range(surface.geometryId());
      return {Iterator{begin}, Iterator{end}};
    }
  };

  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::EigenStepper<>;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsTrk::TrackFindingTool::traj_Type>;

  // get Athena SiDetectorElement from Acts surface
  static const InDetDD::SiDetectorElement *actsToDetElem(const Acts::Surface &surface)
  {
    const auto *actsElement = dynamic_cast<const ActsDetectorElement *>(surface.associatedDetectorElement());
    if (!actsElement)
    {
      return nullptr;
    }
    return dynamic_cast<const InDetDD::SiDetectorElement *>(actsElement->upstreamDetectorElement());
  }

  /// =========================================================================
  /// Debug printout routines
  /// This is only required by code in this file, so we keep it in the anonymous namespace.
  /// The actual TrackFindingTool class definition comes later.
  /// =========================================================================

  /// format all arguments and return as a string.
  /// Used here to apply std::setw() to the combination of values.
  template <typename... Types>
  static std::string to_string(Types &&...values)
  {
    std::ostringstream os;
    (os << ... << values);
    return os.str();
  }

  static std::string trackStateName(Acts::TrackStateType trackState)
  {
    static constexpr std::array<std::tuple<bool, Acts::TrackStateFlag, char>, Acts::TrackStateFlag::NumTrackStateFlags> trackStateNames{{
        {false, Acts::TrackStateFlag::ParameterFlag, '-'},
        {true, Acts::TrackStateFlag::MeasurementFlag, 'M'},
        {true, Acts::TrackStateFlag::OutlierFlag, 'O'},
        {true, Acts::TrackStateFlag::HoleFlag, 'H'},
        {true, Acts::TrackStateFlag::MaterialFlag, 'm'},
        {true, Acts::TrackStateFlag::SharedHitFlag, 'S'},
    }};
    std::string s;
    for (auto [b, f, c] : trackStateNames)
    {
      if (trackState[f] == b)
        s += c;
    }
    return s;
  }

  // compact surface/boundary name
  static std::string surfaceName(const Acts::Surface &surface)
  {
    std::string name = surface.name();
    if (name.compare(0, 6, "Acts::") == 0)
    {
      name.erase(0, 6);
    }
    if (name.size() > 7 && name.compare(name.size() - 7, 7, "Surface") == 0)
    {
      name.erase(name.size() - 7, 7);
    }
    static const std::map<Acts::SurfaceBounds::BoundsType, const char *> boundsNames{{
        {Acts::SurfaceBounds::BoundsType::eCone, "Cone"},
        {Acts::SurfaceBounds::BoundsType::eCylinder, "Cylinder"},
        {Acts::SurfaceBounds::BoundsType::eDiamond, "Diamond"},
        {Acts::SurfaceBounds::BoundsType::eDisc, "Disc"},
        {Acts::SurfaceBounds::BoundsType::eEllipse, "Ellipse"},
        {Acts::SurfaceBounds::BoundsType::eLine, "Line"},
        {Acts::SurfaceBounds::BoundsType::eRectangle, "Rectangle"},
        {Acts::SurfaceBounds::BoundsType::eTrapezoid, "Trapezoid"},
        {Acts::SurfaceBounds::BoundsType::eTriangle, "Triangle"},
        {Acts::SurfaceBounds::BoundsType::eDiscTrapezoid, "DiscTrapezoid"},
        {Acts::SurfaceBounds::BoundsType::eConvexPolygon, "ConvexPolygon"},
        {Acts::SurfaceBounds::BoundsType::eAnnulus, "Annulus"},
        {Acts::SurfaceBounds::BoundsType::eBoundless, "Boundless"},
        {Acts::SurfaceBounds::BoundsType::eOther, "Other"},
    }};
    if (auto it = boundsNames.find(surface.bounds().type());
        it != boundsNames.end() && it->second != name)
    {
      name += ' ';
      name += it->second;
    }
    return name;
  }

  static void printHeader(bool isMeasurement = false)
  {
    std::cout << std::left
              << std::setw(5) << "Index" << ' '
              << std::setw(4) << "Type" << ' '
              << std::setw(21) << "SurfaceBounds" << ' ';
    if (isMeasurement)
    {
      std::cout << std::setw(21) << "GeometryId" << ' '
                << std::right
                << std::setw(9) << "loc0(-)" << ' '
                << std::setw(9) << "loc1(-)" << ' '
                << std::setw(6) << "Pos R" << ' '
                << std::setw(6) << "phid" << ' '
                << std::setw(7) << "eta" << ' '
                << std::setw(9) << "loc0+" << ' '
                << std::setw(9) << "loc1+" << ' '
                << std::setw(6) << "Pos R" << ' '
                << std::setw(6) << "phid" << ' '
                << std::setw(7) << "eta" << '\n';
    }
    else
    {
      std::cout << std::setw(21) << "GeometryId / stats" << ' '
                << std::right
                << std::setw(9) << "loc0" << ' '
                << std::setw(9) << "loc1" << ' '
                << std::setw(6) << "Pos R" << ' '
                << std::setw(6) << "phid" << ' '
                << std::setw(7) << "eta" << ' '
                << std::setw(9) << "q*pT" << ' '
                << std::setw(6) << "phid" << ' '
                << std::setw(7) << "eta" << ' '
                << std::setw(6) << "TrkLen" << ' '
                << std::setw(7) << "chi2" << ' '
                << std::setw(6) << "Flags" << '\n';
    }
  }

  // Return local position of the ends of the 1D strips.
  static std::array<Acts::Vector2, 2>
  stripEnds(const Acts::SurfaceBounds &bounds, const Acts::Vector2 &loc)
  {
    auto b = bounds.values();
    switch (bounds.type())
    {
    case Acts::SurfaceBounds::eRectangle:
    {
      return {{{loc[0], b[Acts::RectangleBounds::eMinY]}, {loc[0], b[Acts::RectangleBounds::eMaxY]}}};
    }
    case Acts::SurfaceBounds::eTrapezoid:
    {
      auto halfLengthY = b[Acts::TrapezoidBounds::eHalfLengthY];
      return {{{loc[0], -halfLengthY}, {loc[0], halfLengthY}}};
    }
    case Acts::SurfaceBounds::eAnnulus:
    {
      // Annulus needs measurement coordinate as loc1, but is input as 1D in loc[0].
      // This method doesn't rotate into the strip coordinate system, so won't be exact,
      // but surely good enough for printout.
      return {{{b[Acts::AnnulusBounds::eMinR], loc[0]}, {b[Acts::AnnulusBounds::eMaxR], loc[0]}}};
    }
    case Acts::SurfaceBounds::eDiscTrapezoid:
    {
      auto averagePhi = b[Acts::DiscTrapezoidBounds::eAveragePhi];
      auto alphaMinR = std::atan2(b[Acts::DiscTrapezoidBounds::eMinR], b[Acts::DiscTrapezoidBounds::eHalfLengthXminR]);
      auto alphaMaxR = std::atan2(b[Acts::DiscTrapezoidBounds::eMaxR], b[Acts::DiscTrapezoidBounds::eHalfLengthXmaxR]);
      auto alpha = std::max(alphaMinR, alphaMaxR);
      return {{{loc[0], averagePhi - alpha}, {loc[0], averagePhi + alpha}}};
    }
    case Acts::SurfaceBounds::eDisc:
    {
      auto averagePhi = b[Acts::RadialBounds::eAveragePhi];
      auto halfPhiSector = b[Acts::RadialBounds::eHalfPhiSector];
      return {{{loc[0], averagePhi - halfPhiSector}, {loc[0], averagePhi + halfPhiSector}}};
    }
    default:
    {
      return {{loc, loc}};
    }
    }
  }

  static void
  printMeasurement(const Acts::GeometryContext &tgContext, const Acts::Surface &surface, const Acts::Vector2 &loc)
  {
    auto p = surface.localToGlobal(tgContext, loc, Acts::Vector3{0.0, 0.0, 0.0});
    std::cout << ' '
              << std::setw(9) << std::setprecision(3) << loc[0] << ' '
              << std::setw(9) << std::setprecision(3) << loc[1] << ' '
              << std::setw(6) << std::setprecision(1) << p.head<2>().norm() << ' '
              << std::setw(6) << std::setprecision(1) << std::atan2(p[1], p[0]) / Acts::UnitConstants::degree << ' '
              << std::setw(7) << std::setprecision(3) << std::atanh(p[2] / p.norm());
  }

  static void
  printSourceLink(const Acts::GeometryContext &tgContext,
                  const Acts::TrackingGeometry &trackingGeometry,
                  const ATLASUncalibSourceLink &sl,
                  size_t index)
  {
    std::cout << std::setw(5) << index << ' '
              << std::setw(3) << sl.dim() << "D ";
    auto surface = trackingGeometry.findSurface(sl.geometryId());
    if (!surface)
    {
      std::cout << "*** no surface ***\n";
      return;
    }
    std::cout << std::left
              << std::setw(21) << surfaceName(*surface) << ' '
              << std::setw(21) << to_string(surface->geometryId())
              << std::right << std::defaultfloat << std::fixed;
    Acts::Vector2 loc{sl.values().head<2>()};
    if (sl.dim() != 1)
    {
      printMeasurement(tgContext, *surface, loc);
    }
    else
    {
      auto ends = stripEnds(surface->bounds(), loc);
      printMeasurement(tgContext, *surface, ends[0]);
      if (ends[0] == ends[1])
      {
        std::cout << " *** zero strip length ***";
      }
      else
      {
        printMeasurement(tgContext, *surface, ends[1]);
      }
    }
    std::cout << std::defaultfloat << '\n';
  }

  static void printParameters(const Acts::Surface &surface, const Acts::GeometryContext &tgContext, const Acts::BoundVector &bound)
  {
    auto p = Acts::detail::transformBoundToFreeParameters(surface, tgContext, bound);
    std::cout << std::defaultfloat << std::fixed
              << std::setw(9) << std::setprecision(3) << bound[Acts::eBoundLoc0] << ' '
              << std::setw(9) << std::setprecision(3) << bound[Acts::eBoundLoc1] << ' '
              << std::setw(6) << std::setprecision(1) << p.segment<2>(Acts::eFreePos0).norm() << ' '
              << std::setw(6) << std::setprecision(1) << std::atan2(p[Acts::eFreePos1], p[Acts::eFreePos0]) / Acts::UnitConstants::degree << ' '
              << std::setw(7) << std::setprecision(3) << std::atanh(p[Acts::eFreePos2] / p.segment<3>(Acts::eFreePos0).norm()) << ' '
              << std::setw(9) << std::setprecision(3) << p.segment<2>(Acts::eFreeDir0).norm() / p[Acts::eFreeQOverP] << ' '
              << std::setw(6) << std::setprecision(1) << std::atan2(p[Acts::eFreeDir1], p[Acts::eFreeDir0]) / Acts::UnitConstants::degree << ' '
              << std::setw(7) << std::setprecision(2) << std::atanh(p[Acts::eFreeDir2])
              << std::defaultfloat;
  }

  static void
  printTrackState(const Acts::GeometryContext &tgContext,
                  const Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type>::ConstTrackStateProxy &state)
  {
    std::cout << std::defaultfloat << std::fixed
              << std::setw(5) << state.index() << ' ';
    if (state.hasCalibrated())
    {
      std::cout << std::setw(3) << state.calibratedSize() << 'D';
    }
    else if (state.typeFlags()[Acts::TrackStateFlag::HoleFlag])
    {
      std::cout << std::setw(4) << "hole";
    }
    else
    {
      std::cout << std::setw(4) << " ";
    }
    std::cout << ' '
              << std::left
              << std::setw(21) << surfaceName(state.referenceSurface()) << ' '
              << std::setw(21) << to_string(state.referenceSurface().geometryId()) << ' '
              << std::right;
    printParameters(state.referenceSurface(), tgContext, state.parameters());
    std::cout << std::defaultfloat << std::fixed << ' '
              << std::setw(6) << std::setprecision(1) << state.pathLength() << ' '
              << std::setw(7) << std::setprecision(1) << state.chi2() << ' '
              << std::defaultfloat
              << std::setw(Acts::TrackStateFlag::NumTrackStateFlags) << trackStateName(state.typeFlags()) << '\n';
  }

  static void
  printTracks(const Acts::GeometryContext &tgContext,
              const Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder> &tracks,
              const std::vector<typename Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>::TrackProxy> &fitResult,
              const Acts::BoundTrackParameters &seed,
              size_t iseed,
              size_t ntracks)
  {
    printHeader();
    std::cout << std::setw(5) << iseed << ' '
              << std::left
              << std::setw(4) << "seed" << ' '
              << std::setw(21) << surfaceName(seed.referenceSurface()) << ' '
              << std::setw(21) << to_string("#traj=", fitResult.size(), ", #trk=", ntracks) << ' '
              << std::right;
    printParameters(seed.referenceSurface(), tgContext, seed.parameters());
    std::cout << '\n';

    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();
      // to print track states from inside outward, we need to reverse the order of visitBackwards().
      std::vector<Acts::MultiTrajectory<ActsTrk::TrackFindingTool::traj_Type>::ConstTrackStateProxy> states;
      states.reserve(lastMeasurementIndex + 1); // could be an overestimate
      size_t npixel = 0, nstrip = 0;
      tracks.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&](const Acts::VectorMultiTrajectory::ConstTrackStateProxy &state) -> void
          {
            if (state.hasCalibrated())
            {
              if (state.calibratedSize() == 1)
                ++nstrip;
              else if (state.calibratedSize() == 2)
                ++npixel;
            }
            states.push_back(state);
          });

      const Acts::BoundTrackParameters per(track.referenceSurface().getSharedPtr(),
                                           track.parameters(),
                                           track.covariance());
      std::cout << std::setw(5) << lastMeasurementIndex << ' '
                << std::left
                << std::setw(4) << "parm" << ' '
                << std::setw(21) << surfaceName(per.referenceSurface()) << ' '
                << std::setw(21) << to_string("#pix=", npixel, ", #strip=", nstrip) << ' '
                << std::right;
      printParameters(per.referenceSurface(), tgContext, per.parameters());
      std::cout << '\n';

      for (auto i = states.size(); i > 0;)
      {
        printTrackState(tgContext, states[--i]);
      }
    }
    std::cout << std::defaultfloat << std::flush;
  }

} // anonymous namespace

/// =========================================================================
namespace ActsTrk
{
  struct TrackFindingTool::CKF_pimpl : public CKF
  {
    using CKF::CKF;
  };

  TrackFindingTool::TrackFindingTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
      : base_class(type, name, parent)
  {
  }

  TrackFindingTool::~TrackFindingTool() = default;

  StatusCode TrackFindingTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);
    ATH_MSG_DEBUG("   " << m_doPrintTrackStates);

    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_trkSummaryTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_boundaryCheckTool.retrieve());
    if (!m_RotCreatorTool.empty())
    {
      ATH_MSG_INFO("RotCreatorTool will be used");
      ATH_CHECK(m_RotCreatorTool.retrieve());
    }

    m_logger = makeActsAthenaLogger(this, "Acts");

    auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
    auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    Stepper stepper(std::move(magneticField));
    Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    Navigator navigator(cfg);
    Propagator propagator(std::move(stepper), std::move(navigator), logger().cloneWithSuffix("Prop"));
    m_trackFinder.reset(new CKF_pimpl(std::move(propagator), logger().cloneWithSuffix("CKF")));

    m_pOptions.maxSteps = m_maxPropagationStep;

    Acts::MeasurementSelector::Config measurementSelectorCfg{{Acts::GeometryIdentifier(),
                                                              {m_etaBins, m_chi2CutOff, m_numMeasurementsCutOff}}};
    m_measurementSelector.reset(new Acts::MeasurementSelector{measurementSelectorCfg});

    m_ckfExtensions.updater.connect<&gainMatrixUpdate>();
    m_ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    m_ckfExtensions.calibrator.connect<&ATLASSourceLinkCalibrator::calibrate<traj_Type, ATLASUncalibSourceLink>>();
    m_ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<traj_Type>>(m_measurementSelector.get());

    return StatusCode::SUCCESS;
  }

  // finalize
  StatusCode TrackFindingTool::finalize()
  {
    ATH_MSG_INFO(name() << " statistics:");
    ATH_MSG_INFO("- total seeds: " << m_nTotalSeeds);
    ATH_MSG_INFO("- failed seeds: " << m_nFailedSeeds);
    ATH_MSG_INFO("- failure ratio: " << static_cast<double>(m_nFailedSeeds) / m_nTotalSeeds);
    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingTool::findTracks(const EventContext &ctx,
                               const std::vector<std::pair<UncalibratedMeasurementContainerPtr, const InDetDD::SiDetectorElementCollection *>> &measurements,
                               const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                               ::TrackCollection &tracksContainer) const
  {
    /// ACTS container is just a std::vector of reconstructed track states for multiple tracks.
    /// Athena uses a DataVector.
    using TrackParametersContainer = std::vector<Acts::BoundTrackParameters>;

    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    auto numMeasurements = std::accumulate(measurements.begin(), measurements.end(), size_t{0u},
                                           [](size_t n, const auto &m)
                                           { return n + std::visit([](auto &&v) -> size_t
                                                                   { return v->size(); },
                                                                   m.first); });
    ATH_MSG_DEBUG("Create " << numMeasurements << " source links");

    UncalibSourceLinkMultiset sourceLinks;
    sourceLinks.reserve(numMeasurements);

    std::vector<ATLASUncalibSourceLink::ElementsType> elementsCollection;
    elementsCollection.reserve(numMeasurements);

    for (auto &[clusterContainerVar, detEleCollPtr] : measurements)
    {
      auto detEleColl = detEleCollPtr; // structured bindings can't be captured in C++17
      std::visit([&](auto &&clusterContainer)
                 {
                   for (auto meas : *clusterContainer) {
                     sourceLinks.insert(sourceLinks.end(),
                                        m_ATLASConverterTool->uncalibratedTrkMeasurementToSourceLink(*detEleColl, *meas, elementsCollection));
                   } },
                 clusterContainerVar);
    }

    TrackParametersContainer initialParameters;
    initialParameters.reserve(estimatedTrackParameters.size());
    for (auto *trkParms : estimatedTrackParameters)
    {
      initialParameters.push_back(*trkParms);
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    // CalibrationContext converter not implemented yet.
    Acts::CalibrationContext calContext = Acts::CalibrationContext();

    UncalibSourceLinkAccessor slAccessor;
    slAccessor.container = &sourceLinks;
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, traj_Type>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               m_ckfExtensions,
                               m_pOptions,
                               &(*pSurface));

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << initialParameters.size() << " seeds.");

    Acts::TrackContainer tc{Acts::VectorTrackContainer{},
                            Acts::VectorMultiTrajectory{}};

    m_nTotalSeeds += initialParameters.size();

    auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();
    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < initialParameters.size(); ++iseed)
    {
      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects

      auto result = m_trackFinder->findTracks(initialParameters[iseed], options, tc);

      // The result for this seed
      if (not result.ok())
      {
        ATH_MSG_WARNING("Track finding failed for seed " << iseed << " with error" << result.error());
        m_nFailedSeeds++;
        continue;
      }
      // Get the track finding output and add to tracksContainer
      size_t ntracks = makeTracks(ctx, tgContext, tc, result.value(), tracksContainer);

      if (m_doPrintTrackStates)
      {
        if (iseed == 0)
        {
          ATH_MSG_INFO("CKF input hits:");
          printHeader(true);
          size_t index = 0;
          for (auto &sourceLink : sourceLinks)
          {
            printSourceLink(tgContext, *trackingGeometry, sourceLink, index++);
          }
          std::cout << std::defaultfloat << std::flush;
          ATH_MSG_INFO("CKF results for " << initialParameters.size() << " seeds:");
        }
        printTracks(tgContext, tc, result.value(), initialParameters[iseed], iseed, ntracks);
      }

      if (ntracks == 0)
      {
        ATH_MSG_WARNING("Track finding found no track candidates for seed " << iseed);
        m_nFailedSeeds++;
      }
    }

    ATH_MSG_DEBUG("Completed track finding with " << tracksContainer.size() << " track candidates.");

    return StatusCode::SUCCESS;
  }

  /// based on Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  size_t
  TrackFindingTool::makeTracks(const EventContext &ctx,
                               const Acts::GeometryContext &tgContext,
                               const Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder> &tracks,
                               const std::vector<typename Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail_tc::ValueHolder>::TrackProxy> &fitResult,
                               ::TrackCollection &tracksContainer) const
  {
    size_t ntracks = 0;
    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();

      auto finalTrajectory = DataVector<const Trk::TrackStateOnSurface>();
      // initialise the number of dead Pixel and Acts strip
      int numberOfDeadPixel = 0;
      int numberOfDeadSCT = 0;

      std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
      // Loop over all the output state to create track state
      tracks.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&](const Acts::VectorMultiTrajectory::ConstTrackStateProxy &state) -> void
          {
            // First only consider states with an associated detector element
            if (!state.referenceSurface().associatedDetectorElement())
            {
              return;
            }

            // We need to determine the type of state
            auto flag = state.typeFlags();
            std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
            std::unique_ptr<const Trk::TrackParameters> parm;

            // State is a hole (no associated measurement), use predicted parameters
            if (flag[Acts::TrackStateFlag::HoleFlag])
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.predicted(),
                                                         state.predictedCovariance());
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);

              // Check if this is a hole, a dead sensors or a state outside the sensor boundary
              if (boundaryCheck == Trk::BoundaryCheckResult::DeadElement)
              {
                auto *detElem = actsToDetElem(state.referenceSurface());
                if (!detElem)
                {
                  return;
                }
                if (detElem->isPixel())
                {
                  ++numberOfDeadPixel;
                }
                else if (detElem->isSCT())
                {
                  ++numberOfDeadSCT;
                }
                // Dead sensors states are not stored
                return;
              }
              else if (boundaryCheck != Trk::BoundaryCheckResult::Candidate)
              {
                // States outside the sensor boundary are ignored
                return;
              }
              typePattern.set(Trk::TrackStateOnSurface::Hole);
            }
            // The state was tagged as an outlier or (TODO!) was missed in the reverse filtering, use filtered parameters
            else if (flag[Acts::TrackStateFlag::OutlierFlag]
                     //  || (fitResult.reversed &&     // TODO!
                     //      std::find(fitResult.passedAgainSurfaces.begin(),
                     //                fitResult.passedAgainSurfaces.end(),
                     //                state.referenceSurface().getSharedPtr().get()) == fitResult.passedAgainSurfaces.end())
            )
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.filtered(),
                                                         state.filteredCovariance());
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Outlier);
            }
            // The state is a measurement state, use smoothed parameters
            else
            {
              const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                         state.smoothed(),
                                                         state.smoothedCovariance());

              // is it really necessary to keep our own copy of all the smoothed parameters?
              actsSmoothedParam.push_back(std::make_unique<const Acts::BoundTrackParameters>(Acts::BoundTrackParameters(actsParam)));
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Measurement);
            }

            std::unique_ptr<const Trk::MeasurementBase> measState;
            if (state.hasUncalibratedSourceLink())
            {
              const auto &sl = state.uncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
              const xAOD::UncalibratedMeasurement &uncalibMeas = sl.atlasHit();
              measState = makeRIO_OnTrack(uncalibMeas, parm.get());
            }

            double nDoF = state.calibratedSize();
            auto quality = Trk::FitQualityOnSurface(state.chi2(), nDoF);
            auto perState = new Trk::TrackStateOnSurface(quality,
                                                         std::move(measState),
                                                         std::move(parm),
                                                         nullptr,
                                                         typePattern);

            // If a state was succesfully created add it to the trajectory
            finalTrajectory.insert(finalTrajectory.begin(), perState);
          });

      // Convert the perigee state and add it to the trajectory
      const Acts::BoundTrackParameters actsPer(track.referenceSurface().getSharedPtr(),
                                               track.parameters(),
                                               track.covariance());

      std::unique_ptr<const Trk::TrackParameters> per = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsPer, tgContext);
      std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
      typePattern.set(Trk::TrackStateOnSurface::Perigee);
      auto perState = new Trk::TrackStateOnSurface(nullptr,
                                                   std::move(per),
                                                   nullptr,
                                                   typePattern);
      finalTrajectory.insert(finalTrajectory.begin(), perState);

      // Create the track using Athena TrackFitter::KalmanFitter and TrackPatternRecoInfo::SiSPSeededFinder algorithm enums
      Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::KalmanFitter, Trk::noHypothesis);
      newInfo.setPatternRecognitionInfo(Trk::TrackInfo::TrackPatternRecoInfo::SiSPSeededFinder);

      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      // TODO: use TrackSummaryTool to create trackSummary?
      if (!newtrack->trackSummary())
      {
        newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
        newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
        newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
      }
      m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
      tracksContainer.push_back(std::move(newtrack));
      ++ntracks;
    }

    return ntracks;
  }

  std::unique_ptr<const Trk::MeasurementBase>
  TrackFindingTool::makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
                                    const Trk::TrackParameters *parm) const
  {
    const Trk::PrepRawData *rio = nullptr;
    if (auto pixcl = dynamic_cast<const xAOD::PixelCluster *>(&uncalibMeas))
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::PixelClusterCollection>> pixelLinkAcc("pixelClusterLink");
      if (!pixelLinkAcc.isAvailable(*pixcl))
      {
        ATH_MSG_WARNING("no pixelClusterLink for cluster associated to measurement");
        return nullptr;
      }
      auto pix = *pixelLinkAcc(*pixcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = pix->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access pixel detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::PixelClusterOnTrack without correction");
        return std::make_unique<const InDet::PixelClusterOnTrack>(pix,
                                                                  Trk::LocalParameters(pix->localPosition()),
                                                                  pix->localCovariance(),
                                                                  element->identifyHash(),
                                                                  pix->globalPosition(),
                                                                  pix->gangedPixel(),
                                                                  false);
      }
      rio = pix;
    }
    else if (auto stripcl = dynamic_cast<const xAOD::StripCluster *>(&uncalibMeas))
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::SCT_ClusterCollection>> stripLinkAcc("sctClusterLink");
      if (!stripLinkAcc.isAvailable(*stripcl))
      {
        ATH_MSG_WARNING("no sctClusterLink for clusters associated to measurement");
        return nullptr;
      }
      auto sct = *stripLinkAcc(*stripcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = sct->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access strip detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::SCT_ClusterOnTrack without correction");
        return std::make_unique<const InDet::SCT_ClusterOnTrack>(sct,
                                                                 Trk::LocalParameters(sct->localPosition()),
                                                                 sct->localCovariance(),
                                                                 element->identifyHash(),
                                                                 sct->globalPosition(),
                                                                 false);
      }
      rio = sct;
    }
    else
    {
      ATH_MSG_WARNING("xAOD::UncalibratedMeasurement is neither xAOD::PixelCluster nor xAOD::StripCluster");
      return nullptr;
    }

    ATH_MSG_DEBUG("use Trk::RIO_OnTrackCreator::correct to create corrected Trk::RIO_OnTrack");
    assert(!m_RotCreatorTool.empty());
    assert(rio != nullptr);
    return std::unique_ptr<const Trk::MeasurementBase>(m_RotCreatorTool->correct(*rio, *parm));
  }

} // namespace ActsTrk
