/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDatabaseObj object implementation
//--------------------------------------------------------------------
//
//  Package    : System (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

#include "StorageSvc/DbString.h"
#include "DbDatabaseObj.h"
#include "DbContainerObj.h"

// Public POOL include files
#include "POOLCore/DbPrint.h"
#include "StorageSvc/DbIter.h"
#include "StorageSvc/DbToken.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/IOODatabase.h"
#include "StorageSvc/IDbDatabase.h"
#include "StorageSvc/IDbContainer.h"

#include <memory>
#include <cstdio>


using namespace pool;

std::ostream& operator << (std::ostream& os, const Token::OID_t oid ) {
   os << "("<<oid.first<<","<<oid.second<<")";
   return os;
}

static const Guid s_localDb("00000000-0000-0000-0000-000000000000");
static const DbPrintLvl::MsgLevel dbg_lvl = DbPrintLvl::Debug;


// Standard Constructor
DbDatabaseObj::DbDatabaseObj( DbDomain&       dom, 
                              const std::string&   pfn, 
                              const std::string&   fid, 
                              DbAccessMode    mod) 
: Base(fid, mod, dom.type(), dom.db()), m_dom(dom), 
  m_info(0), m_string_t(0), m_fileAge(0)
{
  DbPrint log( m_dom.name() );
  m_logon = pfn;
  std::unique_ptr<DbToken> tok(new DbToken());
  tok->setTechnology(dom.type().type());
  tok->setClassID(Guid::null());
  tok->setDb(fid);
  tok->oid().first  = INVALID;
  tok->oid().second = INVALID;
  tok->setKey(DbToken::TOKEN_FULL_KEY);
  tok->setKey(DbToken::TOKEN_CONT_KEY);
  m_token = tok.release();
  if ( 0 == db() )    {
    log << DbPrintLvl::Error
        << "->  Access   DbDatabase   " << accessMode(mode())  
        << " [" << type().storageName() << "] " << name() 
        << " impossible."
        << DbPrint::endmsg
        << "                          " << logon()
        << DbPrint::endmsg;
    type().missingDriver(log);
    return;
  }
  if ( !m_dom.add( name(), this ).isSuccess() )    {
    log << DbPrintLvl::Error
        << "->  Access   DbDatabase   " << accessMode(mode())  
        << " [" << type().storageName() << "] " << name() 
        << " impossible."
        << DbPrint::endmsg
        << "                          " << logon()
        << " Error inserting DbDatabaseObj into domain!"
        << DbPrint::endmsg;
    return;
  }
  log << DbPrintLvl::Info
      << "->  Access   DbDatabase   " << accessMode(mode())  
      << " [" << type().storageName() << "] " << name() 
      << DbPrint::endmsg;
  if ( logon() != name() )  {
    log  << "                          " << logon() << DbPrint::endmsg;
  }
  DbString s;
  DbTypeInfo::Columns c;
  c.push_back(new DbColumn("db_string",DbColumn::STRING,size_t((std::string*)&s)-size_t(&s),0,1,0));
  m_string_t = DbTypeInfo::create(std::string("pool::DbString"), c);
  if ( m_string_t ) m_string_t->addRef();
}

// Standard Destructor
DbDatabaseObj::~DbDatabaseObj()  {
  clearEntries();
  cleanup();
  if( m_string_t ) {
     m_string_t->deleteRef();
     m_string_t = 0;
  }
  m_dom.remove(this);
  m_token->release();
}

/// Access the size of the database: May be undefined for some technologies
long long int DbDatabaseObj::size() {
  if ( 0 == m_info )    {  // Re-open the database if it was retired
     open();
  }
  return 0==m_info ? -1 : m_info->size();
}

