/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H 1

// ACTS
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "src/ITrackStatePrinter.h"

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <utility>
#include <vector>
#include <variant>

namespace
{
  /// =========================================================================
  /// Include all sorts of stuff needed to interface with the Acts Core classes.
  /// This is only required by code in TrackFindingAlg.cxx, so we keep it in the anonymous namespace.
  /// =========================================================================

  /// Borrowed from Athena Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  /// We could also access them directly from there, but that would pull inline a lot of other stuff we
  /// don't need.

  static Acts::Result<void>
  gainMatrixUpdate(const Acts::GeometryContext &gctx,
                   typename ActsTrk::TrackStateBackend::TrackStateProxy trackState,
                   Acts::Direction direction,
                   const Acts::Logger &logger)
  {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<ActsTrk::TrackStateBackend>(gctx, trackState, direction, logger);
  }

  static Acts::Result<void>
  gainMatrixSmoother(const Acts::GeometryContext &gctx,
                     ActsTrk::TrackStateBackend &trajectory,
                     size_t entryIndex,
                     const Acts::Logger &logger)
  {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<ActsTrk::TrackStateBackend>(gctx, trajectory, entryIndex, logger);
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
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsTrk::TrackStateBackend>;

  // Small holder class to keep CKF and related objects.
  // Keep a unique_ptr<CKF_pimpl> in TrackFindingAlg, so we don't have to expose the
  // Acts class definitions to in TrackFindingAlg.h.
  // ActsTrk::TrackFindingAlg::CKF_pimpl inherits from CKF_config to prevent -Wsubobject-linkage warning.
  struct CKF_config
  {
    // CKF algorithm
    CKF ckf;
    // CKF configuration
    Acts::MeasurementSelector measurementSelector;
    Acts::PropagatorPlainOptions pOptions;
    Acts::CombinatorialKalmanFilterExtensions<ActsTrk::TrackStateBackend> ckfExtensions;
  };

  // === DuplicateSeedDetector ================================
  // Identify duplicate seeds: seeds where all measurements were already located in a previously followed trajectory.
  class DuplicateSeedDetector
  {
  public:
    DuplicateSeedDetector(size_t numSeeds, bool enabled)
        : m_disabled(!enabled),
          m_nUsedMeasurements(enabled ? numSeeds : 0u, 0u),
          m_nSeedMeasurements(enabled ? numSeeds : 0u, 0u),
          m_isDuplicateSeed(enabled ? numSeeds : 0u, false)
    {
      if (m_disabled)
        return;
      m_seedIndex.reserve(6 * numSeeds); // 6 hits/seed for strips (3 for pixels)
    }

    DuplicateSeedDetector() = delete;
    DuplicateSeedDetector(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector &operator=(const DuplicateSeedDetector &) = delete;

    // add seeds from an associated measurements collection.
    // measurementOffset non-zero is only needed if measurements holds more than one collection (eg. kept for TrackStatePrinter).
    size_t addSeeds(const ActsTrk::SeedContainer &seeds, const std::vector<ATLASUncalibSourceLink> &measurements, size_t measurementOffset = 0)
    {
      size_t seedOffset = m_numSeed;
      if (m_disabled)
        return seedOffset;
      for (const auto *seed : seeds)
      {
        if (!seed)
          continue;
        for (const auto *sp : seed->sp())
        {
          for (auto index : sp->measurementIndexes())
          {
            size_t imeasurement = measurementOffset + index;
            assert(imeasurement < measurements.size());
            m_seedIndex.insert({&measurements[imeasurement].atlasHit(), m_numSeed});
            ++m_nSeedMeasurements[m_numSeed];
          }
        }
        ++m_numSeed;
      }
      return seedOffset;
    }

    void newTrajectory()
    {
      if (m_disabled || m_found == 0 || m_nextSeed == m_nUsedMeasurements.size())
        return;
      auto beg = m_nUsedMeasurements.begin();
      if (m_nextSeed < m_nUsedMeasurements.size())
        std::advance(beg, m_nextSeed);
      std::fill(beg, m_nUsedMeasurements.end(), 0u);
    }

    void addMeasurement(const ATLASUncalibSourceLink &sl)
    {
      if (m_disabled || m_nextSeed == m_nUsedMeasurements.size())
        return;
      for (auto [iiseed, eiseed] = m_seedIndex.equal_range(&sl.atlasHit()); iiseed != eiseed; ++iiseed)
      {
        size_t iseed = iiseed->second;
        assert(iseed < m_nUsedMeasurements.size());
        if (iseed < m_nextSeed || m_isDuplicateSeed[iseed])
          continue;
        if (++m_nUsedMeasurements[iseed] >= m_nSeedMeasurements[iseed])
        {
          assert(m_nUsedMeasurements[iseed] == m_nSeedMeasurements[iseed]); // shouldn't ever find more
          m_isDuplicateSeed[iseed] = true;
        }
        ++m_found;
      }
    }

    // For complete removal of duplicate seeds, assumes isDuplicate(iseed) is called for monotonically increasing iseed.
    bool isDuplicate(size_t iseed)
    {
      if (m_disabled)
        return false;
      assert(iseed < m_isDuplicateSeed.size());
      // If iseed not increasing, we will miss some duplicate seeds, but won't exclude needed seeds.
      if (iseed >= m_nextSeed)
        m_nextSeed = iseed + 1;
      return m_isDuplicateSeed[iseed];
    }

  private:
    bool m_disabled = false;
    boost::container::flat_multimap<const xAOD::UncalibratedMeasurement *, size_t> m_seedIndex;
    std::vector<size_t> m_nUsedMeasurements;
    std::vector<size_t> m_nSeedMeasurements;
    std::vector<bool> m_isDuplicateSeed;
    size_t m_numSeed = 0u;  // count of number of seeds so-far added with addSeeds()
    size_t m_nextSeed = 0u; // index of next seed expected with isDuplicate()
    size_t m_found = 0u;    // count of found seeds for this/last trajectory
  };

  // === TrackFindingMeasurements ================================

  // Helper class to convert xAOD::PixelClusterContainer or xAOD::StripClusterContainer to UncalibSourceLinkMultiset.
  class TrackFindingMeasurements
  {
  public:

    TrackFindingMeasurements(size_t numMeasurements,
                             size_t maxMeasurements,
                             bool doTrackStatePrinter,
                             std::vector<ATLASUncalibSourceLink::ElementsType> *elementsCollection)
        : m_elementsCollection(elementsCollection)
    {
      m_sourceLinks.reserve(numMeasurements);
      m_elementsCollection->reserve(numMeasurements);
      if (doTrackStatePrinter)
      {
        m_sourceLinksVec.reserve(numMeasurements);
        m_measurementOffset.reserve(2); // pixels+strips
      }
      else
      {
        m_sourceLinksVec.reserve(maxMeasurements);
      }
      m_seedOffset.reserve(2);
    }

    TrackFindingMeasurements() = delete;
    TrackFindingMeasurements(const TrackFindingMeasurements &) = delete;
    TrackFindingMeasurements &operator=(const TrackFindingMeasurements &) = delete;

    void addMeasurements(size_t typeIndex,
                         const EventContext &ctx,
                         const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                         const InDetDD::SiDetectorElementCollection &detElems,
                         const ActsTrk::SeedContainer *seeds,
                         const ToolHandle<ActsTrk::IActsToTrkConverterTool> &ATLASConverterTool,
                         const ToolHandle<ActsTrk::ITrackStatePrinter> &trackStatePrinter,
                         DuplicateSeedDetector &duplicateSeedDetector)
    {
      size_t measurementOffset = 0;
      if (!trackStatePrinter.empty())
      {
        measurementOffset = m_sourceLinksVec.size();
        if (!(typeIndex < m_measurementOffset.size()))
          m_measurementOffset.resize(typeIndex + 1);
        m_measurementOffset[typeIndex] = measurementOffset;
      }
      // the following is just in case we call addMeasurements out of order or not for 2 types of measurements
      if (!(typeIndex < m_seedOffset.size()))
        m_seedOffset.resize(typeIndex + 1);

      for (auto *measurement : clusterContainer)
      {
        auto sl = ATLASConverterTool->uncalibratedTrkMeasurementToSourceLink(detElems, *measurement, *m_elementsCollection);
        m_sourceLinks.insert(m_sourceLinks.end(), sl);
        m_sourceLinksVec.push_back(sl);
      }

      if (seeds)
      {
        m_seedOffset[typeIndex] = duplicateSeedDetector.addSeeds(*seeds, m_sourceLinksVec, measurementOffset);
      }

      if (!trackStatePrinter.empty())
      {
        trackStatePrinter->printSourceLinks(ctx, m_sourceLinksVec, typeIndex, measurementOffset);
      }
      else
      {
        m_sourceLinksVec.clear(); // don't need any more
      }
    }

    const UncalibSourceLinkMultiset &sourceLinks() const { return m_sourceLinks; }
    const std::vector<ATLASUncalibSourceLink> &sourceLinkVec() const { return m_sourceLinksVec; }
    size_t measurementOffset(size_t typeIndex) const { return typeIndex < m_measurementOffset.size() ? m_measurementOffset[typeIndex] : 0u; }
    size_t seedOffset(size_t typeIndex) const { return typeIndex < m_seedOffset.size() ? m_seedOffset[typeIndex] : 0u; }

  private:
    UncalibSourceLinkMultiset m_sourceLinks;
    std::vector<ATLASUncalibSourceLink> m_sourceLinksVec;
    std::vector<ATLASUncalibSourceLink::ElementsType> *m_elementsCollection;
    std::vector<size_t> m_measurementOffset;
    std::vector<size_t> m_seedOffset;
  };

} // anonymous namespace

#endif
