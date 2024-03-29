/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1CALOBYTESTREAM_TRIGT1CALOMONERRORTOOL_H
#define TRIGT1CALOBYTESTREAM_TRIGT1CALOMONERRORTOOL_H

#include <string>
#include <vector>

#include "AsgTools/AsgTool.h"
#include "TrigT1CaloMonitoringTools/ITrigT1CaloMonErrorTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

class IInterface;
class StatusCode;

namespace LVL1 {

/** Tool to retrieve ROB status and ROD unpacking errors from StoreGate.
 *
 *  Forces retrieve of all containers to ensure error vector is complete.
 *
 *  The error vector returned by @c retrieve() is empty if there are no errors.<br>
 *  Otherwise the first word is the number of ROB status errors,               <br>
 *  followed by the ROB ID and error word for each ROB status error (if any),  <br>
 *  followed by the ROB ID and error code for each unpacking error (if any).   <br>
 *  Since the bytestream decoders skip a ROB fragment as soon as an error is detected
 *  there cannot be more than one error per ROB ID.
 *
 *  The behaviour of @c corrupt() is determined by the property @c FlagCorruptEvents:
 *  <table>
 *  <tr><th> @c FlagCorruptEvents      </th><th> @c corrupt() returns                                     </th></tr>
 *  <tr><td> "FullEventTimeout"        <br>
 *           (default)                 </td><td> @c true if Full Event status generic timeout bit set     <br>
 *                                               @b and there are missing ROB/ROD fragments               <br>
 *                                               @b and there are no ROB status or unpacking errors.      </td></tr>
 *  <tr><td> "AnyROBOrUnpackingError"  </td><td> @c true if there are any ROB status or unpacking errors. </td></tr>
 *  <tr><td> "None"                    <br>
 *           (or anything else)        </td><td> @c false.                                                </td></tr>
 *  </table>
 *  @c corrupt() is called by all the monitoring tools and if @c true the event is skipped.
 *
 *  <b>Related Documentation:</b>                              <!-- UPDATE!! -->
 *
 *  <a href="https://edms.cern.ch/document/445840/4.0e/eformat.pdf">
 *  The raw event format in the ATLAS Trigger & DAQ, ATL-D-ES-0019</a><br>
 *  <a href="https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ROBINFragmentErrors">
 *  Definition of the status words in a ROB fragment header</a><br>
 *  See the documentation for
 *  <a href="../../TrigT1CaloMonitoring/html/classTrigT1CaloRodMonTool.html#_details">TrigT1CaloRodMonTool</a>
 *  in the package TrigT1CaloMonitoring for the meanings of the unpacking error codes.
 *
 *  @author Peter Faulkner
 */

class TrigT1CaloMonErrorTool :  public asg::AsgTool,
   virtual public ITrigT1CaloMonErrorTool {
   ASG_TOOL_CLASS(TrigT1CaloMonErrorTool, ITrigT1CaloMonErrorTool)
public:
   TrigT1CaloMonErrorTool(const std::string& name);
   virtual ~TrigT1CaloMonErrorTool();

   virtual StatusCode initialize();
   virtual StatusCode finalize();

   /// Retrieve error vector
   StatusCode retrieve(const std::vector<unsigned int>*& errColl);
   /// Return true if current event has any corruption errors
   bool corrupt();
   /// Return true if current event has Full Event status generic timeout bit set
   bool fullEventTimeout();
   /// Return true if any ROB/ROD fragments are missing
   bool missingFragment();
   /// Return true if current event has any ROB or unpacking errors
   bool robOrUnpackingError();
   /// Return corrupt events flag string
   const std::string& flagCorruptEvents() const { return m_flagCorruptEvents; }

private:

   /// Trigger Tower container StoreGate key
   std::string m_triggerTowerLocation;
   /// CPM core tower container StoreGate key
   std::string m_cpmTowerLocation;
   /// CPM overlap tower container StoreGate key
   std::string m_cpmTowerLocationOverlap;
   /// CMX-CP TOB container StoreGate key
   std::string m_cmxCpTobLocation;
   /// CMX-CP hits container StoreGate key
   std::string m_cmxCpHitsLocation;
   /// CPM RoI container StoreGate key
   std::string m_cpmRoiLocation;
   /// Core Jet Element container StoreGate key
   std::string m_jetElementLocation;
   /// Overlap Jet Element container StoreGate key
   std::string m_jetElementLocationOverlap;
   /// CMX-Jet TOB container StoreGate key
   std::string m_cmxJetTobLocation;
   /// CMX-Jet hits container StoreGate key
   std::string m_cmxJetHitsLocation;
   /// JEM RoI container StoreGate key
   std::string m_jemRoiLocation;
   /// CMX RoI container StoreGate key
   std::string m_cmxRoiLocation;
   /// JEM Et sums container StoreGate key
   std::string m_jemEtSumsLocation;
   /// CMX Et sums container StoreGate key
   std::string m_cmxEtSumsLocation;
   /// ROD header container StoreGate key
   std::string m_rodHeaderLocation;
   /// CP RoIB ROD header container StoreGate key
   std::string m_cpRoibRodHeaderLocation;
   /// JEP RoIB ROD header container StoreGate key
   std::string m_jepRoibRodHeaderLocation;
   /// ROB and Unpacking Error vector StoreGate key
   std::string m_robErrorVectorLocation;
   /// Flag corrupt events
   std::string m_flagCorruptEvents;

   SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
     { this, "EventInfoKey", "EventInfo", "SG key for EventInfo" };
};

} // end namespace

#endif
