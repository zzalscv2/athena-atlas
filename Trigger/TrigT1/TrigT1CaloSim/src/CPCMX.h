/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

 /***************************************************************************
                           CPCMX.h  -  description
                              -------------------
     begin                : Mon Jul 28 2014
     email                : Alan.Watson@CERN.CH
  ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *                                                                         *
  ***************************************************************************/
 #ifndef CPCMX_H
 #define CPCMX_H

 // STL
 #include <string>
 #include <vector>

 // Athena/Gaudi
 #include "AthenaBaseComps/AthAlgorithm.h"
 #include "GaudiKernel/ServiceHandle.h"
 #include "GaudiKernel/ToolHandle.h"
 
 #include "AthContainers/DataVector.h"

 // Include for the configuration service:
 #include "TrigConfInterfaces/ILVL1ConfigSvc.h"
 #include "TrigT1CaloEvent/CMXCPTob.h"
 #include "TrigT1CaloEvent/CPCMXTopoData.h"
 #include "TrigT1CaloEvent/CMXCPHits.h"

// Outputs to CTP
#include "TrigT1Interfaces/EmTauCTP.h"
#include "TrigT1Interfaces/JetCTP.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

 namespace LVL1 {

   //using namespace TrigConf;

 class CPMCMXData;

   //Doxygen Class definition below:
   /**
  The algorithm responsible for simulating the Em/tau calo trigger.
   */
 class CPCMX : public AthAlgorithm
 {

  typedef DataVector<LVL1::CPMCMXData> t_cpmDataContainer;

  public:

   //-------------------------
   // Constructors/Destructors
   //
   // Athena requires that the constructor takes certain arguments
   // (and passes them directly to the constructor of the base class)
   //-------------------------

   CPCMX( const std::string& name, ISvcLocator* pSvcLocator ) ;

   //------------------------------------------------------
   // Methods used by Athena to run the algorithm
   //------------------------------------------------------

   StatusCode initialize() ;
   StatusCode execute() ;

 private: // Private methods
   /** Store CTP SLink data objects in the TES. */
//   void storeCTPData();
   /** Store L1Topo inputs in the TES */
//   void storeTopoTOBs();
   /** Store DAQ objects in the TES */
//   void storeReadoutObjects();

  /** Debug routine: dump trigger menu at start of run */
  void printTriggerMenu();
  
 private: // Private attributes

   /** Where to store the CMXCPHits (for CMX readout simulation) */
   SG::WriteHandleKey<DataVector<CMXCPHits>> m_CMXCPHitsLocation { this, "CMXCPHitsLocation", TrigT1CaloDefs::CMXCPHitsLocation};
   /** Where to store the CMXCPTobs (for CMX readout simulation) */
   SG::WriteHandleKey<DataVector<CMXCPTob>> m_CMXCPTobLocation { this, "CMXCPTobLocation", TrigT1CaloDefs::CMXCPTobLocation};
   /** Locations of real-time outputs in StoreGate */
   SG::WriteHandleKey<DataVector<CPCMXTopoData>> m_TopoOutputLocation { this, "TopoOutputLocation", TrigT1CaloDefs::EmTauTopoTobLocation};

   SG::WriteHandleKey<EmTauCTP> m_CTPOutputKey { this, "CTPOutputLocation", TrigT1CaloDefs::EmTauCTPLocation, "Output to CTP" };


   /** Location of input data in StoreGate */
   SG::ReadHandleKey<DataVector<LVL1::CPMCMXData>> m_CPMCMXDataLocation { this, "CPMCMXDataLocation", TrigT1CaloDefs::CPMCMXDataLocation};
   
   /** The essentials - data access, configuration, tools */
   ServiceHandle<TrigConf::ILVL1ConfigSvc> m_configSvc {
    this, "LVL1ConfigSvc", "TrigConf::LVL1ConfigSvc/LVL1ConfigSvc", "Service providing L1 menu thresholds"};
   
   /** Topo format parameters */
   static const int s_SourceLocal = 3;
   static const int s_SourceTotal = 4;

};

 } // end of namespace bracket


 #endif












