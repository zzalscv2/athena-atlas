/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATE_READCONDHANDLE_H
#define STOREGATE_READCONDHANDLE_H 1

#include "AthenaKernel/getMessageSvc.h"
#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/IOVEntryT.h"
#include "AthenaKernel/ExtendedEventContext.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/exceptions.h"
#include "PersistentDataModel/AthenaAttributeList.h"
#include "CxxUtils/AthUnlikelyMacros.h"

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <string>
#include <stdexcept>
#include <any>


namespace SG {


  /**
   * @brief Report a conditions container lookup failure.
   * @param cc The conditions container.
   * @param eid The time for which to search.
   * @param The key corresponding to the conditions container.
   */
  void ReadCondHandleNotFound (const CondContBase& cc,
                               const EventIDBase& eid,
                               const std::string& key);

  template <typename T>
  class ReadCondHandle {

  public: 
    typedef T*               pointer_type; // FIXME: better handling of
    typedef const T*   const_pointer_type; //        qualified T type ?
    typedef T&             reference_type;
    typedef const T& const_reference_type;

  public:
    ReadCondHandle(const SG::ReadCondHandleKey<T>& key);
    ReadCondHandle(const SG::ReadCondHandleKey<T>& key, 
                   const EventContext& ctx);
    
    ~ReadCondHandle() {};
    
    const std::string& key() const { return m_hkey.key(); }
    const DataObjID& fullKey() const { return m_hkey.fullKey(); }

    const_pointer_type retrieve();
    const_pointer_type retrieve( const EventIDBase& t);

    const_pointer_type  operator->()  { return  retrieve(); }
    const_pointer_type  operator*()   { return  retrieve(); }
    const_pointer_type  cptr()        { return  retrieve(); }

    
    bool isValid();
    bool isValid(const EventIDBase& t) const ;

    bool range(EventIDRange& r);
    bool range(const EventIDBase& t, EventIDRange& r) const;
    const EventIDRange& getRange();

    CondCont<T>* getCC() { return m_cc; }
    
  private:

    bool initCondHandle();
        
    EventIDBase m_eid;
    CondCont<T>*  m_cc {nullptr};
    const T* m_obj { nullptr };
    const EventIDRange* m_range { nullptr };
    
    const SG::ReadCondHandleKey<T>& m_hkey;
  };


  //---------------------------------------------------------------------------

  template <typename T>
  ReadCondHandle<T>::ReadCondHandle(const SG::ReadCondHandleKey<T>& key):
  ReadCondHandle(key, Gaudi::Hive::currentContext())
  { 
  }

  //---------------------------------------------------------------------------

  template <typename T>
  ReadCondHandle<T>::ReadCondHandle(const SG::ReadCondHandleKey<T>& key,
                                    const EventContext& ctx):
    m_eid( ctx.eventID() ),
    m_cc( key.getCC() ),
    m_hkey(key)
  {
    try {
      EventIDBase::number_type conditionsRun =
        Atlas::getExtendedEventContext(ctx).conditionsRun();
      if (conditionsRun != EventIDBase::UNDEFNUM) {
        m_eid.set_run_number (conditionsRun);
      }
    }
    catch (const std::bad_any_cast& e) {
      throw SG::ExcBadContext (ctx, key.objKey());
    }

    if (ATH_UNLIKELY(!key.isInit())) {
      throw SG::ExcUninitKey (key.clid(), key.key(), key.storeHandle().name(),
                              "", "ReadCond");
    }

    if (ATH_UNLIKELY(m_cc == 0)) {
      // try to retrieve it
      StoreGateSvc* cs = m_hkey.getCS();
      CondContBase *cb(nullptr);
      if (cs->retrieve(cb, m_hkey.key()).isFailure()) {
        throw SG::ExcNoCondCont (m_hkey.fullKey().key(), "Can't retrieve.");
      } else {
        m_cc = dynamic_cast< CondCont<T>* > (cb);
        if (m_cc == 0) {
          throw SG::ExcNoCondCont (m_hkey.fullKey().key(), "Can't dcast CondContBase.");
        }
      }
    }
  }

  //---------------------------------------------------------------------------

  template <typename T>
  bool
  ReadCondHandle<T>::initCondHandle() {

    if (m_obj != 0) return true;

    if ( ATH_UNLIKELY(!m_cc->find(m_eid, m_obj, &m_range)) ) {
      ReadCondHandleNotFound (*m_cc, m_eid, m_hkey.objKey());
      m_obj = nullptr;
      return false;
    }

    return true;
  }

  //---------------------------------------------------------------------------

  template <typename T>
  const T*
  ReadCondHandle<T>::retrieve() {
    
    if (m_obj == 0) {
      if (!initCondHandle()) {
      // std::ostringstream ost;
      // m_cc->list(ost);
      // MsgStream msg(Athena::getMessageSvc(), "ReadCondHandle");
      // msg << MSG::ERROR 
      //     << "ReadCondHandle::retrieve() could not find EventTime " 
      //     << m_eid  << " for key " << objKey() << "\n"
      //     << ost.str()
      //     << endmsg;
        return nullptr;
      }
    }

    return m_obj;
  }

  //---------------------------------------------------------------------------

  template <typename T>
  const T*
  ReadCondHandle<T>::retrieve(const EventIDBase& eid) {
    if (eid == m_eid) {
      return retrieve();
    }

    //    pointer_type obj(0);
    const_pointer_type cobj(0);
    if (! (m_cc->find(eid, cobj) ) ) {
      ReadCondHandleNotFound (*m_cc, eid, m_hkey.objKey());
      return nullptr;
    }
  
    //    const_pointer_type cobj = const_cast<const_pointer_type>( obj );

    return cobj;
  }

  //---------------------------------------------------------------------------

  template <typename T>
  bool 
  ReadCondHandle<T>::isValid()  {

    return initCondHandle();
  }

  //---------------------------------------------------------------------------

  template <typename T>
  bool 
  ReadCondHandle<T>::isValid(const EventIDBase& t) const {

    return (m_cc->valid(t));
  }

  //---------------------------------------------------------------------------
  
  template <typename T>
  bool 
  ReadCondHandle<T>::range(EventIDRange& r) {

    if (m_obj == 0) {
      if (!initCondHandle()) {
        return false;
      }
    }

    if (m_range) {
      r = *m_range;
      return true;
    }
      
    return false;
  }

  template <typename T>
  const EventIDRange&
  ReadCondHandle<T>::getRange() {

    if (m_obj == 0) {
      if (!initCondHandle()) {
        throw SG::ExcBadReadCondHandleInit();
      }
    }

    if (!m_range) {
        throw SG::ExcNoRange();
    }
    return *m_range;

  }

  
  //---------------------------------------------------------------------------
  
  template <typename T>
  bool 
  ReadCondHandle<T>::range(const EventIDBase& eid, EventIDRange& r) const {
    
    return ( m_cc->range(eid, r) );
  }

}

#endif
  
