/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Identifier package
 -----------------------------------------
 ***************************************************************************/


#ifndef IDENTIFIER_IDCONTEXT_H
# define IDENTIFIER_IDCONTEXT_H

#include "Identifier/ExpandedIdentifier.h"


/**
 *  class IdContext
 *
 *  This class saves the "context" of an expanded identifier
 *  (ExpandedIdentifier) for compact or hash versions (Identifier32 or
 *  IdentifierHash). This context is composed of 
 *
 *    1) begin and end indices of fields that are stored in the
 *       compact/hash id
 *    2) a possible "prefix" identifier for cases where the begin
 *       index is not 0 or the top level of the expaneded identifier. 
 *
 *  The IdContext is needed when only some of the identifier levels
 *   are to encoded in the compact/hash ids. 
 */

class IdContext
{
public:

    //
    // Define public typedefs
    //
    typedef ExpandedIdentifier::size_type 	size_type;

    // default constructor
    IdContext();
    // with no prefix
    IdContext(size_type begin_index, 
	      size_type end_index);
    // constructor with full initialization
    IdContext(const ExpandedIdentifier& prefix, 
	      size_type begin_index, 
	      size_type end_index);
    IdContext(ExpandedIdentifier&& prefix, 
	      size_type begin_index, 
	      size_type end_index);

    //
    // accessors
    //
    const ExpandedIdentifier&		prefix_id	(void) const;

    // indices of the first/last identifier fields
    size_type				begin_index	(void) const;
    size_type				end_index 	(void) const;
    
    //
    // modifiers
    //
    void				set	(const ExpandedIdentifier& prefix,
						 size_type begin_index,
						 size_type end_index);
    
private:
    
    ExpandedIdentifier	m_prefix;
    size_type		m_begin_index;
    size_type		m_end_index;
};

    

inline IdContext::IdContext()
    :
    m_begin_index(0),
    m_end_index(0)
{}

inline IdContext::IdContext(size_type begin_index, 
			    size_type end_index)
    :
    m_begin_index(begin_index),
    m_end_index(end_index)
{}

inline const ExpandedIdentifier&		
IdContext::prefix_id	(void) const
{
    return (m_prefix);
}

inline IdContext::size_type			
IdContext::begin_index	(void) const
{
    return (m_begin_index);
}

inline IdContext::size_type			
IdContext::end_index 	(void) const
{
    return (m_end_index);
}

inline void			
IdContext::set (const ExpandedIdentifier& prefix,
		size_type begin_index,
		size_type end_index)
{
    m_prefix    	= prefix;
    m_begin_index 	= begin_index;
    m_end_index 	= end_index;
}

#endif // IDENTIFIER_IDCONTEXT_H
