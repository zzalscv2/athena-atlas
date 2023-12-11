/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_ACTSTOOLS_ACTSFATRASSIMTOOL_H
#define ISF_ACTSTOOLS_ACTSFATRASSIMTOOL_H

// Gaudi
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"
// Barcode
#include "BarcodeInterfaces/IBarcodeSvc.h"

#include "CxxUtils/checker_macros.h"

// ISF
#include "ISF_Event/ISFParticle.h"
#include "ISF_Interfaces/BaseSimulatorTool.h"
#include "ISF_Interfaces/IParticleFilter.h"
#include "ISF_Interfaces/ITruthSvc.h"

// ACTS
#include "Acts/Utilities/UnitVectors.hpp"
#include "ActsInterop/Logger.h"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/EventData/GenericCurvilinearTrackParameters.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Definitions/ParticleData.hpp"
#include "ActsFatras/EventData/ProcessType.hpp"
#include "ActsFatras/Kernel/InteractionList.hpp"
#include "ActsFatras/Kernel/Simulation.hpp"
#include "ActsFatras/Kernel/SimulationResult.hpp"
#include "ActsFatras/Physics/Decay/NoDecay.hpp"
#include "ActsFatras/Physics/StandardInteractions.hpp"
#include "ActsFatras/Selectors/SurfaceSelectors.hpp"
// Tracking
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

#include <algorithm>
#include <cassert>
#include <vector>

class AtlasDetectorID;

namespace iFatras {
  class ISimHitCreator;
}

namespace ISF {
class IParticleHelper;

/**
   @class ACTSFATRASSIMTOOL

   Standard ATLAS hit creator, with Acts-fatras simulation
*/

class ActsFatrasSimTool : public BaseSimulatorTool {
 
 public:
  /// Simple struct to select surfaces where hits should be generated.
  struct HitSurfaceSelector {
    /// Check if the surface should be used.
    bool operator()(const Acts::Surface &surface) const {
      bool isSensitive = surface.associatedDetectorElement();
      return isSensitive;
    }
  };
  // SingleParticleSimulation
  /// Single particle simulation with fixed propagator, interactions, and decay.
  ///
  /// @tparam generator_t random number generator
  /// @tparam interactions_t interaction list
  /// @tparam hit_surface_selector_t selector for hit surfaces
  /// @tparam decay_t decay module
  template <typename propagator_t, typename interactions_t,
            typename hit_surface_selector_t, typename decay_t>
  struct SingleParticleSimulation {
    /// How and within which geometry to propagate the particle.
    propagator_t propagator;
    /// Decay module.
    decay_t decay;
    /// Interaction list containing the simulated interactions.
    interactions_t interactions;
    /// Selector for surfaces that should generate hits.
    hit_surface_selector_t selectHitSurface;
    /// parameters for propagator options
    double maxStepSize = 3.0; // leght in m
    double maxStep = 1000;
    double maxRungeKuttaStepTrials = 10000;
    double pathLimit = 100.0; // lenght in cm
    bool   loopProtection = true;
    double loopFraction = 0.5;
    double stepTolerance = 0.0001;
    double stepSizeCutOff = 0.;
    // parameters for densEnv propagator options
    double meanEnergyLoss = true;
    bool   includeGgradient = true;
    double momentumCutOff = 0.;

  /// Logger for debug output.
  std::unique_ptr<const Acts::Logger> logger;

    /// Alternatively construct the simulator with an external logger.
    SingleParticleSimulation(propagator_t &&propagator_,
                           std::unique_ptr<const Acts::Logger> _logger)
      : propagator(propagator_), logger(std::move(_logger)) {}

