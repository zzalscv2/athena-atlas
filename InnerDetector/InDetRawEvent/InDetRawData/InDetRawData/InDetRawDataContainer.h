/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InDetRawDataContainer.h
//   Header file for class InDetRawDataContainer
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Container for Raw Data Objects collections for Pixel, SCT and TRT
///////////////////////////////////////////////////////////////////
// Version 1.0 13/08/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_INDETRAWDATACONTAINER_H
#define INDETRAWDATA_INDETRAWDATACONTAINER_H

// Base classes
#include "EventContainers/IdentifiableContainer.h"
//Needed Classes
#include "InDetRawData/InDetRawDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

template<class CollectionT>
class InDetRawDataContainer 
: public IdentifiableContainer<CollectionT>{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  InDetRawDataContainer(unsigned int max);

  InDetRawDataContainer(unsigned int max, EventContainers::Mode);

  InDetRawDataContainer(EventContainers::IdentifiableCache<CollectionT>*);
  
  // Destructor:
  virtual ~InDetRawDataContainer();

   /** return class ID */
   static const CLID& classID() 
   {
     return ClassID_traits< InDetRawDataContainer <CollectionT> > ::ID();
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

  InDetRawDataContainer(const InDetRawDataContainer&);
  InDetRawDataContainer &operator=(const InDetRawDataContainer&);

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:

};
// member functions that use Collection T
#include"InDetRawData/InDetRawDataContainer.icc"

#endif // INDETRAWDATA_INDETRAWDATACONTAINER_H
