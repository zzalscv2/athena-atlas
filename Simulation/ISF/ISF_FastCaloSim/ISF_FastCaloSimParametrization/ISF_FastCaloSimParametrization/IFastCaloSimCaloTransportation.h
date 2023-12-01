/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFastCaloSimCaloTransportation_H
#define IFastCaloSimCaloTransportation_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
/* Transport steps will be return as G4FieldTracks*/
#include "G4FieldTrack.hh"

class TFCSTruthState;

static const InterfaceID IID_IFastCaloSimCaloTransportation("IFastCaloSimCaloTransportation", 1, 0);

class IFastCaloSimCaloTransportation : virtual public IAlgTool
{
 public:
   /** AlgTool interface methods */
   static const InterfaceID& interfaceID() { return IID_IFastCaloSimCaloTransportation; }

   virtual std::vector<G4FieldTrack> transport(const TFCSTruthState* truth, bool forceNeutral) const = 0;
};

#endif // IFastCaloSimCaloTransportation_H