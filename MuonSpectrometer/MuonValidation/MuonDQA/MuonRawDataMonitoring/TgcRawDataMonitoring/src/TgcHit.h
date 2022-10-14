/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TGCRAWDATAMONITORING_TGCHIT_H
#define TGCRAWDATAMONITORING_TGCHIT_H
#include "TgcChamber.h"
#include <map>
#include "xAODMuon/Muon.h"
namespace TGC {
  
  class TgcHit : public TgcChamber {
  public:
    TgcHit(double x,double y,double z,
	   double shortWidth,double longWidth,double length,
	   bool isStrip,int gasGap,int channel,int eta,int phi,int station, int bcmask);

    const double& X() const;
    const double& Y() const;
    const double& Z() const;
    const double& shortWidth() const;
    const double& longWidth() const;
    const double& length() const;
    const bool& isStrip() const;
    const int& gasGap() const;
    const int& channel() const;
    const int& stationEta() const;
    const int& stationPhi() const;
    const int& stationName() const;
    const int& bcmask() const;
    const std::string& gap_name() const;
    const std::string& type_name() const;
    const std::string& channel_name() const;
    const std::map<const xAOD::Muon*,const double>& residuals() const;
    void addResidual(const xAOD::Muon* m, const double v);

  private:
    double m_x{};
    double m_y{};
    double m_z{};
    double m_shortWidth{};
    double m_longWidth{};
    double m_length{};
    bool m_isStrip{};
    int m_gasGap{};
    int m_channel{};
    int m_stationEta{};
    int m_stationPhi{};
    int m_stationName{};
    int m_bcmask{};
    std::map<const xAOD::Muon*,const double> m_residuals{};
    std::string m_gap_name{"none"};
    std::string m_type_name{"none"};
    std::string m_channel_name{"none"};
  };
  
  inline const double& TgcHit::X() const {return m_x;}
  inline const double& TgcHit::Y() const {return m_y;}
  inline const double& TgcHit::Z() const {return m_z;}
  inline const double& TgcHit::shortWidth() const {return m_shortWidth;}
  inline const double& TgcHit::longWidth() const {return m_longWidth;}
  inline const double& TgcHit::length() const {return m_length;}
  inline const bool& TgcHit::isStrip() const {return m_isStrip;}
  inline const int& TgcHit::gasGap() const {return m_gasGap;}
  inline const int& TgcHit::channel() const {return m_channel;}
  inline const int& TgcHit::stationEta() const {return m_stationEta;}
  inline const int& TgcHit::stationPhi() const {return m_stationPhi;}
  inline const int& TgcHit::stationName() const {return m_stationName;}
  inline const int& TgcHit::bcmask() const {return m_bcmask;}
  inline const std::string& TgcHit::gap_name() const {return m_gap_name;}
  inline const std::string& TgcHit::type_name() const {return m_type_name;}
  inline const std::string& TgcHit::channel_name() const {return m_channel_name;}
  inline const std::map<const xAOD::Muon*,const double>& TgcHit::residuals() const {return m_residuals;}
  inline void TgcHit::addResidual(const xAOD::Muon* m, const double v) {m_residuals.insert(std::make_pair(m,v));}
  
}
#endif

