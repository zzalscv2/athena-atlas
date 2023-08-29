/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
// FileName : TileHitVecToCntTool.cxx
// Author   : Vishnu Zutshi
// Created  : Dec. 2009
//************************************************************

// Tile includes
#include "TileSimAlgs/TileHitVecToCntTool.h"
#include "TileIdentifier/TileHWID.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileConditions/TileCablingService.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Calo includes
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/TileTBID.h"

// Athena includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"

// Trigger time
#include "AthenaKernel/ITriggerTime.h"
#include "AthenaKernel/errorcheck.h"
// For the Athena-based random numbers.
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"

#include "CLHEP/Random/Randomize.h"
#include "CLHEP/Random/RandomEngine.h"

#include <algorithm>

using CLHEP::RandFlat;
using CLHEP::RandGaussQ; // FIXME CLHEP::RandGaussZiggurat is faster and more accurate.
using CLHEP::RandPoissonT;
using CLHEP::RandGeneral;

TileHitVecToCntTool::TileHitVecToCntTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : PileUpToolBase(type,name,parent)
{
}

StatusCode TileHitVecToCntTool::initialize() {

  ATH_MSG_DEBUG("TileHitVecToCntTool initialization started");

  bool error = false;

  ATH_CHECK(m_rndmSvc.retrieve());

  // retrieve TileID helper from det store
  ATH_CHECK(detStore()->retrieve(m_tileID));

  ATH_CHECK(detStore()->retrieve(m_tileTBID));

  ATH_CHECK(detStore()->retrieve(m_tileHWID));

  ATH_CHECK( m_samplingFractionKey.initialize() );

  ATH_CHECK(m_cablingSvc.retrieve());
  m_cabling = m_cablingSvc->cablingService();

  m_run2 = m_cabling->isRun2Cabling();
  m_run2plus = m_cabling->isRun2PlusCabling();

  if (!m_usePhotoStatistics) {
    ATH_MSG_INFO("No photostatistics effect will be simulated");
  }

  if (m_rndmEvtOverlay) {
    m_pileUp = false;
    ATH_MSG_INFO("Zero-luminosity pile-up selected");
    ATH_MSG_INFO("Taking hits from original event only");
  }

  if (m_pileUp || m_rndmEvtOverlay) {
    ATH_MSG_INFO("take events from PileUp service");

    if (m_onlyUseContainerName) {
      ATH_CHECK(m_mergeSvc.retrieve());
    }

    if (m_useTriggerTime) {
      ATH_MSG_INFO(" In case of pileup, the trigger time subtraction is done in PileUpSvc");
      ATH_MSG_INFO("  => TileHitVecToCnt will not apply Trigger Time ");
      m_useTriggerTime = false;
    }
    m_timeFlag = 0;

    if (m_pileUp) {
      // prepare vector with all hits
      m_mbtsOffset = m_tileID->pmt_hash_max();
      if (m_run2){
        m_allHits.resize(m_mbtsOffset + N_MBTS_CELLS + N_E4PRIME_CELLS);
        m_allHits_DigiHSTruth.resize(m_mbtsOffset + N_MBTS_CELLS + N_E4PRIME_CELLS);
      } else {
        m_allHits.resize(m_mbtsOffset + N_MBTS_CELLS);
        m_allHits_DigiHSTruth.resize(m_mbtsOffset + N_MBTS_CELLS);
      }

      Identifier hit_id;
      IdContext pmt_context = m_tileID->pmt_context();
      for (int i = 0; i < m_mbtsOffset; ++i) {
        m_tileID->get_id((IdentifierHash) i, hit_id, &pmt_context);
        TileHit * pHit = new TileHit(hit_id, 0., 0.);
        pHit->reserve(71); // reserve max possible size for pileup
        m_allHits[i] = pHit;
        if(m_doDigiTruth){
          TileHit * pHit_DigiHSTruth = new TileHit(hit_id, 0., 0.);
          pHit_DigiHSTruth->reserve(71); // reserve max possible size for pileup
          m_allHits_DigiHSTruth[i] = pHit_DigiHSTruth;
        }
      }
      for (int side = 0; side < N_SIDE; ++side) {
        for (int phi = 0; phi < N_PHI; ++phi) {
          for (int eta = 0; eta < N_ETA; ++eta) {
            hit_id = m_tileTBID->channel_id((side > 0) ? 1 : -1, phi, eta);
            TileHit * pHit = new TileHit(hit_id, 0., 0.);
            pHit->reserve(71); // reserve max possible size for pileup
            m_allHits[mbts_index(side, phi, eta)] = pHit;
            if(m_doDigiTruth){
              TileHit * pHit_DigiHSTruth = new TileHit(hit_id, 0., 0.);
              pHit_DigiHSTruth->reserve(71); // reserve max possible size for pileup
              m_allHits_DigiHSTruth[mbts_index(side, phi, eta)] = pHit_DigiHSTruth;
            }
          }
        }
      }
      if (m_run2) {
        for (int phi = 0; phi < E4_N_PHI; ++phi) {
          hit_id = m_tileTBID->channel_id(E4_SIDE, phi, E4_ETA);
          TileHit * pHit = new TileHit(hit_id, 0., 0.);
          pHit->reserve(71); // reserve max possible size for pileup
          m_allHits[e4pr_index(phi)] = pHit;
          if(m_doDigiTruth){
            TileHit * pHit_DigiHSTruth = new TileHit(hit_id, 0., 0.);
            pHit_DigiHSTruth->reserve(71); // reserve max possible size for pileup
            m_allHits_DigiHSTruth[e4pr_index(phi)] = pHit_DigiHSTruth;
          }
        }
      }
    }

  } else {

    ATH_MSG_INFO("no pile up");

    if (m_useTriggerTime) {

      m_timeFlag = 2;

      if (!m_triggerTimeTool.empty()) {
        ATH_MSG_INFO( "Trigger time is taken from external tool '" << m_triggerTimeTool.name()
                     << "'; therefore set HitTimeFlag to 2");
        if (m_triggerTimeTool.retrieve().isFailure()) {
          error = true;
          ATH_MSG_ERROR("Unable to find tool for " << m_triggerTimeTool.name());
          ATH_MSG_ERROR("Take average time from all hits in event as trigger time");
          m_useTriggerTime = false;
          m_triggerTimeTool.setTypeAndName("");
        }
      }
    }

    switch (m_timeFlag) {
      case 2:
        if (m_triggerTimeTool.empty()) {
          if (m_triggerTime > 0.0) {
            m_useTriggerTime = true;
            ATH_MSG_INFO("Fixed trigger time of " << m_triggerTime << " ns will be used");
          } else if (m_triggerTime < 0.0) {
            m_useTriggerTime = false;
            ATH_MSG_INFO( "Minimal hit time will be used as trigger time"
                         << " with random additional shift between 0 and " << -m_triggerTime << " ns");
          } else {
            m_useTriggerTime = false;
            ATH_MSG_INFO("Average time will be calculated in every event");
          }
        }
        break;
      case 1:
        ATH_MSG_INFO("Time of all hits will be reset to zero");
        break;
      default:
        ATH_MSG_INFO("Time of all hits will be preserved during copy");
        m_timeFlag = 0;
        break;
    }
  }

  if (m_run2plus) {
    m_fragHashFunc.initialize(m_tileHWID);

    m_E1merged.resize(m_tileHWID->drawer_hash_max());
    m_MBTSmerged.resize(m_tileHWID->drawer_hash_max());

    for (int ros = 3; ros < 5; ++ros) {
      for (int drawer = 0; drawer < 64; ++drawer) {
        int frag_id = m_tileHWID->frag(ros, drawer);
        IdentifierHash frag_hash = m_fragHashFunc(frag_id);
        m_E1merged[frag_hash] = (m_cabling->E1_merged_with_run2plus(ros, drawer) != 0);
        m_MBTSmerged[frag_hash] = (m_cabling->is_MBTS_merged_run2plus(drawer));
      }
    }
    ATH_MSG_INFO("Number of E1 cell to be merged: " << std::count (m_E1merged.begin(), m_E1merged.end(), true));
    ATH_MSG_INFO("Number of MBTS cell to be merged: " << std::count (m_MBTSmerged.begin(), m_MBTSmerged.end(), true));
  }

  if (m_onlyUseContainerName) {
    m_hitVectorNames = m_inputKeys.value();
  }
  else {
    ATH_CHECK(m_hitVectorKeys.assign(m_inputKeys.value()));
  }
  ATH_MSG_DEBUG("Input objects in these containers : '" << m_hitVectorNames << "'");

  // Initialize ReadHandleKey
  ATH_CHECK(m_hitVectorKeys.initialize(!m_onlyUseContainerName && !m_hitVectorKeys.empty() ));

  ATH_CHECK( m_hitContainerKey.initialize() );
  ATH_CHECK( m_hitContainer_DigiHSTruthKey.initialize(m_doDigiTruth) );

  ATH_MSG_DEBUG("TileHitVecToCntTool initialization completed");

  if (error)
    return StatusCode::RECOVERABLE;
  else
    return StatusCode::SUCCESS;
}