// Perform cleanup of internal structures.
DbStatus DbDatabaseObj::cleanup()  {
  for(LinkVector::iterator i=m_linkVec.begin(); i != m_linkVec.end(); ++i) {
    delete (*i);
  }
  for(ShapeMap::iterator j=m_shapeMap.begin(); j != m_shapeMap.end(); ++j) {
    ((*j).second)->deleteRef();
  }
  m_linkMap.clear();
  m_linkVec.clear();
  m_indexMap.clear();
  m_shapeMap.clear();
  m_paramMap.clear();
  m_classMap.clear();
  if ( m_info )   {
    deletePtr( m_info );
    DbPrint log( m_dom.name() );
    log << DbPrintLvl::Info
        << "->  Deaccess DbDatabase   " << accessMode(mode())  
        << " [" << type().storageName() << "] " << name()
        << DbPrint::endmsg;
  }
  return Success;
}

// Add association entry
DbStatus DbDatabaseObj::makeLink(Token* pTok, Token::OID_t& refLnk) {
  if ( pTok )   {
    int   is_dbTok  = (typeid(*pTok) == typeid(DbToken));
    LinkMap::iterator i;
    if ( is_dbTok )   {
      DbToken* pdbTok = static_cast<DbToken*>(pTok);
      pdbTok->setKey(DbToken::TOKEN_CONT_KEY);
      i = m_linkMap.find(pdbTok->contKey());
    }
    else  {
      Guid tmp_key;
      DbToken::makeKey(pTok, DbToken::TOKEN_CONT_KEY, tmp_key);
      i = m_linkMap.find(tmp_key);
    }
    if ( i != m_linkMap.end() )   {
      DbToken* t = (*i).second;
      refLnk.first  = t->oid().first;
      refLnk.second = pTok->oid().second;
      return Success;
    }
    else if ( mode() != pool::READ ) {
      const Guid& dbn = pTok->dbID();
      std::unique_ptr<DbToken> link(new DbToken());
      link->fromString(pTok->toString());
      link->oid().first = m_linkVec.size();
      link->setKey(DbToken::TOKEN_FULL_KEY);
      link->setKey(DbToken::TOKEN_CONT_KEY);
      // Add the persistent entry to the links container
      if ( 0 != m_string_t )   {
        DbPrint log( m_logon );
        log << dbg_lvl 
            << "--->Adding Assoc :" << link->dbID() 
            << "/" << link->contID() 
            << " [" << std::hex << link->technology() << "] " 
            << " (" << link->oid().first << " , " << link->oid().second
            << ")" << DbPrint::endmsg;
        log << "---->ClassID:" << link->classID().toString() << DbPrint::endmsg;
        refLnk.first  = link->oid().first;
        refLnk.second = pTok->oid().second;
        if ( dbn == name() )  {
          link->setDb(s_localDb);
	  link->setLocal(true);
        }
        // Update link to use persistent oid
        link->oid().first = m_links->info()->nextRecordId() + 2; // Taking into account unsaved ##Container links
        DbHandle<DbString> persH = new(m_links, m_string_t) DbString(link->toString());
        if ( !m_links.save(persH, m_string_t).isSuccess() )    {
          return Error;
        }
        persH.ptr()->~DbString(); m_links.free(persH.ptr());
        link->setDb(dbn);
        // Update the transient list of links
	m_linkMap.insert( LinkMap::value_type(link->contKey(), link.get()));
        m_indexMap.insert( IndexMap::value_type(link->oid().first, m_linkVec.size()));
        m_linkVec.push_back( link.release() );
        return Success;
      }
    }
  }
  return Error;
}

// Retrieve shape information for a specified object by shape ID
const DbTypeInfo* DbDatabaseObj::objectShape(const Guid& id)  {
  if ( 0 == m_info ) open();
  ShapeMap::const_iterator i = m_shapeMap.find(id);
  if( i != m_shapeMap.end() ) return (*i).second;
  if( id == m_string_t->shapeID() ) return m_string_t;
  return nullptr;
}

