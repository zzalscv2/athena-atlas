/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloDetDescr/CaloDetectorElements.h"

#include "LArReadoutGeometry/EMBDetectorRegion.h"
#include "LArReadoutGeometry/EMECDetectorRegion.h"
#include "LArReadoutGeometry/HECDetectorRegion.h"

#include "CaloGeoHelpers/CaloPhiRange.h"

#include "GeoModelUtilities/GeoAlignmentStore.h"

#include "GaudiKernel/SystemOfUnits.h"

namespace {

// estimate deta,dphi of Fcal cells (code from Dag Gillberg)
void fcal_deta_dphi (const CaloDetDescrElement& elt,
                     float& deta,
                     float& dphi)
{
  // half width of fcal cells
  const double Dx = 0.5 * elt.dx();
  const double Dy = 0.5 * elt.dy();
  const double phi = elt.phi();
  const double r = elt.r();
  const double dxcphi = Dx*std::cos(phi);
  const double dxsphi = Dx*std::sin(phi);
  const double dycphi = Dy*std::cos(phi);
  const double dysphi = Dy*std::sin(phi);
  // approximate width orthogonal to radial vector
  const double DrT = std::sqrt(dxsphi*dxsphi+dycphi*dycphi);
  // total width in phi
  const double inv_r = 1. / r;
  dphi = 2.*DrT * inv_r;

  // extension in radius
  double dr = std::sqrt(dxcphi*dxcphi+dysphi*dysphi);

  // half-width in eta..
  // sinh(eta) = z/r = f
  // d(f) = d(z)/r (+) z/r/r*d(r)
  // cosh(eta)*deta = df
  // deta = df / cosh(eta) = df / sqrt(1+f*f)
  // to avoid overlaps between cells, assume a plane geometry with dz =0
  double f=elt.z() * inv_r;
  double df  = elt.z()*dr*inv_r*inv_r;
  deta = 2.*std::abs(df) /std::sqrt(f*f+1.);
}


} // anonymous namespace


// -- EMBDetectorElement --
EMBDetectorElement::EMBDetectorElement(const IdentifierHash subcaloHash
				       , const IdentifierHash onl1
				       , const IdentifierHash onl2
				       , const CaloDetDescriptor* descriptor
				       , EMBCellConstLink& embCell
				       , const EMBDetectorRegion* embRegion
				       , bool isTestBeam
				       , const GeoAlignmentStore* geoAlignStore
                                       , const CaloElementPositionShift* posShift)
  : CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
  , m_cell(embCell)
  , m_region(embRegion)
{
  init_description(geoAlignStore,posShift);
  if(isTestBeam) propagateRaw();
}

void EMBDetectorElement::init_description(const GeoAlignmentStore* geoAlignStore
					  , const CaloElementPositionShift* posShift)
{
  m_eta_raw = static_cast<float> ((m_cell->getEtaMin() + m_cell->getEtaMax())/2.);
  if(m_cell->getEndcapIndex()==0)
    m_eta_raw *= -1;

  const double phi_loc = (m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.;

  double x_loc, y_loc, z_loc, r_loc;
  const Amg::Transform3D &xfDef = m_region->getDefAbsoluteTransform(geoAlignStore);
  const Amg::Transform3D &xfAbs = m_region->getAbsoluteTransform(geoAlignStore);

  z_loc = (m_cell->getZMaxLocal(EMBCell::CENTER) + m_cell->getZMinLocal(EMBCell::CENTER))/2.;
  r_loc = m_cell->getRLocal(EMBCell::CENTER);

  x_loc = r_loc*std::cos(phi_loc);
  y_loc = r_loc*std::sin(phi_loc);

  Amg::Vector3D globalDefCoords = xfDef*Amg::Vector3D(x_loc,y_loc,z_loc);
  Amg::Vector3D globalAbsCoords = posShift
    ? xfAbs*Amg::Vector3D(x_loc+posShift->dx,y_loc+posShift->dy,z_loc+posShift->dz)
    : xfAbs*Amg::Vector3D(x_loc,y_loc,z_loc);

  m_x_raw = static_cast<float> (globalDefCoords.x());
  m_y_raw = static_cast<float> (globalDefCoords.y());
  m_z_raw = static_cast<float> (globalDefCoords.z());

  m_x = static_cast<float> (globalAbsCoords.x());
  m_y = static_cast<float> (globalAbsCoords.y());
  m_z = static_cast<float> (globalAbsCoords.z());

  const double r_raw= std::sqrt(globalDefCoords.x()*globalDefCoords.x()+globalDefCoords.y()*globalDefCoords.y());
  m_r_raw = static_cast<float> (r_raw);

  const double r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y());
  m_r=static_cast<float>(r);


  if (r>0) {
    const double big_r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y()+globalAbsCoords.z()*globalAbsCoords.z());
    const double inv_big_r = 1. / big_r;
    const double inv_r = 1. / r;
    m_eta     = static_cast<float> (-std::log((big_r-globalAbsCoords.z()) * inv_r));
    m_sinTh = static_cast<float> (r * inv_big_r);
    m_cosTh = static_cast<float> (globalAbsCoords.z() * inv_big_r);
    m_cosPhi=globalAbsCoords.x() * inv_r;
    m_sinPhi=globalAbsCoords.y() * inv_r;
  } 
  else  {
    m_eta  = 0.;
    m_sinTh = 0.;
  }

  m_phi_raw = static_cast<float> (std::atan2(globalDefCoords.y(),globalDefCoords.x()));
  m_phi = static_cast<float> (std::atan2(globalAbsCoords.y(),globalAbsCoords.x()));
  
  m_deta = static_cast<float> (std::abs(m_cell->getEtaMax() - m_cell->getEtaMin()));
  m_dphi = static_cast<float> (std::abs(m_cell->getPhiLocalUpper() - m_cell->getPhiLocalLower()));

  // TO DO, find better value
  m_dr = static_cast<float> (std::abs(m_cell->getRLocal(EMBCell::FRONT) - m_cell->getRLocal(EMBCell::BACK))/2.);

  // -- from CaloDDE --
  m_dx = 0.;
  m_dy = 0.;
  m_dz = 0.;
  // -- from CaloDDE --

}

