// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: JEMEtSums_v2.h 646335 2015-02-12 01:16:10Z morrisj $
#ifndef XAODTRIGL1CALO_VERSIONS_JEMETSUMS_V2_H
#define XAODTRIGL1CALO_VERSIONS_JEMETSUMS_V2_H

#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"

// System include(s):
#include <stdint.h>

namespace xAOD {

  /// Description of JEMEtSums_v2
  ///
  /// @author John Morris <john.morris@cern.ch>
  ///
  /// $Revision: 646335 $
  /// $Date: 2015-02-12 02:16:10 +0100 (Thu, 12 Feb 2015) $  
  
  class JEMEtSums_v2 : public SG::AuxElement{
    public:
      /// Default constructor
      JEMEtSums_v2();
      /// Default desturctor
      virtual ~JEMEtSums_v2(){}        
      
      /// initialize
      virtual void initialize(const uint_least8_t crate,const uint_least8_t module);
      
      /// initialize
      virtual void initialize(const uint_least8_t crate,const uint_least8_t module,
                              const std::vector<uint_least16_t>& etVec,
                              const std::vector<uint_least16_t>& exVec,
                              const std::vector<uint_least16_t>& eyVec,
                              const uint_least8_t peak);
      
      /// get crate number
      uint_least8_t crate() const;
      /// set crate number
      void setCrate(uint_least8_t);
      
      /// get module
      uint_least8_t module() const;
      /// set module
      void setModule(uint_least8_t);      
      
      /// get peak
      uint_least8_t peak() const;
      /// set peak
      void setPeak(uint_least8_t); 
      
      /// Please note that in the context of JEMEtSums, et is total energy, not transverse energy
      /// For transverse energy, please use Pythagoras' theorem on ex and ey
      
      /// get etVec - et for all time slices
      const std::vector<uint_least16_t>& etVec() const; 
      /// set etVec - et for all time slices
      void setEtVec(const std::vector<uint_least16_t>&);      
          
      /// get exVec - ex for all time slices
      const std::vector<uint_least16_t>& exVec() const;  
      /// set exVec - ex for all time slices
      void setExVec(const std::vector<uint_least16_t>&); 
      
      /// get eyVec - ey for all time slices
      const std::vector<uint_least16_t>& eyVec() const;
      /// set eyVec - ey for all time slices
      void setEyVec(const std::vector<uint_least16_t>&);  
      
      /// Is this JEMEtSums forward?
      uint_least8_t forward() const;
      
      /// get et for etVec[peak] - time slice that (hopefully) contains the collision
      uint_least16_t et() const;
      /// get ex for exVec[peak] - time slice that (hopefully) contains the collision
      uint_least16_t ex() const;
      /// get ey for eyVec[peak] - time slice that (hopefully) contains the collision
      uint_least16_t ey() const;      
  };
} // namespace xAOD

#endif // XAODTRIGL1CALO_VERSIONS_JEMETSUMS_V2_H
