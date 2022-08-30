//
// Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "AsgExampleTools/EventStoreTestTool.h"

// EDM include(s).
#include "xAODBase/IParticleContainer.h"
#include "xAODCore/CLASS_DEF.h"

// System include(s).
#include <memory>
#include <string>

namespace asg {

   EventStoreTestTool::EventStoreTestTool( const std::string& toolName )
   : asg::AsgTool( toolName ) {

   }

   StatusCode EventStoreTestTool::performTest() const {

      // Create an IParticle container.
      auto particles = std::make_unique< xAOD::IParticleContainer >();
      const xAOD::IParticleContainer* particlesPtr = particles.get();

      // Make sure that the getName(...) and getKey(...) functions don't
      // know about it at this point yet.
      if( getName( particlesPtr ) != "" ) {
         ATH_MSG_ERROR( "There is a problem with the asg::AsgTool::getName "
                        "function" );
         return StatusCode::FAILURE;
      }
      if( getKey( particlesPtr ) != 0 ) {
         ATH_MSG_ERROR( "There is a problem with the asg::AsgTool::getKey "
                        "function" );
         return StatusCode::FAILURE;
      }

      // Record the container into the event store with some elaorate name.
      static const std::string PARTICLES_NAME = "AsgTestIParticles";
      ATH_CHECK( evtStore()->record( std::move( particles), PARTICLES_NAME ) );

      // Depending on the environment, figure out what the hashed key for this
      // object is supposed to be.
#ifdef XAOD_STANDALONE
      const SG::sgkey_t particlesKey =
         evtStore()->event()->getHash( PARTICLES_NAME );
#else
      static const CLID IPARTICLE_CLID =
         ClassID_traits< xAOD::IParticleContainer >::ID();
      const SG::sgkey_t particlesKey =
         evtStore()->stringToKey( PARTICLES_NAME, IPARTICLE_CLID );
#endif // XAOD_STANDALONE

      // Now check that the getName(...) and getKey(...) functions would
      // return the expected values.
      if( getName( particlesPtr ) != PARTICLES_NAME ) {
         ATH_MSG_ERROR( "There is a problem with the asg::AsgTool::getName "
                        "function" );
         return StatusCode::FAILURE;
      }
      if( getKey( particlesPtr ) != particlesKey ) {
         ATH_MSG_ERROR( "There is a problem with the asg::AsgTool::getKey "
                        "function" );
         return StatusCode::FAILURE;
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace asg