int EMBDetectorElement::getLayer() const
{
  return m_region->getSamplingIndex();
}

// -- EMECDetectorElement --

EMECDetectorElement::EMECDetectorElement(const IdentifierHash subcaloHash
					 , const IdentifierHash onl1
					 , const IdentifierHash onl2
					 , const CaloDetDescriptor* descriptor
					 , EMECCellConstLink& emecCell
					 , const EMECDetectorRegion* emecRegion
					 , bool isTestBeam
					 , const GeoAlignmentStore* geoAlignStore
					 , const CaloElementPositionShift* posShift)
  : CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
  , m_cell(emecCell)
  , m_region(emecRegion)
{
  init_description(isTestBeam,geoAlignStore,posShift);
  init_interpretation();
  if(isTestBeam) propagateRaw();
}

void EMECDetectorElement::init_description(bool isTestBeam
					   , const GeoAlignmentStore* geoAlignStore
					   , const CaloElementPositionShift* posShift)
{
  m_eta_raw = static_cast<float> ((m_cell->getEtaMin() + m_cell->getEtaMax())/2.);
  if(m_cell->getEndcapIndex()==0)
    m_eta_raw *= -1;

  m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);

  double x_loc, y_loc, z_loc, r_loc;
  const Amg::Transform3D &xfDef = m_region->getDefAbsoluteTransform(geoAlignStore);

// we need to apply the famous ZShift. 
  Amg::Transform3D xfNominal;
  if(m_cell->getEndcapIndex()==0)
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,m_region->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef; // Negative EMEC
  else
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,-m_region->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef; // Positive EMEC

  if(isTestBeam){
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,3689.5*Gaudi::Units::mm));
  }

  const Amg::Transform3D &xfAbs = m_region->getAbsoluteTransform(geoAlignStore);

  z_loc = m_cell->getZLocal(EMECCell::CENTER);
  r_loc = (m_cell->getRMinLocal(EMECCell::CENTER) + m_cell->getRMaxLocal(EMECCell::CENTER))/2.;

  x_loc = r_loc*std::cos(m_phi_raw);
  y_loc = r_loc*std::sin(m_phi_raw);

  Amg::Vector3D globalNomCoords = xfNominal*Amg::Vector3D(x_loc,y_loc,z_loc);
  Amg::Vector3D globalAbsCoords = (posShift!=nullptr ?
				xfAbs*Amg::Vector3D(x_loc+posShift->dx,y_loc+posShift->dy,z_loc+posShift->dz) :
				xfAbs*Amg::Vector3D(x_loc,y_loc,z_loc));

  m_x_raw = static_cast<float> (globalNomCoords.x());
  m_y_raw = static_cast<float> (globalNomCoords.y());
  m_z_raw = static_cast<float> (globalNomCoords.z());

  m_x = static_cast<float> (globalAbsCoords.x());
  m_y = static_cast<float> (globalAbsCoords.y());
  m_z = static_cast<float> (globalAbsCoords.z());


  const double r_raw= sqrt(globalNomCoords.x()*globalNomCoords.x()+globalNomCoords.y()*globalNomCoords.y());
  m_r_raw = static_cast<float> (r_raw);

  const double r = sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y());
  m_r=static_cast<float>(r);


  if (r>0) {
    const double big_r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y()+globalAbsCoords.z()*globalAbsCoords.z());
    const double inv_big_r = 1. / big_r;
    const double inv_r = 1. /r;
    m_eta     = static_cast<float> (-log((big_r-globalAbsCoords.z()) * inv_r));
    m_sinTh = static_cast<float> (r * inv_big_r);
    m_cosTh = static_cast<float> (globalAbsCoords.z() * inv_big_r);
    m_cosPhi=globalAbsCoords.x() * inv_r;
    m_sinPhi=globalAbsCoords.y() * inv_r;
  } 
  else  {
    m_eta  = 0.;
    m_sinTh = 0.;
  }

  m_phi = static_cast<float> (std::atan2(globalAbsCoords.y(),globalAbsCoords.x()));

  m_deta = static_cast<float> (std::abs(m_cell->getEtaMax() - m_cell->getEtaMin()));
  m_dphi = static_cast<float> (std::abs(m_cell->getPhiLocalLower() - m_cell->getPhiLocalUpper()));
  m_dz = static_cast<float> (std::abs(m_cell->getZLocal(EMECCell::BACK) - m_cell->getZLocal(EMECCell::FRONT))/2.);

  // -- from CaloDDE --
  m_dx = 0.;
  m_dy = 0.;
  m_dr = 0.;
  // -- from CaloDDE --
}

