///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// CondInputLoader.cxx 
// Implementation file for class CondInputLoader
/////////////////////////////////////////////////////////////////// 

#include "CondInputLoader.h"

// FrameWork includes
#include "Gaudi/Property.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "StoreGate/ReadHandle.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/IOVTime.h"
#include "AthenaKernel/IOVRange.h"
#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/IIOVSvc.h"
#include "StoreGate/CondHandleKey.h"
#include "AthenaKernel/CondContMaker.h"
#include "AthenaKernel/ITPCnvBase.h"
#include "RootUtils/WithRootErrorHandler.h"

#include "xAODEventInfo/EventInfo.h"
#include "AthenaKernel/BaseInfo.h"
#include "ICondSvcSetupDone.h"

#include "TClass.h"
#include "CxxUtils/starts_with.h"


namespace
{
  struct DataObjIDSorter {
    bool operator()( const DataObjID* a, const DataObjID* b ) { return a->fullKey() < b->fullKey(); }
  };

  // Sort a DataObjIDColl in a well-defined, reproducible manner.
  // Used for making debugging dumps.
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll )
  {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }
}

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
CondInputLoader::CondInputLoader( const std::string& name, 
                                  ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_condStore("StoreGateSvc/ConditionStore", name),
  m_condSvc("CondSvc",name),
  m_IOVSvc("IOVSvc",name),
  m_IOVDbSvc("IOVDbSvc",name),
  m_clidSvc("ClassIDSvc",name),
  m_rcuSvc("Athena::RCUSvc",name)
{
  //
  // Property declaration
  // 
  auto props = getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->name() == "ExtraOutputs" || prop->name() == "ExtraInputs") {
      prop->declareUpdateHandler
        (&CondInputLoader::extraDeps_update_handler, this);
    }
  }
}

//-----------------------------------------------------------------------------

// Athena Algorithm's Hooks
////////////////////////////
StatusCode 
CondInputLoader::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  ATH_CHECK( m_condSvc.retrieve() );
  ATH_CHECK( m_condStore.retrieve() );
  ATH_CHECK( m_clidSvc.retrieve() );
  ATH_CHECK( m_rcuSvc.retrieve() );
  ATH_CHECK( m_dictLoader.retrieve() );
  ATH_CHECK( m_tpCnvSvc.retrieve() );

  // Trigger read of IOV database
  ServiceHandle<IIOVSvc> ivs("IOVSvc",name());
  ATH_CHECK( ivs.retrieve() );

  // Update the SG keys if different from Folder Names
  ATH_CHECK( m_IOVDbSvc.retrieve() );
  std::vector<std::string> keys =  m_IOVDbSvc->getKeyList();
  IIOVDbSvc::KeyInfo info;
  DataObjIDColl handles_to_load;

  for (auto key : keys) {
    if( m_IOVDbSvc->getKeyInfo(key, info) ) {
      m_keyFolderMap[key] = info.folderName;
    } else {
      ATH_MSG_WARNING("unable to retrieve keyInfo for " << key );
    }
  }

  // We can get warnings later if we don't get this defined first.
  TClass::GetClass ("coral::AttributeList", true, false);

  for (const auto& itr : m_keyFolderMap) { //loop over keys of IOVDbSvc
    for (auto id : m_load) {
      if (id.key() == itr.second) {//CondInputLoader deals with this folder
	if (itr.second  != itr.first) {
	  ATH_MSG_DEBUG(" mapping folder " << id.key() << " to SGkey " 
			<< itr.first);
	  id.updateKey( itr.first );
	}//end if folder-name doesn't match SG key

	SG::VarHandleKey vhk(id.clid(),id.key(),Gaudi::DataHandle::Writer,
			       StoreID::storeName(StoreID::CONDITION_STORE));
	handles_to_load.emplace(vhk.fullKey());

        // Loading root dictionaries in a multithreaded environment
        // is unreliable.
        // So try to be sure all dictionaries are loaded now.
        RootType rt = loadDict (id.clid());

        // Special case for LArConditionsSubset classes.
        if (rt.Class()) {
          size_t nbases = rt.BaseSize();
          std::string pat = "LArConditionsContainer<";
          for (size_t ibase = 0;  ibase < nbases; ++ibase) {
            std::string basename = rt.BaseAt(ibase).Name();
            if (CxxUtils::starts_with (basename, pat)) {
              std::string subset = "LArConditionsSubset<" + basename.substr (pat.size(), std::string::npos);
              loadDict (subset);
              loadDict ("LArConditionsSubset_p1");
            }
          }
        }

	break; //quit loop over m_load
      } // end if CondInputLoader deals with this folder
    }//end loop over m_load
  }//end loop over m_keyFolderMap

  m_load = handles_to_load;
  m_handlesToCreate = handles_to_load;

  // Add in all the base classes known to StoreGate, in case a handle
  // is read via its base class. These will be added to the output deps,
  // and also registered with the CondSvc, as the scheduler needs this
  // info.

  std::ostringstream ost;
  ost << "Adding base classes:";
  for (auto &e : sortedDataObjIDColl (handles_to_load)) {
    // ignore empty keys
    if (e->key().empty()) continue;

    ost << "\n  + " << *e  << "  ->";
    CLID clid = e->clid();
    const SG::BaseInfoBase* bib = SG::BaseInfoBase::find( clid );
    if ( ! bib ) {
      ost << " no bases";
    } else {
      for (CLID clid2 : bib->get_bases()) {
        if (clid2 != clid) {
          std::string base("UNKNOWN");
          m_clidSvc->getTypeNameOfID(clid2,base).ignore();
          ost << " " << base << " (" << clid2 << ")";
          SG::VarHandleKey vhk(clid2,e->key(),Gaudi::DataHandle::Writer,
                               StoreID::storeName(StoreID::CONDITION_STORE));
          m_load.value().emplace(vhk.fullKey());
          // Again, make sure all needed dictionaries are loaded.
          m_dictLoader->load_type (clid2, true);
        }
      }
    }
  }
  ATH_MSG_INFO(ost.str());


  // Update the properties, set the ExtraOutputs for Alg deps
  const Gaudi::Details::PropertyBase &p = getProperty("Load");

  ATH_MSG_DEBUG("setting prop ExtraOutputs to " <<  p.toString());
  if (!setProperty("ExtraOutputs", p).isSuccess()) {
    ATH_MSG_ERROR("failed setting property ExtraOutputs");
    return StatusCode::FAILURE;
  }

  StatusCode sc(StatusCode::SUCCESS);
  std::ostringstream str;
  str << "Will create WriteCondHandle dependencies for the following DataObjects:";
  for (auto &e : sortedDataObjIDColl(m_load)) {
    str << "\n    + " << *e;
    if (e->key().empty()) {
      sc = StatusCode::FAILURE;
      str << "   ERROR: empty key is not allowed!";
    } else {
      SG::VarHandleKey vhk(e->clid(),e->key(),Gaudi::DataHandle::Writer,
                           StoreID::storeName(StoreID::CONDITION_STORE));
      if (m_condSvc->regHandle(this, vhk).isFailure()) {
        ATH_MSG_ERROR("Unable to register WriteCondHandle " << vhk.fullKey());
        sc = StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Ignoring proxy: " << vhk.key());
      m_IOVSvc->ignoreProxy(vhk.fullKey().clid(), vhk.key());
    }
  }
  ATH_MSG_INFO(str.str());

  return sc;
}