StatusCode TileHitVecToCntTool::createContainers() {

  ATH_MSG_VERBOSE("TileHitVecToCntTool createContainers started");

  if (m_pileUp) {
    m_hits = std::make_unique<TileHitNonConstContainer>(SG::VIEW_ELEMENTS);
    std::vector<TileHit *>::iterator iHit = m_allHits.begin();
    std::vector<TileHit *>::iterator lastHit = m_allHits.end();
    for (; iHit != lastHit; ++iHit) {
      TileHit *pHit = (*iHit);
      pHit->setZero();
    }

    if(m_doDigiTruth){
      m_hits_DigiHSTruth = std::make_unique<TileHitNonConstContainer>(SG::OWN_ELEMENTS);
      iHit = m_allHits_DigiHSTruth.begin();
      lastHit = m_allHits_DigiHSTruth.end();
      for (; iHit != lastHit; ++iHit) {
          TileHit *pHit = (*iHit);
          if(pHit == nullptr) continue;
          pHit->setZero();

      }
    }
  } else {
    m_hits = std::make_unique<TileHitNonConstContainer>(SG::OWN_ELEMENTS);
    if(m_doDigiTruth) m_hits_DigiHSTruth = std::make_unique<TileHitNonConstContainer>(SG::OWN_ELEMENTS);

  }

  ATH_MSG_VERBOSE("TileHitVecToCntTool createContainers finished");

  return StatusCode::SUCCESS;

}

StatusCode TileHitVecToCntTool::prepareEvent(const EventContext& ctx, unsigned int /*nInputEvents*/) {

  ATH_MSG_DEBUG("TileHitVecToCntTool prepareEvent initialization started");

  CHECK(this->createContainers());

  ATH_MSG_DEBUG("TileHitVecToCntTool prepareEvent finished");

  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );

  return StatusCode::SUCCESS;
}

void TileHitVecToCntTool::processHitVectorForOverlay(const TileHitVector* inputHits, int& nHit, double& eHitTot) {

  TileHitVecConstIterator inpItr = inputHits->begin();
  TileHitVecConstIterator end = inputHits->end();

  for (; inpItr != end; ++inpItr) {

    const TileHit * cinp = &(*inpItr);

    eHitTot += cinp->energy(); // not really correct if TileHit contains vector of energies
    // but eHitTot is needed for debug purposes only
    TileHit * pHit = new TileHit(*cinp);
    m_hits->push_back(pHit);
    ++nHit;

    if (msgLvl(MSG::VERBOSE)) {
      int hitsize = cinp->size();
      double eHit = 0.0;
      double tHit = 0.0;
      for (int i = 0; i < hitsize; ++i) {
        eHit += cinp->energy(i);
        tHit += cinp->time(i) * cinp->energy(i);
      }
      if (eHit > 0.0)
        tHit /= eHit;
      else
        tHit = cinp->time();

      eHitTot += eHit - cinp->energy(); // put total energy instead of first hit energy

      msg(MSG::VERBOSE) << " nHit=" << nHit
                        << " id=" << m_tileID->to_string(cinp->identify(), -1)
                        << " eHit=" << eHit
                        << " tHit=" << tHit
                        << " Copy hit: ener=";
      for (int i = 0; i < hitsize; ++i)
        msg(MSG::VERBOSE) << cinp->energy(i) << " ";
      msg(MSG::VERBOSE) << "time=";
      for (int i = 0; i < hitsize; ++i)
        msg(MSG::VERBOSE) << cinp->time(i) << " ";
      msg(MSG::VERBOSE) << endmsg;
    }
  }
  return;
}