void EMECDetectorElement::init_interpretation()
{
  // Interpretation of PHI coming from LAr Readout Geometry
  if(m_cell->getEndcapIndex()==1)
  { // Positive EMEC
    if(m_cell->getPhiLocalUpper()<M_PI)
      m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
    else
      m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2. - 2.0*M_PI);
  }
  else
  { // Negative EMEC
    if(m_cell->getPhiLocalLower()<0)
      m_phi_raw = static_cast<float> (-M_PI - (m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
    else
      m_phi_raw = static_cast<float> (M_PI - (m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
  }
}

int EMECDetectorElement::getLayer() const
{
  return m_region->getSamplingIndex();
}

// -- HECDetectorElement --
HECDetectorElement::HECDetectorElement(const IdentifierHash subcaloHash
				       , const IdentifierHash onl1
				       , const IdentifierHash onl2
				       , const CaloDetDescriptor* descriptor
				       , HECCellConstLink& hecCell
				       , const HECDetectorRegion* hecRegion
				       , bool isTestBeam
				       , const GeoAlignmentStore* geoAlignStore
				       , const CaloElementPositionShift* posShift)
  : CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
  , m_cell(hecCell)
  , m_region(hecRegion)
{
  init_description(isTestBeam,geoAlignStore,posShift);
  init_interpretation();

  if(isTestBeam)
    propagateRaw();
}

void HECDetectorElement::init_description(bool isTestBeam
					  , const GeoAlignmentStore* geoAlignStore
					  , const CaloElementPositionShift* posShift)
{
  // take PHI_RAW from LAr Readout geometry and use it xor calculations of x_loc and y_loc
  m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);

  m_eta_raw = static_cast<float> ((m_cell->getEtaMaxNominal() + m_cell->getEtaMinNominal())/2.);
  if(m_cell->getEndcapIndex()==0)
    m_eta_raw *= -1;

  double x_loc, y_loc, z_loc, r_loc;
  r_loc = (m_cell->getRMaxLocalNominal(HECCell::CENTER) + m_cell->getRMinLocalNominal(HECCell::CENTER))/2.;
  z_loc = m_cell->getZLocal(HECCell::CENTER);


  const Amg::Transform3D &xfDef = m_region->getDefAbsoluteTransform(geoAlignStore);

  // we need to apply the famous ZShift.
  Amg::Transform3D xfNominal;
  if(m_cell->getEndcapIndex()==0)
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,m_region->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef; // Negative HEC
  else
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,-m_region->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef; // Positive HEC

  if(isTestBeam){
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0., 4277.*Gaudi::Units::mm));
  }

  const Amg::Transform3D &xfAbs = m_region->getAbsoluteTransform(geoAlignStore);

  x_loc = r_loc*std::cos(m_phi_raw);
  y_loc = r_loc*std::sin(m_phi_raw);

  Amg::Vector3D globalNomCoords = xfNominal*Amg::Vector3D(x_loc,y_loc,z_loc);
  Amg::Vector3D globalAbsCoords = (posShift!=nullptr ?
				xfAbs*Amg::Vector3D(x_loc+posShift->dx,y_loc+posShift->dy,z_loc+posShift->dz) :
				xfAbs*Amg::Vector3D(x_loc,y_loc,z_loc));

  m_x_raw = static_cast<float> (globalNomCoords.x());
  m_y_raw = static_cast<float> (globalNomCoords.y());
  m_z_raw = static_cast<float> (globalNomCoords.z());

  m_x = static_cast<float> (globalAbsCoords.x());
  m_y = static_cast<float> (globalAbsCoords.y());
  m_z = static_cast<float> (globalAbsCoords.z());


  const double r_raw= std::sqrt(globalNomCoords.x()*globalNomCoords.x()+globalNomCoords.y()*globalNomCoords.y());
  m_r_raw = static_cast<float> (r_raw);

  const double r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y());
  m_r=static_cast<float>(r);


  if (r>0) {
    const double big_r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y()+globalAbsCoords.z()*globalAbsCoords.z());
    const double inv_r = 1. / r;
    const double inv_big_r = 1. / big_r;
    m_eta     = static_cast<float> (-std::log((big_r-globalAbsCoords.z()) * inv_r));
    m_sinTh = static_cast<float> (r * inv_big_r);
    m_cosTh = static_cast<float> (globalAbsCoords.z() * inv_big_r);
    m_cosPhi=globalAbsCoords.x() * inv_r;
    m_sinPhi=globalAbsCoords.y() * inv_r;
  } 
  else  {
    m_eta  = 0.;
    m_sinTh = 0.;
  }

  m_phi = static_cast<float> (std::atan2(globalAbsCoords.y(),globalAbsCoords.x()));

  m_deta = static_cast<float> (m_cell->getEtaMaxNominal() - m_cell->getEtaMinNominal());
  m_dphi = static_cast<float> (std::abs(m_cell->getPhiLocalUpper() - m_cell->getPhiLocalLower()));
  m_dz = static_cast<float> (std::abs(m_cell->getZLocal(HECCell::FRONT)-m_cell->getZLocal(HECCell::BACK))/2.);

  // -- from CaloDDE --
  m_dx = 0.;
  m_dy = 0.;
  m_dr = 0.;
  // -- from CaloDDE --
}

