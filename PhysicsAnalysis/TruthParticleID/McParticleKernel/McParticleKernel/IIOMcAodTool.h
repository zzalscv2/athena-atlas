/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// IIOMcAodTool.h 
// Header file for class IIOMcAodTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLEKERNEL_IIOMCAODTOOL_H 
#define MCPARTICLEKERNEL_IIOMCAODTOOL_H 

// STL includes

// FrameWork includes
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IProperty.h"

// Forward declaration

static const InterfaceID IID_IIOMcAodTool("IIOMcAodTool", 1, 0);

class IIOMcAodTool : virtual public IAlgTool,
		     virtual public IProperty
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Destructor: 
   */
  virtual ~IIOMcAodTool();

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() = 0;
  virtual StatusCode  execute()    = 0;
  virtual StatusCode  finalize()   = 0;


  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  static const InterfaceID& interfaceID();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

}; 


/////////////////////////////////////////////////////////////////// 
/// Inline methods: 
/////////////////////////////////////////////////////////////////// 
inline const InterfaceID& IIOMcAodTool::interfaceID() 
{ 
   return IID_IIOMcAodTool; 
}

#endif //> MCPARTICLEKERNEL_IIOMCAODTOOL_H
