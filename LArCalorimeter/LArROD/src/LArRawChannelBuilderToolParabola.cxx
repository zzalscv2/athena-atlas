/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "LArRawChannelBuilderToolParabola.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "Identifier/Identifier.h"

#include "LArRawEvent/LArDigit.h"
#include <cmath>

#define MAXINT 2147483000
#define MAXINT2 -2147483000


LArRawChannelBuilderToolParabola::LArRawChannelBuilderToolParabola(const std::string& type,
								   const std::string& name,
								   const IInterface* parent):
  LArRawChannelBuilderToolBase(type,name,parent),
  m_parabolaRecoTool("LArOFPeakRecoTool"),
  m_emId(nullptr)
{
  m_helper = new LArRawChannelBuilderStatistics( 3,      // number of possible errors
					       0x03);  // bit pattern special for this tool,
                                                       // to be stored in  "uint16_t provenance"
  m_helper->setErrorString(0, "no errors");
  m_helper->setErrorString(1, "Fit failed");
  m_helper->setErrorString(2, "is FCAL");
}

StatusCode LArRawChannelBuilderToolParabola::initTool()
{
  ATH_CHECK( m_parabolaRecoTool.retrieve() );
  
  const CaloCell_ID* idHelper = nullptr;
  ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_ID") );
  m_emId=idHelper->em_idHelper(); 
  
  return StatusCode::SUCCESS;
}
 
bool LArRawChannelBuilderToolParabola::buildRawChannel(const LArDigit* digit,
						       float pedestal,
						       const std::vector<float>& ramps,
						       MsgStream* /* pLog */ )
{
  float ADCPeak=0;
  
  int time=0;
  uint16_t iquality=0;
  uint16_t iprovenance=0;
  
  int layer=0;
  if(m_emId->is_em_barrel(currentID()))
    layer=m_emId->sampling(currentID());
  
  std::vector<float> peak = m_parabolaRecoTool->peak(digit->samples(),
						     layer, pedestal);
  ADCPeak=peak[0]-pedestal;
  float power=1;
  float energy=0;
  for( unsigned int i=0; i<ramps.size(); i++)
    {
      energy += ramps[i] * power;
      power  *= ADCPeak;
    }
  
  if(peak.size()==2)
    time = int(peak[1]);
  else
    time = int(peak[2]);
  
  // store which tool created this channel
  iprovenance |= m_parent->qualityBitPattern;
  iprovenance |= m_helper->returnBitPattern();
  iprovenance = iprovenance & 0x3FFF;

  const float fMAXINT = static_cast<float>(MAXINT);
  const float fMAXINT2 = static_cast<float>(MAXINT2);

  if (time>MAXINT) time=MAXINT;
  if (time<MAXINT2) time=MAXINT2;

  if (energy>fMAXINT) energy=fMAXINT;
  if (energy<fMAXINT2) energy=fMAXINT2;

  
  // Suppress false positive seen wth gcc.
#if __GNUC__ >= 11 && __GNUC__ <= 13
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
  (this->*m_buildIt)((int)(floor(energy+0.5)),time,iquality,iprovenance,digit);
#if __GNUC__ >= 11 && __GNUC__ <= 13
# pragma GCC diagnostic pop
#endif

  m_helper->incrementErrorCount(0);
  
  return true;
}
