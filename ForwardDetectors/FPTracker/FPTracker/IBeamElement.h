/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_IBEAMELEMENT_H
#define FPTRACKER_IBEAMELEMENT_H


#include "FPTracker/FPTrackerConstants.h"
#include <memory>

#include <string>
#include <vector>
#include <list>
#include <iosfwd>

namespace FPTracker{
  class IParticle;
  class Point;



  class IBeamElement{
  public:
    virtual ~IBeamElement();
    virtual double frontFace()                  const = 0;
    virtual double rearFace()                   const = 0;
    virtual double zsignedpos()                 const = 0;
    virtual double zabspos()                    const = 0;
    virtual Point position()                    const = 0;
    virtual Side   side()                       const = 0;
    virtual std::string label()                 const = 0;
    virtual bool isEndElement()                 const = 0;
    virtual void track(IParticle&)              const = 0;
    virtual void calibrate(IParticle&)                = 0;
    virtual std::string str()                   const = 0;
    
    virtual std::shared_ptr< const IBeamElement > clone()  const = 0;

    typedef std::shared_ptr< const IBeamElement >  ConstPtr_t;
    typedef std::shared_ptr< IBeamElement >        Ptr_t;

    typedef std::vector< Ptr_t >                     Container_t;
    typedef Container_t::iterator                    Iter_t;
    typedef Container_t::const_iterator              ConstIter_t;
    
    typedef std::list< Ptr_t >                       List_t;
    typedef List_t::iterator                         ListIter_t;
    typedef List_t::const_iterator                   ConstListIter_t;
  };





  std::ostream& operator<<(std::ostream&, const IBeamElement&);
}
#endif
