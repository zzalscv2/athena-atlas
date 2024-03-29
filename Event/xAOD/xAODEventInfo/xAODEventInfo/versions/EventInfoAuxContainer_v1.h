// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODEVENTINFO_VERSIONS_EVENTINFOAUXCONTAINER_V1_H
#define XAODEVENTINFO_VERSIONS_EVENTINFOAUXCONTAINER_V1_H

// System include(s):
#include <stdint.h>
#include <vector>
#include <string>

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"
#include "CxxUtils/ConcurrentBitset.h"

namespace xAOD {

   /// Auxiliary information about the pileup events
   ///
   /// This object describes the "static" auxiliary information about
   /// all the pileup events.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 636390 $
   /// $Date: 2014-12-16 22:52:18 +0100 (Tue, 16 Dec 2014) $
   ///
   class EventInfoAuxContainer_v1 : public AuxContainerBase {

   public:
      /// Default constructor
      EventInfoAuxContainer_v1();

     /**
       * @brief Return the data vector for one aux data decoration item.
       * @param auxid The identifier of the desired aux data item.
       * @param size The current size of the container (in case the data item
       *             does not already exist).
       * @param capacity The current capacity of the container (in case
       *                 the data item does not already exist).
       */
      virtual void* getDecoration (SG::auxid_t auxid,
                                   size_t size,
                                   size_t capacity) override;


     /**
      * @brief Test if a particular variable is tagged as a decoration.
      * @param auxid The identifier of the desired aux data item.
      */
     virtual bool isDecoration (auxid_t auxid) const override;


     /**
      * @brief Lock a decoration.
      * @param auxid Identifier of the decoration to lock.
      */
     virtual void lockDecoration (SG::auxid_t auxid) override;


     /**
      * @brief Called after one of these objects is read.
      *        Locks any detector flag words that appear to have already
      *        been set.
      */
     void toTransient();


   private:
      /// @name Basic event information
      /// @{
      std::vector< uint32_t > runNumber;
      std::vector< uint64_t > eventNumber;
      std::vector< uint32_t > lumiBlock;
      std::vector< uint32_t > timeStamp;
      std::vector< uint32_t > timeStampNSOffset;
      std::vector< uint32_t > bcid;
      std::vector< uint32_t > detectorMask0;
      std::vector< uint32_t > detectorMask1;
      std::vector< uint32_t > detectorMask2;
      std::vector< uint32_t > detectorMask3;
      /// @}

      /// @name Event type information
      /// @{
      std::vector< std::vector< std::pair< std::string, std::string > > >
         detDescrTags;
      std::vector< uint32_t > eventTypeBitmask;
      /// @}

      /// @name Detector flags
      /// @{
      std::vector< uint32_t > pixelFlags;
      std::vector< uint32_t > sctFlags;
      std::vector< uint32_t > trtFlags;
      std::vector< uint32_t > larFlags;
      std::vector< uint32_t > tileFlags;
      std::vector< uint32_t > muonFlags;
      std::vector< uint32_t > forwardDetFlags;
      std::vector< uint32_t > coreFlags;
      std::vector< uint32_t > backgroundFlags;
      std::vector< uint32_t > lumiFlags;
      /// @}

      /// @name Monte-Carlo information
      /// @{
      std::vector< uint32_t > mcChannelNumber;
      std::vector< uint64_t > mcEventNumber;
      std::vector< std::vector< float > > mcEventWeights;
      /// @}

      /// @name Pileup information
      /// @{
      std::vector< uint64_t > pileUpMixtureIDLowBits;
      std::vector< uint64_t > pileUpMixtureIDHighBits;
      /// @}

      /// Keep track of the event status flags.
      /// The set bits here correspond to the auxids of all unlocked
      /// detector flag words.  This is not persistent,
      /// but is filled in the constructor and toTransient.
      CxxUtils::ConcurrentBitset m_decorFlags;

   }; // class EventInfoAuxContainer_v1

} // namespace xAOD

// Declare the inheritance of the type to StoreGate:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::EventInfoAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODEVENTINFO_VERSIONS_EVENTINFOAUXCONTAINER_V1_H