//-----------------------------------------------------------------------------

StatusCode 
CondInputLoader::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode
CondInputLoader::start()
{
  //
  // now create the CondCont<T>. This has to be done after initialize(), 
  // as we need to make sure that all Condition Objects have been instantiated
  // so as to fill the static condition obj registry via REGISTER_CC
  //
  // we use a VHK to store the info instead of a DataObjIDColl, as this saves
  // us the trouble of stripping out the storename from the key later.
  //

  m_vhk.clear();  // in case of multiple start transitions

  bool fail(false);
  for (const DataObjID* ditr : sortedDataObjIDColl (m_handlesToCreate)) {
    SG::VarHandleKey vhk(ditr->clid(),ditr->key(),Gaudi::DataHandle::Writer);
    if ( ! m_condStore->contains<CondContBase>( vhk.key() ) ){
      std::string tp("UNKNOWN");
      if (m_clidSvc->getTypeNameOfID(ditr->clid(),tp).isFailure()) {
        ATH_MSG_WARNING("unable to convert clid " << ditr->clid() << " to a classname."
                        << "This is a BAD sign, but will try to continue");
      }
      SG::DataObjectSharedPtr<DataObject> cb = 
        CondContainer::CondContFactory::Instance().Create( *m_rcuSvc, ditr->clid(), ditr->key() );
      if (cb == 0) {
        // try to force a load of libraries using ROOT
        (void)TClass::GetClass (tp.c_str());
        cb =
          CondContainer::CondContFactory::Instance().Create( *m_rcuSvc, ditr->clid(), ditr->key() );
      }
      if (cb == 0) {
        ATH_MSG_ERROR("failed to create CondCont<" << tp
                      << "> clid=" << ditr->clid()
                      << " : no factory found");
        fail = true;
      } else {
        ATH_MSG_INFO("created CondCont<" << tp << "> with key '"
                     << ditr->key() << "'");
        if (m_condStore->recordObject(cb, vhk.key(), true, false) == nullptr) {
          ATH_MSG_ERROR("while creating a CondContBase for " 
                        << vhk.fullKey());
          fail = true;
        } else {
          m_vhk.push_back(vhk);
        }
      }
    } else {
      m_vhk.push_back(vhk);
    }
  }
  
  if (fail && m_abort) {
    ATH_MSG_FATAL("Unable to setup some of the requested CondCont<T>. "
                  << "Aborting");
    return StatusCode::FAILURE;
  }

  // Let the conditions service know that we've finished creating
  // conditions containers.
  ServiceHandle<ICondSvcSetupDone> condSvcDone ("CondSvc", name());
  ATH_CHECK( condSvcDone.retrieve() );
  ATH_CHECK( condSvcDone->setupDone() );

  return StatusCode::SUCCESS;

}