void HECDetectorElement::init_interpretation()
{
  // Interpretation of PHI coming from LAr Readout Geometry
  if(m_cell->getEndcapIndex()==1)
  { // Positive HEC
    if(m_cell->getPhiLocalUpper()<M_PI)
      m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
    else
      m_phi_raw = static_cast<float> ((m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2. - 2.0*M_PI);
  }
  else
  { // Negative HEC
    if(m_cell->getPhiLocalLower()<0)
      m_phi_raw = static_cast<float> (-M_PI - (m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
    else
      m_phi_raw = static_cast<float> (M_PI - (m_cell->getPhiLocalLower() + m_cell->getPhiLocalUpper())/2.);
  }

  if(m_phi_raw>M_PI)
    m_phi_raw = static_cast<float> (m_phi_raw - (2.0*M_PI));
  else if(m_phi_raw<-M_PI)
    m_phi_raw = static_cast<float> (m_phi_raw + (2.0*M_PI));
}

int HECDetectorElement::getLayer() const
{
  return m_region->getSamplingIndex();
}

// -- FCALDetectorElement --

FCALDetectorElement::FCALDetectorElement(const IdentifierHash subcaloHash
					 , const IdentifierHash onl1
					 , const IdentifierHash onl2
					 , const CaloDetDescriptor* descriptor
					 , const FCALTile* fcalTile
					 , const FCALModule* fcalModule
					 , bool isTestBeam
					 , const GeoAlignmentStore* geoAlignStore
					 , const CaloElementPositionShift* posShift)
  : CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
  , m_tile(fcalTile)
  , m_module(fcalModule)
{
  init_description(isTestBeam,geoAlignStore,posShift);
  if(isTestBeam) propagateRaw();
}

void FCALDetectorElement::init_description(bool isTestBeam
					   , const GeoAlignmentStore* geoAlignStore
					   , const CaloElementPositionShift* posShift)
{
  double x_loc = m_tile->getX();
  double y_loc = m_tile->getY();
  //  double z_loc = -m_module->getFullDepthZ(*m_tile)/2.;
  double z_loc = 0.;

  const Amg::Transform3D &xfDef = m_module->getDefAbsoluteTransform(geoAlignStore);
// we need to apply the famous ZShift. 
  Amg::Transform3D xfNominal;
  if(m_module->getEndcapIndex()==FCALModule::POS)
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,-m_module->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef;
  else
    xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0.,m_module->getProjectivityDisplacement()*Gaudi::Units::cm))*xfDef;

  if(isTestBeam){
     if(m_module->getModuleIndex() == FCALModule::FCAL1)
           xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0., 4668.5*Gaudi::Units::mm));
     else if (m_module->getModuleIndex() == FCALModule::FCAL2)
           xfNominal = Amg::Translation3D(Amg::Vector3D(0.,0., 5128.3*Gaudi::Units::mm));
     else {
        std::cout<<"Wrong FCAL module for TB, using FCAL1 !!!"<<std::endl;
        xfNominal = Amg::Translation3D( Amg::Vector3D(0.,0.,4668.5*Gaudi::Units::mm));
     }
  }

  const Amg::Transform3D &xfAbs = m_module->getAbsoluteTransform(geoAlignStore);

  Amg::Vector3D globalDefCoords = xfNominal*Amg::Vector3D(x_loc,y_loc,z_loc);
  Amg::Vector3D globalAbsCoords = (posShift!=nullptr ?
				xfAbs*Amg::Vector3D(x_loc+posShift->dx,y_loc+posShift->dy,z_loc+posShift->dz) :
				xfAbs*Amg::Vector3D(x_loc,y_loc,z_loc));

  m_x_raw = static_cast<float> (globalDefCoords.x());
  m_y_raw = static_cast<float> (globalDefCoords.y());
  m_z_raw = static_cast<float> (globalDefCoords.z());

  m_x = static_cast<float> (globalAbsCoords.x());
  m_y = static_cast<float> (globalAbsCoords.y());
  m_z = static_cast<float> (globalAbsCoords.z());

  m_dx =static_cast<float> ( m_module->getFullWidthX(*m_tile));
  m_dy =static_cast<float> ( m_module->getFullWidthY(*m_tile));
  m_dz =static_cast<float> ( m_module->getFullDepthZ(*m_tile)/2.);

  m_deta = 0.; 
  m_dphi = 0.;
  m_dr = 0.;

  // From Calo DDE
  const double r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y());
  m_r=static_cast<float>(r);


  if (r>0) {
    const double big_r = std::sqrt(globalAbsCoords.x()*globalAbsCoords.x()+globalAbsCoords.y()*globalAbsCoords.y()+globalAbsCoords.z()*globalAbsCoords.z());
    const double inv_big_r = 1. / big_r;
    const double inv_r = 1. / r;
    m_eta     = static_cast<float> (-log((big_r-globalAbsCoords.z()) * inv_r));
    m_sinTh = static_cast<float> (r * inv_big_r);
    m_cosTh = static_cast<float> (globalAbsCoords.z() * inv_big_r);
    m_cosPhi=globalAbsCoords.x() * inv_r;
    m_sinPhi=globalAbsCoords.y() * inv_r;
  } 
  else  {
    m_eta  = 0.;
    m_sinTh = 0.;
  }

  m_phi_raw = static_cast<float> (std::atan2(globalDefCoords.y(),globalDefCoords.x()));
  m_phi = static_cast<float> (std::atan2(globalAbsCoords.y(),globalAbsCoords.x()));

