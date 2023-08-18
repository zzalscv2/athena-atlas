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

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  class  UncalibSourceLinkAccessor
  {
  private:
    const std::vector<ATLASUncalibSourceLink>      *m_sourceLinks;
    const std::vector<Acts::GeometryIdentifier>                  *m_orderedGeoIds;
    const std::vector< std::pair< unsigned int , unsigned int> > *m_measurementRanges;

  public:
    using BaseIterator = std::vector<ATLASUncalibSourceLink>::const_iterator;
    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;
    UncalibSourceLinkAccessor(const std::vector<ATLASUncalibSourceLink> &source_links,
                              const std::vector<Acts::GeometryIdentifier> &ordered_geoIds,
                              const std::vector< std::pair< unsigned int , unsigned int> > &measurement_ranges)
       : m_sourceLinks(&source_links),
         m_orderedGeoIds(&ordered_geoIds),
         m_measurementRanges(&measurement_ranges)
    {}
    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
       std::vector<Acts::GeometryIdentifier>::const_iterator
          geo_iter = std::lower_bound( m_orderedGeoIds->begin(),m_orderedGeoIds->end(), surface.geometryId());
       if (geo_iter == m_orderedGeoIds->end() || *geo_iter != surface.geometryId()) {
          return {Iterator{ m_sourceLinks->end()}, Iterator{ m_sourceLinks->end()}};
       }

       assert( geo_iter - m_measurementRanges.begin() < m_measurementRanges->size());
       const std::pair<unsigned int, unsigned int> &range = (*m_measurementRanges).at(geo_iter - m_orderedGeoIds->begin());
       return {Iterator{ m_sourceLinks->begin() + range.first}, Iterator{ m_sourceLinks->begin() + range.second}};
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
                             const std::vector< Acts::GeometryIdentifier > &ordered_geo_ids,
                             bool doTrackStatePrinter,
                             std::vector<ATLASUncalibSourceLink::ElementsType> *elementsCollection)
       : m_elementsCollection(elementsCollection),
         m_orderedGeoIds(&ordered_geo_ids)
    {
      m_measurementRanges.resize(m_orderedGeoIds->size(), std::make_pair(std::numeric_limits<unsigned int>::max(),
                                                                         std::numeric_limits<unsigned int>::max()) );
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
      size_t measurementOffset = m_sourceLinksVec.size();
      if (!trackStatePrinter.empty())
      {
        if (!(typeIndex < m_measurementOffset.size()))
          m_measurementOffset.resize(typeIndex + 1);
        m_measurementOffset[typeIndex] = measurementOffset;
      }
      // the following is just in case we call addMeasurements out of order or not for 2 types of measurements
      if (!(typeIndex < m_seedOffset.size()))
        m_seedOffset.resize(typeIndex + 1);

      xAOD::UncalibMeasType    last_measurement_type = xAOD::UncalibMeasType::Other;
      xAOD::DetectorIDHashType last_id_hash = std::numeric_limits<xAOD::DetectorIDHashType>::max();
      unsigned int range_idx = m_measurementRanges.size();
      for (auto *measurement : clusterContainer) {
         auto sl = ATLASConverterTool->uncalibratedTrkMeasurementToSourceLink(detElems, *measurement, *m_elementsCollection);
         unsigned int sl_idx=m_sourceLinksVec.size();
         m_sourceLinksVec.push_back( sl );
         if (measurement->identifierHash() != last_id_hash || measurement->type() != last_measurement_type) {
            std::vector<Acts::GeometryIdentifier>::const_iterator
               geo_iter = std::lower_bound( m_orderedGeoIds->begin(),m_orderedGeoIds->end(), sl.geometryId());
            if (geo_iter == m_orderedGeoIds->end() || *geo_iter != sl.geometryId()) {
               std::stringstream msg;
               msg << "Measurement with unexpected Acts geometryId: " << sl.geometryId()
                   << " type = " << static_cast<unsigned int >(measurement->type())
                   << " idHash=" << measurement->identifierHash();
               throw std::runtime_error(msg.str());
            }
            range_idx = geo_iter - m_orderedGeoIds->begin();
            if (m_measurementRanges[ range_idx ].first != std::numeric_limits<unsigned int>::max()) {
               std::stringstream msg;
               msg << "Measurement not clustered by identifierHash / geometryId. New measurement "
                   << measurement->index() << " with geo Id " << sl.geometryId()
                   << " type = " << static_cast<unsigned int>(measurement->type())
                   << " idHash=" << measurement->identifierHash()
                   << " but already recorded for this geo ID the range : " << m_measurementRanges[ range_idx ].first
                   << " .. " << m_measurementRanges[ range_idx ].second;
               throw std::runtime_error(msg.str());
            }
            m_measurementRanges[ range_idx ].first = sl_idx;
            last_id_hash = measurement->identifierHash();
            last_measurement_type = measurement->type();
         }
         m_measurementRanges[ range_idx ].second = sl_idx+1;
      }

      if (seeds)
      {
        m_seedOffset[typeIndex] = duplicateSeedDetector.addSeeds(*seeds, m_sourceLinksVec, measurementOffset);
      }

      if (!trackStatePrinter.empty())
      {
        trackStatePrinter->printSourceLinks(ctx, m_sourceLinksVec, typeIndex, measurementOffset);
      }
    }

    const std::vector<ATLASUncalibSourceLink> &sourceLinkVec() const { return m_sourceLinksVec; }
    size_t measurementOffset(size_t typeIndex) const { return typeIndex < m_measurementOffset.size() ? m_measurementOffset[typeIndex] : 0u; }
    size_t seedOffset(size_t typeIndex) const { return typeIndex < m_seedOffset.size() ? m_seedOffset[typeIndex] : 0u; }

    const std::vector<Acts::GeometryIdentifier> &orderedGeoIds() const { return *m_orderedGeoIds; }
    const std::vector< std::pair< unsigned int , unsigned int> > &measurementRanges() const { return m_measurementRanges; }

  private:
    std::vector<ATLASUncalibSourceLink> m_sourceLinksVec;
    std::vector<ATLASUncalibSourceLink::ElementsType> *m_elementsCollection;
    std::vector<size_t> m_measurementOffset;
    std::vector<size_t> m_seedOffset;
    const std::vector< Acts::GeometryIdentifier >  *m_orderedGeoIds;
    std::vector< std::pair< unsigned int , unsigned int> > m_measurementRanges;


  };

} // anonymous namespace

#endif