void TileHitVecToCntTool::processHitVectorForPileUp(const TileHitVector* inputHits, double SubEvtTimOffset, int& nHit,
    double& eHitTot, bool isSignal) {

  IdContext pmt_context = m_tileID->pmt_context();
  IdContext tbchannel_context = m_tileTBID->channel_context();
  IdentifierHash hit_idhash;

  const bool inTimeEvent(fabs(SubEvtTimOffset) < m_deltaT);
  // Loop over hits in this HitVector
  TileHitVecConstIterator inpItr = inputHits->begin();
  TileHitVecConstIterator end = inputHits->end();

  for (; inpItr != end; ++inpItr) {

    const TileHit * cinp = &(*inpItr);
    Identifier hit_id = cinp->identify();

    if (m_tileTBID->is_tiletb(hit_id)) {
      int side = std::max(0, m_tileTBID->type(hit_id));
      int phi = m_tileTBID->module(hit_id);
      int eta = m_tileTBID->channel(hit_id);
      if (eta < 2)
        hit_idhash = mbts_index(side, phi, eta);
      else
        hit_idhash = e4pr_index(phi);
    } else {
      m_tileID->get_hash(hit_id, hit_idhash, &pmt_context);
    }

    if (hit_idhash >= m_allHits.size()) {
      // Seems to be E4pr or MBTS hit in minimum bias while geometry is used without them => skipping
      continue;
    }

    double ener = cinp->energy();
    double time = cinp->time() + SubEvtTimOffset;

    ++nHit;
    eHitTot += ener;

    TileHit * pHit = m_allHits[hit_idhash];
    TileHit * pHit_DigiHSTruth(nullptr);
    if(m_doDigiTruth) pHit_DigiHSTruth = m_allHits_DigiHSTruth[hit_idhash];

    if (0 == pHit) {

      // keep this "if" just in case there is a bug somewhere ... ( will be removed soon)
      ATH_MSG_ERROR(" new hit AND MEMORY LEAK HERE!!!");

      if (inTimeEvent) {
        pHit = new TileHit(hit_id, ener, time, m_deltaT);
        if(m_doDigiTruth && isSignal) pHit_DigiHSTruth = new TileHit(hit_id, ener, time, m_deltaT);
        else if(m_doDigiTruth)  pHit_DigiHSTruth = new TileHit(hit_id, 0.0, 0.0);
      } else {
        pHit = new TileHit(hit_id, 0.0, 0.0); // create in-time hit with zero energy
        pHit->add(ener, time, m_deltaT);
        if(m_doDigiTruth){ 
          pHit_DigiHSTruth = new TileHit(hit_id, 0.0, 0.0); // create in-time hit with zero energy
          pHit_DigiHSTruth->add(ener, time, m_deltaT);
        }
      }
      m_allHits[hit_idhash] = pHit;
      if(m_doDigiTruth) m_allHits_DigiHSTruth[hit_idhash] = pHit_DigiHSTruth;

      if (msgLvl(MSG::VERBOSE)) {
        HWIdentifier channel_id = pHit->pmt_HWID();
        msg(MSG::VERBOSE) << " nH=" << nHit
                          << " id=" << m_tileID->to_string(hit_id, -1)
                          << " HWid=" << m_tileID->to_string(channel_id)
                          << " e=" << ener
                          << " time=" << time
                          << " offs=" << SubEvtTimOffset
                          << " new hit" << endmsg;
      }

    } else {

      if (time < m_maxHitTime){
        pHit->add(ener, time, m_deltaT);
        if(m_doDigiTruth){
          if(isSignal) {
            pHit_DigiHSTruth->add(ener, time, m_deltaT);
          } else {
            pHit_DigiHSTruth->add(0,time, m_deltaT);
          }
        }
      }

      if (msgLvl(MSG::VERBOSE)) {
        if (pHit->size() > 1 || pHit->energy() != 0.0)
          msg(MSG::VERBOSE) << " nHit=" << nHit
                            << " id=" << m_tileID->to_string(hit_id, -1)
                            << " ener=" << ener
                            << " time=" << time
                            << " offs=" << SubEvtTimOffset
                            << " double hit" << endmsg;
        else
          msg(MSG::VERBOSE) << " nH=" << nHit
                            << " id=" << m_tileID->to_string(hit_id, -1)
                            << " HWid=" << m_tileID->to_string(pHit->pmt_HWID())
                            << " e=" << ener
                            << " time=" << time
                            << " offs=" << SubEvtTimOffset
                            << " new hit" << endmsg;
      }
    }

    int hitsize = cinp->size();
    for (int ind = 1; ind < hitsize; ++ind) { // if we have double hits in original hit
      ener = cinp->energy(ind);           // merge all of them with appropriate time
      time = cinp->time(ind) + SubEvtTimOffset;

      ++nHit;
      eHitTot += ener;

      if (time < m_maxHitTime){
        pHit->add(ener, time, m_deltaT);
        if(m_doDigiTruth){
          if(isSignal)
            pHit_DigiHSTruth->add(ener, time, m_deltaT);
          else
            pHit_DigiHSTruth->add(0, time, m_deltaT);
        }
      }

      if (msgLvl(MSG::VERBOSE))
        msg(MSG::VERBOSE) << " nHit=" << nHit
                          << " id=" << m_tileID->to_string(hit_id, -1)
                          << " ener=" << ener
                          << " time=" << time
                          << " double hit from single hit" << endmsg;
    }
  }         // loop over hits in one vector
  return;
}

