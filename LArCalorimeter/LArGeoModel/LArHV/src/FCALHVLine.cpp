/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHV/FCALHVLine.h"
#include "LArHV/FCALHVModule.h"
#include "LArHV/FCALHVManager.h"
#include "FCALHVPayload.h"

class FCALHVLine::Clockwork {
 
public:

  const FCALHVModule *module;
  
  unsigned int iLine;

};

//##ModelId=47ABB4D90312
FCALHVLine::FCALHVLine(FCALHVModuleConstLink module, unsigned int iLine):c(new Clockwork())
{
  c->module=&*module;
  c->iLine=iLine;
}

//##ModelId=47ABB4D90315
FCALHVModuleConstLink FCALHVLine::getModule() const
{
  return c->module;
}

//##ModelId=47ABB4D90317
unsigned int FCALHVLine::getLineIndex() const
{
  return c->iLine;
}

//##ModelId=47ABB4D90319
bool FCALHVLine::hvOn() const
{
  FCALHVPayload *payload = c->module->getManager()->getPayload(*this);
  if (payload->voltage<-9999) return false;
  else return true;
}

//##ModelId=47ABB4D9031B
FCALHVLine::~FCALHVLine()
{
  delete c;
}

double FCALHVLine::voltage() const {
  FCALHVPayload *payload = c->module->getManager()->getPayload(*this);
  return payload->voltage;
}

double FCALHVLine::current() const {
  FCALHVPayload *payload = c->module->getManager()->getPayload(*this);
  return payload->current;
}

void FCALHVLine::voltage_current(double& voltage, double&current) const {
 FCALHVPayload *payload = c->module->getManager()->getPayload(*this);
 voltage = payload->voltage;
 current = payload->current;
}


int FCALHVLine::hvLineNo() const {
  FCALHVPayload *payload = c->module->getManager()->getPayload(*this);
  return payload->hvLineNo;
}

