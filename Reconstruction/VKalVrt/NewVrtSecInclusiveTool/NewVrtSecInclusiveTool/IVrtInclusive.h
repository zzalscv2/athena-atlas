/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
//
// IVrtInclusive.h - Description
//
/*
   Interface for inclusive secondary vertex reconstruction
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Tool creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   



    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _Rec_IVrtSecInclusive_H
#define _Rec_IVrtSecInclusive_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "VxSecVertex/VxSecVertexInfo.h"

 
//------------------------------------------------------------------------
namespace Rec {

//------------------------------------------------------------------------
  static const InterfaceID IID_IVrtInclusive("IVrtInclusive", 1, 0);

  class IVrtInclusive : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_IVrtInclusive;}
//---------------------------------------------------------------------------
//Interface itself

      virtual Trk::VxSecVertexInfo* findAllVertices( const std::vector<const xAOD::TrackParticle*> & inputTracks,
                                                     const xAOD::Vertex & PV) const =0;

  };

}  //end namespace

#endif
