/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsInterop/Logger.h"
#include "ActsInterop/LoggerUtils.h"

#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/CommonMessaging.h"

#include "Acts/Utilities/Logger.hpp"

#include <boost/optional.hpp>

#include <iostream>
#include <string>

void
ActsAthenaPrintPolicy::flush(const Acts::Logging::Level& lvl, const std::string& input)
{
  MSG::Level athLevel = ActsTrk::athLevelVector(lvl);
  (*m_msg) << athLevel << input << endmsg;
}
  
const std::string& 
ActsAthenaPrintPolicy::name() const 
{
  return m_name;
}

std::unique_ptr<Acts::Logging::OutputPrintPolicy> 
ActsAthenaPrintPolicy::clone(const std::string& name) const
{
  auto msg = std::make_shared<MsgStream>(*m_msg.get());
  return std::make_unique<ActsAthenaPrintPolicy>(msg, name);
}

bool
ActsAthenaFilterPolicy::doPrint(const Acts::Logging::Level& lvl) const 
{

  MSG::Level athLevel = ActsTrk::athLevelVector(lvl);
  return m_msg->level() <= athLevel;
}

Acts::Logging::Level 
ActsAthenaFilterPolicy::level() const 
{
  return ActsTrk::actsLevelVector(m_msg->level());
}

std::unique_ptr<Acts::Logging::OutputFilterPolicy> 
ActsAthenaFilterPolicy::clone(Acts::Logging::Level level) const 
{
  auto msg = std::make_shared<MsgStream>(*m_msg.get());
  msg->setLevel(ActsTrk::athLevelVector(level));
  return std::make_unique<ActsAthenaFilterPolicy>(msg);
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(IMessageSvc *svc, const std::string& name, int level, boost::optional<std::string> parent_name)
{
  using namespace Acts::Logging;

  std::string full_name = name;
  if (parent_name) {
    full_name = *parent_name + "." + full_name;
  }

  auto msg = std::make_shared<MsgStream>(svc, full_name);
  msg->setLevel(level);
  auto filter = std::make_unique<ActsAthenaFilterPolicy>(msg);
  auto print = std::make_unique<ActsAthenaPrintPolicy>(msg, full_name);
  return std::make_unique<const Acts::Logger>(std::move(print), std::move(filter));
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name)
{
  // no explicit name, get from component
  const INamedInterface *inamed = dynamic_cast<const INamedInterface*>(parent);
  boost::optional<std::string> parent_name = boost::none;
  // this will not prefix if parent is not named (which it should be)
  if (inamed == nullptr) {
    throw std::invalid_argument("parent needs to be INamedInterface");
  }
  parent_name = inamed->name();
  return makeActsAthenaLogger(parent, name, parent_name);
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name, boost::optional<std::string> parent_name)
{
  int level = 0;
  const INamedInterface *inamed = dynamic_cast<const INamedInterface*>(parent);
  if (inamed != nullptr) {
    level = parent->msg().level();
  }
  return makeActsAthenaLogger(parent->msgSvc().get(), name, level, parent_name);
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name, const std::string& parent_name)
{
  
  return makeActsAthenaLogger(parent, name, boost::optional<std::string>(parent_name));
}
