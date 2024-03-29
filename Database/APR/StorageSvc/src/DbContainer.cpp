/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbContainerObj handle implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

// Framework include files
#include "StorageSvc/DbHeap.h"
#include "StorageSvc/DbObject.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbToken.h"
#include "StorageSvc/DbReflex.h"
#include "DbContainerObj.h"
#include "CxxUtils/checker_macros.h"

#include <memory>

using namespace std;
using namespace pool;
static const string s_empty = "";

/// Add reference count to object if present
int DbContainer::refCount() const  {
  return isValid() ? ptr()->refCount() : int(INVALID);
}

// Open container from handle
DbStatus DbContainer::open( DbDatabase&  dbH, 
                            const string& nam, 
                            const DbTypeInfo*  typ, 
                            const DbType&      dbtyp,
                            DbAccessMode mod)
{
  if ( dbH.isValid() )  {
    if ( !(dbH.openMode() == pool::READ && mod != pool::READ) ) {
      DbContainerObj* q = dbH.find(nam);
      switchPtr(q ? q : new DbContainerObj(dbH, nam, dbtyp, mod));
      if ( mod == pool::READ || mod == pool::UPDATE )  {
        if ( 0 == typ )  {
          typ = dbH.contShape(nam);
        }
      }
      if ( ptr()->open(typ).isSuccess() )  {
        return Success;
      }
      close();
    }
  }
  return Error;
}

void DbContainer::switchPtr(DbContainerObj* obj) {
  if (   obj ) obj->addRef();
  if ( m_ptr ) {
    if (m_ptr->release() == 0) m_ptr = 0;
  }
  m_ptr = obj;
  if ( m_ptr )  {
    m_type = obj->type();
  }
}

DbDatabase& DbContainer::containedIn() {
  if (!isValid()) std::abort();
  return m_ptr->database();
}

uint64_t DbContainer::size() {
  return isValid() ? m_ptr->size() : 0;
}

/// Access to access mode member
DbAccessMode DbContainer::openMode() const {
  return isValid() ? m_ptr->mode() : DbAccessMode(pool::NOT_OPEN);
}

/// Access to db name
const string& DbContainer::name() const {
  return isValid() ? m_ptr->name() : s_empty;
}

/// Close container object if handle is valid
DbStatus DbContainer::close() {
  if ( isValid() )  {
    DbStatus res = m_ptr->close();
    switchPtr(0);
    return res;
  }
  return Error;
}

/// Check if the container was opened
bool DbContainer::isOpen() const  {
  return isValid();
}

/// Execute Database Transaction Action
DbStatus DbContainer::transAct(Transaction::Action action) {
  return isValid() ? m_ptr->transAct(action) : Error;
}

/// Pass options to the implementation
DbStatus DbContainer::setOption(const DbOption& refOpt) {
  return isValid() ? m_ptr->setOption(refOpt) : Error;
}

/// Access options
DbStatus DbContainer::getOption(DbOption& refOpt) {
  return isValid() ? m_ptr->getOption(refOpt) : Error;
}

/// Start/Commit/Rollback Database Transaction
const Token* DbContainer::token() const {
  return isValid() ? m_ptr->token() : 0;
}

/// Access implementation internals
const IDbContainer* DbContainer::info() const {
  // Be sure to use the const version of info() to avoid checker warnings.
  return isValid() ? std::as_const(*m_ptr).info() : 0;
}
IDbContainer* DbContainer::info() {
  return isValid() ? m_ptr->info() : 0;
}

/// Allow access to the Database implementation
IOODatabase* DbContainer::db() {
  return isValid() ? m_ptr->db() : 0;
}

/// Retrieve persistent type information
const DbTypeInfo* 
DbContainer::objectShape(const Guid& guid) {
  return isValid() ? m_ptr->objectShape(guid) : 0;
}

/// Perform DELETE statement
DbStatus DbContainer::update(DbSelect& sel)  {
  return isValid() ? m_ptr->update(sel) : Error;
}

/// Perform DELETE statement
DbStatus DbContainer::destroy(DbSelect& sel)  {
  return isValid() ? m_ptr->destroy(sel) : Error;
}

/// Perform selection
DbStatus DbContainer::select(DbSelect& sel) {
  return isValid() ? m_ptr->select(sel) : Error;
}