// -- from CaloDDE --
  const double r_raw= sqrt(globalDefCoords.x()*globalDefCoords.x()+globalDefCoords.y()*globalDefCoords.y());
  m_r_raw = static_cast<float> (r_raw);

  if (m_r_raw>0.001) 
  {
    const double big_r_raw = std::sqrt(globalDefCoords.x()*globalDefCoords.x()+globalDefCoords.y()*globalDefCoords.y()+globalDefCoords.z()*globalDefCoords.z());
      // bug !!
    // m_sinTh = m_r/big_r_raw;
      m_eta_raw = static_cast<float> (-std::log((big_r_raw-globalDefCoords.z())/r_raw));
  }
  else 
  {
    m_eta_raw = 0.;
  }

  m_phi = static_cast<float> (std::atan2(globalAbsCoords.y(),globalAbsCoords.x()));
  m_phi_raw= static_cast<float> (std::atan2(globalDefCoords.y(),globalDefCoords.x()));
  // -- from CaloDDE --


  if (m_r>0.) {
    // estimate deta,dphi of Fcal cells
    fcal_deta_dphi (*this, m_deta, m_dphi);
  }

}

int FCALDetectorElement::getLayer() const
{
  return m_module->getModuleIndex();
}

TileDetectorElement::TileDetectorElement(const IdentifierHash subcaloHash,
					 const IdentifierHash onl1,
					 const IdentifierHash onl2,
					 const CaloDetDescriptor* descriptor):
  CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
{
}

void TileDetectorElement::set_cylindric(double eta, 
					double phi, 
					double r)
{
  m_eta = static_cast<float> (eta);
  m_r= static_cast<float> (r);

  m_phi = static_cast<float> (phi);
  if(phi<-M_PI)
    m_phi = static_cast<float> (phi + 2.0*M_PI);
  else if(phi>M_PI)
    m_phi = static_cast<float> (phi - 2.0*M_PI);

  m_cosPhi=static_cast<float> (std::cos(m_phi));
  m_sinPhi=static_cast<float> (std::sin(m_phi));
  m_sinTh=static_cast<float> (1/std::cosh(eta));
  m_cosTh=static_cast<float> (std::tanh(eta));
  m_x = static_cast<float> (r*m_cosPhi);
  m_y = static_cast<float> (r*m_sinPhi);
  m_z = static_cast<float> (r*std::sinh(eta));
  
}

void TileDetectorElement::set_cylindric_raw(double eta_raw, 
					    double phi_raw, 
					    double r_raw)
{
  m_eta_raw = static_cast<float> (eta_raw);
  m_r_raw = static_cast<float> (r_raw);
  
  m_phi_raw = static_cast<float> (phi_raw);
  if(phi_raw<-M_PI)
    m_phi_raw = static_cast<float> (phi_raw + 2.0*M_PI);
  else if(phi_raw>M_PI)
    m_phi_raw = static_cast<float> (phi_raw - 2.0*M_PI);
  
  m_x_raw = static_cast<float> (r_raw*std::cos(m_phi_raw));
  m_y_raw = static_cast<float> (r_raw*std::sin(m_phi_raw));
  m_z_raw = static_cast<float> (r_raw*std::sinh(eta_raw));
  
}

