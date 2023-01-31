/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_BEAMLINE_H
#define FPTRACKER_BEAMLINE_H

#include "IBeamElement.h"
#include <string>
#include <iosfwd>

namespace FPTracker{

  class IParticle;
  class Beamline{
  public:
    Beamline();
    Beamline(IBeamElement::ListIter_t, IBeamElement::ListIter_t);
    Beamline(const Beamline&);
    Beamline& operator = (Beamline);

    void track(IParticle&) const;
    void calibrate(IParticle&);
    std::string str() const;

  private:
    IBeamElement::Container_t m_elements;
    void swap(Beamline& other);
 };

  std::ostream& operator<<(std::ostream& os, const Beamline& bl);

}
#endif
