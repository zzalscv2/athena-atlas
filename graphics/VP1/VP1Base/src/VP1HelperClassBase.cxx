/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1HelperClassBase               //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: February 2008                            //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1Base/VP1HelperClassBase.h"
#include "VP1Base/IVP1System.h"
#include "VP1Base/VP1QtUtils.h"
#include "VP1Base/VP1Msg.h"
#include <iostream>
#include <map>

const bool VP1HelperClassBase::s_vp1verbose = VP1QtUtils::environmentVariableIsOn("VP1_VERBOSE_OUTPUT");

//only filled in verbose mode:
static std::map<VP1HelperClassBase*,QString> vp1helperclassbase_instanceMap;//instance -> systemname

//____________________________________________________________________
VP1HelperClassBase::VP1HelperClassBase(IVP1System * sys, QString helpername)
  : m_helpername(helpername), m_system(sys)
{
  if(VP1Msg::verbose()){
    messageVerbose("base constructor ("+str(this)+")"+(m_system?" system = "+m_system->name():QString("")));
    vp1helperclassbase_instanceMap[this] = (m_system?m_system->name():QString(""));
  }
}

//____________________________________________________________________
VP1HelperClassBase::~VP1HelperClassBase()
{
  if(VP1Msg::verbose()){
    messageVerbose("base destructor ("+str(this)+")"+(m_system?" system = "+m_system->name():QString("")));
    std::map<VP1HelperClassBase*,QString>::iterator it = vp1helperclassbase_instanceMap.find(this);
    if (it!=vp1helperclassbase_instanceMap.end())
      vp1helperclassbase_instanceMap.erase(it);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::message( const QString& str ) const
{
  if (m_helpername.isEmpty()) {
    if (m_system)
      m_system->message(str);
    else
      std::cout<<VP1Msg::prefix_msg()<<" [nameless helper class]: "<<str.toStdString()<<std::endl;
  } else {
    if (m_system)
      m_system->message("["+m_helpername+"] " + str);
    else
      std::cout<<VP1Msg::prefix_msg()<<" ["<<m_helpername.toStdString()<<"]: "<<str.toStdString()<<std::endl;
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageDebug( const QString& str ) const
{  
  if (!VP1Msg::debug())
    return;
  std::string sysstring(m_system ? " in "+m_system->name().toStdString() : std::string(""));
  if (m_helpername.isEmpty()) {
    std::cout<<VP1Msg::prefix_debug()<<" [helper"<<sysstring<<"]: "<<str.toStdString()<<std::endl;
  } else {
    std::cout<<VP1Msg::prefix_debug()<<" ["<<m_helpername.toStdString()<<sysstring<<"]: "<<str.toStdString()<<std::endl;
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageVerbose( const QString& str ) const
{
  if (!VP1Msg::verbose())
    return;
  std::string sysstring(m_system ? " in "+m_system->name().toStdString() : std::string(""));
  if (m_helpername.isEmpty()) {
    std::cout<<VP1Msg::prefix_verbose()<<" [helper"<<sysstring<<"]: "<<str.toStdString()<<std::endl;
  } else {
    std::cout<<VP1Msg::prefix_verbose()<<" ["<<m_helpername.toStdString()<<sysstring<<"]: "<<str.toStdString()<<std::endl;
  }
}

//____________________________________________________________________
void VP1HelperClassBase::message(const QStringList& l, const QString& addtoend ) const
{
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      message(s);
  } else {
    for (QString s :  l)
      message(s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageDebug(const QStringList& l, const QString& addtoend ) const
{
  if (!VP1Msg::debug())
    return;
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      messageDebug(s);
  } else {
    for (QString s :  l)
      messageDebug(s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageVerbose(const QStringList& l, const QString& addtoend ) const
{
  if (!VP1Msg::verbose())
    return;
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      messageVerbose(s);
  } else {
    for (QString s :  l)
      messageVerbose(s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::message( const QString& addtostart, const QStringList& l, const QString& addtoend ) const
{
  if (addtostart.isEmpty()) {
    message(l,addtoend);
    return;
  }
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      message(addtostart+s);
  } else {
    for (QString s :  l)
      message(addtostart+s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageDebug(const QString& addtostart, const QStringList& l, const QString& addtoend ) const
{
  if (!VP1Msg::debug())
    return;
  if (addtostart.isEmpty()) {
    messageDebug(l,addtoend);
    return;
  }
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      messageDebug(addtostart+s);
  } else {
    for (QString s :  l)
      messageDebug(addtostart+s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::messageVerbose(const QString& addtostart, const QStringList& l, const QString& addtoend ) const
{
  if (!VP1Msg::verbose())
    return;
  if (addtostart.isEmpty()) {
    messageVerbose(l,addtoend);
    return;
  }
  if (addtoend.isEmpty()) {
    for (QString s :  l)
      messageVerbose(addtostart+s);
  } else {
    for (QString s :  l)
      messageVerbose(addtostart+s+addtoend);
  }
}

//____________________________________________________________________
void VP1HelperClassBase::warnUndeletedInstances()
{
  if (vp1helperclassbase_instanceMap.empty())
    return;
  std::cout << "WARNING: Detected "<<vp1helperclassbase_instanceMap.size()<<" undeleted helper class instances:"<<std::endl;

  std::map<VP1HelperClassBase*,QString>::iterator it,itE(vp1helperclassbase_instanceMap.end());
  for (it = vp1helperclassbase_instanceMap.begin();it!=itE;++it) {
    std::cout << "  ==> "<<it->first<<": "<<it->first->m_helpername.toStdString()
	      << (it->second.isEmpty()?QString(""):" (in system "+it->second+")").toStdString()<<std::endl;
  }
}

//____________________________________________________________________
void VP1HelperClassBase::setSystemBasePointer(IVP1System*sys)
{
  m_system = sys;
  if (VP1Msg::verbose()){
    vp1helperclassbase_instanceMap[this] = (m_system?m_system->name():QString(""));
  }
}
