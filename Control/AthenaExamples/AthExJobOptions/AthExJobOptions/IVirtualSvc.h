///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// IVirtualSvc.h 
// Header file for class IVirtualSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXJOBOPTIONS_IVIRTUALSVC_H
#define ATHEXJOBOPTIONS_IVIRTUALSVC_H

/** @class IVirtualSvc
 *  This is the interface to a test service
 */

// STL includes

// FrameWork includes
#include "GaudiKernel/IService.h"

// forward declaration

class IVirtualSvc : virtual public IService
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Destructor: 
   */
  virtual ~IVirtualSvc();

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// Delivers important informations
  virtual StatusCode qotd( std::string& quote ) = 0;

  /// identifier for the framework
  static const InterfaceID& interfaceID();

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 
inline const InterfaceID& IVirtualSvc::interfaceID() 
{ 
  static const InterfaceID IID_IVirtualSvc("IVirtualSvc", 1, 0);
  return IID_IVirtualSvc; 
}

#endif //> ATHEXJOBOPTIONS_IVIRTUALSVC_H
