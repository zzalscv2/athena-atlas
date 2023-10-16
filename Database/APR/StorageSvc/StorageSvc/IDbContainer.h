/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//
//  Package    : pool/StorageSvc (The pool framework)
//
//  @author      M.Frank
//
//====================================================================
#ifndef POOL_IDBCONTAINER_H
#define POOL_IDBCONTAINER_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbObject.h"
#include "StorageSvc/Transaction.h"

#include <vector>
#include <cstdint>

/*
 *   POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class DbTypeInfo;
  class DbDatabase;
  class DbContainer;
  class DataCallBack;
  class DbOption;
  class DbSelect;

  typedef const class Shape        *ShapeH;

  /** @class IDbContainer IDbContainer.h StorageSvc/IDbContainer.h 
    *
    * Description:
    *
    * Interface to the implementation specific part of a container object
    * objects.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class IDbContainer    {
  protected:
    /// Destructor (called only by sub-classes)
    virtual ~IDbContainer()   {     }

  public:
    /// Release the technology specific implementation
    virtual void release() = 0;
    /// Access to container size
    virtual uint64_t size() = 0;
    /// Get container name
    virtual std::string name() const = 0;
    /// Ask if a given shape is supported
    virtual DbStatus isShapeSupported(const DbTypeInfo* typ) const = 0;
    /// Set options
    virtual DbStatus setOption(const DbOption& refOpt) = 0;
    /// Access options
    virtual DbStatus getOption(DbOption& refOpt) = 0;
    /// In place allocation of raw memory
    virtual void* allocate( unsigned long siz, 
                            DbContainer&  cntH,
                            ShapeH shape) = 0; 
    /// In place allocation of object location
    virtual DbStatus allocate(DbContainer& cntH, 
                              const void* object,
                              ShapeH shape, 
                              Token::OID_t& oid) = 0;
    /// In place deletion of raw memory
    virtual DbStatus free(  void* ptr,
                            DbContainer&  cntH) = 0;

    /// Number of next record in the container (=size if no delete is allowed)
    virtual uint64_t nextRecordId() = 0;
    /// Suggest next Record ID for tbe next object written - used only with synced indexes
    virtual void useNextRecordId(uint64_t) = 0;

    /// Close the container
    virtual DbStatus close() = 0;
    /// Open the container
    virtual DbStatus open(  DbDatabase&        dbH, 
                            const std::string& nam, 
                            const DbTypeInfo* info, 
                            DbAccessMode mode) = 0;
    /// Perform UPDATE select
    virtual DbStatus update(  DbSelect& sel) = 0;
    /// Perform DELETE select
    virtual DbStatus destroy( DbSelect& sel) = 0;
    /// Define selection criteria
    virtual DbStatus select(  DbSelect& criteria) = 0;
    /// Fetch next object address of the selection to set token
    virtual DbStatus fetch(DbSelect& sel) = 0;

    /// Find object within the container and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  linkH     [IN]   Preferred object OID
      * @param  oid      [OUT]   Actual object OID
      * @param  any_next  [IN]   On selection, objects may be skipped.
      *                          If objects are skipped, the actual oid
      *                          will differ from the preferred oid.
      * @return Status code indicating success or failure.
      */
    virtual DbStatus load( void** ptr, ShapeH shape, 
                           const Token::OID_t& lnkH, 
                           Token::OID_t&       oid,
                           bool                any_next=false) = 0;

    /// Save new object in the container and return its handle
    /** @param  cntH      [IN]   Handle to container object.
      * @param  object    [IN]   Pointer to feed data.
      * @param  linkH     [OUT]  Internal OID to identify object.
      *
      * @return DbStatus code indicating success or failure.
      */
    virtual DbStatus save(  DbContainer&  cntH,
                            const void* object,
                            ShapeH shape, 
                            Token::OID_t& linkH) = 0;

    /// Add object to the container
    virtual DbStatus save(  DbObjectHandle<DbObject>& objH) = 0;
    /// Update existing object in the container
    /** @param cntH      [IN]     Valid handle to container 
      * @param  object   [IN]     Pointer to feed data.
      * @param objH      [IN]     Object handle
      *
      * @return Status code indicating success or failure.
      */
    virtual DbStatus update(DbContainer&  cntH,
                            const void* object,
                            ShapeH shape, 
                            const DbObjectHandle<DbObject>& objH) = 0;

    /// Update existing object in the container
    /** @param cntH      [IN]     Valid handle to container 
      * @param  object   [IN]     Pointer to feed data.
      * @param linkH     [IN/OUT] Object identifier
      *
      * @return Status code indicating success or failure.
      */
    virtual DbStatus update(DbContainer&  cntH,
                            const void* object,
                            ShapeH shape, 
                            const Token::OID_t& linkH) = 0;

    /// Destroy an object in a container
    /** @param linkH     [IN]     Object identifier
      *
      * @return Status code indicating success or failure.
      */
    virtual DbStatus destroy( const Token::OID_t& linkH) = 0;

    /// Execute Transaction Action
    virtual DbStatus transAct(Transaction::Action) = 0;

    /// Query the pending transaction stack
    virtual bool updatesPending() const = 0;
  };
}      // End namespace pool
#endif // POOL_IDBCONTAINER_H
