/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "LArReadoutGeometry/EMBCell.h"
#include "LArReadoutGeometry/EMBDetDescr.h"
#include "LArReadoutGeometry/EMBDetectorManager.h"
#include "LArHV/EMBHVManager.h"
#include "LArHV/EMBHVModule.h"
#include "LArHV/EMBHVElectrode.h"
#include "LArHV/EMBPresamplerHVModule.h"
#include "LArHV/EMBPresamplerHVManager.h"
#include "LArHV/EMBPresamplerHVDescriptor.h"

EMBCell::~EMBCell()
= default;


double EMBCell::getPhiLocalLower(double /*r*/) const {
  return getPhiLocalLower();
}

double EMBCell::getPhiLocalUpper(double /*r*/) const {
  return getPhiLocalUpper();
}

unsigned int EMBCell::getNumElectrodes() const {
  return getHVInfo().m_electrode.size();
}

const EMBHVElectrode & EMBCell::getElectrode (unsigned int i) const {
  return *getHVInfo().m_electrode[i];
}

const EMBPresamplerHVModule& EMBCell::getPresamplerHVModule () const {
  return *getHVInfo().m_presamplerModule;
}


const EMBCell::HVInfo& EMBCell::getHVInfo() const
{
  if (!m_hvinfo.isValid()) {
    HVInfo hvinfo;
    initHV (hvinfo);
    m_hvinfo.set (std::move (hvinfo));
  }
  return *m_hvinfo.ptr();
}


void EMBCell::initHV (HVInfo& hvinfo) const
{
  if (getSamplingIndex()==0) {
    const EMBPresamplerHVManager& presamplerHVManager=getDescriptor()->getManager()->getPresamplerHVManager();
    double phiUpper = getPhiMaxNominal();
    double phiLower = getPhiMinNominal();
    double eta=fabs(getEtaMax()+getEtaMin())/2.0;
    double phi=fabs(phiUpper+phiLower)/2.0;

    const CellPartitioning & etaBinning=presamplerHVManager.getDescriptor()->getEtaPartitioning();
    const CellBinning & phiBinning=presamplerHVManager.getDescriptor()->getPhiBinning();

    unsigned int iPhi = int((phi - phiBinning.getStart())/phiBinning.getDelta()) + phiBinning.getFirstDivisionNumber();
    unsigned int iEta = etaBinning.getFirstDivisionNumber();
    unsigned int lastDivision=etaBinning.getFirstDivisionNumber()+etaBinning.getNumDivisions();
    while (iEta<lastDivision) {
      double etaMax=etaBinning.binUpper(iEta);
      double etaMin=etaBinning.binLower(iEta);
      if (eta>etaMin && eta<etaMax) break;
      iEta++;
    }
    if (iEta==lastDivision) throw std::runtime_error ("Error in EMBCell:  Presampler HV not found");

    unsigned int iSide=getEndcapIndex();
    hvinfo.m_presamplerModule = &(presamplerHVManager.getHVModule(iSide,iEta,iPhi));

  }
  else {
    // M_PI - phi for left side
    
    const EMBHVManager& hvManager=getDescriptor()->getManager()->getHVManager();
    double phiUpper = getPhiMaxNominal();
    double phiLower = getPhiMinNominal();
    
    
    
    double eta=fabs(getEtaMax()+getEtaMin())/2.0;
    double phi=fabs(phiUpper+phiLower)/2.0;
    const CellBinning & etaBinning=hvManager.getDescriptor().getEtaBinning();
    const CellBinning & phiBinning=hvManager.getDescriptor().getPhiBinning();
    unsigned int iEta = int((eta - etaBinning.getStart())/etaBinning.getDelta()) + etaBinning.getFirstDivisionNumber();
    unsigned int iPhi = int((phi - phiBinning.getStart())/phiBinning.getDelta()) + phiBinning.getFirstDivisionNumber();
    unsigned int iSector = int(2.0*(phi - phiBinning.binLower(iPhi))/phiBinning.getDelta());
    unsigned int iSide=getEndcapIndex();
    
    if (iEta==8) iEta=7;
    
    const EMBHVModule& hvMod = hvManager.getHVModule(iSide,iEta,iPhi,iSector);
    double dPhi = fabs((phiUpper-phiLower));
    double phi0 = (iSide==1) ? M_PI/1024 : -(M_PI)/1024;      // delta phi between first absorber and electrode at phi=0
    double sPhi = phiLower-hvMod.getPhiMin() + phi0;    
    unsigned int iOffset=int(sPhi*(M_1_PI/2*1024.)+0.5);
    unsigned int N      =int(dPhi*(M_1_PI/2*1024.)+0.5);
    //  std::cout << " EMBCell   eta,phi1,phi2 cell, phiMinModule " << eta << " " << phiLower << " " << phiUpper << " " << hvMod->getPhiMin() << std::endl;
    
    for (unsigned int iElectrode=iOffset;iElectrode<iOffset+N;iElectrode++) {
      const EMBHVElectrode& hvElec = hvMod.getElectrode(iElectrode);
      hvinfo.m_electrode.push_back(&hvElec);
    }
    
  }
} 


void EMBCell::addHVPathologies (const EMBHVPathologiesConstLink& pathologies) {
  m_hvPathologies.push_back(pathologies);
}

unsigned int EMBCell::getNumHVPathologies ()const{
  return m_hvPathologies.size();
}

const EMBHVPathologiesConstLink & EMBCell::getHVPathologies (unsigned int i) const{
  return m_hvPathologies[i];
}
