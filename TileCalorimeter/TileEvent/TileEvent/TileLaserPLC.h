/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/*author Renato Febbraro*/
/*date 27/5/2008*/
/*renato.febbraro@cern.ch*/

#ifndef TILELASERPLC
#define TILELASERPLC


#include <string>


class TileLaserPLC{

  public:

    TileLaserPLC() 
      : m_alphaPos(0)
      ,  m_LVdiodes(0.0)
      , m_HVpmts(0.0)
      , m_shutter(0)
      , m_interlock(0)
      ,  m_alarm(0) { }


    int getAlphaPos() const     { return m_alphaPos;  }   
    double getLVdiodes() const  { return m_LVdiodes;  }   
    double getHVpmts() const    { return m_HVpmts;    }     
    int getShutter() const      { return m_shutter;   }    
    int getInterlock() const    { return m_interlock; } 
    int getAlarm() const        { return m_alarm;     }

    void setPLC(const int alphaPos,
		const double LVdiodes,
		const double HVpmts,
		const int shutter,
		const int interlock,
		const int alarm)
    {
      m_alphaPos = alphaPos;
      m_LVdiodes = LVdiodes;
      m_HVpmts = HVpmts;
      m_shutter = shutter;
      m_interlock = interlock;
      m_alarm = alarm;
    }


  /** Convertion operator to a std::string,
   * can be used in a cast operation : (std::string) TileLaserPmt */
  operator std::string() const;


private:

    int m_alphaPos;
    double m_LVdiodes;
    double m_HVpmts;
    int m_shutter;
    int m_interlock;
    int m_alarm;
 
};


#endif
