/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StripSurfaceChargesGenerator.h"

// DD
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

// Athena
#include "GeneratorObjects/HepMcParticleLink.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetSimEvent/SiHit.h" // for SiHit, SiHit::::xDep, etc
#include "HitManagement/TimedHitPtr.h" // for TimedHitPtr

// ROOT
#include "TH1.h" // for TH1F
#include "TH2.h" // for TH2F
#include "TProfile.h" // for TProfile

// CLHEP
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

#include "GaudiKernel/ConcurrencyFlags.h"

// C++ Standard Library
#include <cmath>

using InDetDD::SiDetectorElement;
using InDetDD::SCT_ModuleSideDesign;
using InDetDD::SiLocalPosition;

namespace ITk
{

// constructor
StripSurfaceChargesGenerator::StripSurfaceChargesGenerator(const std::string& type,
                                                         const std::string& name,
                                                         const IInterface* parent)
  : base_class(type, name, parent) {
}

// ----------------------------------------------------------------------
// Initialize
// ----------------------------------------------------------------------
StatusCode StripSurfaceChargesGenerator::initialize() {
  ATH_MSG_DEBUG("StripSurfaceChargesGenerator::initialize()");

  // Get ISiPropertiesTool
  ATH_CHECK(m_siPropertiesTool.retrieve());

  // Get ISiliconConditionsSvc
  ATH_CHECK(m_siConditionsTool.retrieve());

  ATH_CHECK(m_fieldCacheCondObjInputKey.initialize(m_doInducedChargeModel));

  if (m_doTrapping) {
    // -- Get Radiation Damage Tool
    ATH_CHECK(m_radDamageTool.retrieve());
  } else {
    m_radDamageTool.disable();
  }

  if (m_doHistoTrap) {
    if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
      ATH_MSG_FATAL("Filling histograms not supported in MT jobs.");
      return StatusCode::FAILURE;
    }

    // -- Get Histogram Service
    ATH_CHECK(m_thistSvc.retrieve());
    m_h = std::make_unique<Hists>();
    ATH_CHECK(m_h->book(*m_thistSvc));
  }
  ///////////////////////////////////////////////////

  // Induced Charge Module.
  if (m_doInducedChargeModel) {
    const SCT_ID* sct_id{nullptr};
    ATH_CHECK(detStore()->retrieve(sct_id, "SCT_ID"));
    m_InducedChargeModel = std::make_unique<InducedChargeModel>(sct_id->wafer_hash_max());
  }

  // Surface drift time calculation Stuff
  m_tHalfwayDrift = m_tSurfaceDrift * 0.5;
  m_distHalfInterStrip = m_distInterStrip * 0.5;
  if ((m_tSurfaceDrift > m_tHalfwayDrift) and (m_tHalfwayDrift >= 0.0) and
      (m_distHalfInterStrip > 0.0) and (m_distHalfInterStrip < m_distInterStrip)) {
    m_SurfaceDriftFlag = true;
  } else {
    ATH_MSG_INFO("\tsurface drift still not on, wrong params");
  }

  // Make sure these two flags are not set simultaneously
  if (m_tfix>-998. and m_tsubtract>-998.) {
    ATH_MSG_FATAL("\tCannot set both FixedTime and SubtractTime options!");
    ATH_MSG_INFO("\tMake sure the two flags are not set simultaneously in jo");
    return StatusCode::FAILURE;
  }

  if (m_doDistortions) {
    ATH_CHECK(m_distortionsTool.retrieve());
  } else {
    m_distortionsTool.disable();
  }