// Retrieve shape information for a specified object by reflection handle
const DbTypeInfo* DbDatabaseObj::objectShape(const TypeH& id)  {
  if ( 0 == m_info ) open();
  std::map<TypeH, const DbTypeInfo*>::const_iterator i = m_classMap.find(id);
  if( i != m_classMap.end() ) return i->second;
  if( id == m_string_t->clazz() or id.Name() == "string" ) {
     // hack to enable reading DbStrings from KeyContainer::fetch()
     return m_string_t;
  }
  return nullptr;
}

// Retrieve shape information for a specified object by container name
const DbTypeInfo* DbDatabaseObj::contShape(const std::string& nam) {
  if ( 0 == m_info )    {
    open();
  }
  LinkVector::const_iterator j=m_linkVec.begin();
  for(; j != m_linkVec.end(); ++j ) {
    DbToken* t = (*j);
    if ( !t->typeInfo() )    {
      t->setTypeInfo(objectShape(t->classID()));
    }
    if ( t->typeInfo() )    {
      if ( t->dbID() == name() && t->contID() == nam )  { // in ##Links
        return t->typeInfo();
      }
    }
  }
  return 0;
}

// Add persistent shape to the Database
DbStatus DbDatabaseObj::addShape (const DbTypeInfo* pShape) {
  if ( pShape )    {
    const Guid& id = pShape->shapeID();
    ShapeMap::iterator i = m_shapeMap.find(id);
    if ( i != m_shapeMap.end() )   {
      return Success;
    }
    else if ( m_string_t and (pShape == m_string_t) )  {
      return Success;
    }
    else if ( m_string_t and (id == m_string_t->shapeID()) )  {
      return Success;
    }
    else if ( mode() != pool::READ )  {
      const std::string& dsc = pShape->toString();
      // Add the persistent entry to the links container
      if ( 0 != m_string_t )   {
        DbPrint log( m_logon );
        //log << DbPrint::Always << persH->c_str() << DbPrint::endmsg;
        // Update the transient list of links
        // This must be done BEFORE the entry 
        // is inserted into the container!
        // Otherwise save will add the type 
        // again and again ending in an 
        // infinite recursion
        const DbTypeInfo *pShape2 = DbTypeInfo::fromString(dsc);
        const DbTypeInfo::Columns& cols = pShape2->columns();
        log << dbg_lvl << "--->Adding Shape[" << m_shapeMap.size() << " , "
            << pShape2->shapeID().toString() << "]: ";
        log << " [" << cols.size() << " Column(s)] " << DbPrint::endmsg;        
        if ( pShape2->clazz() ) {
          log << "---->Class:" << DbReflex::fullTypeName(pShape2->clazz()) << DbPrint::endmsg;
        } else {
          log << "---->Class:" << "<not availible>" << DbPrint::endmsg;
        }
        for (size_t ic=0; ic < cols.size();++ic)  {
          const DbColumn* c = cols[ic];
          log << "---->[" << ic << "]:" << c->name()
              << " Typ:" << c->typeName() << " ["<< c->typeID() << ']'
              << " Size:" << c->size()
              << " Offset:" << c->offset()
              << " #Elements:" << c->nElement()
              << DbPrint::endmsg;
        }
        bool inserted = m_shapeMap.insert( ShapeMap::value_type(id, pShape2) ).second;
        if ( pShape2 == m_string_t || id == m_string_t->shapeID() )   {
          return Success;
        }
        DbHandle<DbString> persH = new(m_shapes, m_string_t) DbString(dsc);
        if ( !m_shapes.save(persH, m_string_t).isSuccess() )  {
          i = m_shapeMap.find(id);
          m_shapeMap.erase(i);
          return Error;
        }
        persH.ptr()->~DbString(); m_shapes.free(persH.ptr());
        if ( inserted ) pShape2->addRef();
        if ( pShape2->clazz() )  {
          m_classMap.insert(std::make_pair(pShape2->clazz(), pShape2));
        }
        return Success;
      }
    }
  }
  return Error;
}