void TileHitVecToCntTool::processHitVectorWithoutPileUp(const TileHitVector* inputHits, int& nHit, double& eHitTot, TileHitNonConstContainer* hitCont, CLHEP::HepRandomEngine * engine) {

  TileHitVecConstIterator inpItr = inputHits->begin();
  TileHitVecConstIterator end = inputHits->end();

  //**
  //* Iterate over hits, creating new TileHits
  //* Add each TileHit to the TileHitContainer.
  //**

  switch (m_timeFlag) {

    case 0: {
      for (; inpItr != end; ++inpItr) {
        const TileHit * cinp = &(*inpItr);
        eHitTot += cinp->energy(); // not really correct if TileHit contains vector of energies
        // but eHitTot is needed for debug purposes only
        TileHit * pHit = new TileHit(*cinp);
        hitCont->push_back(pHit);
        ++nHit;

        if (msgLvl(MSG::VERBOSE)) {
          int hitsize = cinp->size();
          double eHit = 0.0;
          double tHit = 0.0;
          for (int i = 0; i < hitsize; ++i) {
            eHit += cinp->energy(i);
            tHit += cinp->time(i) * cinp->energy(i);
          }
          if (eHit > 0.0)
            tHit /= eHit;
          else
            tHit = cinp->time();

          eHitTot += eHit - cinp->energy(); // put total energy instead of first hit energy

          msg(MSG::VERBOSE) << " nHit=" << nHit
                            << " id=" << m_tileID->to_string(cinp->identify(), -1)
                            << " eHit=" << eHit
                            << " tHit=" << tHit
                            << " Copy hit: ener=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << cinp->energy(i) << " ";
          msg(MSG::VERBOSE) << "time=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << cinp->time(i) << " ";
          msg(MSG::VERBOSE) << endmsg;
        }
      }
      break;
    }

    case 1: {

      for (; inpItr != end; ++inpItr) {
        const TileHit * cinp = &(*inpItr);
        int size = cinp->size();
        double eHit = 0.0;
        for (int i = 0; i < size; ++i) {
          eHit += cinp->energy(i);
        }
        eHitTot += eHit;

        // create hit with total energy at time=0 instead of original one
        Identifier pmID = cinp->pmt_ID();
        TileHit * pHit = new TileHit(pmID, eHit, 0.);

        hitCont->push_back(pHit);
        ++nHit;

        if (msgLvl(MSG::VERBOSE)) {
          int hitsize = cinp->size();
          msg(MSG::VERBOSE) << " nHit=" << nHit
                            << " id=" << m_tileID->to_string(cinp->identify(), -1)
                            << " eHit=" << eHit
                            << " tHit=0.0"
                            << " Input hit: ener=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << cinp->energy(i) << " ";
          msg(MSG::VERBOSE) << "time=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << cinp->time(i) << " ";
          msg(MSG::VERBOSE) << endmsg;
        }
      }
      break;

    }

    case 2: {

      double avtime = 0.0;

      if (m_useTriggerTime) {

        if (m_triggerTimeTool.empty()) {
          avtime = m_triggerTime;
        } else {
          avtime = m_triggerTimeTool->time();
        }
        ATH_MSG_DEBUG("Trigger time used : " << avtime);

      } else {

        if (m_triggerTime < 0.0) {

          avtime = 1.0e20;

          // loop to find minimal time
          for (; inpItr != end; ++inpItr) {
            const TileHit * cinp = &(*inpItr);
            int size = cinp->size();
            for (int i = 0; i < size; ++i) {
              if (cinp->time(i) < avtime) avtime = cinp->time(i);
            }
          }
          ATH_MSG_DEBUG("Minimal time in input event " << avtime);
          double shift = RandFlat::shoot(engine, m_triggerTime, 0.0);
          ATH_MSG_DEBUG("Minimal time after random shift " << shift);
          avtime -= shift; // subtracting negative shift value here

        } else {

          double weight = 0.0;

          // loop to calculate average time
          for (; inpItr != end; ++inpItr) {
            const TileHit * cinp = &(*inpItr);
            int size = cinp->size();
            for (int i = 0; i < size; ++i) {
              avtime += cinp->time(i) * cinp->energy(i);
              weight += cinp->energy(i);
            }
          }
          if (weight > 0.0)
            avtime /= weight;
          else
            avtime = 0.0;

          ATH_MSG_DEBUG("Average time used : " << avtime);
        }

        // reset iterator to the first hit
        inpItr = inputHits->begin();
      }

      for (; inpItr != end; ++inpItr) {
        const TileHit * cinp = &(*inpItr);
        TileHit * pHit = new TileHit(*cinp);
        // subract average time from all time bins in the hit
        int size = pHit->size();
        for (int i = 0; i < size; ++i) {
          pHit->setTime(pHit->time(i) - avtime, i);
          eHitTot += cinp->energy(i);
        }

        hitCont->push_back(pHit);
        ++nHit;

        if (msgLvl(MSG::VERBOSE)) {
          int hitsize = pHit->size();
          double eHit = 0.0;
          double tHit = 0.0;
          for (int i = 0; i < hitsize; ++i) {
            eHit += pHit->energy(i);
            tHit += pHit->time(i) * cinp->energy(i);
          }
          if (eHit > 0.0)
            tHit /= eHit;
          else
            tHit = cinp->time(); // just first time

          msg(MSG::VERBOSE) << " nHit="  << nHit
                            << " id=" << m_tileID->to_string(pHit->identify(), -1)
                            << " eHit=" << eHit
                            << " tHit=" << tHit
                            << " Output hit: ener=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << pHit->energy(i) << " ";
          msg(MSG::VERBOSE) << "time=";
          for (int i = 0; i < hitsize; ++i)
            msg(MSG::VERBOSE) << pHit->time(i) << " ";
          msg(MSG::VERBOSE) << endmsg;
        }
      }
      break;
    }

    default:
      ATH_MSG_ERROR("unexpected value m_timeFlag=" << m_timeFlag);
      break;
  }

  return;
}

