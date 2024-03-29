/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ConditionsData/RtRelationContainer.h"

namespace TRTCond
{

  RtRelationContainer::~RtRelationContainer() 
  { 
    // delete owned objects
    clear() ; 
  }
  
  RtRelationContainer::RtRelationContainer(const RtRelationContainer& rhs) 
    : NestedContainer<ExpandedIdentifier::DETECTOR, RtRelation*, RtRelationTrait>(rhs)
  { 
    *this = rhs ; 
  }

  RtRelationContainer& RtRelationContainer::operator=(const RtRelationContainer& rhs) 
  {
    // this is a bit tricky since we own the contents. the following
    // solution is not particularly efficient, , but the least amount
    // of work for me.
    
    // first delete owned objects
    clear() ;

    // now create a 'FlatContainer'
    FlatContainer allentries;
    rhs.getall(allentries) ;
    
    // store them one-by-one
    for(const auto & allentrie : allentries)
      set( allentrie.first, *(allentrie.second) ) ;

    return *this ;
  }

}
