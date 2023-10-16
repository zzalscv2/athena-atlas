/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_ICOLLECTION_H
#define COLLECTIONBASE_ICOLLECTION_H

#include <string>

namespace pool {

  class ICollectionDescription;
  class ICollectionSchemaEditor;
  class ICollectionDataEditor;
  class ICollectionQuery;

  /** 
   * @class ICollection ICollection.h CollectionBase/ICollection.h
   *
   * An interface to a storage technology specific collection of event references
   * This class may also be used as an interface to a 
   * "collection fragment" which, by definition, contains a subset of the event level 
   * metadata, does not contain the event reference and which can reference an existing 
   * collection or collection fragment and/or be referenced by a chain of collection 
   * fragments to form a collection or collection fragment with extended schema.
   */
   class ICollection
  {
  public:
    /// Enumeration of the possible open modes of the collection.
    typedef enum { CREATE,
                   CREATE_MISSING_FRAGMENTS,
		   CREATE_AND_OVERWRITE,
		   UPDATE,
		   READ } OpenMode;

    /// Enumeration of the possible types of allowable accesses to the collection.
    typedef enum { READ_ACCESS,
                   UPDATE_ACCESS,
		   WRITE_ACCESS,
		   DELETE_ACCESS } Privilege;

    /// Returns the open mode of the collection for the present transaction.
    virtual ICollection::OpenMode openMode() const = 0;

    /// Opens the collection and initializes it if necessary.
    virtual void open() = 0;

    /// Checks if the collection is open.
    virtual bool isOpen() const = 0;

    /// Commits the latest changes made to the collection.
    virtual void commit( bool restartTransaction = true ) = 0;

    /// Closes the collection and terminates any database connections.
    virtual void close() = 0;

    /// Returns an object used to describe the collection properties.
    virtual const ICollectionDescription& description() const = 0;

    /// Returns an object used to modify the collection schema.
    virtual ICollectionSchemaEditor& 	schemaEditor() = 0;

    /// Returns an object used to add, update or delete rows of the collection.
    virtual ICollectionDataEditor& 	dataEditor() = 0;

    /// Returns an object used to query the collection.
    virtual ICollectionQuery* 		newQuery() = 0;
    
    /// Empty destructor.
    virtual ~ICollection() {}

  };

}

#endif