StatusCode TileHitVecToCntTool::processBunchXing(int bunchXing
                                                 , SubEventIterator bSubEvents
                                                 , SubEventIterator eSubEvents)
{

  ATH_MSG_DEBUG("Inside TileHitVecToCntTool processBunchXing" << bunchXing);
  //  setFilterPassed(true);

  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomStreamName);
  CLHEP::HepRandomEngine * engine = rngWrapper->getEngine(Gaudi::Hive::currentContext());

  SubEventIterator iEvt(bSubEvents);
  if (m_rndmEvtOverlay && bunchXing != 0) iEvt = eSubEvents; // in overlay skip all events except BC=0

  while (iEvt != eSubEvents) {
    /* zero all counters and sums */
    int nHit(0);
    double eHitTot(0.0);

    std::vector<std::string>::const_iterator hitVecNamesItr = m_hitVectorNames.begin();
    std::vector<std::string>::const_iterator hitVecNamesEnd = m_hitVectorNames.end();
    for (; hitVecNamesItr != hitVecNamesEnd; ++hitVecNamesItr) {

      const std::string hitVectorName(*hitVecNamesItr);

      if (m_pileUp || m_rndmEvtOverlay) {

        const TileHitVector* inputHits;
	if (!(m_mergeSvc->retrieveSingleSubEvtData(hitVectorName, inputHits, bunchXing, iEvt))){
	  ATH_MSG_ERROR(" Tile Hit container not found for event key " << hitVectorName);
	}

        const double SubEvtTimOffset(iEvt->time());

        if (m_rndmEvtOverlay) { // overlay code
          if (fabs(SubEvtTimOffset) > 0.1) {
            ATH_MSG_ERROR("Wrong time for in-time event: " << SubEvtTimOffset << " Ignoring all hits ");
          } else {
            ATH_MSG_DEBUG(" New HitCont.  TimeOffset=" << SubEvtTimOffset << ", size =" << inputHits->size());
            this->processHitVectorForOverlay(inputHits, nHit, eHitTot);
            //if( m_doDigiTruth && iEvt == bSubEvents) this->processHitVectorWithoutPileUp(inputHits, nHit, eHitTot, m_signalHits, engine);
          }
        } else if (m_pileUp) { // pileup code
          bool isSignal = false;
          if(iEvt == bSubEvents) isSignal = true;
          this->processHitVectorForPileUp(inputHits, SubEvtTimOffset, nHit, eHitTot, isSignal);
        }
      } else {  // no PileUp
        //**
        //* Get TileHits from TileHitVector
        //**
        const TileHitVector * inputHits;
	if (!(m_mergeSvc->retrieveSingleSubEvtData(hitVectorName, inputHits, bunchXing, iEvt))){
	  ATH_MSG_ERROR(" Tile Hit container not found for event key " << hitVectorName);
	}

	this->processHitVectorWithoutPileUp(inputHits, nHit, eHitTot, m_hits.get(), engine);
        if(m_doDigiTruth) this->processHitVectorWithoutPileUp(inputHits, nHit, eHitTot, m_hits_DigiHSTruth.get(), engine);
      } // to pile-up or not

    } // end of the loop over different input hitVectorNames (normal hits and MBTS hits)

    ++iEvt;
    if (m_rndmEvtOverlay) iEvt = eSubEvents; // in overlay skip all events except fisrt one
  } // subEvent loop

  ATH_MSG_DEBUG("Exiting processBunchXing in TileHitVecToCntTool");

  return StatusCode::SUCCESS;
}

StatusCode TileHitVecToCntTool::processAllSubEvents(const EventContext& ctx) {

  ATH_MSG_DEBUG("TileHitVecToCntTool processAllSubEvents started");
  typedef PileUpMergeSvc::TimedList<TileHitVector>::type TimedHitContList;

  ATH_CHECK(this->createContainers());

  /* zero all counters and sums */
  int nHit(0);
  double eHitTot(0.0);

  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( name(), ctx );
  CLHEP::HepRandomEngine * engine = rngWrapper->getEngine(ctx);

  if(!m_onlyUseContainerName) {
    auto hitVectorHandles = m_hitVectorKeys.makeHandles(ctx);
    for (auto & inputHits : hitVectorHandles) {
      if (!inputHits.isValid()) {
        ATH_MSG_ERROR("BAD HANDLE"); //FIXME improve error here
        return StatusCode::FAILURE;
      }
      const double SubEvtTimeOffset(0.0);
      // get HitVector for this subevent
      ATH_MSG_DEBUG(" New HitCont.  TimeOffset=" << SubEvtTimeOffset << ", size =" << inputHits->size());
      this->processHitVectorForOverlay(inputHits.cptr(), nHit, eHitTot);
      if(m_doDigiTruth) this->processHitVectorWithoutPileUp(inputHits.cptr(), nHit, eHitTot, m_hits_DigiHSTruth.get(), engine);
    }
    ATH_CHECK(this->mergeEvent(ctx));
    return StatusCode::SUCCESS;
  }

  for (const auto& hitVectorName : m_hitVectorNames) {

    if (m_pileUp || m_rndmEvtOverlay) {
      TimedHitContList hitContList;
      // retrive list of pairs (time,container) from PileUp service
      if (!(m_mergeSvc->retrieveSubEvtsData(hitVectorName, hitContList).isSuccess()) || hitContList.size() == 0) {
        ATH_MSG_WARNING("Could not fill TimedHitContList for hit vector " << hitVectorName);
        continue; // continue to the next hit vector
      }

      // loop over this list
      TimedHitContList::iterator iCont(hitContList.begin());
      TimedHitContList::iterator iEndCont(hitContList.end());

      if (m_rndmEvtOverlay) {  // overlay code
        if (iCont != iEndCont) { // use only hits from first container
          // get time for this subevent
          const double SubEvtTimeOffset(iCont->first.time());
          if (fabs(SubEvtTimeOffset) > 0.1) {
            ATH_MSG_ERROR("Wrong time for in-time event: " << SubEvtTimeOffset << " Ignoring all hits ");
          } else {
            // get HitVector for this subevent
            const TileHitVector* inputHits = &(*(iCont->second));
            ATH_MSG_DEBUG(" New HitCont.  TimeOffset=" << SubEvtTimeOffset << ", size =" << inputHits->size());
            this->processHitVectorForOverlay(inputHits, nHit, eHitTot);
            if(m_doDigiTruth) this->processHitVectorWithoutPileUp(inputHits, nHit, eHitTot, m_hits_DigiHSTruth.get(), engine);
          }
        }
      } else if (m_pileUp) {  // pileup code

        for (; iCont != iEndCont; ++iCont) {
          // get time for this subevent
          const double SubEvtTimeOffset(iCont->first.time());
          // get HitVector for this subevent
          const TileHitVector* inputHits = &(*(iCont->second));
          ATH_MSG_VERBOSE(" New HitCont.  TimeOffset=" << SubEvtTimeOffset << ", size =" << inputHits->size());
          bool isSignal = false;
          if(iCont == hitContList.begin() ) isSignal = true;
          this->processHitVectorForPileUp(inputHits, SubEvtTimeOffset, nHit, eHitTot, isSignal);
        }
      }           // loop over subevent list
    } else {  // no PileUp

      //**
      //* Get TileHits from TileHitVector
      //**
      SG::ReadHandle<TileHitVector> inputHits(hitVectorName);
      if (!inputHits.isValid()) {
        ATH_MSG_WARNING("Hit Vector "<< hitVectorName << " not found in StoreGate");
        continue; // continue to the next hit vector
      }
      this->processHitVectorWithoutPileUp(inputHits.cptr(), nHit, eHitTot, m_hits.get(), engine);
      if(m_doDigiTruth) this->processHitVectorWithoutPileUp(inputHits.cptr(), nHit, eHitTot, m_hits_DigiHSTruth.get(), engine);
    }

  } // end of the loop over different input hitVectorNames (normal hits and MBTS hits)

  ATH_CHECK(this->mergeEvent(ctx));

  return StatusCode::SUCCESS;
}

