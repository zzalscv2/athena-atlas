/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ConditionsData/RtRelationMultChanContainer.h"

namespace TRTCond
{

  RtRelationMultChanContainer::~RtRelationMultChanContainer() 
  { 
    // delete owned objects
    clear() ; 
  }
  
  RtRelationMultChanContainer::RtRelationMultChanContainer(const RtRelationMultChanContainer& rhs) 
    :  MultChanContainer<RtRelationLayerContainer>(rhs)
  {
    *this = rhs ;
  }
  
  RtRelationMultChanContainer& RtRelationMultChanContainer::operator=(const RtRelationMultChanContainer& rhs) 
  {
    // this is a bit tricky since we own the contents. the following
    // solution is not particularly efficient, but the least amount of
    // work for me.
    
    // first delete owned objects
    clear() ;
    
    // now create a 'FlatContainer'
    FlatContainer allentries;
    rhs.getall(allentries) ;
    
    // store them one-by-one
    for(const auto & entry : allentries)
      set( entry.first, *(entry.second) ) ;
    
    return *this ;
  }

}
