/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/** @file AddressRemappingSvc.cxx
 *  @brief This file contains the implementation for the AddressRemappingSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AddressRemappingSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IClassIDSvc.h"

#include "AthContainersInterfaces/IConstAuxStore.h"
#include "AthenaKernel/IRCUSvc.h"
#include "AthenaKernel/ClassID_traits.h"

#include "SGTools/DataProxy.h"
#include "SGTools/TransientAddress.h"
#include "AthLinks/tools/DataProxyHolder.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "CxxUtils/checker_macros.h"

#include <algorithm>

#include "boost/range.hpp"


//________________________________________________________________________________
AddressRemappingSvc::AddressRemappingSvc(const std::string& name, ISvcLocator* pSvcLocator) :
  extends1<AthService, Athena::IInputRename>(name, pSvcLocator),
  m_clidSvc("ClassIDSvc", name),
  m_RCUSvc("Athena::RCUSvc", name),
  m_oldTads(),
  m_newTads()
{}

//__________________________________________________________________________
AddressRemappingSvc::~AddressRemappingSvc() {
}
//__________________________________________________________________________
StatusCode AddressRemappingSvc::initialize() {
   ATH_MSG_VERBOSE("Initializing " << name());

   ATH_CHECK( ::AthService::initialize() );
   ATH_CHECK( m_clidSvc.retrieve() );
   ATH_CHECK( m_RCUSvc.retrieve() );
   //ATH_CHECK( m_algResourcePool.retrieve() );

   m_oldTads.clear();
   m_newTads.clear();
   for (const std::string& overwrite : m_overwriteMaps) {
      const std::string::size_type p_sep = overwrite.find("->");
      if (p_sep == std::string::npos) {
         ATH_MSG_ERROR("Unexpected format in TypeKeyOverwriteMaps: " << overwrite);
         return(StatusCode::FAILURE);
      }
      //const std::pair<std::string, std::string> entry(overwrite.substr(0, p_sep), overwrite.substr(p_sep + 2));
      const std::pair<std::string, std::string> entry(overwrite.substr(p_sep + 2), overwrite.substr(0, p_sep));
      ATH_MSG_INFO("TypeKeyOverwriteMaps for: " << entry.second
                   << " -> " << entry.first);
      const std::string::size_type p_oldSep = entry.first.find('#');
      if (p_oldSep == std::string::npos) {
         ATH_MSG_ERROR("Unexpected format in TypeKeyOverwriteMaps: " << overwrite);
         return(StatusCode::FAILURE);
      }

      std::string clidStr = entry.first.substr(0, p_oldSep);
      std::set<CLID> symClids;
      for (std::string::size_type p_clidSep = clidStr.rfind(','); p_clidSep != std::string::npos; clidStr.resize(p_clidSep), p_clidSep = clidStr.rfind(',')) {
         symClids.insert(getClid(clidStr.substr(p_clidSep + 1)));
      }
      std::string keyStr = entry.first.substr(p_oldSep + 1);
      std::set<std::string> aliases;
      for (std::string::size_type p_keySep = keyStr.rfind(','); p_keySep != std::string::npos; keyStr.resize(p_keySep), p_keySep = keyStr.rfind(',')) {
         aliases.insert(keyStr.substr(p_keySep + 1));
      }
      SG::TransientAddress oldTad(getClid(clidStr), keyStr);
      for (CLID clid : symClids) {
         oldTad.setTransientID(clid);
      }
      oldTad.setAlias(aliases);

      const std::string::size_type p_newSep = entry.second.find('#');
      if (p_newSep == std::string::npos) {
         ATH_MSG_ERROR("Unexpected format in TypeKeyOverwriteMaps: " << overwrite);
         return(StatusCode::FAILURE);
      }
      SG::TransientAddress newTad(getClid(entry.second.substr(0, p_newSep)), entry.second.substr(p_newSep + 1));
      ATH_MSG_INFO("TypeKeyOverwriteMaps for: " << newTad.clID()
                   << "#" << newTad.name()
                   << " -> " << oldTad.clID()
                   << "#" << oldTad.name());
      m_oldTads.push_back(std::move(oldTad));
      m_newTads.push_back(std::move(newTad));
   }

   m_inputRenames = m_RCUSvc->newrcu<InputRenameMap_t>();
   ATH_CHECK( initInputRenames() );
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
/**
 * @brief Merge in additional input rename mappings.
 * @param toadd Mappings to add.
 *
 * Additional sgkey->sgkey input rename mappings are merged into the rename map,
 * using RCU synchronization.
 */