StatusCode TileHitVecToCntTool::mergeEvent(const EventContext& ctx) {

  ATH_MSG_DEBUG("Entering mergeEvent in TileHitVecToCntTool");

  if (m_pileUp) {

    std::vector<TileHit *>::iterator iHit = m_allHits.begin();
    std::vector<TileHit *>::iterator lastHit = m_allHits.end();
    std::vector<TileHit *>::iterator iHit_DigiHSTruth = m_allHits_DigiHSTruth.begin();

    int nHitUni = 0;
    double eHitInTime = 0.0;

    ATH_MSG_DEBUG("Hits being stored in container");

    for (; iHit != lastHit; ++iHit) {
      TileHit *pHit = (*iHit);
      TileHit *pHit_DigiHSTruth(nullptr);
      if(m_doDigiTruth) pHit_DigiHSTruth = new TileHit(**iHit_DigiHSTruth);
      if (pHit->size() > 1 || pHit->energy() != 0.0) {       // hit exists
        m_hits->push_back(pHit);   // store hit in container
        if(m_doDigiTruth){
          m_hits_DigiHSTruth->push_back(pHit_DigiHSTruth);   // store hit in container
        }
        ++nHitUni;
        eHitInTime += pHit->energy();
      }
      if(m_doDigiTruth) ++iHit_DigiHSTruth;
    }


    ATH_MSG_DEBUG(" nHitUni=" << nHitUni << " eHitInTime="<< eHitInTime);
  } else {
    if (m_mergeMultipleHitsInChannel) {
      findAndMergeMultipleHitsInChannel(m_hits);
      if (m_doDigiTruth) {
        findAndMergeMultipleHitsInChannel(m_hits_DigiHSTruth);
      }
    }
  }

  if (m_run2plus) {
    // Merge MBTS and E1 where it is needed.

    for (std::unique_ptr<TileHitCollection>& coll : *m_hits ) {
      int frag_id = coll->identify();
      IdentifierHash frag_hash = m_fragHashFunc(frag_id);
      if (m_E1merged[frag_hash])
        findAndMergeE1(coll.get(), frag_id, m_hits.get());
      else if (m_MBTSmerged[frag_hash]) findAndMergeMBTS(coll.get(), frag_id, m_hits.get());
    }
    if(m_doDigiTruth){
      TileHitNonConstContainer::iterator collIt = m_hits_DigiHSTruth->begin();
      TileHitNonConstContainer::iterator endcollIt = m_hits_DigiHSTruth->end();

      for (; collIt != endcollIt; ++collIt) {
        int frag_id = (*collIt)->identify();
        IdentifierHash frag_hash = m_fragHashFunc(frag_id);
        if (m_E1merged[frag_hash]) findAndMergeE1((*collIt).get(), frag_id, m_hits_DigiHSTruth.get());
        else if (m_MBTSmerged[frag_hash]) findAndMergeMBTS((*collIt).get(), frag_id, m_hits_DigiHSTruth.get());
      }
    }
  }

  //photoelectron statistics.
  //loop over all hits in TileHitContainer and take energy deposited in certain period of time
  //std::vector<std::string>::const_iterator hitVecNamesEnd = m_hitVectorNames.end();

  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomStreamName);
  CLHEP::HepRandomEngine * engine = rngWrapper->getEngine(ctx);

  SG::ReadCondHandle<TileSamplingFraction> samplingFraction(m_samplingFractionKey, ctx);
  ATH_CHECK( samplingFraction.isValid() );

  TileHitNonConstContainer::iterator collIt_DigiHSTruth; 
  TileHitNonConstContainer::iterator endColl_DigiHSTruth;
  if(m_doDigiTruth) {
    collIt_DigiHSTruth = m_hits_DigiHSTruth->begin();
    endColl_DigiHSTruth = m_hits_DigiHSTruth->end();
  }

  for (std::unique_ptr<TileHitCollection>& coll : *m_hits ) {
    TileHitCollection* coll_DigiHSTruth;
    TileHitCollection::iterator hitItr_DigiHSTruth;
    TileHitCollection::iterator hitEnd_DigiHSTruth;
    if(m_doDigiTruth) {
      coll_DigiHSTruth = (*collIt_DigiHSTruth).get();
      hitItr_DigiHSTruth = coll_DigiHSTruth->begin();
      hitEnd_DigiHSTruth = coll_DigiHSTruth->end();
    }

    HWIdentifier drawer_id = m_tileHWID->drawer_id(coll->identify());
    int ros = m_tileHWID->ros(drawer_id);
    int drawer = m_tileHWID->drawer(drawer_id);
    int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);

    for (TileHit* pHit : *coll) {
      double ehit = 0.0;
      int hitsize = pHit->size();
      for (int i = 0; i < hitsize; ++i) {
        double thit = pHit->time(i);
        if (fabs(thit) < m_photoStatisticsWindow) ehit += pHit->energy(i);
      }

      Identifier pmt_id = pHit->pmt_ID();
      //HWIdentifier channel_id = (*hitItr)->pmt_HWID();
      // for gap/crack scintillators
      if (m_tileID->sample(pmt_id) == 3) {
        pmt_id = m_tileID->pmt_id(m_tileID->cell_id(pmt_id), 0);
        //channel_id = m_cabling->s2h_channel_id(pmt_id);
      }

      double scaleFactor = 1.0;
      if (m_usePhotoStatistics) {
        scaleFactor = applyPhotoStatistics(ehit, pmt_id, engine, *samplingFraction, drawerIdx);
        pHit->scale(scaleFactor);
      }

      if(m_doDigiTruth){
        TileHit *pHit_DigiHSTruth = (*hitItr_DigiHSTruth);
        pHit_DigiHSTruth->scale(scaleFactor);

        ++hitItr_DigiHSTruth;
      }
    }

	  if(m_doDigiTruth) ++collIt_DigiHSTruth;
  }



  /* Register the set of TileHits to the event store. */
  auto hits = std::make_unique<TileHitContainer>
                 (false, m_pileUp ? SG::VIEW_ELEMENTS : SG::OWN_ELEMENTS);
  size_t hashId = 0;
  for (std::unique_ptr<TileHitCollection>& coll : *m_hits ) {
    CHECK(hits->addCollection (coll.release(), hashId++));
  }

  SG::WriteHandle<TileHitContainer> hitContainer(m_hitContainerKey, ctx);
  ATH_CHECK( hitContainer.record(std::move(hits)) );

  ATH_MSG_DEBUG("TileHit container registered to the TES with name" << m_hitContainerKey.key());

  //  if (m_skipNoHit && nHit==0) {
  //    setFilterPassed(false);
  //    ATH_MSG_DEBUG ( " No hits, skip this event "  );
  //  }

	if(m_doDigiTruth){
    auto hits_DigiHSTruth = std::make_unique<TileHitContainer>
                 (false, m_pileUp ? SG::VIEW_ELEMENTS : SG::OWN_ELEMENTS);
    size_t hashId_DigiHSTruth = 0;
    for (std::unique_ptr<TileHitCollection>& coll : *m_hits_DigiHSTruth ) {
      ATH_CHECK(hits_DigiHSTruth->addCollection (coll.release(), hashId_DigiHSTruth++));
    }

    SG::WriteHandle<TileHitContainer> hitContainer_DigiHSTruth(m_hitContainer_DigiHSTruthKey, ctx);
    ATH_CHECK( hitContainer_DigiHSTruth.record(std::move(hits_DigiHSTruth)) );
  }

  ATH_MSG_DEBUG("Exiting mergeEvent in TileHitVecToCntTool");
  return StatusCode::SUCCESS;
}

