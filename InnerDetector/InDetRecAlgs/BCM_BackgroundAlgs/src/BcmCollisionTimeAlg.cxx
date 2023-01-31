/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "BcmCollisionTimeAlg.h"

#include "AthenaKernel/errorcheck.h"
#include "BCM_CollisionTime/BcmCollisionTime.h"
#include "Identifier/Identifier.h"

// Constructor
BcmCollisionTimeAlg::BcmCollisionTimeAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

//__________________________________________________________________________
// Destructor
BcmCollisionTimeAlg::~BcmCollisionTimeAlg() = default;

//__________________________________________________________________________
StatusCode BcmCollisionTimeAlg::initialize() {
  ATH_CHECK(m_bcmContainerName.initialize());
  ATH_CHECK(m_bcmCollisionTimeName.initialize());
  return StatusCode::SUCCESS;
}

StatusCode BcmCollisionTimeAlg::execute(const EventContext& ctx) const{

  ATH_MSG_DEBUG("BcmCollisionTimeAlg execute()");

  // declare variables here
  std::vector<float> deltaT;
  unsigned int multiLG = 0;
  unsigned int multiHG = 0;
  std::vector<deltat_data> deltaTdataA_HG;
  std::vector<deltat_data> deltaTdataC_HG;

  SG::ReadHandle<BCM_RDO_Container> bcmRDO(m_bcmContainerName, ctx);
  if (!bcmRDO.isValid()) {
    ATH_MSG_WARNING("Cannot find BCM RDO " << m_bcmContainerName.key()
                                           << " ! ");
    return StatusCode::SUCCESS;
  } else {
    int num_collect = bcmRDO->size();
    if (num_collect != 16) {
      ATH_MSG_WARNING(" Number of collections: " << num_collect);
    }
    int channelID = 0;

    for (const BCM_RDO_Collection* chan : *bcmRDO) {
      channelID = chan->getChannel();

      // Loop over all BCM hits in this collection
      for (const BCM_RawData* bcm : *chan) {
        if (bcm->getPulse1Width() != 0 && bcm->getLVL1A() == 18) {
          if (channelID < 8) {
            multiLG++;
          } else {
            multiHG++;
          }
          deltat_data hit(channelID, bcm->getLVL1A(), bcm->getPulse1Position());
          if (channelID > 7 && channelID < 12) {
            deltaTdataA_HG.push_back(hit);
          }
          if (channelID > 11) {
            deltaTdataC_HG.push_back(hit);
          }

          if (bcm->getPulse2Width() != 0) {
            if (channelID < 8) {
              multiLG++;
            } else {
              multiHG++;
            }
            deltat_data hit2(channelID, bcm->getLVL1A(),
                             bcm->getPulse2Position());
            if (channelID > 7 && channelID < 12) {
              deltaTdataA_HG.push_back(hit2);
            }
            if (channelID > 11 && bcm->getPulse2Width() != 0) {
              deltaTdataC_HG.push_back(hit2);
            }
          }
        }
      }  // end of loop over raw data
    }    // end of loop over collections

    // calculate deltaTs from deltatdata now
    for (auto& i : deltaTdataA_HG) {
      for (auto& j : deltaTdataC_HG) {
        if (i.m_bcid == j.m_bcid) {
          float deltaTtime = (static_cast<float>(i.m_position) -
                              static_cast<float>(j.m_position)) /
                             64 * 25;
          deltaT.push_back(deltaTtime);
        }
      }
    }
  }

  SG::WriteHandle<BcmCollisionTime> bbw(m_bcmCollisionTimeName, ctx);
  if (bbw.record(std::make_unique<BcmCollisionTime>(multiLG, multiHG, deltaT))
          .isFailure()) {
    ATH_MSG_WARNING(" Cannot record BcmCollisionTime ");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