// Open Database object
DbStatus DbDatabaseObj::open()   {
  if ( !m_info && m_dom.isValid() && db() )    {
    DbPrint log( m_logon );
    log << dbg_lvl;
    m_info = db()->createDatabase();
    if ( m_info->open(m_dom, m_logon, mode()).isSuccess() )    {
      // Age open databases. Aging is only effective
      // for read-only databases. Otherwise no aging
      // is applied, because it is assumed, that objects
      // with pending connections may still be written.
      setAge(0);
      if ( 0==(mode()&pool::CREATE) && 0==(mode()&pool::UPDATE) )  {
        m_dom.ageOpenDbs();
        setAge(0);
        m_dom.closeAgedDbs();
      }
      if ( 0 != m_string_t )   {
        DbDatabase dbH(this);
        const Guid& guid = m_string_t->shapeID();

        // Add link to "##Shapes" container
        std::unique_ptr<DbToken> l1(new DbToken());
        l1->setDb(name());
        l1->setCont("##Shapes");
        l1->setTechnology(type().type());
        l1->setClassID(guid);
        l1->oid().first  = m_linkVec.size();
        l1->oid().second = INVALID;
        l1->setKey(DbToken::TOKEN_FULL_KEY);
        l1->setKey(DbToken::TOKEN_CONT_KEY);
        // Update the transient list of links
        m_linkMap.insert( LinkMap::value_type(l1->contKey(), l1.get()));
        m_indexMap.insert( IndexMap::value_type(l1->oid().first, m_linkVec.size()));
        m_linkVec.push_back( l1.release() );

        // Add link to "##Links" container
        std::unique_ptr<DbToken> l2(new DbToken());
        l2->setDb(name());
        l2->setCont("##Links");
        l2->setTechnology(type().type());
        l2->setClassID(guid);
        l2->oid().first  = m_linkVec.size();
        l2->oid().second = INVALID;
        l2->setKey(DbToken::TOKEN_FULL_KEY);
        l2->setKey(DbToken::TOKEN_CONT_KEY);
        // Update the transient list of links
        m_linkMap.insert( LinkMap::value_type(l2->contKey(), l2.get()));
        m_indexMap.insert( IndexMap::value_type(l2->oid().first, m_linkVec.size()));
        m_linkVec.push_back( l2.release() );

        if ( m_shapes.open(dbH,"##Shapes",m_string_t,type(),mode()).isSuccess() )    {
          DbIter<DbString> it;
          for ( it.scan(m_shapes, m_string_t); it.next().isSuccess(); ) {
            //log << DbPrint::Always << "Oid=" << (*it).oid().first << (*it).oid().second << " " << **it << DbPrint::endmsg;
            const DbTypeInfo* pShape = DbTypeInfo::fromString(**it);
            const DbTypeInfo::Columns& cols = pShape->columns();
            log << "--->Reading Shape[" << m_shapeMap.size() << " , "
                << pShape->shapeID().toString() << "]: ";
            log << "[" << cols.size() << " Column(s)]" << DbPrint::endmsg;
            for (size_t ic=0; ic < cols.size();++ic)  {
              const DbColumn* c = cols[ic];
              log << "---->[" << ic << "]:" << c->name()
                  << " Typ:" << c->typeName() << " ["<< c->typeID() << ']'
                  << " Size:" << c->size()
                  << " Offset:" << c->offset()
                  << " #Elements:" << c->nElement()
                  << DbPrint::endmsg;
            }
            // Update the transient list of links
            if( m_shapeMap.insert(ShapeMap::value_type(pShape->shapeID(), pShape)).second )
               pShape->addRef();
            const bool noIdScan = true;
            if ( pShape->clazz(noIdScan) )  {
               m_classMap.insert(std::make_pair(pShape->clazz(), pShape));
            }
            it.object()->~DbString(); m_shapes.free(it.object());
          }
        }

        if ( m_links.open(dbH,"##Links",m_string_t,type(),mode()).isSuccess() )  {
          DbIter<DbString> it;
          for ( it.scan(m_links, m_string_t); it.next().isSuccess(); )   {
            std::unique_ptr<DbToken> link(new DbToken());
            link->fromString(**it);
            // Update the transient list of links
            if ( s_localDb == link->dbID() ) {
              link->setDb(name());
	      link->setLocal(true);
            }
            log << "--->Reading Assoc:" << link->dbID() 
                << "/" << link->contID() 
                << " [" << std::hex << link->technology() << "] " 
                << " (" << link->oid().first << " , " << link->oid().second
                << ")" << DbPrint::endmsg;
            log << "---->ClassID:" << link->classID().toString() << DbPrint::endmsg;
            link->setKey(DbToken::TOKEN_FULL_KEY);
            link->setKey(DbToken::TOKEN_CONT_KEY);
	    if ( m_linkMap.find(link->contKey()) == m_linkMap.end() )  {
	      m_linkMap.insert( LinkMap::value_type(link->contKey(), link.get()));
	    }
            m_indexMap.insert( IndexMap::value_type(link->oid().first, m_linkVec.size()));
	    m_linkVec.push_back(link.release());
            it.object()->~DbString(); m_links.free(it.object());
          }
        }
        
        if ( m_params.open(dbH,"##Params",m_string_t,type(),mode()).isSuccess() )    {
	  std::vector<std::string> fids;
          DbIter<DbString> it;
          //it.scan(m_params, m_string_t);
          //it.next();
          for ( it.scan(m_params, m_string_t); it.next().isSuccess(); )   {
            std::string dsc = **it;
            size_t id1 = dsc.find("[NAME=");
            size_t id2 = dsc.find("[VALUE=");
            if ( id1 != std::string::npos && id2 != std::string::npos )  {
              size_t id11 = dsc.find(']', id1+6);
              size_t id22 = dsc.find(']', id2+7);
              if ( id11 != std::string::npos && id22 != std::string::npos )  {
                std::string n = dsc.substr(id1+6, id11-id1-6);
                std::string v = dsc.substr(id2+7, id22-id2-7);
                // ParamMap::value_type val(n, v);
                log << "--->Reading Param:" << n << "=[" << v << ']' 
                    << DbPrint::endmsg;
                m_paramMap[n] = v;
                if (n == "FID") fids.push_back(v);
              }
            }
            it.object()->~DbString(); m_params.free(it.object());
          }
	  // We assume that the last FID is the true FID of the file...
	  ParamMap::const_iterator fidIt = m_paramMap.find("FID");
	  if ( fidIt != m_paramMap.end() ) {
	    const std::string& fid = (*fidIt).second;
	    for(size_t i=0; fids.size()>0 && i<fids.size()-1;++i)  {	    
	      char num[32];
	      ::sprintf(num, "FID.%d", int(i+1));
	      log << "--->Redirect FID[" << i << "]: " << fids[i] 
		  << " to " << fid << DbPrint::endmsg;
	      m_paramMap[num] = fid;
	    }
	  }
	}
        if ( mode()&pool::CREATE || mode()&pool::UPDATE)  {
          std::string par_val;
          if ( !param("FID", par_val).isSuccess() )  {
            if ( !addParam("FID", name()).isSuccess() )  {
              log << "Failed to write parameter FID=" << name() << DbPrint::endmsg;
            }
          }
          if ( !param("PFN", par_val).isSuccess() )  {
            if ( !addParam("PFN", m_logon).isSuccess() )  {
              log << "Failed to write parameter PFN=" << m_logon << DbPrint::endmsg;
            }
          }
          if ( !param("POOL_VSN", par_val).isSuccess() )  {
            if ( !addParam("POOL_VSN", "1.1").isSuccess() )  {
              log << "Failed to write parameter POOL_VSN." << DbPrint::endmsg;
            }
          }
        }
        DbDatabase dbd (this);
        return m_info->onOpen(dbd, mode());
      }
    }
    deletePtr(m_info);
    return Error;
  }
  return Success;
}

