/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_INTERFACES_IGENEVENTFILTER_H
#define ISF_INTERFACES_IGENEVENTFILTER_H 1

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "AtlasHepMC/GenEvent.h"

namespace ISF {

  class IGenEventFilter : virtual public IAlgTool {
     public:

       /** Virtual destructor */
       virtual ~IGenEventFilter(){}

       /// Creates the InterfaceID and interfaceID() method
       DeclareInterfaceID(IGenEventFilter, 1, 0);

       /** Returns a pass boolean on the particle  */
       virtual std::unique_ptr<HepMC::GenEvent> filterGenEvent(const HepMC::GenEvent& inputEvent) const = 0;

  };

} // end of namespace

#endif // ISF_INTERFACES_IGENEVENTFILTER_H
