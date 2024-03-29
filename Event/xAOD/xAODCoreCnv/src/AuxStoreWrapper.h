// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCORECNV_AUXSTOREWRAPPER_H
#define XAODCORECNV_AUXSTOREWRAPPER_H

// System include(s):
#include <vector>
#include <set>
#include <string>
#include <map>

// Gaudi/Athena include(s):
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

// Forward declaration(s):
namespace SG {
   class IAuxStore;
   class DataProxy;
}

namespace xAODMaker {

   /**
    *  @short Algorithm preparing auxiliary store objects for slimming
    *
    *         This algorithm can be used to update some or all of the auxiliary
    *         store objects in StoreGate to save all of their auxiliary
    *         variables as separate branches in the output TTree.
    *
    * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
    *
    */
   class ATLAS_NOT_THREAD_SAFE AuxStoreWrapper : public AthAlgorithm {

   public:
      /// Regular Algorithm constructor
      AuxStoreWrapper( const std::string& name, ISvcLocator* svcLoc );

      /// Function initialising the algorithm
      virtual StatusCode initialize();
      /// Function executing the algorithm
      virtual StatusCode execute();

   private:
      /// Create a new container store
      StatusCode changeContainer( const SG::IAuxStore* store,
                                  const std::string& name  );
      /// Create a new element store
      StatusCode changeElement( const SG::IAuxStore* store,
                                const std::string& name );

      /// StoreGate keys of the store objects to be wrapped
      std::vector< std::string > m_keys;
      /// Keys of the objects to be wrapped, in a set container
      std::set< std::string > m_keysSet;

      /// Flag specifying whether the internal caches were set already
      bool m_cachesSet;
      /// Keys of the auxiliary stores that need to be converted in the end
      std::set< std::string > m_toConvert;
      /// Keys and CLIDs of the interface containers touched
      std::map< std::string, CLID > m_clids;

      /// Handle to the Class ID service
      ServiceHandle< ::IClassIDSvc > m_clidSvc;

   }; // class AuxStoreWrapper

} // namespace xAODMaker

#endif // XAODCORECNV_AUXSTOREWRAPPER_H
