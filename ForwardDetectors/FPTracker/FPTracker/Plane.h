/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_PLANE_H
#define FPTRACKER_PLANE_H

#include "IBeamElement.h"
#include "Point.h"
#include "FPTrackerConstants.h"
#include <memory>
#include <string>
#include <vector>

namespace FPTracker{
  class Magnet;
  class IParticle;

 class Plane:public IBeamElement{
  public:
    Plane(double, Side);

    IBeamElement::ConstPtr_t clone() const;
    double frontFace()               const;
    double rearFace()                const;
    double zabspos()                 const;
    double zsignedpos()              const;
    Side   side()                    const;
    Point  position()                const;

    std::string label()              const;
    std::string str()                const;

    bool isEndElement()               const; 
    void track(IParticle&)            const;
    void calibrate(IParticle&);

    typedef std::shared_ptr< const Plane >   ConstPtr_t;
    typedef std::vector< ConstPtr_t >          Container_t;



  private:

    static const std::string s_label;
    Point                    m_position;
    Side                     m_side;
  };


  std::ostream& operator<<(std::ostream&, const Plane&);
}
#endif