    /// Simulate a single particle without secondaries.
    ///
    /// @tparam generator_t is the type of the random number generator
    ///
    /// @param geoCtx is the geometry context to access surface geometries
    /// @param magCtx is the magnetic field context to access field values
    /// @param generator is the random number generator
    /// @param particle is the initial particle state
    /// @returns Simulated particle state, hits, and generated particles.
    template <typename generator_t>
    Acts::Result<ActsFatras::SimulationResult> simulate(
        const Acts::GeometryContext &geoCtx,
        const Acts::MagneticFieldContext &magCtx, generator_t &generator,
        const ActsFatras::Particle &particle) const {
      // propagator-related additional types
      using SteppingLogger = Acts::detail::SteppingLogger;
      using Actor = ActsFatras::detail::SimulationActor<generator_t, decay_t, interactions_t, hit_surface_selector_t>;
      using Aborter = typename Actor::ParticleNotAlive;
      using Result = typename Actor::result_type;
      using Actions = Acts::ActionList<SteppingLogger, Actor>;
      using Abort = Acts::AbortList<Aborter, Acts::EndOfWorldReached>;
      using PropagatorOptions = Acts::DenseStepperPropagatorOptions<Actions, Abort>;

      // Construct per-call options.
      PropagatorOptions options(geoCtx, magCtx);
      // setup the interactor as part of the propagator options
      auto &actor = options.actionList.template get<Actor>();
      actor.generator = &generator;
      actor.decay = decay;
      actor.interactions = interactions;
      actor.selectHitSurface = selectHitSurface;
      actor.initialParticle = particle;
      // use AnyCharge to be able to handle neutral and charged parameters
      Acts::GenericCurvilinearTrackParameters startPoint(
          particle.fourPosition(), particle.direction(),
          particle.qOverP(), std::nullopt, particle.hypothesis());
      options.pathLimit = pathLimit * Acts::UnitConstants::cm;
      options.loopProtection = loopProtection;
      options.maxStepSize = maxStepSize * Acts::UnitConstants::m;
      options.maxSteps = maxStep;
      options.maxRungeKuttaStepTrials = maxRungeKuttaStepTrials;
      options.loopFraction = loopFraction;
      options.stepTolerance = stepTolerance;
      options.stepSizeCutOff = stepSizeCutOff;
      // parameters for densEnv propagator options
      options.meanEnergyLoss = meanEnergyLoss;
      options.includeGgradient = includeGgradient;
      options.momentumCutOff = momentumCutOff;

      auto result = propagator.propagate(startPoint, options);
      if (not result.ok()) {
        return result.error();
      }
      return result.value().template get<Result>();
    }
  };// end of SingleParticleSimulation

  // Standard generator
  using Generator = std::ranlux48;
  // Use default navigator
  using Navigator = Acts::Navigator;
  // Propagate charged particles numerically in the B-field
  using ChargedStepper = Acts::EigenStepper<Acts::StepperExtensionList<
                                            Acts::DefaultExtension,
                                            Acts::DenseEnvironmentExtension
                                          >
                        >;
  using ChargedPropagator = Acts::Propagator<ChargedStepper, Navigator>;
  // Propagate neutral particles in straight lines
  using NeutralStepper = Acts::StraightLineStepper;
  using NeutralPropagator = Acts::Propagator<NeutralStepper, Navigator>;

  // ===============================
  // Setup ActsFatras simulator types
  // Charged
  using ChargedSelector = ActsFatras::ChargedSelector;
  using ChargedInteractions =
          ActsFatras::StandardChargedElectroMagneticInteractions;
  using ChargedSimulation = SingleParticleSimulation<
          ChargedPropagator, ChargedInteractions, HitSurfaceSelector,
          ActsFatras::NoDecay>;
  // Neutral
  using NeutralSelector = ActsFatras::NeutralSelector;
  using NeutralInteractions = ActsFatras::InteractionList<>;
  using NeutralSimulation = SingleParticleSimulation<
          NeutralPropagator, NeutralInteractions, ActsFatras::NoSurface,
          ActsFatras::NoDecay>;
  // Combined
  using Simulation = ActsFatras::Simulation<ChargedSelector, ChargedSimulation,
                                            NeutralSelector, NeutralSimulation>;
  // ===============================

  ActsFatrasSimTool(const std::string& type, const std::string& name,
                    const IInterface* parent);
  virtual ~ActsFatrasSimTool();

  // ISF BaseSimulatorTool Interface methods
  virtual StatusCode initialize() override;
  virtual StatusCode simulate(ISFParticle& isp, ISFParticleContainer&,
                              McEventCollection*) override;
  virtual StatusCode simulateVector(
            const ISFParticleVector& particles,
            ISFParticleContainer& secondaries,
            McEventCollection* mcEventCollection, McEventCollection *shadowTruth=nullptr) override;
  virtual StatusCode setupEvent(const EventContext&) override {
    return StatusCode::SUCCESS; };
  virtual StatusCode releaseEvent(const EventContext&) override {
    return StatusCode::SUCCESS; };
  virtual ISF::SimulationFlavor simFlavor() const override{
    return ISF::Fatras; };

