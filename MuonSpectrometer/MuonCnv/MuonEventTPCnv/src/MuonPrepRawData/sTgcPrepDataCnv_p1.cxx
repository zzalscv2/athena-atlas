/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   sTgcPrepDataCnv_p1.cxx
//
//-----------------------------------------------------------------------------

#include "MuonPrepRawData/TgcPrepData.h"
#include "MuonEventTPCnv/MuonPrepRawData/sTgcPrepDataCnv_p1.h"
namespace{
   template <class T> T copy(const T& obj) { return T{obj}; }
}

Muon::sTgcPrepData sTgcPrepDataCnv_p1::createsTgcPrepData(const Muon::sTgcPrepData_p1 *persObj,
                                                          const Identifier clusId,
                                                          const MuonGM::sTgcReadoutElement* detEl,
                                                          MsgStream & /**log*/ )  {
  Amg::Vector2D localPos{persObj->m_locX, 0.};
  // copy the list of identifiers of the cluster
  std::vector<Identifier> rdoList;
  unsigned int clusIdCompact = clusId.get_identifier32().get_compact();
  std::vector<signed char> rdoListPers = persObj->m_rdoList;
  for ( auto& diff : rdoListPers ) {
    Identifier rdoId (clusIdCompact + diff);
    rdoList.push_back(std::move(rdoId));
  }

  auto cmat = Amg::MatrixX(1,1);
  cmat(0,0) = static_cast<double>(persObj->m_errorMat);
    
  Muon::sTgcPrepData data (clusId,
                           0, // collectionHash
                           std::move(localPos),
                           std::move(rdoList),
                           std::move(cmat),
                           detEl,
			                     copy(persObj->m_charge),
			                     copy(persObj->m_time),
			                     copy(persObj->m_bcBitMap),
			                     copy(persObj->m_stripNumbers),
			                     copy(persObj->m_stripTimes),
			                     copy(persObj->m_stripCharges));

  return data;
}

void sTgcPrepDataCnv_p1::persToTrans(const Muon::sTgcPrepData_p1 *persObj, 
                                     Muon::sTgcPrepData *transObj,
                                     MsgStream & log ) {
  *transObj = createsTgcPrepData (persObj,
                                  transObj->identify(),
                                  transObj->detectorElement(),
                                  log);
}

void sTgcPrepDataCnv_p1::transToPers(const Muon::sTgcPrepData *transObj, 
                                     Muon::sTgcPrepData_p1 *persObj, 
                                     MsgStream & ) {

  persObj->m_locX         = transObj->localPosition()[Trk::locX];
  persObj->m_errorMat     = transObj->localCovariance()(0,0);
  
  persObj->m_charge       = transObj->charge();
  persObj->m_time         = transObj->time();
  
  persObj->m_bcBitMap     = transObj->getBcBitMap();
  
  persObj->m_stripNumbers = transObj->stripNumbers(); 
  persObj->m_stripTimes   = transObj->stripTimes(); 
  persObj->m_stripCharges = transObj->stripCharges(); 

  /// store the rdoList in a vector with the difference with respect to the 32-bit cluster identifier
  Identifier32::value_type clusIdCompact = transObj->identify().get_identifier32().get_compact(); // unsigned int
  std::vector<signed char> rdoListPers;
  const std::vector<Identifier>& rdoListTrans = transObj->rdoList();
  for ( const auto& rdo_id : rdoListTrans ) {
    // get the difference with respect to the 32-bit cluster identifier
    // (this only works if the absolute value of the difference is smaller than 128)
    Identifier32::value_type rdoIdCompact = rdo_id.get_identifier32().get_compact(); // unsigned int
    int diff = (int)(rdoIdCompact-clusIdCompact);
    rdoListPers.push_back((signed char)diff);
  } 
  
  persObj->m_rdoList = rdoListPers; 
    
}


