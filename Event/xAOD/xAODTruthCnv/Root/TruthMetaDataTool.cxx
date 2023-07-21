/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include <memory>



#include "xAODTruthCnv/TruthMetaDataTool.h"

namespace xAODMaker {

   // Helper typedef:
   using Cont_t = xAOD::TruthMetaDataContainer;

   TruthMetaDataTool::TruthMetaDataTool( const std::string& name )
      : asg::AsgMetadataTool( name ),
        m_truthMeta(), m_truthMetaAux() {

      declareProperty( "InputKey", m_inputKey = "TruthMetaData" );
      declareProperty( "OutputKey", m_outputKey = "TruthMetaData" );

#ifndef XAOD_STANDALONE
      declareInterface< ::IMetaDataTool >( this );
#endif // XAOD_STANDALONE
   }

   StatusCode TruthMetaDataTool::initialize() {

      // Greet the user:
      ATH_MSG_DEBUG( "Initialising TruthMetaDataTool" );
      ATH_MSG_DEBUG( "  InputKey  = " << m_inputKey );
      ATH_MSG_DEBUG( "  OutputKey = " << m_outputKey );

      // Reset the internal variable(s):
      m_truthMeta.reset(); m_truthMetaAux.reset();

      // Retrun gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode TruthMetaDataTool::beginInputFile() {

      // If the input file doesn't have any trigger configuration metadata,
      // then finish right away:
      if( ! inputMetaStore()->contains< Cont_t >( m_inputKey ) ) {
         return StatusCode::SUCCESS;
      }

      // Retrieve the input container:
      const Cont_t* input = nullptr;
      ATH_CHECK( inputMetaStore()->retrieve( input, m_inputKey ) );

      // Create an output container if it doesn't exist yet:
      if( ( ! m_truthMeta ) && ( ! m_truthMetaAux ) ) {
         ATH_MSG_DEBUG( "Creating output container" );
         m_truthMeta = std::make_unique<xAOD::TruthMetaDataContainer>( );
         m_truthMetaAux = std::make_unique<xAOD::TruthMetaDataAuxContainer>( );
         m_truthMeta->setStore( m_truthMetaAux.get() );
      }

      // Loop over the configurations of the input file:
      for( const xAOD::TruthMetaData* meta : *input ) {

         // Check if this configuration is already in the output container:
         bool exists = false;
         for( const xAOD::TruthMetaData* existing : *m_truthMeta ) {
            if( existing->mcChannelNumber() == meta->mcChannelNumber() ){
               const std::string generators{"generators"};
               if (!existing->isAvailable<std::string>(generators) ||
                   !meta->isAvailable<std::string>(generators) ||
                   existing->generators() == meta->generators() ) {
                     exists = true;
                     break;
               }
            }
         }
         if( exists ) {
            continue;
         }

         // If it's a new configuration, put it into the output container:
         ATH_MSG_VERBOSE( "Copying configuration with ChannelID: "
                          << meta->mcChannelNumber() );
         xAOD::TruthMetaData* out = new xAOD::TruthMetaData();
         m_truthMeta->push_back( out );
         *out = *meta;
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode TruthMetaDataTool::endInputFile() {
      return StatusCode::SUCCESS;
   }

   StatusCode TruthMetaDataTool::metaDataStop() {

      // The output may already have trigger configuration metadata in it.
      // For instance from TrigConf::xAODMenuWriter. In this case let that
      // object take precedence.
      if( outputMetaStore()->contains< Cont_t >( m_outputKey ) ) {
         ATH_MSG_DEBUG( "xAOD::TruthMetaDataContainer already in the output" );
         return StatusCode::SUCCESS;
      }

      // Record the truth metadata, if any was found in the processed input files.
      if( m_truthMeta.get() && m_truthMetaAux.get() ) {
         ATH_MSG_DEBUG( "Recording truth metadata" );
         ATH_CHECK( outputMetaStore()->record( m_truthMeta.release(),
                                               m_outputKey ) );
         ATH_CHECK( outputMetaStore()->record( m_truthMetaAux.release(),
                                               m_outputKey + "Aux." ) );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

} // namespace xAODMaker
