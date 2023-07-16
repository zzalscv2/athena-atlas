/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   GaussianSumFitter.h
 * @date   Monday 7th March 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * @brief  Class for fitting according to the Gaussian Sum Filter  formalism
 */

#include "TrkGaussianSumFilter/IMultiStateExtrapolator.h"
#include "TrkGaussianSumFilterUtils/GsfMeasurementUpdator.h"
//
#include "TrkTrack/MultiComponentStateOnSurface.h"
//
#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"
#include "TrkDetElementBase/TrkDetElementBase.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkFitterUtils/TrackFitInputPreparator.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
//
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
//
#include <atomic>

namespace Trk {
class IMultiStateMeasurementUpdator;
class MultiComponentStateOnSurface;
class FitQuality;
class Track;

class GaussianSumFitter final
  : virtual public ITrackFitter
  , public AthAlgTool
{
public:
  /** Constructor with parameters to be passed to AlgTool */
  GaussianSumFitter(const std::string&, const std::string&, const IInterface*);

  /** Virtual destructor */
  virtual ~GaussianSumFitter() = default;

  /** AlgTool initialise method */
  virtual StatusCode initialize() override final;

  /** AlgTool finalise method */
  virtual StatusCode finalize() override final;

  /** Refit a track using the Gaussian Sum Filter */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis particleHypothesis =
      nonInteracting) const override final;

  /** Fit a collection of 'PrepRawData' objects using the Gaussian Sum Filter
      - This requires that an trackParameters object be supplied also as an
     initial guess */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const PrepRawDataSet&,
    const TrackParameters&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis particleHypothesis =
      nonInteracting) const override final;

  /** Fit a collection of 'RIO_OnTrack' objects using the Gaussian Sum Filter
      - This requires that an trackParameters object be supplied also as an
     initial guess */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const MeasurementSet&,
    const TrackParameters&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis particleHypothesis =
      nonInteracting) const override final;

  /** Refit a track adding a PrepRawDataSet*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const PrepRawDataSet&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis matEffects = nonInteracting) const override final;

  /** Refit a track adding a measurement base set*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const MeasurementSet&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis matEffects = nonInteracting) const override final;

  /** Combine two tracks by refitting */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const Track&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis matEffects = nonInteracting) const override final;

private:
  using GSFTrajectory = DataVector<const Trk::MultiComponentStateOnSurface>;
  /** Produces a perigee from a smoothed trajectory */
  std::unique_ptr<MultiComponentStateOnSurface> makePerigee(
    const EventContext& ctx,
    Trk::IMultiStateExtrapolator::Cache&,
    const GSFTrajectory&,
    const ParticleHypothesis particleHypothesis = nonInteracting) const;

  /** Gsf smoothe trajectory*/
  GSFTrajectory smootherFit(
    const EventContext& ctx,
    Trk::IMultiStateExtrapolator::Cache&,
    const GSFTrajectory&,
    const ParticleHypothesis particleHypothesis = nonInteracting,
    const CaloCluster_OnTrack* ccot = nullptr) const;

  /** Methof to add the CaloCluster onto the track */
  MultiComponentState addCCOT(
    const EventContext& ctx,
    const Trk::MultiComponentStateOnSurface* currentState,
    const Trk::CaloCluster_OnTrack* ccot,
    GSFTrajectory& smoothedTrajectory) const;

  /** Forward GSF fit using PrepRawData */
  GSFTrajectory fitPRD(
    const EventContext& ctx,
    IMultiStateExtrapolator::Cache&,
    const PrepRawDataSet&,
    const TrackParameters&,
    const ParticleHypothesis particleHypothesis = nonInteracting) const;

  /** Forward GSF fit using MeasurementSet */
  GSFTrajectory fitMeasurements(
    const EventContext& ctx,
    IMultiStateExtrapolator::Cache&,
    const MeasurementSet&,
    const TrackParameters&,
    const ParticleHypothesis particleHypothesis = nonInteracting) const;

  /** Progress one step along the fit */
  bool stepForwardFit(
    const EventContext& ctx,
    IMultiStateExtrapolator::Cache&,
    GSFTrajectory&,
    const PrepRawData*,
    const MeasurementBase*,
    const Surface&,
    MultiComponentState&,
    const ParticleHypothesis particleHypothesis = nonInteracting) const;

private:
  ToolHandle<IMultiStateExtrapolator> m_extrapolator{
    this,
    "ToolForExtrapolation",
    "Trk::GsfExtrapolator/GsfExtrapolator",
    ""
  };
  ToolHandle<IRIO_OnTrackCreator> m_rioOnTrackCreator{
    this,
    "ToolForROTCreation",
    "Trk::RioOnTrackCreator/RIO_OnTrackCreator",
    ""
  };

  Gaudi::Property<unsigned int> m_maximumNumberOfComponents{
    this,
    "MaximumNumberOfComponents",
    12,
    "Maximum number of components"
  };

  Gaudi::Property<bool> m_reintegrateOutliers{ this,
                                               "ReintegrateOutliers",
                                               true,
                                               "Reintegrate Outliers" };

  Gaudi::Property<bool> m_refitOnMeasurementBase{ this,
                                                  "RefitOnMeasurementBase",
                                                  true,
                                                  "Refit On Measurement Base" };

  Gaudi::Property<bool> m_combineWithFitter{
    this,
    "CombineStateWithFitter",
    false,
    "Combine with forwards state during Smoothing"
  };

  Gaudi::Property<bool> m_useMode{
    this,
    "useMode",
    true,
    "Combine/Collapse the MultiComponent State using Mode rather than mean"
  };

  Gaudi::Property<double> m_cutChiSquaredPerNumberDOF{
    this,
    "StateChi2PerNDOFCut",
    50.,
    "Cut on Chi2 per NDOF"
  };

  PropDirection m_directionToPerigee;
  TrkParametersComparisonFunction m_trkParametersComparisonFunction;
  std::unique_ptr<TrackFitInputPreparator> m_inputPreparator;
  std::vector<double> m_sortingReferencePoint;

  // Counters for fit statistics
  // Number of Fit PrepRawData Calls
  mutable std::atomic<unsigned long int> m_FitPRD{};
  // Number of Fit MeasurementBase Calls
  mutable std::atomic<unsigned long int> m_FitMeasurementBase{};
  // Number of Tracks that are successfull
  mutable std::atomic<unsigned long int> m_fitSuccess{};
};

} // end Trk namespace
