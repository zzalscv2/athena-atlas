/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/***************************************************************************
                          SystemEnergy.h  -  description
                             -------------------
    begin                : 06/09/2007
    email                : Alan.Watson@cern.ch
 ***************************************************************************/


#include "TrigT1CaloUtils/SystemEnergy.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigConfL1Data/L1DataDef.h"

using namespace TrigConf;

namespace LVL1
{

SystemEnergy::SystemEnergy(const DataVector<CrateEnergy> *crates, const TrigConf::L1Menu* l1Menu) : m_L1Menu(l1Menu),
                                                                                                                    m_systemEx(0),
                                                                                                                    m_systemEy(0),
                                                                                                                    m_systemEt(0),
                                                                                                                    m_overflowX(0),
                                                                                                                    m_overflowY(0),
                                                                                                                    m_overflowT(0),
                                                                                                                    m_restricted(0),
                                                                                                                    m_etMissHits(0),
                                                                                                                    m_etSumHits(0),
                                                                                                                    m_metSigHits(0),
                                                                                                                    m_debug(false)
{

  int xyMax = 1 << (m_sumBits - 1);

  /** Get Ex, Ey, ET sums from crates and form global sums <br>
      Propagate overflows and test for new ones <br> */

  for (const CrateEnergy* cr : *crates)
  {
    if (cr->crate() == 0)
    {
      m_systemEx += cr->ex();
      m_systemEy += cr->ey();
      m_systemEt += cr->et();
    }
    else if (cr->crate() == 1)
    {
      m_systemEx -= cr->ex();
      m_systemEy += cr->ey();
      m_systemEt += cr->et();
    }
    
    m_overflowX = m_overflowX | (cr->ex() == -xyMax);
    m_overflowY = m_overflowY | (cr->ey() == -xyMax);
    m_overflowT = m_overflowT | (cr->et() == m_maxEtSumThr);

    if (cr->restricted())
      m_restricted = 1;
  }

  /** Check for EtSum overflow */
  if (m_overflowT != 0) {
    m_systemEt = m_etSumOverflow;
  }
    


  if ((abs(m_systemEx) >= xyMax) || m_overflowX)
  {
    m_overflowX = 1;
    m_systemEx = -xyMax;
  }

  if ((abs(m_systemEy) >= xyMax) || m_overflowY)
  {
    m_overflowY = 1;
    m_systemEy = -xyMax;
  }

  if (m_debug)
  {
    std::cout << "SystemEnergy results: " << std::endl
              << "   Et " << m_systemEt << " overflow " << m_overflowT << std::endl
              << "   Ex " << m_systemEx << " overflow " << m_overflowX << std::endl
              << "   Ey " << m_systemEy << " overflow " << m_overflowY << std::endl;
  }

  /** Having completed sums, we can now compare with thresholds */
  etMissTrigger();
  etSumTrigger();
  metSigTrigger();
}

SystemEnergy::SystemEnergy(unsigned int et, unsigned int exTC, unsigned int eyTC,
                           unsigned int overflowT, unsigned int overflowX,
                           unsigned int overflowY, unsigned int restricted,
                           const TrigConf::L1Menu* l1Menu) : m_L1Menu(l1Menu),
                                                                             m_systemEx(0),
                                                                             m_systemEy(0),
                                                                             m_systemEt(et),
                                                                             m_overflowX(overflowX),
                                                                             m_overflowY(overflowY),
                                                                             m_overflowT(overflowT),
                                                                             m_restricted(restricted),
                                                                             m_etMissHits(0),
                                                                             m_etSumHits(0),
                                                                             m_metSigHits(0),
                                                                             m_debug(false)
{

  m_systemEx = decodeTC(exTC);
  m_systemEy = decodeTC(eyTC);

  int xyMax = 1 << (m_sumBits - 1);

  m_overflowT = m_systemEt == m_etSumOverflow;
  m_overflowX = m_systemEx == -xyMax;
  m_overflowY = m_systemEy == -xyMax;

  if (m_debug)
  {
    std::cout << "SystemEnergy results: " << std::endl
              << "   Et " << m_systemEt << " overflow " << m_overflowT << std::endl
              << "   Ex " << m_systemEx << " overflow " << m_overflowX << std::endl
              << "   Ey " << m_systemEy << " overflow " << m_overflowY << std::endl;
  }

  /** Having completed sums, we can now compare with thresholds */
  etMissTrigger();
  etSumTrigger();
  metSigTrigger();
}

SystemEnergy::~SystemEnergy()
{
}

/** return crate Et */
int SystemEnergy::et() const
{
  return m_systemEt;
}

/** return crate Ex */
int SystemEnergy::ex() const
{
  return m_systemEx;
}

/** return crate Ey */
int SystemEnergy::ey() const
{
  return m_systemEy;
}

/** return Et overflow bit */
unsigned int SystemEnergy::etOverflow() const
{
  return m_overflowT & 0x1;
}

/** return Ex overflow bit */
unsigned int SystemEnergy::exOverflow() const
{
  return m_overflowX & 0x1;
}

/** return Ey overflow bit */
unsigned int SystemEnergy::eyOverflow() const
{
  return m_overflowY & 0x1;
}

/** return crate Ex in 15-bit twos-complement format (hardware format) */
unsigned int SystemEnergy::exTC() const
{
  return encodeTC(m_systemEx);
}

/** return crate Ey in 15-bit twos-complement format (hardware format) */
unsigned int SystemEnergy::eyTC() const
{
  return encodeTC(m_systemEy);
}

/** return EtMiss hits */
unsigned int SystemEnergy::etMissHits() const
{
  return m_etMissHits;
}

/** return EtSum hits */
unsigned int SystemEnergy::etSumHits() const
{
  return m_etSumHits;
}

/** return MEtSig hits */
unsigned int SystemEnergy::metSigHits() const
{
  return m_metSigHits;
}

/** return RoI word 0 (Ex value & overflow) */
unsigned int SystemEnergy::roiWord0() const
{
  // Start by setting up header
  unsigned int word = TrigT1CaloDefs::energyRoIType << 30;
  word += TrigT1CaloDefs::energyRoI0 << 28;
  // set full/restricted eta range flag
  word += (m_restricted & 1) << 26;
  // add MET Significance hits
  word += metSigHits() << 16;
  // add Ex overflow bit
  word += exOverflow() << 15;
  // add Ex value (twos-complement)
  word += exTC();
  return word;
}

/** return RoI word 1 (Ey value & overflow, EtSum hits) */
unsigned int SystemEnergy::roiWord1() const
{
  // Start by setting up header
  unsigned int word = TrigT1CaloDefs::energyRoIType << 30;
  word += TrigT1CaloDefs::energyRoI1 << 28;
  // set full/restricted eta range flag
  word += (m_restricted & 1) << 26;
  // add EtSum hits
  word += etSumHits() << 16;
  // add Ey overflow bit
  word += eyOverflow() << 15;
  // add Ey value (twos-complement)
  word += eyTC();
  return word;
}

/** return RoI word 2 (Et value & overflow, EtMiss hits) */
unsigned int SystemEnergy::roiWord2() const
{
  // Start by setting up header
  unsigned int word = TrigT1CaloDefs::energyRoIType << 30;
  word += TrigT1CaloDefs::energyRoI2 << 28;
  // set full/restricted eta range flag
  word += (m_restricted & 1) << 26;
  // add EtMiss hits
  word += etMissHits() << 16;
  // add Et overflow bit
  word += etOverflow() << 15;
  // add Et value (unsigned)
  word += et();
  return word;
}

/** Test Ex, Ey sums against ETmiss thresholds <br>
    Regrettably not as simple as it sounds if we emulate hardware! <br>
    See CMM specificiation from L1Calo web pages for details */
void SystemEnergy::etMissTrigger()
{
  /// Start cleanly
  m_etMissHits = 0;

  /// Calculate MET^2
  m_etMissQ = m_systemEx * m_systemEx + m_systemEy * m_systemEy;

  /// Ex or Ey overflow automatically fires all thresholds
  if ((m_overflowX != 0) || (m_overflowY != 0))
  {
    m_etMissHits = (1 << TrigT1CaloDefs::numOfMissingEtThresholds) - 1;
    return;
  }

  /// Otherwise see which thresholds were passed

  // Get thresholds
  std::vector<std::shared_ptr<TrigConf::L1Threshold>> allThresholds = m_L1Menu->thresholds();

  // get Threshold values and test

  for ( const auto& thresh : allThresholds ) {
    if ( thresh->type() == L1DataDef::xeType()) {
      std::shared_ptr<TrigConf::L1Threshold_Calo> thresh_Calo = std::static_pointer_cast<TrigConf::L1Threshold_Calo>(thresh);
      unsigned int thresholdValue = thresh_Calo->thrValueCounts();
      uint32_t tvQ = thresholdValue * thresholdValue;
      int threshNumber = thresh->mapping();
      if (m_restricted == 0 && threshNumber < 8)
      {
        if (m_etMissQ > tvQ)
          m_etMissHits = m_etMissHits | (1 << threshNumber);
      }
      else if (m_restricted != 0 && threshNumber >= 8)
      {
        if (m_etMissQ > tvQ)
          m_etMissHits = m_etMissHits | (1 << (threshNumber - 8));
      }
    }
  }

  return;
}

/** Test Et sum against ETsum thresholds */
void SystemEnergy::etSumTrigger()
{
  // Start cleanly
  m_etSumHits = 0;

  // Overflow of any sort automatically fires all thresholds
  if (m_overflowT != 0 || m_systemEt > m_maxEtSumThr)
  {
    m_etSumHits = (1 << TrigT1CaloDefs::numOfSumEtThresholds) - 1;
    return;
  }

  // Get thresholds
  std::vector<std::shared_ptr<TrigConf::L1Threshold>> allThresholds = m_L1Menu->thresholds();

  // get Threshold values and test
  // Since eta-dependent values are being used to disable TE in regions, must find lowest value for each threshold
  for ( const auto& thresh : allThresholds ) {
    if ( thresh->type() == L1DataDef::teType()) {
      int threshNumber = thresh->mapping();
      int thresholdValue = m_maxEtSumThr;
      std::shared_ptr<TrigConf::L1Threshold_Calo> thresh_Calo = std::static_pointer_cast<TrigConf::L1Threshold_Calo>(thresh);
      auto tvcs = thresh_Calo->thrValuesCounts();
      if (tvcs.size() == 0) {
        tvcs.addRangeValue(thresh_Calo->thrValueCounts(),-49, 49, 1, true);
      }
      for (const auto& tVC : tvcs) {
        if (static_cast<int>(tVC.value()) < thresholdValue) {
          thresholdValue = tVC.value();
        }
      }

      if (m_restricted == 0 && threshNumber < 8)
      {
        if (static_cast<int>(m_systemEt) > thresholdValue)
          m_etSumHits = m_etSumHits | (1 << threshNumber);
      }
      else if (m_restricted != 0 && threshNumber >= 8)
      {
        if (static_cast<int>(m_systemEt) > thresholdValue)
          m_etSumHits = m_etSumHits | (1 << (threshNumber - 8));
      }
    }
  }

  return;
}

/** Test MEt Significance against METSig thresholds */
void SystemEnergy::metSigTrigger()
{
  /// Start cleanly
  m_metSigHits = 0;

  /// No restricted eta MET significance trigger
  if (m_restricted != 0)
    return;

  /// Obtain parameters from configuration service
  auto& params = m_L1Menu->thrExtraInfo().XS();
  unsigned int Scale = params.xsSigmaScale();
  unsigned int Offset = params.xsSigmaOffset();
  unsigned int XEmin = params.xeMin();
  unsigned int XEmax = params.xeMax();
  int sqrtTEmin = params.teSqrtMin();
  int sqrtTEmax = params.teSqrtMax();

  /// Start with overflow and range checks
  if ((m_overflowX > 0) || (m_overflowY > 0) || (m_etMissQ > XEmax * XEmax))
  {
    m_metSigHits = (1 << TrigT1CaloDefs::numOfMEtSigThresholds) - 1;
    return;
  }
  else if ((m_etMissQ < XEmin * XEmin) || (m_systemEt < sqrtTEmin * sqrtTEmin) || (m_systemEt > sqrtTEmax * sqrtTEmax))
  {
    return;
  }

  /// Perform threshold tests. Emulate firmware logic for this, so don't explicitly calculate XS
  /// Prepare factors we wil re-use for each threshold.
  /// Scale bQ to hardware precision
  unsigned long aQ = Scale * Scale;
  unsigned int bQ = ceil(Offset * Offset * 1.e-6);
  unsigned long fourbQTE = 4 * bQ * m_systemEt;

  // Get thresholds
  std::vector<std::shared_ptr<TrigConf::L1Threshold>> allThresholds = m_L1Menu->thresholds();

  /// get Threshold values and test
  /// aQTiQ has to be scaled to hardware precision after product formed
  for ( const auto& thresh : allThresholds ) {
    if ( thresh->type() == L1DataDef::xsType()) {

      int threshNumber = thresh->mapping();
      std::shared_ptr<TrigConf::L1Threshold_Calo> thresh_Calo = std::static_pointer_cast<TrigConf::L1Threshold_Calo>(thresh);
      unsigned int Ti = thresh_Calo->thrValueCounts();
      unsigned long aQTiQ = (0.5 + double(aQ * 1.e-8) * Ti * Ti);

      long left = aQTiQ * aQTiQ * fourbQTE;
      long right = aQTiQ * (m_systemEt + bQ) - m_etMissQ;

      if (right < 0 || left > right * right)
        m_metSigHits = m_metSigHits | (1 << threshNumber);
    }
  }

  return;
}

/** encode int as 15-bit twos-complement format (hardware Ex/Ey format) */
unsigned int SystemEnergy::encodeTC(int input) const
{
  unsigned int value;

  if (input > 0)
  {
    value = input;
  }
  else
  {
    value = (1 << m_sumBits) + input;
  }

  int mask = (1 << m_sumBits) - 1;
  return value & mask;
}

/** decode 15-bit twos-complement format (hardware Ex/Ey format) as int */
int SystemEnergy::decodeTC(unsigned int input) const
{

  int mask = (1 << m_sumBits) - 1;
  int value = input & mask;

  if ((value >> (m_sumBits - 1)))
  {
    int complement = ~value;
    value = -((complement + 1) & mask);
  }

  return value;
}

} // end of ns