/// Re-open database with changing access permissions
DbStatus DbDatabaseObj::reopen(DbAccessMode mod) {
  DbPrint log(m_logon);
  if (mod == pool::READ || mod == pool::UPDATE )  {
    if ( mode() != mod )   {
      if ( mod == pool::READ )  {
        for (const_iterator i=begin(); i != end(); ++i )  {
          if ( !(*i).second->updatesPending() )  {
            log << DbPrintLvl::Error << "Cannot change mode of " << name()
                << " to " << accessMode(mod) 
                << " if updates are queued."
                << DbPrint::endmsg;
            return Error;
          }
        }
      }
      setMode(mod);
      DbStatus sc = (0==m_info) ? open() : m_info->reopen(mod);
      if ( sc.isSuccess() )   {
        for (const_iterator i=begin(); i != end(); ++i )  {
          (*i).second->cancelTransaction();
          (*i).second->setMode(mod);
        }
        return sc;
      }
      log << DbPrintLvl::Error << "Failed to reopen the database " << name()
          << " in mode " << accessMode(mod) 
          << DbPrint::endmsg;
      return sc;
    }
    log << DbPrintLvl::Debug << "Database already open in the requested access mode."
        << DbPrint::endmsg;
    for (const_iterator i=begin(); i != end(); ++i )  {
      (*i).second->setMode(mod);
    }
    return Success;
  }
  log << DbPrintLvl::Error << "A database can only be re-opened in "
      << " UPDATE or READ mode!"
      << DbPrint::endmsg;
  return Error;
}

