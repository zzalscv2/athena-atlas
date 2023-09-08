/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonPrepRawDataContainer.h
//   Header file for class MuonPrepRawDataContainer
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Container for RIO  collections for  Muons
///////////////////////////////////////////////////////////////////
// Version 1.0 106/07/2004 Ketevi A. Assamagan - adapted 
// from Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef MUONPREPRAWDATA_MUONPREPDATACONTAINER_H
#define MUONPREPRAWDATA_MUONPREPDATACONTAINER_H

// Base classes
#include "AthenaKernel/CLASS_DEF.h"
#include "EventContainers/IdentifiableContainer.h"

//Needed Classes
#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/MdtPrepData.h"
#include "MuonPrepRawData/RpcPrepData.h"
#include "MuonPrepRawData/TgcPrepData.h"
// New Small Wheel
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonPrepRawData/sTgcPrepData.h"

#include "MuonPrepRawData/MuonPrepDataCollection.h"

// Identifiable cache construction
#include "EventContainers/IdentifiableCache.h"

namespace Muon{

/** @brief Template for Muon PRD containers (which are basically collections of MuonPrepDataCollections).*/
template<class CollectionT>
class MuonPrepDataContainer : public IdentifiableContainer<CollectionT> {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  //default for POOL
  MuonPrepDataContainer();
    
  // Constructor with parameters:
  MuonPrepDataContainer(unsigned int max);

  // Constructor with IdentifiableCache object
  MuonPrepDataContainer(EventContainers::IdentifiableCache<CollectionT> * cache);

  // Destructor:
  virtual ~MuonPrepDataContainer() = default;

   /** return class ID */
   static const CLID& classID() 
   {
     //	static CLID id = CLID_T ; 
     //	return id; 
     return ClassID_traits< MuonPrepDataContainer <CollectionT> >::ID();
   }

   /** return class ID */
   virtual const CLID& clID() const
    {
      return classID();
    }

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:


  MuonPrepDataContainer(const MuonPrepDataContainer&);
  MuonPrepDataContainer &operator=(const MuonPrepDataContainer&);
  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:

};

///////////////////////////////////////////////////////////////////
// Inline methods:
/////////////////////////////////////////////////////////////////// 
template <class PrdType> using MuonPrepDataContainerT = MuonPrepDataContainer<MuonPrepDataCollection<PrdType>>;

using MdtPrepDataContainer = MuonPrepDataContainerT<MdtPrepData>;
using RpcPrepDataContainer = MuonPrepDataContainerT<RpcPrepData>;
using TgcPrepDataContainer = MuonPrepDataContainerT<TgcPrepData>;
using CscPrepDataContainer = MuonPrepDataContainerT<CscPrepData>;
using CscStripPrepDataContainer = MuonPrepDataContainerT<CscStripPrepData>;
using MMPrepDataContainer = MuonPrepDataContainerT<MMPrepData>;
using sTgcPrepDataContainer = MuonPrepDataContainerT<sTgcPrepData>;

// member functions that use Collection T
#include "MuonPrepRawData/MuonPrepDataContainer.icc"

 
}

#include "MuonPrepRawData/CscPrepDataContainer.h"
#include "MuonPrepRawData/CscStripPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
// New Small Wheel
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"

#endif // TRKPREPRAWDATA_PREPRAWDATACONTAINER_H

