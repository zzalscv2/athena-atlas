/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSlimmingTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkTrackSlimmingTool/TrackSlimmingTool.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrackSummary/TrackSummary.h"

Trk::TrackSlimmingTool::TrackSlimmingTool(const std::string& t,
                                          const std::string& n,
                                          const IInterface* p)
  : AthAlgTool(t, n, p)
  , m_keepCaloDeposit(true)
  , m_keepOutliers(false)
  , m_keepParameters(false)
  , m_detID{}
{
  declareInterface<ITrackSlimmingTool>(this);

  //  template for property decalration
  declareProperty(
    "KeepCaloDeposit",
    m_keepCaloDeposit,
    "If this is set to true, any CaloDeposit with its adjacent MEOT's will be "
    "kept on the slimmed track (combined muon property)");
  declareProperty("KeepOutliers",
                  m_keepOutliers,
                  "If this is set to true, Outlier measurements will be kept "
                  "on the slimmed track");
  declareProperty("KeepParameters",
                  m_keepParameters,
                  "If this is set to true, the first and last parameters will "
                  "be kept on the slimmed track");
}
Trk::TrackSlimmingTool::~TrackSlimmingTool() = default;

StatusCode
Trk::TrackSlimmingTool::initialize()
{

  StatusCode sc = AlgTool::initialize();
  if (sc.isFailure())
    return sc;

  sc = detStore()->retrieve(m_detID, "AtlasID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get AtlasDetectorID ");
    return sc;
  }
  ATH_MSG_DEBUG("Found AtlasDetectorID");

  return StatusCode::SUCCESS;
}

StatusCode
Trk::TrackSlimmingTool::finalize()
{
  return StatusCode::SUCCESS;
}

void
Trk::TrackSlimmingTool::slimTrack(Trk::Track& track) const
{
  setHints(track);
  track.info().setTrackProperties(TrackInfo::SlimmedTrack);
}

void
Trk::TrackSlimmingTool::slimConstTrack(const Trk::Track& track) const
{
  setHints(track);
}

void
Trk::TrackSlimmingTool::setHints(const Trk::Track& track) const
{
  const Trk::TrackStates* oldTrackStates =
    track.trackStateOnSurfaces();
  if (oldTrackStates == nullptr) {
    ATH_MSG_WARNING("Track has no TSOS vector! Skipping track, returning");
    return;
  }

  const TrackStateOnSurface* firstValidIDTSOS(nullptr);
  const TrackStateOnSurface* lastValidIDTSOS(nullptr);
  const TrackStateOnSurface* firstValidMSTSOS(nullptr);
  const TrackStateOnSurface* lastValidMSTSOS(nullptr);
  if (m_keepParameters) {
    // search last valid TSOS first (as won't be found in later loop)
    findLastValidTSoS(oldTrackStates, lastValidIDTSOS, lastValidMSTSOS);
  }

  // If m_keepParameters is true, then we want to keep the first and last
  // parameters of ID & MS.
  const Trk::MeasurementBase* rot = nullptr;
  const Trk::TrackParameters* parameters = nullptr;
  bool keepParameter = false;
  // looping over all TSOS
  Trk::TrackStates::const_iterator itTSoS =
    oldTrackStates->begin();
  for (; itTSoS != oldTrackStates->end(); ++itTSoS) {

    // The hints we want to create for this tsos
    std::bitset<Trk::TrackStateOnSurface::NumberOfPersistencyHints> hints{};
    //
    hints.set(Trk::TrackStateOnSurface::PartialPersistification);
    parameters = nullptr;
    rot = nullptr;
    // if requested: keep calorimeter TSOS with adjacent scatterers (on combined
    // muons)
    if (m_keepCaloDeposit &&
        (**itTSoS).type(TrackStateOnSurface::CaloDeposit)) {
      // preceding TSOS (if Scatterer)
      if (itTSoS != oldTrackStates->begin()) {
        --itTSoS;
        if ((**itTSoS).type(TrackStateOnSurface::Scatterer)) {
          hints.reset(Trk::TrackStateOnSurface::PartialPersistification);
        }
        ++itTSoS;
      }
      // copy removes CaloEnergy (just keep base EnergyLoss)
      const MaterialEffectsOnTrack* meot =
        dynamic_cast<const MaterialEffectsOnTrack*>(
          (**itTSoS).materialEffectsOnTrack());
      if (meot && meot->energyLoss()) {
        hints.set(Trk::TrackStateOnSurface::PersistifySlimCaloDeposit);
        hints.set(Trk::TrackStateOnSurface::PersistifyTrackParameters);
      }
      // following TSOS (if Scatterer)
      ++itTSoS;
      if (itTSoS != oldTrackStates->end() &&
          (**itTSoS).type(TrackStateOnSurface::Scatterer)) {
        hints.reset(Trk::TrackStateOnSurface::PartialPersistification);
      }
      --itTSoS;
    }

    // We only keep TSOS if they either contain a perigee, OR are a measurement
    if ((*itTSoS)->measurementOnTrack() == nullptr &&
        !(*itTSoS)->type(TrackStateOnSurface::Perigee)) {
      // pass the hints to the tsos before we continue to the next
      (*itTSoS)->setHints(hints.to_ulong());
      continue;
    }

    keepParameter = keepParameters((*itTSoS),
                                   firstValidIDTSOS,
                                   lastValidIDTSOS,
                                   firstValidMSTSOS,
                                   lastValidMSTSOS);

    if (keepParameter) {
      parameters = (*itTSoS)->trackParameters();
    }
    if ((*itTSoS)->measurementOnTrack() != nullptr &&
        ((*itTSoS)->type(TrackStateOnSurface::Measurement) ||
         (m_keepOutliers && (*itTSoS)->type(TrackStateOnSurface::Outlier)))) {
      rot = (*itTSoS)->measurementOnTrack();
    }
    if (rot != nullptr || parameters != nullptr) {
      if (rot) {
        hints.set(Trk::TrackStateOnSurface::PersistifyMeasurement);
      }
      if (parameters) {
        hints.set(Trk::TrackStateOnSurface::PersistifyTrackParameters);
      }
    }
    // pass the hints to the tsos
    (*itTSoS)->setHints(hints.to_ulong());
  }
}

