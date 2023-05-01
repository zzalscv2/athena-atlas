
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonGeoModelR4/Utils.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include <set>



namespace MuonGMR4{
    const GeoShape* extractShape(const PVConstLink& physVol, MsgStream& msg) {
        const GeoLogVol* logVol = physVol->getLogVol();
        if (!logVol) {
            msg<<MSG::ERROR<<__FILE__<<":"<<__LINE__<<" Physical volume  has no logical volume attached "<<endmsg;
            return nullptr;
        }
        return extractShape(logVol->getShape(), msg);
    }
    const GeoShape* extractShape(const GeoShape* inShape, MsgStream& msg) {
        if (!inShape) {
            msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given "<<endmsg;
            return nullptr;
        }
        static const std::set<ShapeType> valid_types{
            GeoTrd::getClassTypeID(),
            GeoBox::getClassTypeID(),
        };
        if (valid_types.count(inShape->typeID())) {
            msg<<MSG::VERBOSE<<"Found valid shape type "<<inShape->type()<<endmsg;
            return inShape;
        }
        if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
            const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
            msg<<MSG::VERBOSE<<"Shape is a shift by "<<Amg::toString(shift->getX().translation(), 2)<< ". Continue navigation "
            " "<<shift<<endmsg;
            return extractShape(shift->getOp(), msg);
        }
        msg<<MSG::ERROR<<"The shape "<<inShape->type()<<" is invalid " <<endmsg;
        return nullptr;
    }
    Amg::Transform3D extractShifts(const PVConstLink& physVol, MsgStream& msg) {
        const GeoLogVol* logVol = physVol->getLogVol();
        if (!logVol) {
            msg<<MSG::ERROR<<__FILE__<<":"<<__LINE__<<" Physical volume  has no logical volume attached "<<endmsg;
            return Amg::Transform3D::Identity();
        }
        return extractShifts(logVol->getShape(), msg);      
    }
    Amg::Transform3D extractShifts(const GeoShape* inShape, MsgStream& msg) {
         if (!inShape) {
            msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given "<<endmsg;
            return Amg::Transform3D::Identity();
        }
        Amg::Transform3D sumTrans{Amg::Transform3D::Identity()};
        if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
            const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
            msg<<MSG::VERBOSE<<"Shape is a shift . Continue navigation "<<shift<<endmsg;
            sumTrans = extractShifts(shift->getOp(), msg) * shift->getX();
        }
        msg<<MSG::VERBOSE<<"Extacted transformation translation: "<<Amg::toString(sumTrans.translation(),2)
                <<std::endl<<Amg::toString(sumTrans.linear(),2)<<endmsg;
        return sumTrans;
    }
    
}