/// Close Database object
DbStatus DbDatabaseObj::close()  {
  DbStatus sc = retire();
  std::vector<DbContainerObj*> conts;
  for (const_iterator j=begin(); j != end(); ++j )  {
    DbContainerObj* curr = (*j).second;
    conts.push_back(curr);
  }
  for(std::vector<DbContainerObj*>::iterator i=conts.begin(); i != conts.end(); ++i)  {
    DbContainerObj* curr = (*i);
    if ( curr->isOpen() ) curr->close();
    this->remove(curr);
  }
  clearEntries();
  m_dom.remove(this);
  return sc;
}

/// Check for pending updates
bool DbDatabaseObj::updatesPending() const  {
  for (const_iterator i=begin(); i != end(); ++i )  {
    if ( (*i).second->updatesPending() )  {
      return true;
    }
  }
  return false;
}

/// Close Database object
DbStatus DbDatabaseObj::retire()  {
  if ( !updatesPending() )  {
    DbPrint log( m_logon);
    log << DbPrintLvl::Info << "Database being retired..." << DbPrint::endmsg;

    if (m_links.isValid()) m_links.close();
    if (m_shapes.isValid()) m_shapes.close();
    if (m_params.isValid()) m_params.close();
    for (const_iterator j=begin(); j != end(); ++j )  {
      DbContainerObj* curr = (*j).second;
      curr->retire();
    }
    DbStatus ret = Success;
    if ( m_info )    {
      ret = m_info->close(mode());
    }
    cleanup();
    m_fileAge = 0;
    return ret;
  }
  return Error;
}

/// Retrieve the number of user parameters
int DbDatabaseObj::nParam() {
  if ( 0 == m_info )    {  // Re-open the database if it was retired
    open();
  }
  return 0 == m_info ? -1 : int(m_paramMap.size());
}

/// Add a persistent parameter to the file
DbStatus DbDatabaseObj::addParam(const std::string& nam, const std::string& val) {
  if ( !nam.empty() && !val.empty() ) {
    if ( 0 == m_info ) open();
    if ( m_info )  {
      ParamMap::const_iterator i = m_paramMap.find(nam);
      if ( i == m_paramMap.end() )  {
        std::string dsc = "[NAME=" + nam + "][VALUE=" + val + ']';
        DbHandle<DbString> persH = new(m_params, m_string_t) DbString(dsc);
        if ( !m_params.save(persH, m_string_t).isSuccess() )  {
          return Error;
        }
        persH.ptr()->~DbString(); m_params.free(persH.ptr());
        m_paramMap.insert(ParamMap::value_type(nam, val));
        return Success;
      }
      return (*i).second == val ? Success : Error;
    }
  }
  return Error;
}

