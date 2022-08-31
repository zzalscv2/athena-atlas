/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// AthMessaging.cxx 
// Implementation file for class AthMessaging
// Author: S.Binet<binet@cern.ch>, Frank Winklmeier
/////////////////////////////////////////////////////////////////// 

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"

AthMessaging::AthMessaging (IMessageSvc* msgSvc,
                            const std::string& name) :
  m_imsg(msgSvc), m_nm(name)
{}


AthMessaging::AthMessaging (const std::string& name) :
  m_nm(name)
{}


AthMessaging::~AthMessaging()
{}


void AthMessaging::setLevel (MSG::Level lvl)
{
  if (m_lvl == MSG::NIL) initMessaging();
  m_lvl = lvl;
}


/**
 * Initialize our message level and MessageSvc.
 *
 * This method is only called once when m_lvl == MSG::NIL.
 */
void AthMessaging::initMessaging() const
{
  m_imsg = Athena::getMessageSvc();
  m_lvl = m_imsg ?
    static_cast<MSG::Level>( m_imsg.load()->outputLevel(m_nm) ) :
    MSG::INFO;
}