void AddressRemappingSvc::addInputRenames (const InputRenameMap_t& toadd) const
{
  if (toadd.empty()) return;
  // Ok, managed via RCU.
  InputRenameRCU_t& inputRenames ATLAS_THREAD_SAFE = *m_inputRenames;
  Athena::RCUUpdate<InputRenameMap_t> u (inputRenames);
  auto newmap = std::make_unique<InputRenameMap_t> (*u);
  for (const auto& p : toadd)
    newmap->insert (p);
  u.update (std::move (newmap));
}
//__________________________________________________________________________
/**
 * @brief Set up input rename mappings during initialization.
 */
StatusCode AddressRemappingSvc::initInputRenames()
{
  // Accumulate renames into this map.
  InputRenameMap_t newmap;

  // Parse input properties.
  for (const std::string& remap : m_typeKeyRenameMaps)
  {
    std::string::size_type pos1 = remap.find('#');
    std::string::size_type pos2 = remap.find("->");
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 < pos1)
    {
      ATH_MSG_ERROR ( "Bad InputRemaps string: " << remap );
      continue;
    }

    std::string cname = remap.substr (0, pos1);
    std::string from = remap.substr (pos1+1, pos2-pos1-1);
    std::string to = remap.substr (pos2+2, std::string::npos);
    CLID clid = getClid (cname);
    if (clid == CLID_NULL) {
      ATH_MSG_ERROR( "Can't find CLID for InputRemaps string: " << remap );
      continue;
    }

    // We have a request to rename FROM to TO for CLID.
    // Translate to sgkeys and add to the map.
    SG::sgkey_t from_key = m_proxyDict->stringToKey (from, clid);
    SG::sgkey_t to_key = m_proxyDict->stringToKey (to, clid);
    newmap[from_key] = Athena::IInputRename::Rename { to_key, to };
  }

  // Publish the map.
  addInputRenames (newmap);

  if (!m_typeKeyRenameMaps.empty()) {
    SG::DataProxyHolder::setInputRenameMap (m_inputRenames.get());
    Athena::RCURead<InputRenameMap_t> r (*m_inputRenames);
    SG::AuxTypeRegistry::instance().setInputRenameMap (&*r,
                                                       *m_proxyDict);
  }

  return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode AddressRemappingSvc::finalize()
{
   ATH_CHECK( m_clidSvc.release() );
   ATH_CHECK( m_proxyDict.release() );
   ATH_CHECK( m_RCUSvc.release() );
   ATH_CHECK( m_algResourcePool.release() );
   ATH_CHECK( ::AthService::finalize() );
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode AddressRemappingSvc::preLoadAddresses(StoreID::type storeID,
	IAddressProvider::tadList& tads)
{
  return preLoadAddressesConst (storeID, tads);
}
StatusCode AddressRemappingSvc::preLoadAddressesConst(StoreID::type storeID,
	IAddressProvider::tadList& tads) const
{
   if (storeID == StoreID::EVENT_STORE) {
     ATH_CHECK( renameTads (tads) );
   }

   const std::vector<SG::TransientAddress>* oldTads = nullptr;
   const std::vector<SG::TransientAddress>* newTads = nullptr;

   if (m_skipBadRemappings) {
     // We need to have the cleaned lists built in loadAddresses.
     // If that hasn't already been done, return now.
     if (!m_oldTadsCleaned.isValid() || ! m_newTadsCleaned.isValid()) {
       return StatusCode::SUCCESS;
     }
     oldTads = m_oldTadsCleaned.ptr();
     newTads = m_newTadsCleaned.ptr();
   }
   else {
     // No cleaning needed.
     oldTads = &m_oldTads;
     newTads = &m_newTads;
   }

   StatusCode status = m_proxyDict.retrieve();
   if (!status.isSuccess()) {
      ATH_MSG_ERROR("Unable to get the IProxyDict");
      return(status);
   }
   for (std::vector<SG::TransientAddress>::const_iterator oldIter = oldTads->begin(),
		   newIter = newTads->begin(), oldIterEnd = oldTads->end();
		   oldIter != oldIterEnd; ++oldIter, ++newIter) {
      SG::TransientAddress* tadd = new SG::TransientAddress(*oldIter);
      tads.push_back(tadd);
      m_proxyDict->stringToKey(oldIter->name(), oldIter->clID());
      m_proxyDict->stringToKey(newIter->name(), newIter->clID());
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode AddressRemappingSvc::loadAddresses(StoreID::type storeID,
                                              IAddressProvider::tadList& tads)
{
  if (storeID == StoreID::EVENT_STORE) {
    initDeletes();
    ATH_CHECK( renameTads (tads) );
  }
  return loadAddressesConst (tads);
}

StatusCode AddressRemappingSvc::loadAddressesConst(IAddressProvider::tadList& tads) const
{
   if(!m_skipBadRemappings) return(StatusCode::SUCCESS);
   if (m_oldTadsCleaned.isValid() && m_newTadsCleaned.isValid()) {
     return StatusCode::SUCCESS;
   }

   std::vector<SG::TransientAddress> oldTadsCleaned = m_oldTads;
   std::vector<SG::TransientAddress> newTadsCleaned = m_newTads;

   //do same as in preLoadAddresses, except check each tad will have a valid proxy to remap to 
   StatusCode status = m_proxyDict.retrieve();
   if (!status.isSuccess()) {
      ATH_MSG_ERROR("Unable to get the IProxyDict");
      return(status);
   }
   for (std::vector<SG::TransientAddress>::iterator oldIter = oldTadsCleaned.begin(),
		   newIter = newTadsCleaned.begin(), oldIterEnd = oldTadsCleaned.end();
		   oldIter != oldIterEnd; ++oldIter, ++newIter)
   {
      CLID goodCLID = newIter->clID(); //newIter are the things we are remapping to 
      SG::TransientAddress::TransientClidSet clidvec(oldIter->transientID());
      std::set<CLID> clidToKeep (clidvec.begin(), clidvec.end());
      //try dataproxy, if it fails, try data proxy of next type 
      SG::DataProxy* dataProxy(m_proxyDict->proxy(goodCLID,newIter->name()/*the name of the address in the input file*/));
      if(dataProxy==0) {
         //remove goodCLID from the clidToKeep 
         clidToKeep.erase(goodCLID);
         //try base types 
         const SG::BaseInfoBase* bi = SG::BaseInfoBase::find (newIter->clID());
         if (bi) {
            for (CLID goodCLID : bi->get_bases()) {
               dataProxy = m_proxyDict->proxy(goodCLID, newIter->name());
               if(dataProxy) break;
               clidToKeep.erase(goodCLID); //was a bad CLID, so get rid of it
            }
         }
         if(dataProxy) {
            ATH_MSG_DEBUG("Remapping " << oldIter->clID() << "#" << oldIter->name() << " to base-class CLID: " << goodCLID << "#" << newIter->name());
            //modify newIter to have these restricted clid instead 
            *newIter = SG::TransientAddress(goodCLID, newIter->name());
         }
      }
      if(dataProxy==0) {
         ATH_MSG_INFO("Skipping remapping of " << oldIter->clID() << "#" << oldIter->name() << " because no suitable remapping found");
         continue;
      }
      SG::TransientAddress* tadd = new SG::TransientAddress(goodCLID,oldIter->name(),oldIter->address());
      tadd->setAlias(oldIter->alias());//match the aliases  
      //also match the transientIDs, but only for the ok CLID 
      for(auto tIter = clidToKeep.begin(); tIter != clidToKeep.end(); ++tIter) {
         if(goodCLID == *tIter) continue; //already added in constructor 
         tadd->setTransientID(goodCLID);
      }
      //replace oldIter tad with this new tadd if we have changed the primary CLID
      if(oldIter->clID()!=tadd->clID()) {
         *oldIter = *tadd; //relies on assignment operator
      }
      tads.push_back(tadd);
      m_proxyDict->stringToKey(oldIter->name(), oldIter->clID());
      m_proxyDict->stringToKey(newIter->name(), newIter->clID());
   }


   //only do this once, not every event .. FIXME: Should I be rechecking every new file? For now, we assume will not chain together different sample types
   m_oldTadsCleaned.set (std::move (oldTadsCleaned));
   m_newTadsCleaned.set (std::move (newTadsCleaned));

   return StatusCode::SUCCESS;

}
//________________________________________________________________________________
StatusCode AddressRemappingSvc::updateAddress(StoreID::type storeID,
					      SG::TransientAddress* tad,
                                              const EventContext& ctx)
{
  return updateAddressConst (storeID, tad, ctx);
}
StatusCode AddressRemappingSvc::updateAddressConst(StoreID::type /*storeID*/,
                                                   SG::TransientAddress* tad,
                                                   const EventContext& /*ctx*/) const
{
   const std::vector<SG::TransientAddress>* oldTads = nullptr;
   const std::vector<SG::TransientAddress>* newTads = nullptr;

   if (m_skipBadRemappings && m_oldTadsCleaned.isValid() && m_newTadsCleaned.isValid()) {
     oldTads = m_oldTadsCleaned.ptr();
     newTads = m_newTadsCleaned.ptr();
   }
   else {
     // No cleaning needed.
     oldTads = &m_oldTads;
     newTads = &m_newTads;
   }

   for (std::vector<SG::TransientAddress>::const_iterator oldIter = oldTads->begin(),
		   newIter = newTads->begin(), oldIterEnd = oldTads->end();
		   oldIter != oldIterEnd; ++oldIter, ++newIter) {
      if (oldIter->transientID(tad->clID())
	      && (oldIter->name() == tad->name() || oldIter->alias().find(tad->name()) != oldIter->alias().end())) {
         ATH_MSG_DEBUG("Overwrite for: " << tad->clID() << "#" << tad->name() << " -> " << newIter->clID() << "#" << newIter->name());
         SG::DataProxy* dataProxy(m_proxyDict->proxy(newIter->clID(), newIter->name()));
         if (dataProxy == 0) {
            ATH_MSG_WARNING("Cannot get proxy for: " << newIter->clID() << "#" << newIter->name());
            return(StatusCode::FAILURE);
         }
         GenericAddress* newGadd = 0;
	 if (dataProxy->isValidAddress()) newGadd = dynamic_cast<GenericAddress*>(dataProxy->address());
         if (newGadd == 0) {
            ATH_MSG_WARNING("Cannot get GenericAddress for: " << newIter->clID() << "#" << newIter->name());
            return(StatusCode::FAILURE);
         }
         GenericAddress* oldGadd = new GenericAddress(newGadd->svcType(), tad->clID(), *newGadd->par(), tad->name(), *newGadd->ipar());
         tad->setAddress(oldGadd);
         return(StatusCode::SUCCESS);
      }
   }
   return(StatusCode::FAILURE);
}
//_______________________________________________________________________________
/**
 * @brief Scan TAD list and carry out any requested renamings.
 * @param tads list of TADs from previous providers.
 */
StatusCode AddressRemappingSvc::renameTads (IAddressProvider::tadList& tads) const
{
  // Fetch the current rename map using RCU.
  Athena::RCURead<InputRenameMap_t> r (*m_inputRenames);

  // We can exit early if there's nothing to do.
  // FIXME: m_deletes will almost never be empty, but many times it will have
  // no overlap with the input file.  Should try to speed up by
  // noticing/caching that somehow.
  if (r->empty() && m_deletes.empty()) return StatusCode::SUCCESS;

  // We may discover additional remappings due to autosymlinking.
  // Accumulate them here.
  InputRenameMap_t newmap;

  // Loop over all TADs.
  IAddressProvider::tadList::iterator pos = tads.begin();
  IAddressProvider::tadList::iterator end = tads.end();
  while (pos != end) {
    SG::TransientAddress*& tad = *pos;

    // Has the object described by this TAD been renamed?
    // FIXME: Handle renaming aliases.
    CLID tad_clid = tad->clID();
    SG::sgkey_t sgkey = m_proxyDict->stringToKey (tad->name(), tad_clid);
    auto it = r->find (sgkey);
    if (it != r->end()) {
      const std::string& name_renamed = it->second.m_key;

      // We're doing a renaming.  Make a new TAD with the new name.
      auto tad_new = std::make_unique<SG::TransientAddress>
        (tad_clid, name_renamed,
         tad->address(), tad->clearAddress());
      for (CLID clid : tad->transientID()) {
        if (clid == tad_clid) continue;
        tad_new->setTransientID (clid);
        SG::sgkey_t from_key = m_proxyDict->stringToKey (tad->name(), clid);
        if (r->find(from_key) == r->end()) {
          SG::sgkey_t to_key = m_proxyDict->stringToKey (name_renamed, clid);
          newmap[from_key] = Athena::IInputRename::Rename { to_key, name_renamed };
        }
      }

      size_t namelen = tad->name().size();
      SG::TransientAddress::TransientAliasSet newAliases;
      for (const std::string& a : tad->alias()) {
        std::string a_renamed = a;

        SG::sgkey_t from_key = m_proxyDict->stringToKey (a_renamed, tad_clid);
        auto it_a = r->find (from_key);
        if (it_a != r->end()) {
          a_renamed = it_a->second.m_key;
        }

        if (strncmp (a_renamed.c_str(), tad->name().c_str(), namelen) == 0 &&
            a_renamed[namelen] == '.')
        {
            a_renamed = name_renamed + a_renamed.substr (namelen, std::string::npos);
        }
          
        newAliases.insert (a_renamed);
      }
      tad_new->setAlias (newAliases);
      tad_new->setProvider (tad->provider(), tad->storeID());

      // Replace the old TAD in the list with the new one.
      delete tad;
      tad = tad_new.release();

      ++pos;
    }

    else if (isDeleted (*tad)) {
      // Rename the existing TAD to end in _DELETED.
      // Drop symlinks in the process.
      auto tad_new = std::make_unique<SG::TransientAddress>
        (tad->clID(), tad->name() + "_DELETED",
         tad->address(), tad->clearAddress());
      tad_new->setProvider (tad->provider(), tad->storeID());
      // Preserve alias information, also renaming them to end in _DELETED.
      for (const std::string& a : tad->alias()) {
        tad_new->setAlias (a + "_DELETED");
      }
      // Replace the old TAD in the list with the new one.
      delete tad;
      tad = tad_new.release();

      ++pos;
    }

    else {
      // Check for renaming of an alias; eg., for renaming a dynamic vbl.
      SG::TransientAddress::TransientAliasSet aset = tad->alias();
      for (const std::string& a : aset) {
        SG::sgkey_t from_key = m_proxyDict->stringToKey (a, tad_clid);
        auto it_a = r->find (from_key);
        if (it_a != r->end()) {
          const std::string& s = it_a->second.m_key;
          tad->removeAlias (a);
          tad->setAlias (s);
        }
      }

      ++pos;
    }
  }

  // Publish additional remappings.
  addInputRenames (newmap);

  return StatusCode::SUCCESS;
}


//________________________________________________________________________________
CLID AddressRemappingSvc::getClid(std::string type) const {
   CLID clid(atoi(type.c_str())); // Is type a CLID? 
   if (clid == 0) { // or not
      if (!m_clidSvc->getIDOfTypeName(type, clid).isSuccess()) {
         return(0);
      }
   }
   return(clid);
}
//______________________________________________________________________________
const AddressRemappingSvc::InputRenameRCU_t*
AddressRemappingSvc::inputRenameMap() const
{
  if (m_typeKeyRenameMaps.empty())
    return nullptr;
  return m_inputRenames.get();
}
//______________________________________________________________________________
void AddressRemappingSvc::initDeletes()
{
  if (m_haveDeletes) return;

  std::list<IAlgorithm*> algs = m_algResourcePool->getFlatAlgList();

  std::scoped_lock lock (m_deletesMutex);
  if (m_haveDeletes) return;
  m_haveDeletes = true;

  for (IAlgorithm* ialg : algs) {
    if (Gaudi::Algorithm* alg = dynamic_cast<Gaudi::Algorithm*> (ialg)) {
      // Need to ignore SGInputLoader; it'll have output deps
      // on everything being read.
      if (alg->name() == "SGInputLoader") continue;

      // Also ignore ViewDataVerifier algs, since they have no real output
      if (alg->type() == "AthViews::ViewDataVerifier" ) continue;

      for (const DataObjID& dobj : alg->outputDataObjs()) {
        static const std::string pref = "StoreGateSvc+";
        if (dobj.key().rfind(pref, 0) != std::string::npos) {
          std::string key = dobj.key().substr (pref.size());
          m_deletes.emplace (key, dobj.clid());

          // If this looks like an xAOD type, then also make an entry
          // for an aux store.  Don't want to try to rely on BaseInfo here,
          // as we may not have loaded the proper libraries yet.
          // Second line of the test is to handle the tests in DataModelTest.
          if (dobj.fullKey().find ("xAOD") != std::string::npos ||
              dobj.fullKey().find ("cinfo") != std::string::npos)
          {
            m_deletes.emplace (key + "Aux.",
                               ClassID_traits<SG::IConstAuxStore>::ID());
          }
        }
      }
    } else {
      ATH_MSG_WARNING(ialg->name() << " doesn't seem to be an Algorithm" );
    }
    
  }
}
//______________________________________________________________________________
bool AddressRemappingSvc::isDeleted (const SG::TransientAddress& tad) const
{
  std::string key = tad.name();
  std::scoped_lock lock (m_deletesMutex);
  for (auto p : boost::make_iterator_range (m_deletes.equal_range (key))) {
    CLID clid = p.second;
    if (tad.transientID (clid)) {
      return true;
    }
  }
  return false;
}


