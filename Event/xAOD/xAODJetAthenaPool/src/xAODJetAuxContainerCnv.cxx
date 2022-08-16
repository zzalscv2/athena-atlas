/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODJetAuxContainerCnv.h"

#include <TClass.h>
#include <mutex>


#define LOAD_DICTIONARY( name ) do {  TClass* cl = TClass::GetClass( name ); \
    if( ( ! cl ) || ( ! cl->IsLoaded() ) ) {  ATH_MSG_ERROR( "Couldn't load dictionary for type: " << name ); } } while(0)


xAOD::JetAuxContainer*
xAODJetAuxContainerCnv::
createPersistentWithKey( xAOD::JetAuxContainer* trans,
                         const std::string& key )
{
#if !(defined(GENERATIONBASE) || defined(SIMULATIONBASE))
  // ??? Still needed?
  std::once_flag flag;
  std::call_once (flag,
                  [this] {
                    LOAD_DICTIONARY( "ElementLink<DataVector<xAOD::MuonSegment_v1> >" );
                    LOAD_DICTIONARY( "std::vector<ElementLink<DataVector<xAOD::MuonSegment_v1> > >" );
                    LOAD_DICTIONARY( "std::vector<std::vector<ElementLink<DataVector<xAOD::MuonSegment_v1> > > >" );
                    LOAD_DICTIONARY( "std::vector<ElementLink<DataVector<xAOD::BTagging_v1> > >" );
                  }); 
#endif //ifndef SIMULATIONBASE OR GENERATIONBASE
  // Create a copy of the container:
  return xAODJetAuxContainerCnvBase::createPersistentWithKey (trans, key);
}