  virtual Acts::MagneticFieldContext getMagneticFieldContext(
    const EventContext&) const;

 private:
  // Templated tool retrieval
  template <class T>
  StatusCode retrieveTool(ToolHandle<T>& thandle) {
    if (!thandle.empty() && thandle.retrieve().isFailure()) {
      ATH_MSG_FATAL("Cannot retrieve " << thandle << ". Abort.");
      return StatusCode::FAILURE;
    } else ATH_MSG_DEBUG("Successfully retrieved " << thandle);
    return StatusCode::SUCCESS;
  }

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rngSvc{this, "RNGServec", "AthRNGSvc"};
  ATHRNG::RNGWrapper* m_randomEngine ATLAS_THREAD_SAFE {};
  Gaudi::Property<std::string> m_randomEngineName{this, "RandomEngineName",
    "RandomEngineName", "Name of random number stream"};

  // barcode
  ServiceHandle<Barcode::IBarcodeSvc>  m_barcodeSvc{this, "BarcodeSvc", "BarcodeSvc"};

  // Tracking geometry
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;

  // Magnetic field
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

  // Logging
  std::shared_ptr<const Acts::Logger> m_logger{nullptr};

  // ISF Tools
  PublicToolHandle<ISF::IParticleFilter> m_particleFilter{
      this, "ParticleFilter", "", "Particle filter kinematic cuts, etc."};

  ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{this,"TruthRecordService", "ISF_TruthRecordSvc", "ISF Particle Truth Svc"};

  Gaudi::Property<double> m_interact_minPt{this, "Interact_MinPt", 50.0,
      "Min pT of the interactions (MeV)"};

  // DensEnviroment Propergator option
  Gaudi::Property<bool> m_meanEnergyLoss{this, "MeanEnergyLoss", true, "Toggle between mean and mode evaluation of energy loss"};
  Gaudi::Property<bool> m_includeGgradient{this, "IncludeGgradient", true, "Boolean flag for inclusion of d(dEds)d(q/p) into energy loss"};
  Gaudi::Property<double> m_momentumCutOff{this, "MomentumCutOff", 0., "Cut-off value for the momentum in SI units"};
  // Propergator option
  Gaudi::Property<double> m_maxStep{this, "MaxSteps", 1000,
       "Max number of steps"};
  Gaudi::Property<double> m_maxRungeKuttaStepTrials{this, "MaxRungeKuttaStepTrials", 10000,
       "Maximum number of Runge-Kutta steps for the stepper step call"};
  Gaudi::Property<double> m_maxStepSize{this, "MaxStepSize", 3.0,
      "Max step size (converted to Acts::UnitConstants::m)"};
  Gaudi::Property<double> m_pathLimit{this, "PathLimit", 100.0,
      "Track path limit (converted to Acts::UnitConstants::cm)"};
  Gaudi::Property<bool> m_loopProtection{this, "LoopProtection", true,
      "Loop protection, it adapts the pathLimit"};
  Gaudi::Property<double> m_loopFraction{this, "LoopFraction", 0.5,
      "Allowed loop fraction, 1 is a full loop"};
  Gaudi::Property<double> m_tolerance{this, "Tolerance", 0.0001,
       "Tolerance for the error of the integration"};
  Gaudi::Property<double> m_stepSizeCutOff{this, "StepSizeCutOff", 0.,
       "Cut-off value for the step size"};

  Gaudi::Property<std::map<int,int>> m_processTypeMap{this, "ProcessTypeMap",
      {{0,0}, {1,201}, {2,14}, {3,3}, {4,121}}, "proessType map <ActsFatras,G4>"};
      //{{ActsProcessType::eUndefined,0}, {ActsProcessType::eDecay,201}, {ActsProcessType::ePhotonConversion,14}, {ActsProcessType::eBremsstrahlung,3}, {ActsProcessType::eNuclearInteraction,121}}
  inline int getATLASProcessCode(ActsFatras::ProcessType actspt){return m_processTypeMap[static_cast<uint32_t>(actspt)];};
};

}  // namespace ISF

#endif
