/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcHit.h"
#include "TString.h"

TGC::TgcHit::TgcHit(double x,double y,double z,
		    double shortWidth,double longWidth,double length,
		    bool isStrip,int gasGap,int channel,int stationEta,int stationPhi,int stationName, int bcmask) :
  m_x{x},
  m_y{y},
  m_z{z},
  m_shortWidth{shortWidth},
  m_longWidth{longWidth},
  m_length{length},
  m_isStrip{isStrip},
  m_gasGap{gasGap},
  m_channel{channel},
  m_stationEta{stationEta},
  m_stationPhi{stationPhi},
  m_stationName{stationName},
  m_bcmask{bcmask},
  m_residuals{}
{
  initChamber(stationEta,stationPhi,stationName);
  m_gap_name = Form("%sL%02d",cham_name().data(),m_gasGap);
  m_type_name = Form("%s%s",m_gap_name.data(),(m_isStrip)?("S"):("W"));
  m_channel_name = Form("%sCh%03d",m_type_name.data(),m_channel);
}