void
Trk::TrackSlimmingTool::checkForValidMeas(const Trk::TrackStateOnSurface* tsos,
                                          bool& isIDmeas,
                                          bool& isMSmeas) const
{
  if (tsos->measurementOnTrack() != nullptr) {
    bool isPseudo = (tsos->measurementOnTrack()->type(
      Trk::MeasurementBaseType::PseudoMeasurementOnTrack));
    // Handle cROTs
    const Trk::CompetingRIOsOnTrack* cROT = nullptr;
    if (tsos->measurementOnTrack()->type(
          Trk::MeasurementBaseType::CompetingRIOsOnTrack)) {
      cROT = static_cast<const Trk::CompetingRIOsOnTrack*>(
        tsos->measurementOnTrack());
    }
    Identifier id;
    if (cROT) {
      id = cROT->rioOnTrack(cROT->indexOfMaxAssignProb()).identify();
    } else {
      id = tsos->measurementOnTrack()
             ->associatedSurface()
             .associatedDetectorElementIdentifier();
    }
    isIDmeas = !isPseudo && m_detID->is_indet(id);
    isMSmeas = tsos->measurementOnTrack() != nullptr && !isPseudo &&
               m_detID->is_muon(id);
  }
}

void
Trk::TrackSlimmingTool::findLastValidTSoS(
  const DataVector<const Trk::TrackStateOnSurface>* oldTrackStates,
  const Trk::TrackStateOnSurface*& lastValidIDTSOS,
  const TrackStateOnSurface*& lastValidMSTSOS) const
{

  for (Trk::TrackStates::const_reverse_iterator rItTSoS =
         oldTrackStates->rbegin();
       rItTSoS != oldTrackStates->rend();
       ++rItTSoS) {
    if ((*rItTSoS)->type(TrackStateOnSurface::Measurement) &&
        (*rItTSoS)->trackParameters() != nullptr &&
        (*rItTSoS)->measurementOnTrack() != nullptr &&
        !(*rItTSoS)->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {

      if (m_detID->is_indet((*rItTSoS)
                              ->trackParameters()
                              ->associatedSurface()
                              .associatedDetectorElementIdentifier())) {
        lastValidIDTSOS = (*rItTSoS);
        break;
      }
      if (m_detID->is_muon((*rItTSoS)
                             ->trackParameters()
                             ->associatedSurface()
                             .associatedDetectorElementIdentifier())) {
        lastValidMSTSOS = (*rItTSoS);
        break;
      }
    }
  }
}

bool
Trk::TrackSlimmingTool::keepParameters(
  const Trk::TrackStateOnSurface* TSoS,
  const TrackStateOnSurface*& firstValidIDTSOS,
  const TrackStateOnSurface*& lastValidIDTSOS,
  const TrackStateOnSurface*& firstValidMSTSOS,
  const TrackStateOnSurface*& lastValidMSTSOS) const
{

  if (TSoS->trackParameters() != nullptr &&
      TSoS->type(TrackStateOnSurface::Perigee)) {
    return true;
  }
  // Now do checks for first/last ID/MS measurement (isIDmeas and isMSmeas)
  if (m_keepParameters) {
    bool isIDmeas = false;
    bool isMSmeas = false;
    checkForValidMeas(TSoS, isIDmeas, isMSmeas);
    // entering ID?
    if (isIDmeas && !firstValidIDTSOS &&
        TSoS->type(TrackStateOnSurface::Measurement)) {
      firstValidIDTSOS = TSoS;
      if (TSoS->trackParameters() != nullptr)
        return true;
    }
    // entering MS?
    if (isMSmeas && !firstValidMSTSOS) {
      firstValidMSTSOS = TSoS;
      if (TSoS->trackParameters() != nullptr)
        return true;
    }
    // Is this the last TSOS on the track?
    if (lastValidIDTSOS == TSoS || lastValidMSTSOS == TSoS) {
      if (TSoS->trackParameters() != nullptr)
        return true;
    }
  }
  return false;
}
