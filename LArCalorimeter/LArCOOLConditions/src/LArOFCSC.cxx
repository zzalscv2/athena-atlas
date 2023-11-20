/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArOFCSC.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"

LArOFCSC::LArOFCSC():
  LArCondSuperCellBase("LArOFCSC"),
  m_nChannels(0),
  m_nSamples(0)
{
}

LArOFCSC::~LArOFCSC() {}


LArOFCSC::LArOFCSC(const CondAttrListCollection* attrList) :
  LArCondSuperCellBase("LArOFCSC"),
  m_nChannels(0),
  m_nSamples(0)
{
 
  if (initializeBase().isFailure()) return;

  if (!attrList) return;

  CondAttrListCollection::const_iterator gainIt=attrList->begin();
  CondAttrListCollection::const_iterator gainIt_e=attrList->end();
  
  m_pOFCa.resize(attrList->size());
  m_pOFCb.resize(attrList->size());
  m_pTimeOffset.resize(attrList->size());
  ATH_MSG_DEBUG( "Found data for " << attrList->size() << " gains.");
  
  int blobSize=0;  //FIXME Force size to hash-max??? m_scOnlineID->channelHashMax()

  m_nSamples=0;
 
  for(;gainIt!=gainIt_e;++gainIt) {
    const unsigned gain=gainIt->first;
    if (gain>=attrList->size() || gain>2) {
      ATH_MSG_ERROR("Found unexpected COOL-channel (=gain) number:" << gain);
      return; //ERROR
    }
    const coral::AttributeList& attr=gainIt->second;
    const coral::Blob& ofcaBlob = attr["OFCa"].data<coral::Blob>();
    const coral::Blob& ofcbBlob = attr["OFCb"].data<coral::Blob>();
    const coral::Blob& toBlob = attr["TimeOffset"].data<coral::Blob>();
    if (blobSize==0) blobSize=ofcaBlob.size();
    if (m_nSamples==0) m_nSamples=attr["nSamples"].data<unsigned>();
    
    //Sanity checks:
    if (blobSize!=ofcaBlob.size() || blobSize!=ofcbBlob.size()) {
      ATH_MSG_ERROR("Unequal blob size (" << blobSize << "/" 
		    << ofcaBlob.size() << "/" << ofcbBlob.size() << ")");
      return;
    }
    if (m_nSamples!=attr["nSamples"].data<unsigned>()) {
      ATH_MSG_ERROR("Unequal number of samples (" << m_nSamples << "/" 
		    << attr["nSamples"].data<unsigned>() << ")");
      return;
    }


    m_pOFCa[gain]=static_cast<const float*>(ofcaBlob.startingAddress());
    m_pOFCb[gain]=static_cast<const float*>(ofcbBlob.startingAddress());
    m_pTimeOffset[gain]=static_cast<const float*>(toBlob.startingAddress());
  }// end loop over COOL channels

  if (m_nSamples==0) {
    ATH_MSG_ERROR("Number of samples is zero!");
    return;
  }
  m_nChannels=blobSize/(sizeof(float)*m_nSamples);
  ATH_MSG_DEBUG("Found data for " << m_nChannels);
}

LArOFCSC::OFCRef_t LArOFCSC::OFC_a(const HWIdentifier&  onId, int gain, int tbin) const {
  if (tbin!=0) return OFCRef_t(nullptr,nullptr);
  return this->OFC_a(m_scOnlineID->channel_Hash(onId),gain);  
}

LArOFCSC::OFCRef_t LArOFCSC::OFC_b(const HWIdentifier&  onId, int gain, int tbin) const {
  if (tbin!=0) return OFCRef_t(nullptr,nullptr);
  return this->OFC_b(m_scOnlineID->channel_Hash(onId),gain);  
}

float LArOFCSC::timeOffset(const HWIdentifier&  CellID, int gain) const {
  const IdentifierHash hs=m_scOnlineID->channel_Hash(CellID);
  return m_pTimeOffset[gain][hs];
}


unsigned LArOFCSC::nTimeBins(const HWIdentifier&, int) const {
  return 1;
}


float LArOFCSC::timeBinWidth(const HWIdentifier&, int ) const {
  return (25./24.);
}