StatusCode TileHitVecToCntTool::finalize() {

  ATH_MSG_DEBUG("Finalizing TileHitVecToCntTool");

  if (m_pileUp) {
    std::vector<TileHit *>::iterator iHit = m_allHits.begin();
    std::vector<TileHit *>::iterator lastHit = m_allHits.end();
    for (; iHit != lastHit; ++iHit) {
      delete (*iHit);
    }
    if(m_doDigiTruth){
      iHit = m_allHits_DigiHSTruth.begin();
      lastHit = m_allHits_DigiHSTruth.end();
      for (; iHit != lastHit; ++iHit) {
        delete (*iHit);
      }
    }
  }

  ATH_MSG_DEBUG("TileHitVecToCntTool finalized");

  return StatusCode::SUCCESS;

}

double TileHitVecToCntTool::applyPhotoStatistics(double energy, Identifier pmt_id, CLHEP::HepRandomEngine* engine,
                                                 const TileSamplingFraction* samplingFraction, int drawerIdx) {

  int channel = m_tileHWID->channel(m_cabling->s2h_channel_id(pmt_id));
  // take number of photoelectrons per GeV divide by 1000 to go to MeV
  // and multiply by inverted sampling fraction (about 36, depends on G4 version, sampling and eta)
  // to get number of photoelectrons per 1 MeV energy in scintillator
  float nPhotoElectrons = samplingFraction->getNumberOfPhotoElectrons(drawerIdx, channel)
    / (Gaudi::Units::GeV / Gaudi::Units::MeV) * samplingFraction->getSamplingFraction(drawerIdx, channel);

  nPhotoElectrons = std::round(nPhotoElectrons * 1000) / 1000;

  double pe = energy * nPhotoElectrons;
  double pe_scale = 1., RndmPois = 1.;

  switch (m_photoElectronStatistics) {
    case 2:
      if (pe > 20.0) {
        RndmPois = std::max(0.0, RandGaussQ::shoot(engine, pe, sqrt(pe))); // FIXME CLHEP::RandGaussZiggurat is faster and more accurate.
        pe_scale = RndmPois / pe;
      } else { // pe<=20

        if (pe > 0.) {
          double singleMEAN = 1.0;  //Parameterization of monoelectron spectra
          double singleSIGMA = 1.0;
          RndmPois = RandPoissonT::shoot(engine, pe);

          if (RndmPois > 0) {
            pe_scale = 0;
            for (int i = 0; i < RndmPois; i++)
              pe_scale += 1 / (1.08332) * std::max(0., RandGaussQ::shoot(engine, singleMEAN, singleSIGMA)); // FIXME CLHEP::RandGaussZiggurat is faster and more accurate.

            pe_scale /= RndmPois;
          } else
            pe_scale = 0;  //RndmPois==0
        }
      }
      break;

    case 0:
      if (pe > 0.0) {
        RndmPois = RandPoissonT::shoot(engine, pe);
        pe_scale = RndmPois / pe;
      }
      break;

    case 1:
      if (pe > 0.0) {
        if (pe > 10.0) {
          RndmPois = std::max(0.0, RandGaussQ::shoot(engine, pe, sqrt(pe))); // FIXME CLHEP::RandGaussZiggurat is faster and more accurate.
        } else {
          int nn = std::max(10, (int) (pe * 10.0));
          double * ProbFunc = new double[nn];
          ProbFunc[0] = exp(-pe);
          for (int i = 1; i < nn; ++i) {
            ProbFunc[i] = ProbFunc[i - 1] * pe / i;
          }
          RandGeneral* RandG = new RandGeneral(ProbFunc, nn, 0);
          RndmPois = RandG->shoot(engine) * nn;
          //here RndmPois is continuously distributed random value obtained from Poisson
          //distribution by approximation.
          delete RandG;
          delete[] ProbFunc;
        }
        pe_scale = RndmPois / pe;
      }
      break;
  } //end switch(m_PhElStat)

  ATH_MSG_VERBOSE( "PhotoElec: id=" << m_tileID->to_string(pmt_id,-1)
                  << " totEne=" << energy
                  << ", numPhElec=" << nPhotoElectrons
                  << ", pe=" << pe
                  << ", rndmPoisson=" << RndmPois
                  << ", pe_scale=" << pe_scale);

  return pe_scale;
}