//-----------------------------------------------------------------------------

StatusCode 
CondInputLoader::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  EventIDBase now;
  if (!getContext().valid()) {
    ATH_MSG_WARNING("EventContext not valid! This should not happen!");
    const xAOD::EventInfo* thisEventInfo;
    if(evtStore()->retrieve(thisEventInfo)!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Unable to get Event Info");
      return StatusCode::FAILURE;
    }
    now.set_run_number(thisEventInfo->runNumber());
    now.set_event_number(thisEventInfo->eventNumber());
    now.set_lumi_block(thisEventInfo->lumiBlock());
    now.set_time_stamp(thisEventInfo->timeStamp());
    now.set_time_stamp_ns_offset(thisEventInfo->timeStampNSOffset());
  }
  else {
    now.set_run_number(getContext().eventID().run_number());
    now.set_event_number(getContext().eventID().event_number());
    now.set_lumi_block(getContext().eventID().lumi_block());
    now.set_time_stamp(getContext().eventID().time_stamp());
    now.set_time_stamp_ns_offset(getContext().eventID().time_stamp_ns_offset());
  }

  EventIDBase now_event = now;
  now.set_event_number (EventIDBase::UNDEFEVT);

  // For a MC event, the run number we need to use to look up the conditions
  // may be different from that of the event itself.  Override the run
  // number with the conditions run number from the event context,
  // if it is defined.
  EventIDBase::number_type conditionsRun =
    Atlas::getExtendedEventContext (getContext()).conditionsRun();
  if (conditionsRun != EventIDBase::UNDEFNUM) {
    now.set_run_number (conditionsRun);
  }

  StatusCode sc(StatusCode::SUCCESS);
  std::string tag;
  for (auto &vhk: m_vhk) {
    ATH_MSG_DEBUG( "handling id: " << vhk.fullKey() << " key: " << vhk.key() );

    CondContBase* ccb(0);
    if (! m_condStore->retrieve(ccb, vhk.key()).isSuccess()) {
      ATH_MSG_ERROR( "unable to get CondContBase* for " << vhk.fullKey() 
                     << " from ConditionStore" );
      sc = StatusCode::FAILURE;
      continue;
    }
   
    if (ccb->valid(now)) {
      ATH_MSG_DEBUG( "  CondObj " << vhk.fullKey() << " is still valid at " << now_event );
      continue;
    }

    std::string dbKey = m_keyFolderMap[vhk.key()];
    if (m_IOVSvc->createCondObj( ccb, vhk.fullKey(), now ).isFailure()) {
      ATH_MSG_ERROR("unable to create Cond object for " << vhk.fullKey() << " dbKey: " 
                    << dbKey);
      sc = StatusCode::FAILURE;
      continue;
    }
  }

  if (m_dumpCondStore) {
    ATH_MSG_DEBUG(m_condStore->dump());
  }

  if (m_dumpCondSvc) {
    std::ostringstream ost;
    m_condSvc->dump(ost);
    ATH_MSG_DEBUG(ost.str());
  }
  return sc;
}

//-----------------------------------------------------------------------------

// need to override the handling of the DataObjIDs that's done by
// AthAlgorithm, so we don't inject the name of the Default Store
void 
CondInputLoader::extraDeps_update_handler( Gaudi::Details::PropertyBase& /*ExtraDeps*/ ) 
{  
  // do nothing
}

//-----------------------------------------------------------------------------

RootType CondInputLoader::loadDict (const std::string& name)
{
  // First try to load the persistent class dictionary.
  std::unique_ptr<ITPCnvBase> tpcnv = m_tpCnvSvc->t2p_cnv_unique (name);
  if (tpcnv) {
    RootType rtp = m_dictLoader->load_type (tpcnv->persistentTInfo());
    if (rtp.Class()) {
      return rtp;
    }
  }

  // Otherwise try to load the dictionary for the class itself.
  return m_dictLoader->load_type (name);
}


RootType CondInputLoader::loadDict (CLID clid)
{
  std::string name;
  if (m_clidSvc->getTypeNameOfID (clid, name).isSuccess()) {
    return loadDict (name);
  }
  return RootType();
}
