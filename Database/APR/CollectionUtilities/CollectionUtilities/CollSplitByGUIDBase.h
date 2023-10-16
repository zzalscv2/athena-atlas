/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONuTILITIES_COLLSPLITGUIDBASE_H
#define COLLECTIONuTILITIES_COLLSPLITGUIDBASE_H

 
#include "CollectionBase/ICollection.h"

#include "CollectionUtilities/CollectionPool.h"
#include "CollectionUtilities/Args2Container.h"
#include "CollectionUtilities/SrcInfo.h"
#include "CollectionUtilities/QueryInfo.h"
#include "CollectionUtilities/CatalogInfo.h"
#include "CxxUtils/checker_macros.h"


#include <string>
#include <map>

namespace pool
{

   class CollectionService;

   class ATLAS_NOT_THREAD_SAFE CollSplitByGUIDBase
   {
  public:
     CollSplitByGUIDBase( const std::string& name = "CollSplitByGUID" );
     virtual ~CollSplitByGUIDBase();

     CollSplitByGUIDBase( const CollSplitByGUIDBase& ) = delete;
     CollSplitByGUIDBase& operator= ( const CollSplitByGUIDBase& ) = delete;

     virtual bool	init( std::vector<std::string> argv_v );
     
     virtual int	execute( std::vector<std::string> argv_v );

     /// use a different collection pool handler than the default one
     virtual void	setCollectionPool( CollectionPool* );


     
     std::string 	m_thisProgram;

     /// maximum number of allowed output collections
     int  		m_maxSplit;

     /// name of the Token attribute that is used for splitting
     std::string 	m_splitRef;

     // src collection info
     std::vector<int> 	m_srcCountVec;
     
     // dst collection info
     std::vector<bool> 	m_dstCollExistVec;
     
     // output modifiers
     int       		m_minEvents;
     
     //unsigned int numEventsPerCommit = 10000;
     int       		m_rowsCached;
     
     int         	m_numEventsPerCommit;
     int 		m_numRowsCached;
     
     std::vector<std::string> m_inputQuery;

      // Classes handling command line options
     CatalogInfo 	m_catinfo;
     QueryInfo   	m_queryinfo; 
     SrcInfo     	m_srcinfo; 
    
     pool::CollectionService* m_collectionService;
     coral::MessageStream m_log;

     // Vector of args
     Args2Container m_argsVec;

     std::vector< pool::ICollection* > 	m_srcCollections;

     /// pool of output collections
     CollectionPool*   	m_collectionPool;
 
  protected:
     virtual void	openSourceCollections();
     virtual void	openDestCollections();
     virtual void	copyRows();
     virtual void	finalize();

     /// read user-prepared list of GUIDs and output collection names
     virtual bool 	readGuidList( const std::string& filename );
     /// generate next output collection name
     virtual std::string generateNextCollName( );
     /// get a user-specified output collection for a given GUID (empty string if none given)
     virtual std::string collectionNameForGuid( const std::string& guid );

     
     // map GUID->output collection name (read from guid list file)
     std::map<std::string,std::string>  CollNameforGuidMap;

     // sequence counter to generate unique output collection names
     int     		m_outputCollSeqN;

   };
}

#endif