/// Retrieve existing parameter by name
DbStatus DbDatabaseObj::param(const std::string& nam, std::string& val)  {
  if ( 0 == m_info ) open();
  if ( m_info ) {
    ParamMap::const_iterator i = m_paramMap.find(nam);
    if ( i == m_paramMap.end() )  {
      return Error;
    }
    val = (*i).second;
    return Success;
  }
  return Error;
}

/// Retrieve all parameters
DbStatus DbDatabaseObj::params(Parameters& vals)   {
  vals.clear();
  if ( 0 == m_info ) open();
  if ( m_info ) {
    ParamMap::const_iterator i = m_paramMap.begin();
    for ( ; i != m_paramMap.end(); ++i )  {
      vals.push_back(*i);
    }
    return Success;
  }
  return Error;
}

/// Expand OID into a full Token, based on the Links table.
DbStatus DbDatabaseObj::getLink(const Token::OID_t& oid, Token* pTok)
{
   if ( 0 == m_info ) open();
   if ( 0 != m_info && 0 != pTok && oid.first >= 0 ) {
      pTok->oid() = oid;
      if( !(pTok->type() & DbToken::TOKEN_FULL_KEY) )  {
         if( typeid(*pTok) == typeid(DbToken) )  {
	    DbToken* pdbTok = (DbToken*)pTok;
	    pdbTok->setKey(DbToken::TOKEN_FULL_KEY);
         }
      }
      m_linkVec[ oid.first ]->set(pTok);
      return Success;
   }
   return Error;
}


std::string DbDatabaseObj::cntName(Token& token) {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    int lnk = m_indexMap[token.oid().first]; // Map link to index
    if ( lnk >= 0 )  {
      if ( lnk < int(m_linkVec.size()) )   {
	DbToken* link = m_linkVec[lnk];
        if ( link != 0 ) {
          if ( token.contID().empty() ) {
            token.setCont(link->contID());
          }
          return link->contID(); // in ##Links
        }
      }
    }
  }
  return "";
}

DbStatus DbDatabaseObj::read(const Token& token, ShapeH shape, void** object) 
{
   if( 0 == m_info ) open();
   if( 0 != m_info ) {
      Token::OID_t oid = token.oid();
      std::string containerName = token.contID();
      if( token.dbID() == name() ) {
         // Regular read operation, make sure we know the container name
         if( containerName.empty() ) {
            auto iter = m_indexMap.find(oid.first);
            if( iter != m_indexMap.end() ) {
               containerName = m_linkVec[ iter->second ]->contID();
            } else {
               if( unsigned(oid.first) < m_indexMap.size() ) {
                  // try a direct link table access
                  containerName = m_linkVec[ oid.first ]->contID();
               } else {
                  DbPrint log( name() );
                  log << DbPrintLvl::Error << "OID1 not found in the index redirection map. Token=" << token.toString()
                      << DbPrint::endmsg;
                  return Error;
               }
            }
         }
      }
      else {
         return Error;
      }

      DbContainer cntH( type() );
      const DbTypeInfo* typ_info = objectShape( token.classID() );
      DbDatabase dbd (this);

      if( cntH.open( dbd, containerName, typ_info, token.technology(), mode() ).isSuccess() )  {
         if ( typ_info && typ_info == shape ) {
            return DbObjectAccessor::read(object, shape, cntH, oid );
         }
         DbPrint log( name() );
         log << DbPrintLvl::Error << "Token ClassID " << token.classID().toString()
             << " is different from requested Shape " << shape->shapeID().toString() << DbPrint::endmsg;
      }
   }
   return Error;
}


/// Allow access to all known containers
DbStatus DbDatabaseObj::containers(std::vector<const Token*>& conts,bool with_internals)  {
  conts.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->oid().second == INVALID )  {
        if ( with_internals || (*j)->contID()[0] != '#' )  { // in ##Links
          conts.push_back(*j);
        }
      }
    }
    return Success;
  }
  return Error;
}

