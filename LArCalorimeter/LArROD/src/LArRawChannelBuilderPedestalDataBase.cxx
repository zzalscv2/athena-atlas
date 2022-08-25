/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "LArRawChannelBuilderPedestalDataBase.h"
#include "StoreGate/ReadCondHandle.h"

LArRawChannelBuilderPedestalDataBase::LArRawChannelBuilderPedestalDataBase(const std::string& type,
									   const std::string& name,
									   const IInterface* parent):
  LArRawChannelBuilderPedestalToolBase(type,name,parent)
{
  m_helper = new LArRawChannelBuilderStatistics( 2,      // number of possible errors
					       0x80);  // bit pattern special for this tool,
                                                       // to be stored in  "uint16_t provenance"
  m_helper->setErrorString(0, "no errors");
  m_helper->setErrorString(1, "no DataBase");
  m_helper->setErrorString(2, "no DataBase entry");
}

StatusCode LArRawChannelBuilderPedestalDataBase::initTool()
{
  ATH_CHECK( m_larPedestalKey.initialize() );
  return StatusCode::SUCCESS;
}

bool LArRawChannelBuilderPedestalDataBase::pedestal(float& pedestal, MsgStream* /*pLog*/)
{
  pedestal = 0;
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadCondHandle<ILArPedestal> larPedestal (m_larPedestalKey, ctx);
  float DBpedestal 
    =larPedestal->pedestal(m_parent->curr_chid,m_parent->curr_gain);
  if (DBpedestal <= (1.0+LArElecCalib::ERRORCODE))
    {
      m_helper->incrementErrorCount(2);
      return false;
    }
  pedestal=DBpedestal;
  m_helper->incrementErrorCount(0);
  m_parent->qualityBitPattern |= m_helper->returnBitPattern();
  
  return true;
}