MbtsDetectorElement::MbtsDetectorElement() :
  CaloDetDescrElement(IdentifierHash(),
		      IdentifierHash(),
		      IdentifierHash(),
		      nullptr)
{
}

Identifier MbtsDetectorElement::customID() const
{
  return m_id;
}

void MbtsDetectorElement::compute_derived()
{
  // Compute x,y coordinates of scintillator centre
  m_x = static_cast<float> (m_r*std::cos(m_phi));
  m_y = static_cast<float> (m_r*std::sin(m_phi));
}


/**
 * @brief Update this element's geometry from the given list
 *        of offline elements.
 * @param fromelts List of offline elements.
 */
StatusCode
CaloSuperCellDetectorElement::update
(const std::vector<const CaloDetDescrElement*>& fromelts)
{
  if (fromelts.empty())
    return updateNull();
  else if (fromelts.size() == 1)
    return updateSingle (fromelts[0]);
  else if (fromelts[0]->getSubCalo() == CaloCell_Base_ID::LARFCAL)
    return updateFCAL(fromelts);
  else
    return updateBE (fromelts);
}


/**
 * @brief Set this element's geometry to default values.
 */
StatusCode
CaloSuperCellDetectorElement::updateNull()
{
  m_eta     = 0;
  m_phi     = 0;
  m_sinTh   = 0;
  m_cosTh   = 0;
  m_deta    = 0;
  m_dphi    = 0;
  m_volume  = 0;
  m_sinPhi  = 0;
  m_cosPhi  = 0;
  m_r       = 0;
  m_eta_raw = 0;
  m_phi_raw = 0;
  m_r_raw   = 0;
  m_dr      = 0;
  m_x       = 0;
  m_y       = 0;
  m_z       = 0;
  m_x_raw   = 0;
  m_y_raw   = 0;
  m_z_raw   = 0;
  m_dx      = 0;
  m_dy      = 0;
  m_dz      = 0;

  return StatusCode::SUCCESS;
}


/**
 * @brief Copy this element's geometry from the given offline element.
 */
StatusCode
CaloSuperCellDetectorElement::updateSingle (const CaloDetDescrElement* fromelt)
{
  m_eta     = fromelt->eta();
  m_phi     = fromelt->phi();
  m_sinTh   = fromelt->sinTh();
  m_cosTh   = fromelt->cosTh();
  m_deta    = fromelt->deta();
  m_dphi    = fromelt->dphi();
  m_volume  = fromelt->volume();
  m_sinPhi  = fromelt->cosPhi();
  m_cosPhi  = fromelt->sinPhi();
  m_r       = fromelt->r();
  m_eta_raw = fromelt->eta_raw();
  m_phi_raw = fromelt->phi_raw();
  m_r_raw   = fromelt->r_raw();
  m_dr      = fromelt->dr();
  m_x       = fromelt->x();
  m_y       = fromelt->y();
  m_z       = fromelt->z();
  m_x_raw   = fromelt->x_raw();
  m_y_raw   = fromelt->y_raw();
  m_z_raw   = fromelt->z_raw();
  m_dx      = fromelt->dx();
  m_dy      = fromelt->dy();
  m_dz      = fromelt->dz();

  return StatusCode::SUCCESS;
}


/**
 * @brief Update this element's geometry from a list of elements.
 *        For elements in the barrel / endcap.
 * @param fromelts List of offline elements.
 *
 * Primary variables are eta/phi plus r in the barrel and z in the endcap.
 */
