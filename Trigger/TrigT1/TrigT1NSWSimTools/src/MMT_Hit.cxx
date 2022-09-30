/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigT1NSWSimTools/MMT_Hit.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include <cmath>

MMT_Hit::MMT_Hit(char wedge, hitData_entry entry, const MuonGM::MuonDetectorManager* detManager, const std::shared_ptr<MMT_Parameters> par) {
  m_sector = wedge;

  std::string module(1, wedge);
  module += (std::abs(entry.station_eta) == 1) ? "M1" : "M2";
  m_module = module;

  m_station_name = "MM";
  m_station_name += wedge;
  m_VMM_chip = entry.VMM_chip;
  m_MMFE_VMM = entry.MMFE_VMM;
  m_ART_ASIC = std::ceil(1.*entry.MMFE_VMM/2);
  m_station_eta = entry.station_eta;
  m_station_phi = entry.station_phi;
  m_multiplet = entry.multiplet;
  m_gasgap = entry.gasgap;
  m_plane = entry.plane;
  m_strip = entry.strip;
  m_localX = entry.localX;
  m_BC_time = entry.BC_time;
  m_age = entry.BC_time;
  m_Y = -1.;
  m_Z = -1.;
  m_R = -1.;
  m_Ri = -1.;
  m_isNoise = false;

  Identifier strip_id = detManager->mmIdHelper()->channelID(m_station_name, m_station_eta, m_station_phi, m_multiplet, m_gasgap, m_strip);
  const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(strip_id);
  Amg::Vector3D globalPos(0.0, 0.0, 0.0);
  readout->stripGlobalPosition(strip_id, globalPos);

  MMDetectorHelper aHelper;
  char side = (globalPos.z() > 0.) ? 'A' : 'C';
  MMDetectorDescription* mm = aHelper.Get_MMDetector(wedge, std::abs(m_station_eta), m_station_phi, m_multiplet, side);
  MMReadoutParameters roP   = mm->GetReadoutParameters();

  m_R = globalPos.perp();
  m_Z = globalPos.z();
  m_Ri = m_strip*roP.stripPitch;
  m_RZslope = m_R / m_Z;

  int eta = std::abs(m_station_eta)-1;
  double base = par->ybases[m_plane][eta];
  m_Y = base + m_strip*roP.stripPitch - roP.stripPitch/2.;
  m_YZslope = m_Y / m_Z;
}

MMT_Hit::MMT_Hit(const MMT_Hit* hit) {
  m_sector = hit->m_sector;
  m_module = hit->m_module;
  m_station_name = hit->m_station_name;
  m_VMM_chip = hit->m_VMM_chip;
  m_MMFE_VMM = hit->m_MMFE_VMM;
  m_ART_ASIC = hit->m_ART_ASIC;
  m_station_eta = hit->m_station_eta;
  m_station_phi = hit->m_station_phi;
  m_multiplet = hit->m_multiplet;
  m_gasgap = hit->m_gasgap;
  m_plane = hit->m_plane;
  m_strip = hit->m_strip;
  m_localX = hit->m_localX;
  m_BC_time = hit->m_BC_time;
  m_age = hit->m_age;
  m_Y = hit->m_Y;
  m_Z = hit->m_Z;
  m_R = hit->m_R;
  m_Ri = hit->m_Ri;
  m_RZslope = hit->m_RZslope;
  m_YZslope = hit->m_YZslope;
  m_isNoise = hit->m_isNoise;
}

bool MMT_Hit::isX() const {
  int id = this->getPlane();
  return (id == 0 || id == 1 || id == 6 || id == 7) ? true : false;
}

bool MMT_Hit::isU() const {
  int id = this->getPlane();
  return (id == 2 || id == 4) ? true : false;
}

bool MMT_Hit::isV() const {
  int id = this->getPlane();
  return (id == 3 || id == 5) ? true : false;
}

bool MMT_Hit::verifyHit() const {
  /*
   * Put here all Hit checks, probably redundant if digitization is ok
   */
  if (this->getBC() < 0.) return false;
  else if (isinf(this->getRZSlope())) return false;
  else if (this->getChannel() < 1 || this->getChannel() > 8192) return false;
  else return true;
}
