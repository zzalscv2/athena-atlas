// Dear emacs, this is -*- c++ -*-
// $Id: AsgMetadataTool.h 628138 2014-11-13 11:51:41Z krasznaa $
#ifndef ASGTOOLS_ASGMETADATATOOL_H
#define ASGTOOLS_ASGMETADATATOOL_H

// Local include(s):
#include "AsgTools/AsgTool.h"
#ifdef ASGTOOL_STANDALONE
#   include "AsgTools/SgTEventMeta.h"
#   include "xAODRootAccess/TVirtualIncidentListener.h"
#elif defined(ASGTOOL_ATHENA)
#   include "GaudiKernel/IIncidentListener.h"
#else
#   error "What environment are we in?!?"
#endif // Environment selection

// Forward declaration(s):
class Incident;

namespace asg {

#ifdef ASGTOOL_STANDALONE
   /// Typedef masquerading the standalone code as if it were Gaudi code
   typedef xAOD::TVirtualIncidentListener IIncidentListener;
   /// Typedef masquerading the standalone code as if it were Gaudi code
   typedef xAOD::TIncident Incident;
#endif // ASGTOOL_STANDALONE

   /// Base class for dual-use tools that provide file metadata access
   ///
   /// Didn't want to make the AsgTool base class even heavier with a feature
   /// that only very few dual-use tools will actually use. So instead decided
   /// that tools that need metadata access, will need to inherit from this base
   /// class instead of AsgTool. Very much in sync with how the class hierarchy
   /// looks like in AthenaBaseComps.
   ///
   /// The class provides StoreGate-like access to the input and output metadata
   /// store in a uniform way across Athena and ROOT. In Athena it just relies
   /// on the two StoreGateSvc instances taking care of this, and in ROOT it
   /// uses a wrapper around xAOD::TEvent to achieve the same interface.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 628138 $
   /// $Date: 2014-11-13 12:51:41 +0100 (Thu, 13 Nov 2014) $
   ///
   class AsgMetadataTool : public AsgTool,
                           public virtual IIncidentListener {

   public:
      /// Normal ASG tool constructor with a name
      AsgMetadataTool( const std::string& name );

      /// @name Definition of the StoreGate-like object's definition
      /// @{

#ifdef ASGTOOL_STANDALONE
      /// Type of the metadata store object in standalone mode
      typedef SgTEventMeta  MetaStore_t;
      /// Type of the metadata store pointer in standalone mode
      typedef SgTEventMeta* MetaStorePtr_t;
#elif defined(ASGTOOL_ATHENA)
      /// Type of the metadata store object in Athena
      typedef ServiceHandle< StoreGateSvc > MetaStore_t;
      /// Type of the metadata store pointer in standalone mode
      typedef ServiceHandle< StoreGateSvc >& MetaStorePtr_t;
#else
#   error "What environment are we in?!?"
#endif // Environment selection

      /// @}

      /// @name Functions providing access to the input/output metadata
      /// @{

      /// Accessor for the input metadata store
      MetaStorePtr_t inputMetaStore() const;
      /// Accessor for the output metadata store
      MetaStorePtr_t outputMetaStore() const;

      /// @}

      /// Function initialising the tool in the correct way in Athena
      virtual StatusCode sysInitialize();


      /// Helper function to access IOVMetaDataContainer information held in
      /// the MetaDataStore
      /// For non athena environments, this will just return StatusCode::FAILURE
      ///
      /// Note that having this function here is very bad design. :-( For now
      /// it's marked as deprecated, but Will needs to put replacement code into
      /// AthenaBaseComps to replace this...
      ///
      template< typename T >
      StatusCode retrieveMetadata( const std::string& folder,
                                   const std::string& key,
                                   T& out ) __attribute__ ((deprecated));

   protected:
      /// @name Callback functions helping in metadata reading
      /// @{

      /// Function receiving incidents from IncidentSvc/TEvent
      virtual void handle( const Incident& inc );

      /// Function called when a new input file is opened
      virtual StatusCode beginInputFile();

      /// Function called when a new events is loaded
      virtual StatusCode beginEvent();

      /// @}

   private:
      /// Object accessing the input metadata store
      mutable MetaStore_t m_inputMetaStore;
      /// Object accessing the output metadata store
      mutable MetaStore_t m_outputMetaStore;

   }; // class AsgMetadataTool

} // namespace asg

// Include the template implementation(s):
#include "AsgTools/AsgMetadataTool.icc"

#endif // ASGTOOLS_ASGMETADATATOOL_H