StatusCode
CaloSuperCellDetectorElement::updateBE
(const std::vector<const CaloDetDescrElement*>& fromelts)
{
  bool is_barrel = (fromelts[0]->dr() > 0);

  double eta_raw_min =  99999;
  double eta_raw_max = -99999;
  double phi_raw_min =  99999;
  double phi_raw_max = -99999;
  double  rz_raw_min =  99999;
  double  rz_raw_max = -99999;
  double volume = 0;

  double etasum = 0;
  double detasum = 0;
  double phisum = 0;
  double dphisum = 0;
  double rzsum = 0;
  double drzsum = 0;

  for (const CaloDetDescrElement* fromelt : fromelts) {
    double rz, rz_raw, drz;
    if (is_barrel) {
      rz = fromelt->r();
      rz_raw = fromelt->r_raw();
      drz = fromelt->dr();
    }
    else {
      rz = fromelt->z();
      rz_raw = fromelt->z_raw();
      drz = fromelt->dz();
    }

    eta_raw_min = std::min (eta_raw_min,
                            (double)fromelt->eta_raw() - fromelt->deta()/2);
    eta_raw_max = std::max (eta_raw_max,
                            (double)fromelt->eta_raw() + fromelt->deta()/2);

    phi_raw_min = std::min (phi_raw_min,
                            (double)fromelt->phi_raw() - fromelt->dphi()/2);
    phi_raw_max = std::max (phi_raw_max,
                            (double)fromelt->phi_raw() + fromelt->dphi()/2);

    rz_raw_min = std::min (rz_raw_min, rz_raw - drz / 2);
    rz_raw_max = std::max (rz_raw_min, rz_raw + drz / 2);

    etasum  += fromelt->deta() * fromelt->eta();
    detasum += fromelt->deta();

    phisum  += fromelt->dphi() * fromelt->phi();
    dphisum += fromelt->dphi();

    rzsum  += drz * rz;
    drzsum += drz;

    volume += fromelt->volume();
  }

  m_volume = volume;

  double eta_raw = (eta_raw_min + eta_raw_max) / 2;
  double phi_raw = (phi_raw_min + phi_raw_max) / 2;
  double rz_raw  = (rz_raw_min + rz_raw_max) / 2;

  m_deta = eta_raw_max - eta_raw_min;
  m_eta_raw = eta_raw;

  m_dphi = phi_raw_max - phi_raw_min;
  m_phi_raw = phi_raw;

  m_dx = 0;
  m_dy = 0;

  m_volume = volume;

  double eta = etasum / detasum;
  double phi = phisum / dphisum;

  m_eta = eta;
  m_phi = phi;

  m_cosPhi = std::cos(phi);
  m_sinPhi = std::sin(phi);
  m_sinTh  = 1/std::cosh(eta);
  m_cosTh  = std::tanh(eta);

  double r = 0;
  double r_raw = 0;
  if (is_barrel) {
    r_raw = rz_raw;
    r   = rzsum / drzsum;

    m_dr = rz_raw_max - rz_raw_min;
    m_dz = 0;

    m_z_raw = rz_raw * std::sinh(eta_raw);
    m_z = r * sinh(eta);
  }
  else {
    r_raw = rz_raw / std::sinh(eta_raw);
    double z   = rzsum / drzsum;
    r = z / std::sinh(eta);

    m_dz = rz_raw_max - rz_raw_min;
    m_dr = 0;

    m_z_raw = rz_raw;
    m_z = z;
  }

  m_x_raw = r_raw * std::cos(phi_raw);
  m_y_raw = r_raw * std::sin(phi_raw);
  m_r_raw = r_raw;

  m_x = r * std::cos(phi);
  m_y = r * std::sin(phi);
  m_r = r;

  return StatusCode::SUCCESS;
}


/**
 * @brief Update this element's geometry from a list of elements.
 *        For elements in the FCAL.
 * @param fromelts List of offline elements.
 *
 * Primary variables are x/y/z.
 */
StatusCode
CaloSuperCellDetectorElement::updateFCAL
(const std::vector<const CaloDetDescrElement*>& fromelts)
{
  double x_raw_min =  99999;
  double x_raw_max = -99999;
  double y_raw_min =  99999;
  double y_raw_max = -99999;
  double z_raw_min =  99999;
  double z_raw_max = -99999;
  double volume = 0;

#if 0
  double xsum = 0;
  double dxsum = 0;
  double ysum = 0;
  double dysum = 0;
  double zsum = 0;
  double dzsum = 0;
#endif

  // Set the supercell center to be the geometric center of the
  // offline cells.  Warning: the fcal supercells are asymmetric;
  // this isn't the same as the COG!

  for (const CaloDetDescrElement* fromelt : fromelts) {
    x_raw_min = std::min (x_raw_min,
                          (double)fromelt->x_raw() - fromelt->dx()/2);
    x_raw_max = std::max (x_raw_max,
                          (double)fromelt->x_raw() + fromelt->dx()/2);

    y_raw_min = std::min (y_raw_min,
                          (double)fromelt->y_raw() - fromelt->dy()/2);
    y_raw_max = std::max (y_raw_max,
                          (double)fromelt->y_raw() + fromelt->dy()/2);

    z_raw_min = std::min (z_raw_min,
                          (double)fromelt->z_raw() - fromelt->dz()/2);
    z_raw_max = std::max (z_raw_max,
                          (double)fromelt->z_raw() + fromelt->dz()/2);

#if 0
    xsum  += fromelt->dx() * fromelt->x();
    dxsum += fromelt->dx();

    ysum  += fromelt->dy() * fromelt->y();
    dysum += fromelt->dy();

    zsum  += fromelt->dz() * fromelt->z();
    dzsum += fromelt->dz();
#endif

    volume += fromelt->volume();
  }

  m_volume = volume;

  double x_raw = (x_raw_min + x_raw_max) / 2;
  double y_raw = (y_raw_min + y_raw_max) / 2;
  double z_raw = (z_raw_min + z_raw_max) / 2;

  m_x_raw = x_raw;
  m_y_raw = y_raw;
  m_z_raw = z_raw;

  m_deta = 0;
  m_dphi = 0;
  m_dr = 0;

  m_dx = x_raw_max - x_raw_min;
  m_dy = y_raw_max - y_raw_min;
  m_dz = z_raw_max - z_raw_min;

#if 0
  double x = xsum / dxsum;
  double y = ysum / dysum;
  double z = zsum / dzsum;
#endif
  double x = m_x_raw + fromelts[0]->x() - fromelts[0]->x_raw();
  double y = m_y_raw + fromelts[0]->y() - fromelts[0]->y_raw();
  double z = m_z_raw + fromelts[0]->z() - fromelts[0]->z_raw();

  m_x = x;
  m_y = y;
  m_z = z;

  m_phi_raw = std::atan2 (y_raw, x_raw);
  m_phi = std::atan2 (y, x);

  const double r_raw = hypot (x_raw, y_raw);
  const double r = hypot (x, y);
  const double inv_r = 1. / r;

  m_r_raw = r_raw;
  m_r = r;

  const double big_r = std::sqrt (x*x + y*y + z*z);
  const double big_r_raw = std::sqrt (x_raw*x_raw + y_raw*y_raw + z_raw*z_raw);
  const double inv_big_r = 1. / big_r;

  m_eta = -std::log ((big_r - z) * inv_r);
  m_eta_raw = -std::log ((big_r_raw - z_raw) / r_raw);
  m_sinTh = r * inv_big_r;
  m_cosTh = z * inv_big_r;
  m_cosPhi = x * inv_r;
  m_sinPhi = y * inv_r;

  if (m_r>0.) {
    // estimate deta,dphi of Fcal cells
    fcal_deta_dphi (*this, m_deta, m_dphi);
  }

  return StatusCode::SUCCESS;
}


