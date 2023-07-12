/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonGeoModelR4/MuonGeoUtilityTool.h>
#include <GeoModelKernel/GeoBox.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoShapeShift.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoTransform.h>
#include <GeoModelKernel/GeoSerialTransformer.h>
#include <GeoModelKernel/GeoVolumeCursor.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <set>
#include <sstream>
#include <string>

namespace MuonGMR4{
MuonGeoUtilityTool::~MuonGeoUtilityTool() = default;
MuonGeoUtilityTool::MuonGeoUtilityTool(const std::string &type, const std::string &name,
                     const IInterface *parent):
    AthAlgTool(type,name,parent) {
    declareInterface<IMuonGeoUtilityTool>(this);
}

StatusCode MuonGeoUtilityTool::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

const GeoShape* MuonGeoUtilityTool::extractShape(const PVConstLink& physVol) const {
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
        ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached ");
        return nullptr;
    }
    return extractShape(logVol->getShape());
}
const GeoShape* MuonGeoUtilityTool::extractShape(const GeoShape* inShape) const {
    if (!inShape) {
      ATH_MSG_INFO(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return nullptr;
    }
    static const std::set<ShapeType> valid_types{
        GeoTrd::getClassTypeID(),
        GeoBox::getClassTypeID(),
        GeoTube::getClassTypeID(),
    };
    if (valid_types.count(inShape->typeID())) {
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Found valid shape type "<<inShape->type());
        return inShape;
    }
    if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                        "Shape is a shift by "<<Amg::toString(shift->getX().translation(), 2)
                        << ". Continue navigation "<<shift);
            return extractShape(shift->getOp());
    }
    ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" "<<__func__<<" The shape "<<inShape->type()<<" is unknown to the method ");
    return nullptr;
}   
Amg::Transform3D MuonGeoUtilityTool::extractShifts(const PVConstLink& physVol) const { 
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached. ");
      return Amg::Transform3D::Identity();
    }
    return extractShifts(logVol->getShape());
}

Amg::Transform3D MuonGeoUtilityTool::extractShifts(const GeoShape* inShape) const { 
  if (!inShape) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return Amg::Transform3D::Identity();
  }
  Amg::Transform3D sumTrans{Amg::Transform3D::Identity()};
  if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Shape is a shift . Continue navigation "<<shift);
        sumTrans = extractShifts(shift->getOp()) * shift->getX();
    }
    ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Extacted transformation "<<dump(sumTrans));
    return sumTrans;
}
std::string MuonGeoUtilityTool::dumpShape(const GeoShape* shape) const {
  std::stringstream sstr{};
  if (shape->typeID() == GeoTrd::getClassTypeID()){
      const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
      sstr<<"Trapezoidal geometry shape with width: "<<trd->getXHalfLength1()
          <<", height: "<<trd->getXHalfLength2()<<" -- short/long tube length: "
          <<trd->getYHalfLength1()<<"/" <<trd->getYHalfLength2()<<", chamber width: "<<trd->getZHalfLength();
  } else if (shape->typeID() == GeoBox::getClassTypeID()){
      const GeoBox* box = static_cast<const GeoBox*>(shape);
      const Amg::Vector3D span{box->getXHalfLength(),
                               box->getYHalfLength(),
                               box->getZHalfLength()};
      sstr<<"Box geometry shape: Half lengths "<<Amg::toString(span,1);
  } else if (shape->typeID() == GeoTube::getClassTypeID()){
    const GeoTube* tube = static_cast<const GeoTube*>(shape);
    sstr<<"Tube with minimal and maximal radii of "<<tube->getRMin()<<", "<<tube->getRMax()<<" and length "<<tube->getZHalfLength();
  } else {
      sstr<<"Cake tastes good "<<shape->type();
  }
  return sstr.str();
}
std::string MuonGeoUtilityTool::dump(const Amg::Transform3D& transform) const {
  std::stringstream sstr{};
  sstr<<"translation "<<Amg::toString(transform.translation(),1)<<", "
      <<"rotation {"<<Amg::toString(transform.rotation().col(0),3)<<","
                      <<Amg::toString(transform.rotation().col(1),3)<<","
                      <<Amg::toString(transform.rotation().col(2),3)<<"}";
  return sstr.str();
}

std::string MuonGeoUtilityTool::dumpVolume(const PVConstLink& physVol) const {
   return dumpVolume(physVol, "");
}
std::string MuonGeoUtilityTool::dumpVolume(const PVConstLink& physVol, const std::string& childDelim) const { 
  std::stringstream sstr{};
  if (!physVol || !physVol->getLogVol()){
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<__func__<<" No logical volume attached ");
    return sstr.str();        
  }
  const GeoShape* shape = extractShape(physVol);
  if (!shape) {
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<__func__
                      <<" Failed to extract shape from phys volume "
                      << physVol->getLogVol()->getName());
      return sstr.str();
  }
  sstr<<"logical volume "<<physVol->getLogVol()->getName()<<", ";
  if (physVol->isShared() || !physVol->getParent()){
    sstr<<"Shared volume, ";
  } else {
    sstr<<dump(physVol->getX())<<", ";
  }
  sstr<<dumpShape(shape)<<", ";
  sstr<<"number of children "<<physVol->getNChildVols()<<", ";
  sstr<<std::endl;
  GeoVolumeCursor aV(physVol);
  unsigned int child{1};
  while (!aV.atEnd()) {
    sstr<<childDelim<<child<<": "<<dump(aV.getTransform())<<", "<< dumpVolume(aV.getVolume(),
                                        childDelim + "    ");
    ++child;
    aV.next();
  }
  return sstr.str();
}




}