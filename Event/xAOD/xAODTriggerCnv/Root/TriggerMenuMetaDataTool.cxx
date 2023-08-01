/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TriggerMenuMetaDataTool.cxx 683395 2015-07-16 11:11:56Z krasznaa $

// System include(s):
#include <memory>

#include <stdexcept>

// Local include(s):
#include "xAODTriggerCnv/TriggerMenuMetaDataTool.h"

namespace xAODMaker {

   std::mutex TriggerMenuMetaDataTool::s_mutex;

   TriggerMenuMetaDataTool::TriggerMenuMetaDataTool( const std::string& name )
      : asg::AsgMetadataTool( name ),
        m_menu(), m_menuAux(), m_beginFileIncidentSeen( false ) {

#ifdef ASGTOOL_ATHENA
      declareInterface< ::IMetaDataTool >( this );
#endif // ASGTOOL_ATHENA

      declareProperty("InputKey", m_inputKey = "TriggerMenu");
      declareProperty("OutputKey", m_outputKey = "TriggerMenu");

   }

   StatusCode TriggerMenuMetaDataTool::initialize() {

      // Greet the user:
      ATH_MSG_DEBUG( "Initialising TriggerMenuMetaDataTool" );


      // Reset the internal variable(s):
      m_menu.reset();
      m_menuAux.reset();

      m_beginFileIncidentSeen = false;

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::beginInputFile() {
      // Whatever happens, we've seen the first BeginInputFile incident now.
      m_beginFileIncidentSeen = true;
      ATH_CHECK( checkxAODTriggerMenu() );
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::checkxAODTriggerMenu() {
      // If the input file doesn't have any trigger configuration metadata,
      // then finish right away:
      if( ! inputMetaStore()->contains< xAOD::TriggerMenuContainer >(
                                                                m_inputKey ) ) {
         return StatusCode::SUCCESS;
      }

      // Retrieve the input container:
      const xAOD::TriggerMenuContainer* input = nullptr;
      ATH_CHECK( inputMetaStore()->retrieve( input, m_inputKey ) );

      // Create an internal container if it doesn't exist yet:
      if( ( ! m_menu.get() ) && ( ! m_menuAux.get() ) ) {
         ATH_MSG_DEBUG( "Creating internal container" );
         m_menu = std::make_unique<xAOD::TriggerMenuContainer>( );
         m_menuAux = std::make_unique<xAOD::TriggerMenuAuxContainer>( );
         m_menu->setStore( m_menuAux.get() );
      }

      // Copy (and de-duplicate) from the input collection to the internal collection
      ATH_CHECK( doCopyxAODTriggerMenu(input, m_menu.get()) );

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::doCopyxAODTriggerMenu(const xAOD::TriggerMenuContainer* copyFrom, xAOD::TriggerMenuContainer* copyTo) {
      // Loop over the configurations of the copyFrom collection:
      for( const xAOD::TriggerMenu* menu : *copyFrom ) {

         // Check if this configuration is already in the copyTo container:
         bool exists = false;
         for( const xAOD::TriggerMenu* existing : *copyTo ) {
            if( ( existing->smk() == menu->smk() ) &&
                ( existing->l1psk() == menu->l1psk() ) &&
                ( existing->hltpsk() == menu->hltpsk() ) ) {
               exists = true;
               break;
            }
         }
         if( exists ) {
            continue;
         }

         // If it's a new configuration, put it into the copyTo container:
         ATH_MSG_VERBOSE( "Copying configuration with SMK: "
                          << menu->smk() << ", L1PSK: " << menu->l1psk()
                          << ", HLTPSK: " << menu->hltpsk() );
         xAOD::TriggerMenu* out = new xAOD::TriggerMenu();
         copyTo->push_back( out );
         *out = *menu;
      }
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::beginEvent() {

      // In case the BeginInputFile incident was missed in standalone mode, make
      // sure that the metadata from the first input file is collected at this
      // point at least.
      if( ! m_beginFileIncidentSeen ) {
         ATH_CHECK( beginInputFile() );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::metaDataStop() {
      // Note: Copying into a given collection in the output store should only be done
      // by a single thread at a time.
      std::lock_guard<std::mutex> lock(s_mutex);

      ATH_CHECK( endxAODTriggerMenu() );
      return StatusCode::SUCCESS;
   }

   StatusCode TriggerMenuMetaDataTool::endxAODTriggerMenu() {
      // The output may already have trigger configuration metadata in it.
      // For instance from TrigConf::xAODMenuWriter or other instances of the
      // TriggerMenuMetaDataTool in MP mode. Merge into the output

      // If we don't have an internal store then nothing to do
      if( ( ! m_menu.get() ) && ( ! m_menuAux.get() ) ) {
         ATH_MSG_DEBUG( "No internal xAOD::TriggerMenu store to save/merge to output." );
         return StatusCode::SUCCESS;
      }

      if( not outputMetaStore()->contains<xAOD::TriggerMenuContainer>(m_outputKey) ) {
         // No output yet - hand over ownership of our internal store
         ATH_MSG_DEBUG( "Recording xAOD::TriggerMenu trigger configuration metadata container with " << m_menu->size() << " entries." );
         ATH_CHECK( outputMetaStore()->record( m_menu.release(),
                                               m_outputKey ) );
         ATH_CHECK( outputMetaStore()->record( m_menuAux.release(),
                                               m_outputKey + "Aux." ) );
      } else {
         // Merge into the existing output store
         ATH_MSG_DEBUG( "Merging into existing output xAOD::TriggerMenu configuration metadata container" );
         xAOD::TriggerMenuContainer* output = nullptr;
         ATH_CHECK( outputMetaStore()->retrieve( output, m_outputKey ) );
         // Copy (and de-duplicate) from the internal collection to the output collection
         ATH_CHECK( doCopyxAODTriggerMenu(m_menu.get(), output) );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

} // namespace xAODMaker
