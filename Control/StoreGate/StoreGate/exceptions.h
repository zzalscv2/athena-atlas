// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/exceptions.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Exceptions that can be thrown from StoreGate.
 */


#ifndef STOREGATE_EXCEPTIONS_H
#define STOREGATE_EXCEPTIONS_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "CxxUtils/sgkey_t.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/DataHandle.h"
#include <stdexcept>
#include <typeinfo>
#include <string>


namespace SG {


/**
 * @brief Exception --- Attempt to dereference a Read/Write/UpdateHandle with a null key.
 *
 * An explicit key must always be given when using Read/Write/UpdateHandle.
 */
class ExcNullHandleKey
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcNullHandleKey();
};


/**
 * @brief Throw a SG::ExcNullHandleKey exception.
 */
[[noreturn]]
void throwExcNullHandleKey();


/**
 * @brief Exception --- Bad key format for VarHandleKey.
 *
 * The key for a VarHandle must be of the form KEY or STORE/KEY;
 * no more than one slash may be present in the key string.
 */
class ExcBadHandleKey
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param key The supplied key string.
   */
  ExcBadHandleKey (const std::string& key);
};


/**
 * @brief Exception --- Forbidden method called.
 */
class ExcForbiddenMethod
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param name Name of the called method.
   */
  ExcForbiddenMethod (const std::string& name);
};


/**
 * @brief Exception --- Error initializing VarHandle from VarHandleKey.
 */
class ExcHandleInitError
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcHandleInitError (CLID clid,
                      const std::string& sgkey,
                      const std::string& storename);
};


/**
 * @brief Exception --- Tried to create a handle from an uninitialized key.
 */
class ExcUninitKey
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   * @param holdername IDataHandleHolder holding the key.
   * @param htype Handle type.
   */
  ExcUninitKey (CLID clid,
                const std::string& sgkey,
                const std::string& storename,
                const std::string& holdername = "",
                const std::string& htype = "VarHandle");
};


/**
 * @brief Exception --- Tried to retrieve non-const pointer to const object.
 */
class ExcConstObject
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcConstObject (CLID clid,
                  const std::string& sgkey,
                  const std::string& storename);
};


/**
 * @brief Exception --- Attempt to dereference write handle before record.
 */
class ExcNullWriteHandle
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcNullWriteHandle (CLID clid,
                      const std::string& sgkey,
                      const std::string& storename);
};


/**
 * @brief Throw a SG::ExcNullWriteHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
[[noreturn]]
void throwExcNullWriteHandle (CLID clid,
                              const std::string& sgkey,
                              const std::string& storename);


/**
 * @brief Exception --- Deference of read handle failed.
 */
class ExcNullReadHandle
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcNullReadHandle (CLID clid,
                     const std::string& sgkey,
                     const std::string& storename);
};


/**
 * @brief Throw a SG::ExcNullReadHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
[[noreturn]]
void throwExcNullReadHandle (CLID clid,
                             const std::string& sgkey,
                             const std::string& storename);


/**
 * @brief Exception --- Deference of update handle failed.
 */
class ExcNullUpdateHandle
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcNullUpdateHandle (CLID clid,
                       const std::string& sgkey,
                       const std::string& storename);
};


/**
 * @brief Throw a SG::ExcNullUpdateHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
[[noreturn]]
void throwExcNullUpdateHandle (CLID clid,
                               const std::string& sgkey,
                               const std::string& storename);


/**
 * @brief Exception --- Attempt to get non-const VarHandleKey from non-owning VarHandle.
 *
 * If a Read/Write/Update handle is initialized from a HandleKey object, then
 * that HandleKey is const, and cannot be retrieved as a non-const reference.
 * A non-const HandleKey can only be retrieved if the handle was not
 * initialized from a HandleKey.
 */
class ExcNonConstHandleKey
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid CLID from the key.
   * @param sgkey StoreGate key from the key.
   * @param storename Store name from the key.
   */
  ExcNonConstHandleKey (CLID clid,
                        const std::string& sgkey,
                        const std::string& storename);
};


/**
 * @brief Throw a SG::ExcNonConstHandleKey exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
[[noreturn]]
void throwExcNonConstHandleKey (CLID clid,
                                const std::string& sgkey,
                                const std::string& storename);


/**
 * @brief Exception --- Deference invalid SG::Iterator.
 */
class ExcInvalidIterator
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcInvalidIterator();
};


/**
 * @brief Exception --- Initialization of InitializedReadHandleKey failed.
 */
class ExcBadInitializedReadHandleKey
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcBadInitializedReadHandleKey();
};


/**
 * @brief Exception --- Bad EventContext extension while building ReadCondHandle.
 *
 * The EventContext was not set or is of the wrong type.
 */
class ExcBadContext
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param ctx The bad EventContext.
   * @param key The key of the handle being built.
   */
  ExcBadContext (const EventContext& ctx, const std::string& key);
};


/**
 * @brief Exception --- Can't retrieve CondCont from ReadCondHandle.
 *
 * The CondCont was not in the conditions store or is of the wrong type.
 */
class ExcNoCondCont
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param key The key being looked up.
   * @param why Further description.
   */
  ExcNoCondCont (const std::string& key, const std::string& why);
};


/**
 * @brief Exception --- ReadCondHandle didn't initialize in getRange().
 */
class ExcBadReadCondHandleInit
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcBadReadCondHandleInit();
};


/**
 * @brief Exception --- Range not set in ReadCondHandle::getRange().
 */
class ExcNoRange
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcNoRange();
};




/**
 * @brief Exception --- DecorHandle given an element not in the requested container.
 */
class ExcBadDecorElement
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param mode Reader or Writer, depending on the handle type.
   * @param clid CLID from the handle.
   * @param decorKey Decoration key in CONTAINER.DECOR format.
   */
  ExcBadDecorElement (Gaudi::DataHandle::Mode mode,
                      CLID clid,
                      const std::string& decorKey);
};


/**
 * @brief Throw a SG::ExcBadDecorElement exception.
 */
[[noreturn]]
void throwExcBadDecorElement (Gaudi::DataHandle::Mode mode,
                              CLID clid,
                              const std::string& decorKey);


} // namespace SG


#endif // not STOREGATE_EXCEPTIONS_H
