/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @file DataHeaderCnv_p5.cxx
 *  @brief This file contains the implementation for the DataHeaderCnv_p5 class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: DataHeaderCnv_p5.cxx,v 1.3.2.2 2009-05-22 19:26:54 gemmeren Exp $
 **/

#define private public
#define protected public
#include "PersistentDataModel/DataHeader.h"
#undef private
#undef protected

#include "PersistentDataModelTPCnv/DataHeaderCnv_p5.h"

DataHeaderElementCnv_p5::DataHeaderElementCnv_p5() {}
DataHeaderElementCnv_p5::~DataHeaderElementCnv_p5() {}

//______________________________________________________________________________
void DataHeaderElementCnv_p5::persToTrans(const DataHeaderElement_p5* pers,
	DataHeaderElement* trans,
	const DataHeaderForm_p5& form) {
   delete trans->m_token; trans->m_token = new Token; trans->m_ownToken = true;
   Token* token = const_cast<Token*>(trans->m_token);
   std::vector<unsigned int>::const_iterator intIter = form.params().begin();
   unsigned int keyIdx = 0U, aliasNum = 0U, clidNum = 0U;
// Translate PoolToken
   if ((*intIter&0x0000FFFF) == 0) {
      intIter++;
      keyIdx = *intIter; intIter++;
      aliasNum = *intIter; intIter++;
      clidNum = *intIter; intIter++;
      token->fromString(pers->m_token);
   } else {
      const unsigned int keyPos = (unsigned short)(*intIter>>16); intIter++;
      const unsigned int guidIdx = (unsigned short)(*intIter>>16),
	      classIdx = (unsigned short)(*intIter&0x0000FFFF); intIter++;
      const unsigned int prefixIdx = (unsigned short)(*intIter>>16),
	      typeIdx = (unsigned short)(*intIter&0x0000FFFF); intIter++;
// Add Technology and Offsets
      unsigned int tech = *intIter; intIter++;
      unsigned int oid1 = *intIter; intIter++;
      keyIdx = *intIter; intIter++;
      aliasNum = *intIter; intIter++;
      clidNum = *intIter; intIter++;
// Append DbGuid
      Guid guid(form.map()[guidIdx]);
      token->setDb(guid);
// Container name, may be optimized
      std::string cntName;
      if (prefixIdx > 0) {
         cntName += form.map()[prefixIdx];
         if (typeIdx > 0) {
            cntName += form.map()[typeIdx];
         }
      }
// Insert key
      if (keyPos > 0) {
         cntName += pers->m_token.substr(0, keyPos - 1) + form.map()[keyIdx] + pers->m_token.substr(keyPos - 1);
      } else {
         cntName += pers->m_token;
      }
      //token->setCont(cntName);
// Append ClassId
      Guid clid(form.map()[classIdx]);
      token->setClassID(clid);
      token->setTechnology(tech);
      token->setOid(Token::OID_t(oid1, pers->m_oid2));
   }
   unsigned int aliasCur = 0U, clidCur = 0U;
   trans->m_key = form.map()[keyIdx];
   trans->m_alias.clear();
   for (std::set<std::string>::const_iterator lastAlias = trans->m_alias.begin();
		   aliasCur < aliasNum; aliasCur++) {
      lastAlias = trans->m_alias.insert(lastAlias, form.map()[keyIdx + aliasCur + 1]);
   }
   trans->m_pClid = *intIter; intIter++;
   trans->m_clids.clear();
   const std::vector<unsigned int>::const_iterator intLast = form.params().end();
   for (std::set<CLID>::const_iterator lastClid = trans->m_clids.begin();
		   intIter != intLast && clidCur < clidNum; intIter++, clidCur++) {
      lastClid = trans->m_clids.insert(lastClid, *intIter);
   }
   trans->m_hashes.clear();
   for (; intIter != intLast; intIter++) {
      trans->m_hashes.push_back(*intIter);
   }
}
//______________________________________________________________________________
void DataHeaderElementCnv_p5::transToPers(const DataHeaderElement* trans,
	DataHeaderElement_p5* pers,
	DataHeaderForm_p5& form) {
   unsigned int tech = 0U, oid1 = 0U;
   unsigned short guidIdx = 0U, classIdx = 0U, prefixIdx = 0U, typeIdx = 0U, keyPos = 0U;
// Translate PoolToken
   if (trans->getToken() != 0) {
// Database GUID
      const std::string guid = trans->getToken()->dbID().toString();
      for (std::vector<std::string>::const_iterator iter = form.map().begin(), last = form.map().end();
	      iter != last; iter++, guidIdx++) {
         if (*iter == guid) break;
      }
      if (guidIdx == form.map().size()) {
         form.insertMap(guid);
      }
// Class GUID
      const std::string clid = trans->getToken()->classID().toString();
      for (std::vector<std::string>::const_iterator iter = form.map().begin(), last = form.map().end();
	      iter != last; iter++, classIdx++) {
         if (*iter == clid) break;
      }
      if (classIdx == form.map().size()) {
         form.insertMap(clid);
      }
// Container name, can be optimized
      pers->m_token = trans->getToken()->contID();
// Get Prefix
      std::string::size_type delim = pers->m_token.find_first_of("_/(");
      if (delim != std::string::npos) {
         const std::string persComp1 = pers->m_token.substr(0, delim + 1);
         for (std::vector<std::string>::const_iterator iter = form.map().begin(), last = form.map().end();
	         iter != last; iter++, prefixIdx++) {
            if (*iter == persComp1) break;
         }
         if (prefixIdx == form.map().size()) {
            form.insertMap(persComp1);
         }
         pers->m_token = pers->m_token.substr(delim + 1);
// Get TypeName
// Check whether Key only is used for placement
         if (pers->m_token.find(trans->m_key) != 0) {
            std::string::size_type delim = pers->m_token.find_first_of("/()");
            if (delim != std::string::npos) {
               const std::string persComp2 = pers->m_token.substr(0, delim + 1);
               for (std::vector<std::string>::const_iterator iter = form.map().begin(), last = form.map().end();
	               iter != last; iter++, typeIdx++) {
                  if (*iter == persComp2) break;
               }
               if (typeIdx == form.map().size()) {
                  form.insertMap(persComp2);
               }
               pers->m_token = pers->m_token.substr(delim + 1);
            } else if (pers->m_token == "DataHeader") {
               for (std::vector<std::string>::const_iterator iter = form.map().begin(), last = form.map().end();
	               iter != last; iter++, typeIdx++) {
                  if (*iter == "DataHeader") break;
               }
               if (typeIdx == form.map().size()) {
                  form.insertMap("DataHeader");
               }
               pers->m_token.clear();
            }
         }
      }
      delim = pers->m_token.rfind(trans->m_key);
      if (delim != std::string::npos) {
         keyPos = delim + 1;
         pers->m_token = pers->m_token.substr(0, delim) + pers->m_token.substr(delim + trans->m_key.size());
      }
      tech = trans->getToken()->technology();
      oid1 = trans->getToken()->oid().first;
      pers->m_oid2 = trans->getToken()->oid().second;
   }
   form.insertParam((unsigned int)(keyPos) * 0x00010000U + 0x0000FFFFU);
   form.insertParam((unsigned int)(guidIdx) * 0x00010000U + (unsigned int)(classIdx));
   form.insertParam((unsigned int)(prefixIdx) * 0x00010000U + (unsigned int)(typeIdx));
   form.insertParam(tech);
   form.insertParam(oid1);
   unsigned int keyIdx = form.map().size(), aliasNum = trans->m_alias.size(), clidNum = trans->m_clids.size();
   form.insertParam(keyIdx);
   form.insertParam(aliasNum);
   form.insertParam(clidNum);
   form.insertMap(trans->m_key);
   for (std::set<std::string>::const_iterator iter = trans->m_alias.begin(),
		   last = trans->m_alias.end(); iter != last; iter++) {
      form.insertMap(*iter);
   }
   form.insertParam(trans->m_pClid);
   for (std::set<CLID>::const_iterator iter = trans->m_clids.begin(),
		   last = trans->m_clids.end(); iter != last; iter++) {
      form.insertParam(*iter);
   }
   for (std::vector<unsigned int>::const_iterator iter = trans->m_hashes.begin(),
		   last = trans->m_hashes.end(); iter != last; iter++) {
      form.insertParam(*iter);
   }
}
//______________________________________________________________________________
//______________________________________________________________________________
DataHeaderCnv_p5::DataHeaderCnv_p5() {}
DataHeaderCnv_p5::~DataHeaderCnv_p5() {}
//______________________________________________________________________________
void DataHeaderCnv_p5::persToTrans(const DataHeader_p5* pers, DataHeader* trans) {
   pers->m_dhForm.start();
   const unsigned int provSize = pers->m_dhForm.params()[0];
   trans->m_inputDataHeader.resize(provSize);
   std::vector<DataHeaderElement>::iterator it = trans->m_inputDataHeader.begin();
   std::vector<DataHeaderElement_p5>::const_iterator pit = pers->m_dataHeader.begin();
   for (unsigned int i = 0U; i < provSize; i++, it++, pit++) {
      pers->m_dhForm.next();
      m_elemCnv.persToTrans(&(*pit), &(*it), pers->m_dhForm);
   }
   trans->m_dataHeader.resize(pers->m_dataHeader.size() - provSize);
   it = trans->m_dataHeader.begin();
   for (std::vector<DataHeaderElement_p5>::const_iterator last = pers->m_dataHeader.end();
		   pit != last; it++, pit++) {
      pers->m_dhForm.next();
      m_elemCnv.persToTrans(&(*pit), &(*it), pers->m_dhForm);
   }
}
//______________________________________________________________________________
void DataHeaderCnv_p5::transToPers(const DataHeader* trans, DataHeader_p5* pers) {
   const unsigned int provSize = trans->m_inputDataHeader.size();
   pers->m_dataHeader.resize(provSize + trans->m_dataHeader.size());
   pers->m_dhForm.resize(provSize + trans->m_dataHeader.size() + 2);
   pers->m_dhForm.start();
   pers->m_dhForm.insertParam(provSize);
   std::vector<DataHeaderElement_p5>::iterator pit = pers->m_dataHeader.begin();
   for (std::vector<DataHeaderElement>::const_iterator it = trans->m_inputDataHeader.begin(),
		   last = trans->m_inputDataHeader.end(); it != last; it++, pit++) {
      pers->m_dhForm.next();
      m_elemCnv.transToPers(&(*it), &(*pit), pers->m_dhForm);
   }
   for (std::vector<DataHeaderElement>::const_iterator it = trans->m_dataHeader.begin(),
		   last = trans->m_dataHeader.end(); it != last; it++, pit++) {
      pers->m_dhForm.next();
      m_elemCnv.transToPers(&(*it), &(*pit), pers->m_dhForm);
   }
}
//______________________________________________________________________________
void DataHeaderCnv_p5::insertDHRef(DataHeader_p5* pers,
	const std::string& key,
	const std::string& strToken) {
   Token* token = new Token;
   token->fromString(strToken);
   DataHeaderElement tEle(ClassID_traits<DataHeader>::ID(), key, token);
   DataHeaderElement_p5 pEle;
   pers->m_dhForm.next();
   m_elemCnv.transToPers(&tEle, &pEle, pers->m_dhForm);
   pers->m_dataHeader.push_back(pEle);
}
