/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMATRACKTOOLS_EGAMMATRKREFITTERTOOL_H
#define EGAMMATRACKTOOLS_EGAMMATRKREFITTERTOOL_H
/** @brief
  @class egammaTrackRefitterTool
          It receives a egamma object or Trk::Track
          Refits the track associated with an electron
          NOTE  a  new track has been created to ensure
          that no memory leaks occur you must delete the Trk::Track pointed to
  by this function.
  @author A. Morley
  @author C. Anastopouls
*/

#include "egammaInterfaces/IegammaTrkRefitterTool.h"

#include "egammaInterfaces/ICaloCluster_OnTrackBuilder.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"

#include <memory>

class AtlasDetectorID;

class egammaTrkRefitterTool final
  : virtual public IegammaTrkRefitterTool
  , public AthAlgTool
{

public:
  /** @brief Constructor with AlgTool parameters */
  egammaTrkRefitterTool(const std::string&,
                        const std::string&,
                        const IInterface*);

  /** @brief Destructor */
  virtual ~egammaTrkRefitterTool() = default; 

  /** @brief AlgTool initialise method */
  virtual StatusCode initialize() override;

  /** @brief AlgTool finalise method */
  virtual StatusCode finalize() override;

  typedef IegammaTrkRefitterTool::Cache Cache;

  /** @brief Refit a track*/
  virtual StatusCode refitTrack(const EventContext& ctx,
                                const Trk::Track*,
                                Cache& cache) const override final;

private:
  /** @brief Get the hits from the Inner Detector*/
  std::vector<const Trk::MeasurementBase*> getIDHits(
    const Trk::Track* track) const;

  struct MeasurementsAndTrash
  {
    /*
     * we need to take care of returning all the relevant measurements
     * while at the same time keeping proper ownership only for the ones
     * not handled by the EDM
     */
    std::vector<const Trk::MeasurementBase*> m_measurements;
    std::vector<std::unique_ptr<const Trk::MeasurementBase>> m_trash;
  };
  /** @brief Adds a beam spot to the Measurements passed to the track refitter*/
  MeasurementsAndTrash addPointsToTrack(
    const EventContext& ctx,
    const Trk::Track* track,
    const xAOD::Electron* eg = nullptr) const;

  /** @brief The track refitter */
  ToolHandle<Trk::ITrackFitter> m_ITrackFitter{
    this,
    "FitterTool",
    "Trk__GaussianSumFitter/GSFTrackFitter",
    "ToolHandle for track fitter implementation"
  };

  ToolHandle<ICaloCluster_OnTrackBuilder>
    m_CCOTBuilder{ this, "CCOTBuilder", "CaloCluster_OnTrackBuilder", "" };

  /** @brief type of material interaction in extrapolation*/
  Gaudi::Property<int> m_matEffects{
    this,
    "matEffects",
    1,
    "Type of material interaction in extrapolation (Default Electron)"
  };

  /** @brief Particle Hypothesis*/
  Trk::ParticleHypothesis m_ParticleHypothesis;

  const AtlasDetectorID* m_idHelper;

  Gaudi::Property<bool> m_useClusterPosition{
    this,
    "useClusterPosition",
    false,
    "Switch to control use of Cluster position measurement"
  };

  /** @brief Option to remove TRT hits from track*/
  Gaudi::Property<bool> m_RemoveTRT{ this,
                                     "RemoveTRTHits",
                                     false,
                                     "RemoveTRT Hits" };
};

#endif
