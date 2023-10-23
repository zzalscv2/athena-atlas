/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "src/TrackFindingAlg.h"
#include "src/TrackFindingData.h"

// Athena
#include "TrkParameters/TrackParameters.h"
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
#include "AthenaMonitoringKernel/Monitored.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/SourceLinkAccessorConcept.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
// ACTS glue
#include "ActsEvent/TrackContainer.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/TrackingSurfaceHelper.h"
#include "ActsInterop/Logger.h"

#include "TableUtils.h"
#include "MeasurementCalibrator.h"
// Other
#include <sstream>
#include <functional>
#include <tuple>
#include <algorithm>

namespace {
   template <class T_MeasurementContainer>
   std::pair< xAOD::DetectorIDHashType, bool> getMaxHashAndCheckOrder(const T_MeasurementContainer &measurements) {
      std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered { 0, true };
      for (const auto &measurement :  measurements) {
         xAOD::DetectorIDHashType id_hash = measurement->identifierHash();
         max_hash_ordered.second =  (id_hash >= max_hash_ordered.first);
         max_hash_ordered.first = std::max( max_hash_ordered.first, id_hash );
      }
      return max_hash_ordered;
   }

   void gatherGeoIds(const ActsTrk::IActsToTrkConverterTool &converter_tool,
                     const InDetDD::SiDetectorElementCollection &detectorElements,
                     std::vector<Acts::GeometryIdentifier> &geo_ids,
                     std::vector<const Acts::Surface *> &acts_surfaces) {
      for (const auto *det_el :  detectorElements) {
         const Acts::Surface &surface =
            converter_tool.trkSurfaceToActsSurface(det_el->surface());
         geo_ids.push_back(surface.geometryId());
         acts_surfaces.push_back( &surface );
      }
   }
}

namespace ActsTrk
{
  struct TrackFindingAlg::CKF_pimpl : public CKF_config
  {
  };

  TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() { return *m_trackFinder; }
  const TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() const { return *m_trackFinder; }