  ATH_CHECK(m_lorentzAngleTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode StripSurfaceChargesGenerator::Hists::book (ITHistSvc& histSvc)
{
  m_h_efieldz = new TProfile("efieldz", "", 50, 0., 0.4);
  ATH_CHECK(histSvc.regHist("/file1/efieldz", m_h_efieldz));

  m_h_efield = new TH1F("efield", "", 100, 200., 800.);
  ATH_CHECK(histSvc.regHist("/file1/efield", m_h_efield));

  m_h_spess = new TH1F("spess", "", 50, 0., 0.4);
  ATH_CHECK(histSvc.regHist("/file1/spess", m_h_spess));

  m_h_depD = new TH1F("depD", "", 50, -0.3, 0.3);
  ATH_CHECK(histSvc.regHist("/file1/depD", m_h_depD));

  m_h_drift_electrode = new TH2F("drift_electrode", "", 50, 0., 20., 50, 0., 20.);
  ATH_CHECK(histSvc.regHist("/file1/drift_electrode", m_h_drift_electrode));

  m_h_drift_time = new TH1F("drift_time", "", 100, 0., 20.);
  ATH_CHECK(histSvc.regHist("/file1/drift_time", m_h_drift_time));

  m_h_t_electrode = new TH1F("t_electrode", "", 100, 0., 20.);
  ATH_CHECK(histSvc.regHist("/file1/t_electrode", m_h_t_electrode));

  m_h_ztrap = new TH1F("ztrap", "", 100, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/ztrap", m_h_ztrap));

  // More histograms to check what's going on
  m_h_zhit = new TH1F("zhit", "", 50, -0.2, 0.2);
  ATH_CHECK(histSvc.regHist("/file1/zhit", m_h_zhit));

  m_h_ztrap_tot = new TH1F("ztrap_tot", "", 100, 0., 0.5);
  ATH_CHECK(histSvc.regHist("/file1/ztrap_tot", m_h_ztrap_tot));

  m_h_no_ztrap = new TH1F("no_ztrap", "", 100, 0., 0.5);
  ATH_CHECK(histSvc.regHist("/file1/no_ztrap", m_h_no_ztrap));

  m_h_trap_drift_t = new TH1F("trap_drift_t", "", 100, 0., 20.);
  ATH_CHECK(histSvc.regHist("/file1/trap_drift_t", m_h_trap_drift_t));

  m_h_notrap_drift_t = new TH1F("notrap_drift_t", "", 100, 0., 20.);
  ATH_CHECK(histSvc.regHist("/file1/notrap_drift_t", m_h_notrap_drift_t));

  m_h_mob_Char = new TProfile("mob_Char", "", 200, 100., 1000.);
  ATH_CHECK(histSvc.regHist("/file1/mob_Char", m_h_mob_Char));

  m_h_vel = new TProfile("vel", "", 100, 100., 1000.);
  ATH_CHECK(histSvc.regHist("/file1/vel", m_h_vel));

  m_h_drift1 = new TProfile("drift1", "", 50, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/drift1", m_h_drift1));

  m_h_gen = new TProfile("gen", "", 50, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/gen", m_h_gen));

  m_h_gen1 = new TProfile("gen1", "", 50, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/gen1", m_h_gen1));

  m_h_gen2 = new TProfile("gen2", "", 50, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/gen2", m_h_gen2));

  m_h_velocity_trap = new TProfile("velocity_trap", "", 50, 0., 1000.);
  ATH_CHECK(histSvc.regHist("/file1/velocity_trap", m_h_velocity_trap));

  m_h_mobility_trap = new TProfile("mobility_trap", "", 50, 100., 1000.);
  ATH_CHECK(histSvc.regHist("/file1/mobility_trap", m_h_mobility_trap));

  m_h_trap_pos = new TH1F("trap_pos", "", 100, 0., 0.3);
  ATH_CHECK(histSvc.regHist("/file1/trap_pos", m_h_trap_pos));

  return StatusCode::SUCCESS;
}


// ----------------------------------------------------------------------
// finalize
// ----------------------------------------------------------------------
StatusCode StripSurfaceChargesGenerator::finalize() {
  ATH_MSG_DEBUG("StripSurfaceChargesGenerator::finalize()");
  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
// perpandicular Drift time calculation
// ----------------------------------------------------------------------
float StripSurfaceChargesGenerator::driftTime(float zhit, const SiDetectorElement* element, const EventContext& ctx) const {
  if (element==nullptr) {
    ATH_MSG_ERROR("StripSurfaceChargesGenerator::process element is nullptr");
    return -2.0;
  }
  const SCT_ModuleSideDesign* design{dynamic_cast<const SCT_ModuleSideDesign*>(&(element->design()))};
  if (design==nullptr) {
    ATH_MSG_ERROR("StripSurfaceChargesGenerator::process can not get " << design);
    return -2.0;
  }
  const double thickness{design->thickness()};
  const IdentifierHash hashId{element->identifyHash()};

  if ((zhit < 0.0) or (zhit > thickness)) {
    ATH_MSG_DEBUG("driftTime: hit coordinate zhit=" << zhit / CLHEP::micrometer << " out of range");
    return -2.0;
  }

  float depletionVoltage{0.};
  float biasVoltage{0.};
  if (m_useSiCondDB) {
    depletionVoltage = m_siConditionsTool->depletionVoltage(hashId, ctx) * CLHEP::volt;
    biasVoltage = m_siConditionsTool->biasVoltage(hashId, ctx) * CLHEP::volt;
  } else {
    depletionVoltage = m_vdepl * CLHEP::volt;
    biasVoltage = m_vbias * CLHEP::volt;
  }

  const float denominator{static_cast<float>(depletionVoltage + biasVoltage - (2.0 * zhit * depletionVoltage / thickness))};
  if (denominator <= 0.0) {
    if (biasVoltage >= depletionVoltage) { // Should not happen
      if(not m_isOverlay) {
        ATH_MSG_ERROR("driftTime: negative argument X for log(X) " << zhit);
      }
      return -1.0;
    } else {
      // (m_biasVoltage<m_depletionVoltage) can happen with underdepleted sensors, lose charges in that volume
      return -10.0;
    }
  }

  float t_drift{std::log((depletionVoltage + biasVoltage) / denominator)};
  t_drift *= thickness * thickness / (2.0 * m_siPropertiesTool->getSiProperties(hashId, ctx).holeDriftMobility() * depletionVoltage);
  return t_drift;
}

// ----------------------------------------------------------------------
// Sigma diffusion calculation
// ----------------------------------------------------------------------
float StripSurfaceChargesGenerator::diffusionSigma(float zhit, const SiDetectorElement* element, const EventContext& ctx) const {
  if (element==nullptr) {
    ATH_MSG_ERROR("StripSurfaceChargesGenerator::diffusionSigma element is nullptr");
    return 0.0;
  }
  const IdentifierHash hashId{element->identifyHash()};
  const float t{driftTime(zhit, element, ctx)}; // in ns

  if (t > 0.0) {
    const float sigma{static_cast<float>(std::sqrt(2. * m_siPropertiesTool->getSiProperties(hashId, ctx).holeDiffusionConstant() * t))}; // in mm
    return sigma;
  } else {
    return 0.0;
  }
}

// ----------------------------------------------------------------------
// Maximum drift time
// ----------------------------------------------------------------------
float StripSurfaceChargesGenerator::maxDriftTime(const SiDetectorElement* element, const EventContext& ctx) const {
  if (element) {
    const float sensorThickness{static_cast<float>(element->thickness())};
    return driftTime(sensorThickness, element, ctx);
  } else {
    ATH_MSG_INFO("Error: SiDetectorElement not set!");
    return 0.;
  }
}

// ----------------------------------------------------------------------
// Maximum Sigma difusion
// ----------------------------------------------------------------------
float StripSurfaceChargesGenerator::maxDiffusionSigma(const SiDetectorElement* element, const EventContext& ctx) const {
  if (element) {
    const float sensorThickness{static_cast<float>(element->thickness())};
    return diffusionSigma(sensorThickness, element, ctx);
  } else {
    ATH_MSG_INFO("Error: SiDetectorElement not set!");
    return 0.;
  }
}

// ----------------------------------------------------------------------
// Calculating the surface drift time but I should confess that
// I haven't found out yet where the calculation come from
// ----------------------------------------------------------------------
float StripSurfaceChargesGenerator::surfaceDriftTime(float ysurf) const {
  if (m_SurfaceDriftFlag) {
    if ((ysurf >= 0.0) and (ysurf <= m_distInterStrip)) {
      float t_surfaceDrift{0.};
      if (ysurf < m_distHalfInterStrip) {
        const float y{ysurf / m_distHalfInterStrip};
        t_surfaceDrift= m_tHalfwayDrift * y * y;
      } else {
        const float y{(m_distInterStrip - ysurf) / (m_distInterStrip - m_distHalfInterStrip)};
        t_surfaceDrift = m_tSurfaceDrift + (m_tHalfwayDrift - m_tSurfaceDrift) * y * y;
      }
      return t_surfaceDrift;
    } else {
      ATH_MSG_INFO(" ysurf out of range " << ysurf);
      return -1.0;
    }
  } else {
    return 0.0;
  }
}

// -------------------------------------------------------------------------------------------
// create a list of surface charges from a hit - called from StripDigitization
// AthAlgorithm
// -------------------------------------------------------------------------------------------
void StripSurfaceChargesGenerator::process(const SiDetectorElement* element,
                                          const TimedHitPtr<SiHit>& phit,
                                          ISiSurfaceChargesInserter& inserter,
                                          CLHEP::HepRandomEngine * rndmEngine,
                                          const EventContext& ctx) {
  ATH_MSG_VERBOSE("StripSurfaceChargesGenerator::process starts");
  processSiHit(element, *phit, inserter, phit.eventTime(), phit.eventId(), rndmEngine, ctx);
}

// -------------------------------------------------------------------------------------------
// create a list of surface charges from a hit - called from both AthAlgorithm
// and PileUpTool
// -------------------------------------------------------------------------------------------
void StripSurfaceChargesGenerator::processSiHit(const SiDetectorElement* element,
                                               const SiHit& phit,
                                               ISiSurfaceChargesInserter& inserter,
                                               float p_eventTime,
                                               unsigned short p_eventId,
                                               CLHEP::HepRandomEngine* rndmEngine,
                                               const EventContext& ctx) const {
  const SCT_ModuleSideDesign* design;
  const SCT_ModuleSideDesign* initialDesign{dynamic_cast<const SCT_ModuleSideDesign*>(&(element->design()))};
  if (initialDesign==nullptr) {
    ATH_MSG_ERROR("StripSurfaceChargesGenerator::process can not get " << initialDesign);
    return;
  }

  //If this proves costly, we can pass a (potentially null) motherDesign 
  //in the interface instead, but this will require a new interface class
  //(see comment in SiDigitization/ISurfaceChargesGenerator.h)

  const SCT_ModuleSideDesign* motherDesign  = initialDesign->getMother();

  if(motherDesign!=nullptr){
    ATH_MSG_DEBUG("Found a Mother Design - Using it!");
    design = motherDesign;
  }
  else {
    ATH_MSG_DEBUG("No Mother Design - Using Design from DetElement directly!");
    design = initialDesign;
  }

  const double thickness{design->thickness()};
  const IdentifierHash hashId{element->identifyHash()};
  const double tanLorentz{m_lorentzAngleTool->getTanLorentzAngle(hashId)};

  // ---**************************************
  //  Time of Flight Calculation - separate method?
  // ---**************************************
  // --- Original calculation of Time of Flight of the particle Time needed by the particle to reach the sensor
  float timeOfFlight{p_eventTime + hitTime(phit)};

  // Kondo 19/09/2007: Use the coordinate of the center of the module to calculate the time of flight
  timeOfFlight -= (element->center().mag()) / CLHEP::c_light;
  // !< extract the distance to the origin of the module to Time of flight

  // !< timing set from jo to adjust (subtract) the timing
  if (m_tsubtract > -998.) {
    timeOfFlight -= m_tsubtract;
  }
  // ---**************************************

  const CLHEP::Hep3Vector pos{phit.localStartPosition()};
  const float xEta{static_cast<float>(pos[SiHit::xEta])};
  const float xPhi{static_cast<float>(pos[SiHit::xPhi])};
  const float xDep{static_cast<float>(pos[SiHit::xDep])};

  const CLHEP::Hep3Vector endPos{phit.localEndPosition()};
  const float cEta{static_cast<float>(endPos[SiHit::xEta]) - xEta};
  const float cPhi{static_cast<float>(endPos[SiHit::xPhi]) - xPhi};
  const float cDep{static_cast<float>(endPos[SiHit::xDep]) - xDep};



  const float largeStep{std::sqrt(cEta*cEta + cPhi*cPhi + cDep*cDep)};
  const int numberOfSteps{static_cast<int>(largeStep / m_smallStepLength) + 1};
  const float steps{static_cast<float>(m_numberOfCharges * numberOfSteps)};
  const float e1{static_cast<float>(phit.energyLoss() / steps)};
  const float q1{static_cast<float>(e1 * m_siPropertiesTool->getSiProperties(hashId, ctx).electronHolePairsPerEnergy())};

  // NB this is different to the SCT, where this would be
  //float xhit{xEta};
  //float yhit{xPhi};
  //float zhit{xDep};
  //float cX{cEta};
  //float cY{cPhi};
  //float cZ{cDep};

 float xhit{xDep};
 float yhit{xPhi};
 float zhit{xEta};
 float cX{cDep};
 float cY{cPhi};
 float cZ{cEta};

  InducedChargeModel::SCT_InducedChargeModelData* data{nullptr};
  if (m_doInducedChargeModel) { // Setting magnetic field for the ICM.
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
    const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
    float vdepl{m_vdepl};
    float vbias{m_vbias};
    if (m_useSiCondDB) {
      vdepl = m_siConditionsTool->depletionVoltage(hashId, ctx);
      vbias = m_siConditionsTool->biasVoltage(hashId, ctx);
    }
    data = m_InducedChargeModel->setWaferData(vdepl,
                                              vbias,
                                              element,
                                              fieldCondObj,
                                              m_siConditionsTool,
                                              rndmEngine,
                                              ctx);
  }

  if (m_doDistortions) {
    if (element->isBarrel()) {// Only apply disortions to barrel modules
      Amg::Vector2D BOW;
      BOW[0] = m_distortionsTool->correctSimulation(hashId, xhit, yhit, cX, cY, cZ)[0];
      BOW[1] = m_distortionsTool->correctSimulation(hashId, xhit, yhit, cX, cY, cZ)[1];
      xhit = BOW.x();
      yhit = BOW.y();
    }
  }

  const float stepX{cX / numberOfSteps};
  const float stepY{cY / numberOfSteps};
  const float stepZ{cZ / numberOfSteps};

  // check the status of truth information for this SiHit
  // some Truth information is cut for pile up events
  const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
  const HepMcParticleLink::PositionFlag idxFlag = (p_eventId==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_EVENTNUM;
  const HepMcParticleLink trklink{HepMcParticleLink(phit.trackNumber(), p_eventId, evColl, idxFlag, ctx)};
  SiCharge::Process hitproc{SiCharge::track};
  if (phit.trackNumber() != 0) {
    if (not trklink.isValid()) {
      hitproc = SiCharge::cut_track;
    }
  }

  float dstep{-0.5};
  for (int istep{0}; istep < numberOfSteps; ++istep) {
    dstep += 1.0;
    float z1{zhit + stepZ * dstep};

    // Distance between charge and readout side.
    // design->readoutSide() is +1 if readout side is in +ve depth axis direction and visa-versa.
    float zReadout{static_cast<float>(0.5 * thickness - design->readoutSide() * z1)};
    const double spess{zReadout};

    if (m_doHistoTrap) {
      Hists& h = getHists();
      h.m_h_depD->Fill(z1);
      h.m_h_spess->Fill(spess);
    }

    float t_drift{driftTime(zReadout, element, ctx)};  // !< t_drift: perpandicular drift time
    if (t_drift>-2.0000002 and t_drift<-1.9999998) {
      ATH_MSG_DEBUG("Checking for rounding errors in compression");
      if ((std::abs(z1) - 0.5 * thickness) < 0.000010) {
        ATH_MSG_DEBUG("Rounding error found attempting to correct it. z1 = " << std::fixed << std::setprecision(8) << z1);
        if (z1 < 0.0) {
          z1 = 0.0000005 - 0.5 * thickness;
          // set new coordinate to be 0.5nm inside wafer volume.
        } else {
          z1 = 0.5 * thickness - 0.0000005;
          // set new coordinate to be 0.5nm inside wafer volume.
        }
        zReadout = 0.5 * thickness - design->readoutSide() * z1;
        t_drift = driftTime(zReadout, element, ctx);
        if (t_drift>-2.0000002 and t_drift<-1.9999998) {
          ATH_MSG_WARNING("Attempt failed. Making no correction.");
        } else {
          ATH_MSG_DEBUG("Correction Successful! z1 = " << std::fixed << std::setprecision(8) << z1 << ", zReadout = " << zReadout << ", t_drift = " << t_drift);
        }
      } else {
        ATH_MSG_DEBUG("No rounding error found. Making no correction.");
      }
    }
    if (t_drift > 0.0) {
      const float x1{xhit + stepX * dstep};
      float y1{yhit + stepY * dstep};

      float sigma{0.};
      if (not m_doInducedChargeModel) {
        sigma = diffusionSigma(zReadout, element, ctx);
        y1 += tanLorentz * zReadout; // !< Taking into account the magnetic field
      } // These are treated in Induced Charge Model.

      for (int i{0}; i < m_numberOfCharges; ++i) {
        const float rx{CLHEP::RandGaussZiggurat::shoot(rndmEngine)};
        const float xd{x1 + sigma * rx};
        const float ry{CLHEP::RandGaussZiggurat::shoot(rndmEngine)};
        const float yd{y1 + sigma * ry};

        // For charge trapping with Ramo potential
        const double stripPitch{0.080}; // mm
        double dstrip{y1 / stripPitch}; // mm
        if (dstrip > 0.) {
          dstrip = dstrip - std::trunc(dstrip);
        } else {
          dstrip = dstrip - std::trunc(dstrip) + 1;
        }

        // now y will be x and z will be y ....just to make sure to confuse everebody
        double y0{dstrip * stripPitch}; // mm
        double z0{thickness - zReadout}; // mm

        // -- Charge Trapping
        if (m_doTrapping) {
          if (m_doHistoTrap) {
            Hists& h = getHists();
            h.m_h_zhit->Fill(zhit);
          }
          double trap_pos{-999999.}, drift_time{-999999.}; // FIXME need better default values
          if (chargeIsTrapped(spess, element, trap_pos, drift_time)) {
            if (not m_doRamo) {
              break;
            } else {  // if we want to take into account also Ramo Potential
              double Q_m2{0.}, Q_m1{0.}, Q_00{0.}, Q_p1{0.}, Q_p2{0.}; // Charges

              dstrip = y1 / stripPitch; // mm
              // need the distance from the nearest strips
              // edge not centre, xtaka = 1/2*stripPitch
              // centre of detector, y1=0, is in the middle of
              // an interstrip gap
              if (dstrip > 0.) {
                dstrip -= static_cast<double>(static_cast<int>(dstrip));
              } else {
                dstrip -= static_cast<double>(static_cast<int>(dstrip)) + 1;
              }

              // now y will be x and z will be y ....just to make sure to confuse everebody
              double yfin{dstrip * stripPitch}; // mm
              double zfin{thickness - trap_pos}; // mm

              m_radDamageTool->holeTransport(y0, z0, yfin, zfin, Q_m2, Q_m1, Q_00, Q_p1, Q_p2);
              for (int strip{-2}; strip<=2; strip++) {
                const double ystrip{yd + strip * stripPitch}; // mm
                const SiLocalPosition position(element->hitLocalToLocal(xd, ystrip));
                if (design->inActiveArea(position)) {
                  double charge{0.};
                  if (strip == -2) charge = Q_m2;
                  else if (strip == -1) charge = Q_m1;
                  else if (strip ==  0) charge = Q_00;
                  else if (strip ==  1) charge = Q_p1;
                  else if (strip ==  2) charge = Q_p2;
                  const double time{drift_time};
                  if (charge != 0.) {
                    inserter(SiSurfaceCharge(position, SiCharge(q1*charge, time, hitproc, trklink)));
                    continue;
                  }
                }
              }
              ATH_MSG_INFO("strip zero charge = " << Q_00); // debug
            } // m_doRamo==true
          } // chargeIsTrapped()
        } // m_doTrapping==true

        if (not m_doRamo) {
          if (m_doInducedChargeModel) { // Induced Charge Model
            // Charges storages for 50 ns. 0.5 ns steps.
            double Q_m2[InducedChargeModel::NTransportSteps]={0};
            double Q_m1[InducedChargeModel::NTransportSteps]={0};
            double Q_00[InducedChargeModel::NTransportSteps]={0};
            double Q_p1[InducedChargeModel::NTransportSteps]={0};
            double Q_p2[InducedChargeModel::NTransportSteps]={0};

            const double mm2cm = 0.1; // For mm -> cm conversion
            // Unit for y and z : mm -> cm in InducedChargeModel
            m_InducedChargeModel->holeTransport(*data,
                                                y0*mm2cm, z0*mm2cm,
                                                Q_m2, Q_m1, Q_00, Q_p1, Q_p2,
                                                hashId, m_siPropertiesTool,
                                                ctx);
            m_InducedChargeModel->electronTransport(*data,
                                                    y0*mm2cm, z0*mm2cm,
                                                    Q_m2, Q_m1, Q_00, Q_p1, Q_p2,
                                                    hashId, m_siPropertiesTool,
                                                    ctx);

            for (int it{0}; it<InducedChargeModel::NTransportSteps; it++) {
              if (Q_00[it] == 0.0) continue;
              double ICM_time{(it+0.5)*0.5 + timeOfFlight};
              double Q_new[InducedChargeModel::NStrips]{
                Q_m2[it], Q_m1[it], Q_00[it], Q_p1[it], Q_p2[it]
              };
              for (int strip{InducedChargeModel::StartStrip}; strip<=InducedChargeModel::EndStrip; strip++) {
                double ystrip{y1 + strip * stripPitch};
                SiLocalPosition position{element->hitLocalToLocal(x1, ystrip)};
                if (design->inActiveArea(position)) {
                  inserter(SiSurfaceCharge(position,
                                           SiCharge(q1 * Q_new[strip+InducedChargeModel::Offset],
                                                    ICM_time, hitproc, trklink)));
                }
              }
            }
          } else { // not m_doInducedChargeModel
            const SiLocalPosition position{element->hitLocalToLocal(xd, yd)};
            if (design->inActiveArea(position)) {
              const float sdist{static_cast<float>(design->scaledDistanceToNearestDiode(position))};
              // !< dist on the surface from the hit point to the nearest strip (diode)
              const float t_surf{surfaceDriftTime(2.0 * sdist)}; // !< Surface drift time
              const float totaltime{(m_tfix > -998.) ? m_tfix.value() : t_drift + timeOfFlight + t_surf}; // !< Total drift time
              inserter(SiSurfaceCharge(position, SiCharge(q1, totaltime, hitproc, trklink)));
            } else {
              ATH_MSG_VERBOSE(std::fixed << std::setprecision(8) << "Local position (phi, eta, depth): ("
                              << position.xPhi() << ", " << position.xEta() << ", " << position.xDepth()
                              << ") of the element is out of active area, charge = " << q1);
            }
          }
        }
      } // end of loop on charges
    }
  }
  }

bool StripSurfaceChargesGenerator::chargeIsTrapped(double spess,
                                                  const SiDetectorElement* element,
                                                  double& trap_pos,
                                                  double& drift_time) const {
  if (element==nullptr) {
    ATH_MSG_ERROR("StripSurfaceChargesGenerator::chargeIsTrapped element is nullptr");
    return false;
  }
  bool isTrapped{false};
  const IdentifierHash hashId{element->identifyHash()};
  const SCT_ChargeTrappingCondData condData{m_radDamageTool->getCondData(hashId, spess)};
  const double electric_field{condData.getElectricField()};

  if (m_doHistoTrap) {
    Hists& h = getHists();
    const double mobChar{condData.getHoleDriftMobility()};
    h.m_h_efieldz->Fill(spess, electric_field);
    h.m_h_efield->Fill(electric_field);
    h.m_h_mob_Char->Fill(electric_field, mobChar);
    h.m_h_vel->Fill(electric_field, electric_field * mobChar);
  }
  const double t_electrode{condData.getTimeToElectrode()};
  drift_time = condData.getTrappingTime();
  const double z_trap{condData.getTrappingPositionZ()};
  trap_pos = spess - z_trap;
  if (m_doHistoTrap) {
    Hists& h = getHists();
    h.m_h_drift_time->Fill(drift_time);
    h.m_h_t_electrode->Fill(t_electrode);
    h.m_h_drift_electrode->Fill(drift_time, t_electrode);
    h.m_h_ztrap_tot->Fill(z_trap);
  }
  // -- Calculate if the charge is trapped, and at which distance
  // -- Charge gets trapped before arriving to the electrode
  if (drift_time < t_electrode) {
    isTrapped = true;
    ATH_MSG_INFO("drift_time: " << drift_time << " t_electrode:  " << t_electrode << " spess " << spess);
    ATH_MSG_INFO("z_trap: " << z_trap);
    if (m_doHistoTrap) {
      Hists& h = getHists();
      h.m_h_ztrap->Fill(z_trap);
      h.m_h_trap_drift_t->Fill(drift_time);
      h.m_h_drift1->Fill(spess, drift_time / t_electrode);
      h.m_h_gen->Fill(spess, drift_time);
      h.m_h_gen1->Fill(spess, z_trap);
      h.m_h_gen2->Fill(spess, z_trap / drift_time * t_electrode);
      h.m_h_velocity_trap->Fill(electric_field, z_trap / drift_time);
      h.m_h_mobility_trap->Fill(electric_field, z_trap / drift_time / electric_field);
      h.m_h_trap_pos->Fill(trap_pos);
    }
  } else {
    isTrapped = false;
    if (m_doHistoTrap) {
      const double z_trap{condData.getTrappingPositionZ()};
      Hists& h = getHists();
      h.m_h_no_ztrap->Fill(z_trap);
      h.m_h_notrap_drift_t->Fill(drift_time);
    }
  }
  return isTrapped;
}


StripSurfaceChargesGenerator::Hists&
StripSurfaceChargesGenerator::getHists() const
{
  // We earlier checked that no more than one thread is being used.
  Hists* h ATLAS_THREAD_SAFE = m_h.get();
  return *h;
}


} // namespace ITk
