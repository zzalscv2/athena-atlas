/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARGEOENDCAP_ENDCAPDMCONSTRUCTION_H
#define LARGEOENDCAP_ENDCAPDMCONSTRUCTION_H

class GeoFullPhysVol;

namespace LArGeo {

  class EndcapDMConstruction
  {
  public:
    EndcapDMConstruction(bool);
    ~EndcapDMConstruction() = default;

    void create(GeoFullPhysVol* envelope) const;

  private:
    EndcapDMConstruction(const EndcapDMConstruction&);
    EndcapDMConstruction& operator=(const EndcapDMConstruction&);

    bool m_activateFT;
  };

}

#endif
