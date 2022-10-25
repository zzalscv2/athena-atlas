/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOEVENT_TRIGINDETSILAYER_H
#define TRIGINDETPATTRECOEVENT_TRIGINDETSILAYER_H

class TrigInDetSiLayer {
public:
  int m_subdet;//1 : Pixel, 2 : SCT
  int m_type;//0: barrel, +/-n : endcap
  float m_refCoord;
  float m_minBound, m_maxBound;
};

#endif
