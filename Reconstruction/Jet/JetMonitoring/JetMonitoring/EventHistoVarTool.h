/* this file is -*- C++ -*- 
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETMONITORING_EventVARTOOL_H
#define JETMONITORING_EventVARTOOL_H

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "GaudiKernel/IAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadDecorHandleKey.h"


static const InterfaceID IID_IEventHistoVarTool("IEventHistoVarTool", 1 , 0); 

///////////////////////////////////////////////////////////
/// \class EventHistoVarTool
///
/// This class is a simple tool to access EventInfo or JetContainer
/// variables within the JetMonitoring environment
///

class IEventHistoVarTool : virtual public IAlgTool                         
{
public:
  
  // Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_IEventHistoVarTool; }

  virtual ~IEventHistoVarTool(){}
  
  /// the value of the variable for a given Event
  virtual float value(const xAOD::JetContainer&) const = 0;
  /// a compact description of the variable.
  virtual std::string varName() const =0;
};


class EventHistoVarTool : public AthAlgTool, virtual public IEventHistoVarTool {
public:
  EventHistoVarTool(const std::string & type, const std::string & name, const IInterface* parent);
  virtual ~EventHistoVarTool(){}

  virtual StatusCode initialize() ;  

  virtual float value(const xAOD::JetContainer&) const;
  virtual std::string varName() const {return m_varName;}
  
  
private:

  Gaudi::Property<std::string> m_varName {this,"VarName", ""};
  Gaudi::Property<float> m_defaultValue = {this,"Default", -1.};
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_attName {this, "Attribute", "", "Attribute name retrieved from event info container ('<event info name>.<decoration>'). If no container name prefixed, then EventInfo.<Attribute> is used."};
  
};


#endif