/// Allow access to all known containers
DbStatus DbDatabaseObj::containers(std::vector<IDbContainer*>& conts,bool with_internals)  {
  conts.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
     for (iterator i=begin(); i != end(); ++i )    {
        DbContainerObj* c = (*i).second;
        if( c == m_links.ptr() || c == m_params.ptr() || c == m_shapes.ptr() )
           if( not with_internals) continue;
        if( c->info() ) conts.push_back( c->info() );
     }
     return Success;
  }
  return Error;
}


/// Access local container token (if container exists)
const Token* DbDatabaseObj::cntToken(const std::string& cntName)  {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->contID() == cntName )  { // in ##Links
        return (*j);
      }
    }
  }
  return 0;
}

/// Allow access to all known shapes used by the database
DbStatus DbDatabaseObj::shapes(std::vector<const DbTypeInfo*>& shaps)  {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    shaps.clear();
    for(ShapeMap::iterator j=m_shapeMap.begin(); j != m_shapeMap.end(); ++j) {
      shaps.push_back((*j).second);
    }
    return Success;
  }
  return Error;
}

/// Allow access to all known associations between containers
DbStatus DbDatabaseObj::associations(std::vector<const Token*>& assocs) {
  assocs.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->oid().second != INVALID )  {
        assocs.push_back(*j);
      }
    }
    return Success;
  }
  return Error;
}

/// Execute Database Transaction action
DbStatus DbDatabaseObj::transAct(Transaction::Action action)  {
  bool upda = (0 != (mode()&pool::CREATE) || 0 != (mode()&pool::UPDATE));
  if ( 0 != m_info )  {
    DbStatus iret, status = Success;
    for (iterator i=begin(); i != end(); ++i )    {
      DbContainerObj* c = (*i).second;
      if( c == m_links.ptr() || c == m_params.ptr() || c == m_shapes.ptr() ) continue;
      iret = c->transAct( action );
      if ( !iret.isSuccess() ) {
         status = iret;
      }
    }
    if ( status.isSuccess() )  {
      status = m_params.transAct(action);
    }
    if ( status.isSuccess() )  {
      status = m_shapes.transAct(action);
    }
      if ( status.isSuccess() )  {
      status = m_links.transAct(action);
    }
    // now execute the action on the DB implementation
    if ( status.isSuccess() )  {
      status = m_info->transAct(action);
    }
    return status;
  }
  else if ( upda )  {
    DbPrint err( name());
    err << DbPrintLvl::Error << "The database:" << name() 
        << " was not opened properly. Commit failed."
        << DbPrint::endmsg;
    return Error;
  }
  else  {
    // This means, that the database is retired.
    // Only READONLY databases may be retired.
    // Should be safe: Pending updates were checked on Re-open
    for (iterator i=begin(); i != end(); ++i )    {
      (*i).second->cancelTransaction();
    }
    return Success;
  }
}

/// Pass options to the implementation
DbStatus DbDatabaseObj::setOption(const DbOption& refOpt) {
  if ( 0 == m_info ) open();   // Re-open the database if it was retired
  return (0==m_info) ? Error : m_info->setOption(refOpt);
}

/// Pass options to the implementation
DbStatus DbDatabaseObj::getOption(DbOption& refOpt) {
  if ( 0 == m_info ) open();   // Re-open the database if it was retired
  return (0==m_info) ? Error : m_info->getOption(refOpt);
}

/// Update database age
void DbDatabaseObj::setAge(int value) {
  if ( 0 == m_info )  {
    m_fileAge = 0;
  }
  else if (m_fileAge >= 0) {
    switch ( value )  {
    case 0:
      m_fileAge = 0;
      return;
    case 1:
      ++m_fileAge;
      return;
    case -1:
      --m_fileAge;
      return;
    default:
      m_fileAge += value;
      return;
    }
  }
}