void TileHitVecToCntTool::findAndMergeE1(TileHitCollection* coll, int frag_id, TileHitNonConstContainer* hitCont) {
  int module = frag_id & 0x3F;

  TileHitCollection::iterator hitIt = coll->begin();
  TileHitCollection::iterator endHitIt = coll->end();

  TileHitCollection::iterator fromHitIt = coll->end();
  TileHit* toHit(0);

  for (; hitIt != endHitIt; ++hitIt) {
    Identifier pmt_id = (*hitIt)->pmt_ID();
    if (m_tileID->tower(pmt_id) == E1_TOWER && m_tileID->sample(pmt_id) == TileID::SAMP_E) {
      if (module == m_tileID->module(pmt_id)) {
        toHit = *hitIt;
      } else {
        fromHitIt = hitIt; // need iterator to delete this hit later.
      }
    }
  }
  
  if (fromHitIt != coll->end()) {
    ATH_MSG_VERBOSE("Found TileHit (E1 cell) for merging [" << m_tileID->to_string((*fromHitIt)->pmt_ID(), -1) 
		    << "] in module: " << module);
    if (toHit == 0) {
      int side = m_tileID->side((*fromHitIt)->pmt_ID());
      Identifier to_pmt_id = m_tileID->pmt_id(TileID::GAPDET, side, module, E1_TOWER, TileID::SAMP_E, 0);
      toHit = new TileHit(to_pmt_id);
      hitCont->push_back(toHit);
      ATH_MSG_VERBOSE("New TileHit (E1 cell) for merging added Id: " << m_tileID->to_string(toHit->pmt_ID(), -1) );
    } else {
      ATH_MSG_VERBOSE("Found TileHit (E1 cell) for merging Id: " << m_tileID->to_string(toHit->pmt_ID(), -1) );
    }

    ATH_MSG_DEBUG( "TileHit (E1 cell) Id: " << m_tileID->to_string((*fromHitIt)->pmt_ID(), -1) 
		   << " will be merged to " << m_tileID->to_string(toHit->pmt_ID(), -1) );
    
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "Before merging (E1 cell) => " << (std::string) (**fromHitIt) << endmsg;
      msg(MSG::VERBOSE) << "Before merging (E1 cell) => " << (std::string) (*toHit) << endmsg;
    }

    toHit->add(*fromHitIt, 0.1);

    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "After merging (E1 cell) => " << (std::string) (*toHit) << endmsg;
      msg(MSG::VERBOSE) << "TileHit to be deleted Id (E1 cell): " << m_tileID->to_string((*fromHitIt)->pmt_ID(), -1) << endmsg;
    }
    
    coll->erase(fromHitIt);
  }
}


void TileHitVecToCntTool::findAndMergeMBTS(TileHitCollection* coll, int frag_id, TileHitNonConstContainer* hitCont) {
  int module = frag_id & 0x3F;

  TileHitCollection::iterator hitIt = coll->begin();
  TileHitCollection::iterator endHitIt = coll->end();

  TileHitCollection::iterator fromHitIt = coll->end();
  TileHit* toHit(0);

  for (; hitIt != endHitIt; ++hitIt) {
    Identifier pmt_id = (*hitIt)->pmt_ID();
    if (m_tileTBID->is_tiletb(pmt_id)) {
      if (m_tileTBID->phi(pmt_id) % 2 == 0) {
        toHit = *hitIt;
      } else {
        fromHitIt = hitIt; // need iterator to delete this hit later.
      }
    }
  }
  
  if (fromHitIt != coll->end()) {
    ATH_MSG_VERBOSE("Found TileHit (MBTS) for merging [" << m_tileTBID->to_string((*fromHitIt)->pmt_ID(), 0) 
		    << "] in module: " << module);
    if (toHit == 0) {
      int side = m_tileTBID->side((*fromHitIt)->pmt_ID());
      int phi = m_tileTBID->phi((*fromHitIt)->pmt_ID()) - 1;
      Identifier to_pmt_id = m_tileTBID->channel_id(side, phi, 1);
      toHit = new TileHit(to_pmt_id);
      hitCont->push_back(toHit);
      ATH_MSG_VERBOSE("New TileHit (MBTS) for merging added Id: " << m_tileTBID->to_string(toHit->pmt_ID(), 0) );
    } else {
      ATH_MSG_VERBOSE("Found TileHit (MBTS) for merging Id: " << m_tileTBID->to_string(toHit->pmt_ID(), 0) );
    }
    
    ATH_MSG_DEBUG( "TileHit (MBTS) Id: " << m_tileTBID->to_string((*fromHitIt)->pmt_ID(), 0) 
		   << " will be merged to " << m_tileTBID->to_string(toHit->pmt_ID(), 0) );

    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "Before merging (MBTS) => " << (std::string) (**fromHitIt) << endmsg;
      msg(MSG::VERBOSE) << "Before merging (MBTS) => " <<  (std::string) (*toHit) << endmsg;
    }

    toHit->add(*fromHitIt, 0.1);

    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "After merging (MBTS) => " << (std::string) (*toHit) << endmsg;
      msg(MSG::VERBOSE) << "TileHit to be deleted Id (MBTS): " 
			<< m_tileTBID->to_string((*fromHitIt)->pmt_ID(), 0) << endmsg;
    }
    
    coll->erase(fromHitIt);
  }

}

void TileHitVecToCntTool::findAndMergeMultipleHitsInChannel(std::unique_ptr<TileHitNonConstContainer>& hitCont) {
  for (std::unique_ptr<TileHitCollection>& coll : *hitCont) {
    int module = coll->identify() & 0x3F;
    std::vector<TileHit*> hits(48, nullptr);
    std::vector<std::unique_ptr<TileHit>> otherModuleHits;
    coll->erase(std::remove_if(coll->begin(), coll->end(),
                               [this, &hits, &otherModuleHits, module] (TileHit* hit) {
                                 Identifier pmt_id = hit->pmt_ID();
                                 int channel = m_tileHWID->channel(hit->pmt_HWID());
                                 TileHit* channelHit = hits[channel];
                                 if (channelHit) {
                                   mergeExtraHitToChannelHit(hit, channelHit);
                                   return true;
                                 } else if ((m_tileTBID->is_tiletb(pmt_id) && (m_tileTBID->phi(pmt_id) % 2 == 1))
                                            || m_tileID->module(pmt_id) != module) {
                                   otherModuleHits.push_back(std::make_unique<TileHit>(*hit));
                                   return true;
                                 } else {
                                   hits[channel] = hit;
                                   return false;
                                 }}),
                coll->end());

    for (std::unique_ptr<TileHit>& hit : otherModuleHits) {
      int channel = m_tileHWID->channel(hit->pmt_HWID());
      TileHit* channelHit = hits[channel];
      if (channelHit) {
        mergeExtraHitToChannelHit(hit.get(), channelHit);
      } else {
        hits[channel] = hit.get();
        coll->push_back(std::move(hit));
      }
    }
  }
}

void TileHitVecToCntTool::mergeExtraHitToChannelHit(TileHit* extraHit, TileHit* channelHit) {

  ATH_MSG_DEBUG("Found extra hit for channel Id: "
                << m_tileID->to_string(extraHit->pmt_ID(), -1) << ", will be merged to "
                << m_tileID->to_string(channelHit->pmt_ID(), -1));
  ATH_MSG_VERBOSE("Before merging => " << (std::string) (*extraHit));
  ATH_MSG_VERBOSE("Before merging => " << (std::string) (*channelHit));

  channelHit->add(extraHit, 0.1);

  ATH_MSG_VERBOSE("After merging => " << (std::string) (*channelHit));
}
