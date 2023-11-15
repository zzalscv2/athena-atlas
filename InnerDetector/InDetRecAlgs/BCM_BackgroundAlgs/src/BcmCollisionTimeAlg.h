/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// BcmCollisionTime.h
//

#ifndef BcmCollisionTimeAlg_H
#define BcmCollisionTimeAlg_H

#include "BCM_CollisionTime/BcmCollisionTime.h"
#include "InDetBCM_RawData/BCM_RDO_Container.h"

// Gaudi includes

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

class BcmCollisionTimeAlg : public AthReentrantAlgorithm {
 public:
  // Gaudi style constructor and execution methods
  /** Standard Athena-Algorithm Constructor */
  BcmCollisionTimeAlg(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  virtual ~BcmCollisionTimeAlg();

  /** standard Athena-Algorithm method */
  virtual StatusCode initialize() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<BCM_RDO_Container> m_bcmContainerName{
      this, "BcmContainerName", "BCM_RDOs", ""};
  SG::WriteHandleKey<BcmCollisionTime> m_bcmCollisionTimeName{
      this, "BcmCollisionTimeName", "BcmCollisionTime", ""};
};

class deltat_data {
 public:
  deltat_data();
  deltat_data(unsigned int channel, unsigned int bcid, unsigned int position);

  // private:

  unsigned int m_channel;
  unsigned int m_bcid;
  unsigned int m_position;
};

inline deltat_data::deltat_data() : m_channel(99), m_bcid(99), m_position(99) {}

inline deltat_data::deltat_data(unsigned int channel, unsigned int bcid,
                                unsigned int position)
    : m_channel(channel), m_bcid(bcid), m_position(position) {}

#endif
