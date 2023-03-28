/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Identifier package
 -----------------------------------------
 ***************************************************************************/

#include "Identifier/IdContext.h"
#include <assert.h>
 

IdContext::IdContext(const ExpandedIdentifier& prefix, 
                     size_type begin_index, 
                     size_type end_index)
    :
    m_prefix(prefix),
    m_begin_index(begin_index),
    m_end_index(end_index)
{}
 

IdContext::IdContext(ExpandedIdentifier&& prefix, 
                     size_type begin_index, 
                     size_type end_index)
    :
    m_prefix(std::move(prefix)),
    m_begin_index(begin_index),
    m_end_index(end_index)
{}

