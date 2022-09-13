///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// RootGlobalsRestore.h 
// Header file for class RootGlobalsRestore
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_ATHENA_ROOTGLOBALSRESTORE_H
#define ATHENAROOTCOMPS_ATHENA_ROOTGLOBALSRESTORE_H 1

// STL includes
#include <iosfwd>

// Framework includes
#include "CxxUtils/checker_macros.h"

// Forward declaration
class TDirectory;
class TFile;

namespace Athena {

/** @class RootGlobalsRestore captures the state of a few global resources
 *  from ROOT and restores their initial value upon d-tor call.
 */
class ATLAS_NOT_THREAD_SAFE RootGlobalsRestore
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Default constructor: 
  RootGlobalsRestore();

  /// Destructor: 
  ~RootGlobalsRestore(); 

  /////////////////////////////////////////////////////////////////// 
  // Private methods: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Copy constructor: 
  RootGlobalsRestore( const RootGlobalsRestore& rhs ); //> not impl.

  /// Assignment operator: 
  RootGlobalsRestore& operator=( const RootGlobalsRestore& rhs ); //> not impl

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// current ROOT directory
  TDirectory* m_gd;

  /// current ROOT file
  TFile* m_gf;

  /// current ROOT error-level
  int m_ge;

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 
//std::ostream& operator<<( std::ostream& out, const RootGlobalsRestore& o );

} //> end namespace Athena

#endif //> !ATHENAROOTCOMPS_ATHENA_ROOTGLOBALSRESTORE_H
