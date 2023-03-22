/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_IBENDER_H
#define FPTRACKER_IBENDER_H

#include <memory>
#include <string>

namespace FPTracker{
  class IParticle;
 class IBender{
  public:
   virtual ~IBender();
   virtual void bend(IParticle&)         const = 0;
   virtual std::string label()           const = 0;
   typedef std::shared_ptr< IBender >  ConstPtr_t;
  };

}
#endif
