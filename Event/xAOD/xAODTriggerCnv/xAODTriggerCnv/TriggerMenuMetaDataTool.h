// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TriggerMenuMetaDataTool.h 683395 2015-07-16 11:11:56Z krasznaa $
#ifndef XAODTRIGGERCNV_TRIGGERMENUMETADATATOOL_H
#define XAODTRIGGERCNV_TRIGGERMENUMETADATATOOL_H

// System include(s):
#include <string>
#include <memory>
#include <mutex>

// Gaudi/Athena include(s):
#include "AsgTools/AsgMetadataTool.h"
#ifdef ASGTOOL_ATHENA
#   include "AthenaPoolKernel/IMetaDataTool.h"
#endif // ASGTOOL_ATHENA

// EDM include(s):
#include "xAODTrigger/TriggerMenuContainer.h"
#include "xAODTrigger/TriggerMenuAuxContainer.h"

namespace xAODMaker {

   /// Tool taking care of copying the trigger configuration from file to file
   ///
   /// This tool does the heavy lifting when creating/merging DxAOD files to
   /// make sure that the trigger configuration metadata ends up in the output.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 683395 $
   /// $Date: 2015-07-16 13:11:56 +0200 (Thu, 16 Jul 2015) $
   ///
   class TriggerMenuMetaDataTool : public asg::AsgMetadataTool
#ifdef ASGTOOL_ATHENA
   , public virtual ::IMetaDataTool
#endif // ASGTOOL_ATHENA
   {

      /// Declare the correct constructor for Athena
      ASG_TOOL_CLASS0( TriggerMenuMetaDataTool )

   public:
      /// Regular AlgTool constructor
      TriggerMenuMetaDataTool( const std::string& name =
                                  "TriggerMenuMetaDataTool" );

      /// Function initialising the tool
      virtual StatusCode initialize() override; 

   protected:
      /// @name Functions called by the AsgMetadataTool base class
      /// @{

      /// Function collecting the trigger configuration metadata from the input
      /// file
      virtual StatusCode beginInputFile() override;

      /// Function used to make sure that file openings are not missed
      virtual StatusCode beginEvent() override;

      /// Function writing out the collected metadata
      virtual StatusCode metaDataStop() override;

      /// @}

   private:

      /// Perform the R2 data copy from the input metastore to the internal metastore
      StatusCode checkxAODTriggerMenu();
      /// Perform the R2 data copy from the internal metastore to the output metastore
      StatusCode endxAODTriggerMenu();
      /// Common code to copy and de-duplicate menus from the copyFrom collection into the copyTo collection.
      StatusCode doCopyxAODTriggerMenu(const xAOD::TriggerMenuContainer* copyFrom, 
         xAOD::TriggerMenuContainer* copyTo);

      /// @name Runs 1, 2 data propagation
      /// @{

      /// The key of the trigger menu in the input file
      std::string m_inputKey;
      /// The key of the trigger menu for the output file
      std::string m_outputKey;

      /// The merged trigger menu container
      std::unique_ptr< xAOD::TriggerMenuContainer > m_menu;
      /// The merged trigger menu auxiliary container
      std::unique_ptr< xAOD::TriggerMenuAuxContainer > m_menuAux;

      /// @}

      /// Internal status flag showing whether a BeginInputFile incident was
      /// seen already
      bool m_beginFileIncidentSeen;

      /// Global serial protection over writing to the output store for MP
      static std::mutex s_mutex;

   }; // class TriggerMenuMetaDataTool

} // namespace xAODMaker

#endif // XAODTRIGGERCNV_TRIGGERMENUMETADATATOOL_H
