/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ODDPHICMA_H
#define ODDPHICMA_H

#include <map>
#include <algorithm>
#include "RPCcablingInterface/CMAparameters.h"
#include "RPCcablingSim/WiredOR.h"


namespace RPCcablingSim {

class SectorLogicSetup;

class OddPhiCMA : public CMAparameters
{
     private:
     typedef std::map < int,WiredOR*,std::less < int > > WORlink;
 
     WORlink m_pivot_WORs;
     WORlink m_lowPt_WORs;
     WORlink m_highPt_WORs;     

     bool cable_CMA_channels(void);
     bool cable_CMA_channelsP03(void);
     bool connect(SectorLogicSetup&);
     void get_confirm_strip_boundaries(int,int);
     void get_confirm_strip_boundariesP03(int,int);
     int  get_max_strip_readout(int);

     public:
     OddPhiCMA(int,int,int,CMAcoverage,int,int,int,int,int,int,int,
               int,int,int,int);
     OddPhiCMA(const OddPhiCMA&);
     ~OddPhiCMA();

     OddPhiCMA& operator =(const OddPhiCMA&);

     const WORlink& pivot_WORs(void)  const {return m_pivot_WORs;}
     const WORlink& lowPt_WORs(void)  const {return m_lowPt_WORs;}
     const WORlink& highPt_WORs(void) const {return m_highPt_WORs;}

     bool setup(SectorLogicSetup&);
};

}
#endif