/// Fetch next object address of the selection to set token
DbStatus DbContainer::fetch(DbSelect& sel) {
  return isValid() ? m_ptr->fetch(sel) : Error;
}

/// In place allocation of object location
DbStatus DbContainer::allocate(const void* object, ShapeH shape, Token::OID_t& oid) {
  if ( isValid() && object ) return m_ptr->allocate(*this, object, shape, oid);
  throw bad_alloc();
}

/// In place allocation of raw memory
void* DbContainer::allocate(unsigned long siz, const DbTypeInfo* typ) {
  if ( isValid() )  {
    void* ptr = m_ptr->allocate(siz, *this, typ);
    if ( ptr )    {
      return ptr;
    }
  }
  throw bad_alloc();
}

/// In place free of raw memory
DbStatus DbContainer::free(void* ptr) {
  return isValid() ? m_ptr->free(ptr, *this) : Error;
}

/// Destroy the persistent representation of the object
DbStatus DbContainer::destroy(const Token::OID_t& linkH)    {
  return isValid() ? m_ptr->destroy(linkH) : Error;
}

/// Load object in the container identified by its handle
DbStatus DbContainer::load( void** ptr,
                            ShapeH shape,
                            const Token::OID_t& linkH )
{
  if ( isValid() )  {
    Token::OID_t oid;
    DbStatus sc = m_ptr->load(ptr, shape, linkH, oid, false);
    return sc;
  }
  return Error;
}

/// Save new object in the container and return its handle
DbStatus DbContainer::save(const void* object, ShapeH shape, Token::OID_t& linkH) {
  return isValid() && object ? m_ptr->save(*this, object, shape, linkH) : Error;
}

/// Update object in the container identified by its pointer
DbStatus DbContainer::update(const void* object, ShapeH shape, const Token::OID_t& linkH) {
  return isValid() && object ? m_ptr->update(*this, object, shape, linkH) : Error;
}

/// Save object in the container identified by its handle
DbStatus DbContainer::_save(DbObjectHandle<DbObject>& objH, const DbTypeInfo* typ) {
  if ( isValid() && objH.isValid() )    {
    const DbContainer& cnt = objH.containedIn();
    // Object MUST be allocated on this container!
    if ( cnt.info() == m_ptr->info() ) {
      return m_ptr->save( objH, typ );
    }
  }
  return Error;
}

/// Update an object to the container identified by its handle
DbStatus 
DbContainer::_update(const DbObjectHandle<DbObject>& objH, const DbTypeInfo* typ) {
  if ( isValid() && objH.isValid() && typ )    {
    DbStatus sc = m_ptr->update(*this, 0, typ, objH);
    return sc;
  }
  return Error;
}

/// Remove the transient representation of the object from memory
DbStatus DbContainer::_remove(DbObjectHandle<DbObject>& objH)   {
  return isValid() && objH.isValid() ? m_ptr->remove(objH) : Error;
}

/// Destroy the persistent representation of the object
DbStatus DbContainer::_destroy(DbObjectHandle<DbObject>& objH)    {
  if ( isValid() && objH.isValid() ) {
    if ( m_ptr->destroy(objH.oid()).isSuccess() ) {
      return m_ptr->remove(objH);
    }
  }
  return Error;
}

/// Load object in the container identified by its link handle
DbStatus DbContainer::_load(DbObjectHandle<DbObject>& objH,
                            const Token::OID_t& linkH,
                            const DbTypeInfo* typ,
                            bool any_next)
{
   if( typ ) {
      TypeH cl = typ->clazz();
      if( cl ) {
         DbObject* ptr = DbHeap::allocate( cl.SizeOf(), this, 0, 0 );
         if( ptr ) {
            ptr = cl.Construct(ptr);
            Token::OID_t oid;
            if ( m_ptr->load(&ptr, typ, linkH, oid, any_next).isSuccess() )  {
               objH._setObject( ptr );
               objH.oid() = oid;
               return Success;
            }
            cl.Class()->Destructor(ptr, true);
            this->free( ptr );
            objH._setObject(0);
         }
      }
   }
   return Error;
}

/// Load object in the container identified by its link handle
DbStatus DbContainer::_loadNext(DbObjectHandle<DbObject>& objH,
                                Token::OID_t& linkH,
                                const DbTypeInfo* typ)
{
   if( _load( objH, linkH, typ, true ).isSuccess() ) {
      linkH =  objH.oid();
      return Success;
   }
   return Error;
}