DummyDetDescrElement::DummyDetDescrElement(const IdentifierHash subcaloHash,
					   const IdentifierHash onl1,
					   const IdentifierHash onl2,
					   const CaloDetDescriptor* descriptor):
  CaloDetDescrElement(subcaloHash,onl1,onl2,descriptor)
{
}

void DummyDetDescrElement::set_cylindric(double eta, 
					 double phi, 
					 double r)
{
  m_eta = static_cast<float> (eta);
  m_r= static_cast<float> (r);

  m_phi = static_cast<float> (phi);
  if(phi<-M_PI)
    m_phi = static_cast<float> (phi + 2.0*M_PI);
  else if(phi>M_PI)
    m_phi = static_cast<float> (phi - 2.0*M_PI);
  
  //  m_x = r*cos(m_phi);
  // m_y = r*sin(m_phi);
  //m_z = r*sinh(eta);

  m_cosPhi=static_cast<float> (std::cos(m_phi));
  m_sinPhi=static_cast<float> (std::sin(m_phi));
  m_sinTh=static_cast<float> (1/std::cosh(eta));
  m_cosTh=static_cast<float> (std::tanh(eta));
  m_x = static_cast<float> (r*m_cosPhi);
  m_y = static_cast<float> (r*m_sinPhi);
  m_z = static_cast<float> (r*std::sinh(eta));

  
  //  double big_r = sqrt(m_x*m_x+m_y*m_y+m_z*m_z);
  //if(big_r > 0.001)
  //  m_sinTh = m_r/big_r;
  // else 
  //  m_sinTh = 0;
}

void DummyDetDescrElement::set_cylindric_raw(double eta_raw, 
					     double phi_raw, 
					     double r_raw)
{
  m_eta_raw = static_cast<float> (eta_raw);
  m_r_raw = static_cast<float> (r_raw);

  m_phi_raw = static_cast<float> (phi_raw);
  if(phi_raw<-M_PI)
    m_phi_raw = static_cast<float> (phi_raw + 2.0*M_PI);
  else if(phi_raw>M_PI)
    m_phi_raw = static_cast<float> (phi_raw - 2.0*M_PI);
  
  m_x_raw = static_cast<float> (r_raw*std::cos(m_phi_raw));
  m_y_raw = static_cast<float> (r_raw*std::sin(m_phi_raw));
  m_z_raw = static_cast<float> (r_raw*std::sinh(eta_raw));
  
  //  double big_r = sqrt(m_x_raw*m_x_raw+m_y_raw*m_y_raw+m_z_raw*m_z_raw);
  // if(big_r > 0.001)
  //  m_sinTh = m_r_raw/big_r;
  // else 
  //  m_sinTh = 0;
}

void DummyDetDescrElement::set_cylindric_size(double deta, 
					      double dphi,  
					      double dr)
{
  m_deta = static_cast<float> (deta);
  m_dphi = static_cast<float> (dphi);
  m_dr= static_cast<float> (dr); 
}