  TrackFindingAlg::TrackFindingAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  TrackFindingAlg::~TrackFindingAlg() = default;

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_etaMin);
    ATH_MSG_DEBUG("   " << m_etaMax);
    ATH_MSG_DEBUG("   " << m_absEtaMin);
    ATH_MSG_DEBUG("   " << m_absEtaMax);
    ATH_MSG_DEBUG("   " << m_ptMin);
    ATH_MSG_DEBUG("   " << m_ptMax);
    ATH_MSG_DEBUG("   " << m_minMeasurements);

    // Read and Write handles
    ATH_CHECK(m_pixelSeedsKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripSeedsKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelClusterContainerKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripClusterContainerKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelDetEleCollKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripDetEleCollKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_pixelEstimatedTrackParametersKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_stripEstimatedTrackParametersKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_trackContainerKey.initialize());

    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));

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

    Acts::MeasurementSelector::Config measurementSelectorCfg{{Acts::GeometryIdentifier(),
                                                              {m_etaBins, m_chi2CutOff, m_numMeasurementsCutOff}}};

    Acts::TrackSelector::Config trackSelectorCfg;
    trackSelectorCfg.phiMin = m_phiMin;
    trackSelectorCfg.phiMax = m_phiMax;
    trackSelectorCfg.etaMin = m_etaMin;
    trackSelectorCfg.etaMax = m_etaMax;
    trackSelectorCfg.absEtaMin = m_absEtaMin;
    trackSelectorCfg.absEtaMax = m_absEtaMax;
    trackSelectorCfg.ptMin = m_ptMin;
    trackSelectorCfg.ptMax = m_ptMax;
    trackSelectorCfg.minMeasurements = m_minMeasurements;

    m_trackFinder.reset(new CKF_pimpl{CKF_config{{std::move(propagator), logger().cloneWithSuffix("CKF")}, measurementSelectorCfg, {}, {}, trackSelectorCfg}});

    trackFinder().pOptions.maxSteps = m_maxPropagationStep;

    trackFinder().ckfExtensions.updater.connect<&gainMatrixUpdate>();
    trackFinder().ckfExtensions.smoother.connect<&gainMatrixSmoother>();
    trackFinder().ckfExtensions.measurementSelector.connect<&Acts::MeasurementSelector::select<ActsTrk::MutableTrackStateBackend>>(&trackFinder().measurementSelector);

    if (!m_statEtaBins.empty()) {
       m_useAbsEtaForStat=(m_statEtaBins[0]>0.);
       float last_eta=m_statEtaBins[0];
       for (float eta : m_statEtaBins ) {
          if (eta<last_eta) {
             ATH_MSG_FATAL("Eta bins for statistics counter not in ascending order." );
          }
          last_eta=eta;
       }
    }
    m_stat.resize(nSeedCollections()*seedCollectionStride(), std::array< std::size_t, kNStat >{});
    if (!m_seedLables.empty() && m_seedLables.size() != nSeedCollections()) {
       ATH_MSG_FATAL("SeedLabels should be an empty vector or a vector with " << nSeedCollections()
                     << " enries. But it is a vector with " << m_seedLables.size() << " entries." );
    }
    return StatusCode::SUCCESS;
  }

  namespace {
     std::string makeEtaBinLabel(const std::vector<float> &eta_bins,
                                 unsigned int eta_bin_i,
                                 bool abs_eta=false) {
        std::stringstream eta_range_label;
        eta_range_label << std::fixed << std::setprecision(1);
        if (eta_bin_i==eta_bins.size()+1) {
           eta_range_label << " All eta";
        }
        else {
           if (eta_bin_i==0) {
              eta_range_label << std::setw(4) <<  (abs_eta ? "0.0" : "-inf") << "-";
           }
           else {
              eta_range_label << std::setw(4) << eta_bins.at(eta_bin_i-1) <<"-";
           }
           if (eta_bin_i>=eta_bins.size()) {
              eta_range_label << std::setw(4) << "+inf";
           }
           else {
              eta_range_label << std::setw(4) << eta_bins.at(eta_bin_i);
           }
        }
        return eta_range_label.str();
     }

  }

  // finalize
  StatusCode TrackFindingAlg::finalize()
  {
    if (msgLvl(MSG::INFO)) {
       std::vector<std::string> stat_labels = TableUtils::makeLabelVector(kNStat,{
             std::make_pair(kNTotalSeeds,     "Input seeds"),
             std::make_pair(kNoTrackParam,    "No track parameters"),
             std::make_pair(kNUsedSeeds,      "Used   seeds"),
             std::make_pair(kNoTrack,         "Cannot find track"),
             std::make_pair(kNDuplicateSeeds, "Duplicate seeds"),
             std::make_pair(kNOutputTracks,   "CKF tracks"),
             std::make_pair(kNSelectedTracks, "selected tracks"),
          });
       assert( stat_labels.size() == kNStat);
       std::vector<std::string> categories {
          m_seedLables.empty() ? m_pixelEstimatedTrackParametersKey.key() : m_seedLables.value().at(0),
          m_seedLables.empty() ? m_stripEstimatedTrackParametersKey.key() : m_seedLables.value().at(1),
          "ALL"
       };

       std::vector<std::string> eta_labels;
       eta_labels.reserve(m_statEtaBins.size()+2);
       for (unsigned int eta_bin_i=0; eta_bin_i < m_statEtaBins.size()+2; ++eta_bin_i) {
          eta_labels.push_back( makeEtaBinLabel( m_statEtaBins, eta_bin_i, m_useAbsEtaForStat));
       }

       // vector used as 3D array stat[ eta_bin ][ stat_i ][ seed_type]
       // stat_i = [0, kNStat)
       // eta_bin = [0, m_statEtaBins.size()+2 ); eta_bin == m_statEtaBinsSize()+1 means sum of all etaBins
       // seed_type = [0, nSeedCollections()+1)  seed_type == nSeedCollections() means sum of all seed collections
       std::vector<std::size_t> stat = TableUtils::createCounterArrayWithProjections<std::size_t>(nSeedCollections(),
                                                                                                  m_statEtaBins.size()+1,
                                                                                                  m_stat);

       // the extra columns and rows for the projections are addeded internally:
       unsigned int stat_stride=TableUtils::counterStride(nSeedCollections(),
                                                          m_statEtaBins.size()+1,
                                                          static_cast<std::size_t>(kNStat));
       unsigned int eta_stride=TableUtils::subCategoryStride(nSeedCollections(),
                                                             m_statEtaBins.size()+1,
                                                             static_cast<std::size_t>(kNStat));
       std::stringstream table_out;

       if (m_dumpAllStatEtaBins.value()) {
          // dump for each counter a table with one row per eta bin
          unsigned int max_label_width = TableUtils::maxLabelWidth(stat_labels) + TableUtils::maxLabelWidth(eta_labels);
          for (unsigned int stat_i=0; stat_i<kNStat; ++stat_i) {
             unsigned int dest_idx_offset = stat_i * stat_stride;
             table_out << makeTable(stat, dest_idx_offset, eta_stride,
                                    eta_labels,
                                    categories)
                .columnWidth(10)
                 // only dump the footer for the last eta bin i.e. total
                .dumpHeader(stat_i==0)
                .dumpFooter(stat_i+1 == kNStat)
                .separateLastRow(true) // separate the sum of all eta bins
                .minLabelWidth(max_label_width)
                .labelPrefix(stat_labels.at(stat_i));
          }
       }
       else {
          // dump one table with one row per counter showing the total eta range
          for (unsigned int eta_bin_i=(m_dumpAllStatEtaBins.value() ? 0 : m_statEtaBins.size()+1);
               eta_bin_i<m_statEtaBins.size()+2;
               ++eta_bin_i) {
             unsigned int dest_idx_offset = eta_bin_i * eta_stride;
             table_out << makeTable(stat, dest_idx_offset,stat_stride,
                                    stat_labels,
                                    categories,
                                    eta_labels.at(eta_bin_i))
                .columnWidth(10)
                // only dump the footer for the last eta bin i.e. total
                .dumpFooter(!m_dumpAllStatEtaBins.value() || eta_bin_i==m_statEtaBins.size()+1);
          }
       }
       ATH_MSG_INFO("statistics:" << std::endl << table_out.str() );
       table_out.str("");

       // define retios first element numerator, second element denominator
       // each element contains a vector of counter and a multiplier e.g. +- 1
       // ratios are computed as  (sum_i stat[stat_i] *  multiplier_i ) / (sum_j stat[stat_j] *  multiplier_j )
       auto [ratio_labels, ratio_def] = TableUtils::splitRatioDefinitionsAndLabels( {
             TableUtils::makeRatioDefinition("failed / seeds ",
                                             std::vector< std::pair<unsigned int, int> > {
                                                TableUtils::defineSummand(kNTotalSeeds,      1),
                                                TableUtils::defineSummand(kNUsedSeeds,      -1),
                                                TableUtils::defineSummand(kNDuplicateSeeds, -1),
                                                // no track counted  as used but want to include it as failed
                                                TableUtils::defineSummand(kNoTrack,          1),
                                             },   // failed seeds i.e. seeds which are not duplicates but did not produce a track
                                             std::vector< std::pair<unsigned int, int> >{ TableUtils::defineSummand(kNTotalSeeds,1) }),
             TableUtils::defineSimpleRatio("duplication / seeds",          kNDuplicateSeeds, kNTotalSeeds),
             TableUtils::defineSimpleRatio("selected / CKF tracks",        kNSelectedTracks, kNOutputTracks),
             TableUtils::defineSimpleRatio("selected tracks / used seeds", kNSelectedTracks, kNUsedSeeds)
          });

       std::vector<float> ratio = TableUtils::computeRatios(ratio_def,
                                                            nSeedCollections()+1,
                                                            m_statEtaBins.size()+2,
                                                            stat);

       // the extra columns and rows for the projections are _not_ added internally
       unsigned int ratio_stride=TableUtils::ratioStride(nSeedCollections()+1,
                                                         m_statEtaBins.size()+2,
                                                         ratio_def);
       unsigned int ratio_eta_stride=TableUtils::subCategoryStride(nSeedCollections()+1,
                                                                   m_statEtaBins.size()+2,
                                                                   ratio_def);

       unsigned int max_label_width = TableUtils::maxLabelWidth(ratio_labels) + TableUtils::maxLabelWidth(eta_labels);
       if (m_dumpAllStatEtaBins.value()) {
          // show for each ratio a table with one row per eta bin
          for (unsigned int ratio_i=0; ratio_i<ratio_labels.size(); ++ratio_i) {
             table_out << makeTable(ratio,
                                    ratio_i*ratio_stride,
                                    ratio_eta_stride,
                                    eta_labels,
                                    categories)
                .columnWidth(10)
                // only dump the footer for the last eta bin i.e. total
                .dumpHeader(ratio_i==0)
                .dumpFooter(ratio_i+1==ratio_labels.size())
                .separateLastRow(true) // separate the sum of las
                .minLabelWidth(max_label_width)
                .labelPrefix(ratio_labels.at(ratio_i));
          }
       }
       else {
          // dump one table with one row per ratio showing  the total eta range
          table_out << makeTable(ratio,
                                 (m_statEtaBins.size()+1)*ratio_eta_stride+0*ratio_stride,
                                 ratio_stride,
                                 ratio_labels,
                                 categories)
             .columnWidth(10)
             // only dump the footer for the last eta bin i.e. total
             .minLabelWidth(max_label_width)
             .dumpFooter(false);

          // also dump a table for final tracks over seeds (ratio_i==3) showing one row per eta bin
          eta_labels.erase( eta_labels.end() - 1); // drop last line of table which shows again all eta bins summed.
          unsigned int ratio_i = 3;
          table_out << makeTable(ratio,
                                 ratio_i*ratio_stride,
                                 ratio_eta_stride,
                                 eta_labels,
                                 categories)
             .columnWidth(10)
             .dumpHeader(false)
             // only dump the footer for the last eta bin i.e. total
             .dumpFooter(!m_dumpAllStatEtaBins.value() || ratio_i+1==ratio_labels.size())
             .separateLastRow(false)
             .minLabelWidth(max_label_width)
             .labelPrefix(ratio_labels.at(ratio_i));
       }

      ATH_MSG_INFO("Ratios:" << std::endl << table_out.str());
    }
    return StatusCode::SUCCESS;
  }

  StatusCode TrackFindingAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
    auto mon = Monitored::Group(m_monTool, timer);

    // ================================================== //
    // ===================== INPUTS ===================== //
    // ================================================== //

    // SEED PARAMETERS
    const ActsTrk::BoundTrackParametersContainer *pixelEstimatedTrackParameters = nullptr;
    if (!m_pixelEstimatedTrackParametersKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelEstimatedTrackParametersKey.key());
      SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> pixelEstimatedTrackParametersHandle = SG::makeHandle(m_pixelEstimatedTrackParametersKey, ctx);
      ATH_CHECK(pixelEstimatedTrackParametersHandle.isValid());
      pixelEstimatedTrackParameters = pixelEstimatedTrackParametersHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelEstimatedTrackParameters->size() << " input elements from key " << m_pixelEstimatedTrackParametersKey.key());
    }

    const ActsTrk::BoundTrackParametersContainer *stripEstimatedTrackParameters = nullptr;
    if (!m_stripEstimatedTrackParametersKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripEstimatedTrackParametersKey.key());
      SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> stripEstimatedTrackParametersHandle = SG::makeHandle(m_stripEstimatedTrackParametersKey, ctx);
      ATH_CHECK(stripEstimatedTrackParametersHandle.isValid());
      stripEstimatedTrackParameters = stripEstimatedTrackParametersHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripEstimatedTrackParameters->size() << " input elements from key " << m_stripEstimatedTrackParametersKey.key());
    }

    // SEED TRIPLETS
    const ActsTrk::SeedContainer *pixelSeeds = nullptr;
    if (!m_pixelSeedsKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelSeedsKey.key());
      SG::ReadHandle<ActsTrk::SeedContainer> pixelSeedsHandle = SG::makeHandle(m_pixelSeedsKey, ctx);
      ATH_CHECK(pixelSeedsHandle.isValid());
      pixelSeeds = pixelSeedsHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelSeeds->size() << " input elements from key " << m_pixelSeedsKey.key());
    }

    const ActsTrk::SeedContainer *stripSeeds = nullptr;
    if (!m_stripSeedsKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripSeedsKey.key());
      SG::ReadHandle<ActsTrk::SeedContainer> stripSeedsHandle = SG::makeHandle(m_stripSeedsKey, ctx);
      ATH_CHECK(stripSeedsHandle.isValid());
      stripSeeds = stripSeedsHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripSeeds->size() << " input elements from key " << m_stripSeedsKey.key());
    }

    // MEASUREMENTS
    const xAOD::PixelClusterContainer *pixelClusterContainer = nullptr;
    if (!m_pixelClusterContainerKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_pixelClusterContainerKey.key());
      SG::ReadHandle<xAOD::PixelClusterContainer> pixelClusterContainerHandle = SG::makeHandle(m_pixelClusterContainerKey, ctx);
      ATH_CHECK(pixelClusterContainerHandle.isValid());
      pixelClusterContainer = pixelClusterContainerHandle.get();
      ATH_MSG_DEBUG("Retrieved " << pixelClusterContainer->size() << " input elements from key " << m_pixelClusterContainerKey.key());
    }

    const xAOD::StripClusterContainer *stripClusterContainer = nullptr;
    if (!m_stripClusterContainerKey.empty())
    {
      ATH_MSG_DEBUG("Reading input collection with key " << m_stripClusterContainerKey.key());
      SG::ReadHandle<xAOD::StripClusterContainer> stripClusterContainerHandle = SG::makeHandle(m_stripClusterContainerKey, ctx);
      ATH_CHECK(stripClusterContainerHandle.isValid());
      stripClusterContainer = stripClusterContainerHandle.get();
      ATH_MSG_DEBUG("Retrieved " << stripClusterContainer->size() << " input elements from key " << m_stripClusterContainerKey.key());
    }

    const InDetDD::SiDetectorElementCollection *pixelDetEleColl = nullptr;
    if (!m_pixelDetEleCollKey.empty())
    {
      ATH_MSG_DEBUG("Reading input condition data with key " << m_pixelDetEleCollKey.key());
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleCollHandle(m_pixelDetEleCollKey, ctx);
      ATH_CHECK(pixelDetEleCollHandle.isValid());
      pixelDetEleColl = pixelDetEleCollHandle.retrieve();
      if (pixelDetEleColl == nullptr)
      {
        ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Retrieved " << pixelDetEleColl->size() << " input condition elements from key " << m_pixelDetEleCollKey.key());
    }

    const InDetDD::SiDetectorElementCollection *stripDetEleColl = nullptr;
    if (!m_stripDetEleCollKey.empty())
    {
      ATH_MSG_DEBUG("Reading input condition data with key " << m_stripDetEleCollKey.key());
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleCollHandle(m_stripDetEleCollKey, ctx);
      ATH_CHECK(stripDetEleCollHandle.isValid());
      stripDetEleColl = stripDetEleCollHandle.retrieve();
      if (stripDetEleColl == nullptr)
      {
        ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Retrieved " << stripDetEleColl->size() << " input condition elements from key " << m_stripDetEleCollKey.key());
    }

    std::array<xAOD::DetectorIDHashType, 3> max_hash{ };
    {
       std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered = getMaxHashAndCheckOrder(*pixelClusterContainer);
       if (!max_hash_ordered.second) {
          ATH_MSG_ERROR("Measurements " << m_pixelClusterContainerKey.key() << " not ordered by identifier hash." );
          return StatusCode::FAILURE;
       }
       static_assert( static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) < max_hash.size());
       max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) ]=max_hash_ordered.first;
    }
    {
       std::pair< xAOD::DetectorIDHashType, bool> max_hash_ordered = getMaxHashAndCheckOrder(*stripClusterContainer);
       if (!max_hash_ordered.second) {
          ATH_MSG_ERROR("Measurements " << m_stripClusterContainerKey.key() << " not ordered by identifier hash." );
          return StatusCode::FAILURE;
       }
       static_assert( static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType) < max_hash.size());
       max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]=max_hash_ordered.first;
    }

    // @TODO make this condition data
    std::vector< Acts::GeometryIdentifier > geo_ids;
    std::array<std::vector< const Acts::Surface * >, 4> acts_surfaces;
    geo_ids.reserve(   max_hash[ static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType)]
                     + max_hash[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]);
    acts_surfaces.at(static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) )
       .reserve(   max_hash[ static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType)] );
    acts_surfaces.at(static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType) )
       .reserve(   max_hash[ static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)] );
    gatherGeoIds(*m_ATLASConverterTool, *pixelDetEleColl, geo_ids, acts_surfaces.at(static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType) ));
    gatherGeoIds(*m_ATLASConverterTool, *stripDetEleColl, geo_ids, acts_surfaces.at(static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType) ));
    std::sort( geo_ids.begin(), geo_ids.end());

    TrackingSurfaceHelper tracking_surface_helper(std::move(acts_surfaces));
    TrackFindingMeasurements measurements(geo_ids,
                                          !m_trackStatePrinter.empty());

    DuplicateSeedDetector duplicateSeedDetector(((pixelSeeds ? pixelSeeds->size() : 0u) +
                                                 (stripSeeds ? stripSeeds->size() : 0u)),
                                                m_skipDuplicateSeeds);

    if (pixelClusterContainer && pixelDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << pixelClusterContainer->size() << " source links from pixel measurements");
      tracking_surface_helper.setSiDetectorElements(xAOD::UncalibMeasType::PixelClusterType, pixelDetEleColl);
      measurements.addMeasurements(0, *pixelClusterContainer, *pixelDetEleColl, pixelSeeds,
                                   m_ATLASConverterTool, m_trackStatePrinter, duplicateSeedDetector, ctx);
    }
    if (stripClusterContainer && stripDetEleColl)
    {
      ATH_MSG_DEBUG("Create " << stripClusterContainer->size() << " source links from strip measurements");
      tracking_surface_helper.setSiDetectorElements(xAOD::UncalibMeasType::StripClusterType, stripDetEleColl);
      measurements.addMeasurements(1, *stripClusterContainer, *stripDetEleColl, stripSeeds,
                                   m_ATLASConverterTool, m_trackStatePrinter, duplicateSeedDetector, ctx);
    }

    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //

    auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);

    ActsTrk::MutableTrackContainer tracksContainer;
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //
    std::vector< std::array<unsigned int, kNStat> > event_stat;
    event_stat.resize( m_stat.size(), std::array<unsigned int, kNStat>{});

    // Perform the track finding for all initial parameters.
    // Until the CKF can do a backward search, start with the pixel seeds
    // (will become relevant when we can remove pixel/strip duplicates).
    // Afterwards, we could start with strips where the occupancy is lower.
    if (pixelEstimatedTrackParameters && !pixelEstimatedTrackParameters->empty())
    {
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           tracking_surface_helper,
                           duplicateSeedDetector,
                           *pixelEstimatedTrackParameters,
                           pixelSeeds,
                           tracksContainer,
                           0,
                           "pixel",
                           event_stat));
    }

    if (stripEstimatedTrackParameters && !stripEstimatedTrackParameters->empty())
    {
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           tracking_surface_helper,
                           duplicateSeedDetector,
                           *stripEstimatedTrackParameters,
                           stripSeeds,
                           tracksContainer,
                           1,
                           "strip",
                           event_stat));
    }

    ATH_MSG_DEBUG("    \\__ Created " << tracksContainer.size() << " tracks");

    // copy statistics
    {
       std::lock_guard<std::mutex> lock(m_mutex);
       unsigned int category_i=0;
       for (const std::array<unsigned int, kNStat> &src_stat : event_stat) {
          std::array<std::size_t, kNStat> &dest_stat= m_stat[category_i];
          for (unsigned int i=0;i<kNStat; ++i) {
             assert(i < m_stat[category_i].size() );
             dest_stat[i] += src_stat[i];
          }
          ++category_i;
       }
    }
    
    // ================================================== //
    // ===================== STORE OUTPUT =============== //
    // ================================================== //
    // TODO once have final version of containers, they need to have movable backends also here
    ActsTrk::TrackStateBackend trackStateBackend(tracksContainer.trackStateContainer());
    ActsTrk::TrackBackend trackBackend(tracksContainer.container());
    auto constTrackContainer = std::make_unique<ActsTrk::TrackContainer>(std::move(trackBackend), std::move(trackStateBackend));
    ATH_CHECK(trackContainerHandle.record(std::move(constTrackContainer)));
    if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const TrackFindingMeasurements &measurements,
                              const TrackingSurfaceHelper &tracking_surface_helper,
                              DuplicateSeedDetector &duplicateSeedDetector,
                              const ActsTrk::BoundTrackParametersContainer &estimatedTrackParameters,
                              const ActsTrk::SeedContainer *seeds,
                              ActsTrk::MutableTrackContainer &tracksContainer,
                              size_t typeIndex,
                              const char *seedType,
                              std::vector< std::array<unsigned int, kNStat> > &event_stat) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    if (seeds && seeds->size() != estimatedTrackParameters.size())
    {
      // should be the same, but we can cope if not
      ATH_MSG_WARNING("Have " << seeds->size() << " " << seedType << " seeds, but " << estimatedTrackParameters.size() << "estimated track parameters");
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    // CalibrationContext converter not implemented yet.
    Acts::CalibrationContext calContext = Acts::CalibrationContext();

    UncalibSourceLinkAccessor slAccessor( ctx,
                                          measurements.orderedGeoIds(),
                                          measurements.measurementRanges());
    Acts::SourceLinkAccessorDelegate<UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&UncalibSourceLinkAccessor::range>(&slAccessor);

    // Set the CombinatorialKalmanFilter options
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<UncalibSourceLinkAccessor::Iterator, ActsTrk::MutableTrackStateBackend>;
    TrackFinderOptions options(tgContext,
                               mfContext,
                               calContext,
                               slAccessorDelegate,
                               trackFinder().ckfExtensions,
                               trackFinder().pOptions,
                               &(*pSurface));

    ActsTrk::MutableTrackContainer tracksContainerTemp;

    UncalibratedMeasurementCalibrator<ActsTrk::MutableTrackStateBackend> calibrator(*m_ATLASConverterTool, tracking_surface_helper);
    options.extensions.calibrator.connect(calibrator);

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << estimatedTrackParameters.size() << ' ' << seedType << " seeds.");

    // Loop over the track finding results for all initial parameters
    for (std::size_t iseed = 0; iseed < estimatedTrackParameters.size(); ++iseed)
    {
      unsigned int category_i=typeIndex * (m_statEtaBins.size()+1);
      tracksContainerTemp.clear();

      if (!estimatedTrackParameters[iseed])
      {
        ATH_MSG_WARNING("No " << seedType << " seed " << iseed);
        ++event_stat[category_i][kNoTrackParam];
        continue;
      }

      const Acts::BoundTrackParameters &initialParameters = *estimatedTrackParameters[iseed];

      category_i = getStatCategory(typeIndex, -std::log(std::tan( initialParameters.theta() / 2))  );
      ++event_stat[category_i][kNTotalSeeds];

      if (!m_trackStatePrinter.empty() && seeds)
      {
        if (iseed == 0)
        {
          ATH_MSG_INFO("CKF results for " << estimatedTrackParameters.size() << ' ' << seedType << " seeds:");
        }
        m_trackStatePrinter->printSeed(tgContext, *(*seeds)[iseed], initialParameters, measurements.measurementOffset(typeIndex), iseed);
      }

      if (duplicateSeedDetector.isDuplicate(measurements.seedOffset(typeIndex) + iseed))
      {
        ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
        ++event_stat[category_i][kNDuplicateSeeds];
        continue;
      }

      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects
      ++event_stat[category_i][kNUsedSeeds];

      auto result = trackFinder().ckf.findTracks(initialParameters, options, tracksContainerTemp);

      // The result for this seed
      if (not result.ok())
      {
        ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
        continue;
      }
      const auto &tracksForSeed = result.value();

      // Fill the track infos into the duplicate seed detector
      ATH_CHECK(storeSeedInfo(tracksContainerTemp, tracksForSeed, duplicateSeedDetector));

      size_t ntracks = tracksForSeed.size();
      event_stat[category_i][kNOutputTracks] += ntracks;

      if (!m_trackStatePrinter.empty())
      {
         const MeasurementRangeList &measurement_list = measurements.measurementRanges();
         std::vector< std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t> > offset;
         offset.reserve( measurement_list.m_measurementContainer.size());
         for (std::size_t type_index = 0; type_index < measurement_list.m_measurementContainer.size(); ++type_index) {
            if ( measurement_list.m_measurementContainer[type_index] != nullptr) {
               offset.push_back( std::make_pair( measurement_list.m_measurementContainer[type_index], measurements.measurementOffset(type_index) ));
            }
         }
         m_trackStatePrinter->printTracks(tgContext, tracksContainerTemp, result.value(), offset);
      }

      if (ntracks == 0)
      {
        ATH_MSG_WARNING("Track finding found no track candidates for " << seedType << " seed " << iseed);
        ++event_stat[category_i][kNoTrack];
      }

      // copy selected tracks into output tracksContainer
      size_t itrack = 0;
      for (auto &track : tracksForSeed)
      {
        if (trackFinder().trackSelector.isValidTrack(track))
        {
          auto destProxy = tracksContainer.getTrack(tracksContainer.addTrack());
          destProxy.copyFrom(track, true); // make sure we copy track states!
          ++event_stat[category_i][kNSelectedTracks];
        }
        else
        {
          ATH_MSG_DEBUG("Track " << itrack << " from " << seedType << " seed " << iseed << " failed track selection");
        }
        itrack++;
      }
    }

    ATH_MSG_DEBUG("Completed " << seedType << " track finding with " << computeStatSum(typeIndex, kNOutputTracks, event_stat) << " track candidates.");


    return StatusCode::SUCCESS;
  }

  StatusCode
  TrackFindingAlg::storeSeedInfo(const ActsTrk::MutableTrackContainer &tracksContainer,
                                 const std::vector<ActsTrk::MutableTrackContainer::TrackProxy> &fitResult,
                                 DuplicateSeedDetector &duplicateSeedDetector) const
  {
    for (auto &track : fitResult)
    {
      const auto lastMeasurementIndex = track.tipIndex();
      duplicateSeedDetector.newTrajectory();

      tracksContainer.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&duplicateSeedDetector](const ActsTrk::MutableTrackStateBackend::ConstTrackStateProxy &state) -> void
          {
            // Check there is a source link
            if (not state.hasUncalibratedSourceLink())
              return;

            // Fill the duplicate selector
            auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
            duplicateSeedDetector.addMeasurement(sl);
          }); // end visitBackwards
    }         // end loop on tracks from fitResult

    return StatusCode::SUCCESS;
  }

} // namespace
