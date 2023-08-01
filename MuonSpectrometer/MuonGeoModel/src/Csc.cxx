/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/Csc.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTrd.h"
#include "MuonGeoModel/Component.h"
#include "MuonGeoModel/CscComponent.h"
#include "MuonGeoModel/CscMultiLayer.h"
#include "MuonGeoModel/DetectorElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelKernel/GeoShape.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <string>
#include <utility>

class GeoMaterial;

#define skip_csc false

namespace MuonGM {

    Csc::Csc(const MYSQL& mysql, Component *ss) : DetectorElement(ss->name) {
        CscComponent *s = (CscComponent *)ss;
        m_component = s;
        width = s->dx1;
        longWidth = s->dx2;
        thickness = s->GetThickness(mysql);
        maxwLength = s->maxwdy;
        excent = s->excent;
        physicalLength = s->dy;
        length = physicalLength;
        double num = longWidth * (excent - physicalLength);
        if (std::abs(num) < 1e-10) {
            upWidth = 0;
        } else {
            upWidth = num / (excent - maxwLength);
        }

        layer = std::make_unique<CscMultiLayer>(mysql, s->name);
        layer->width = width;
        layer->longWidth = longWidth;
        layer->upWidth = upWidth;
        layer->excent = excent;
        layer->length = length;
        layer->physicalLength = physicalLength;
        layer->maxwLength = maxwLength;

        index = s->index;
    }


    GeoFullPhysVol *Csc::build(StoredMaterialManager& matManager,
                               const MYSQL& mysql,
                               int minimalgeo) {
        std::vector<Cutout *> vcutdef;
        int cutoutson = 0;
        return build(matManager, mysql, minimalgeo, cutoutson, vcutdef);
    }

    GeoFullPhysVol *Csc::build(StoredMaterialManager& matManager,
                               const MYSQL& mysql,
                               int minimalgeo, int cutoutson,
                               const std::vector<Cutout *>& vcutdef) {
        GeoFullPhysVol *pcsc = nullptr;
        GeoLogVol *lcsc = nullptr;
        const GeoMaterial *mcsc = matManager.getMaterial("std::Air");

        if (excent == length) {
            // CSC is a simple traezoid
            const GeoShape *sCSS = new GeoTrd(thickness / 2., thickness / 2., width / 2., longWidth / 2., length / 2.);
            lcsc = new GeoLogVol(logVolName, sCSS, mcsc);

        } else {
            // CSC is a union of two trapezoids
            GeoTrd *downTrd = new GeoTrd(thickness / 2., thickness / 2., width / 2., longWidth / 2., maxwLength / 2.);
            GeoTrd *upTrd = new GeoTrd(thickness / 2., thickness / 2., longWidth / 2., upWidth / 2., (physicalLength - maxwLength) / 2.);
            const GeoShape *sCSL = &((downTrd->add((*upTrd) << GeoTrf::TranslateZ3D(physicalLength / 2.))) << GeoTrf::TranslateZ3D((maxwLength - physicalLength) / 2.));
            lcsc = new GeoLogVol(logVolName, sCSL, mcsc);
        }

        pcsc = new GeoFullPhysVol(lcsc);
        if (minimalgeo == 1)
            return pcsc;

        GeoVPhysVol *lay = layer->build(matManager, mysql, cutoutson, vcutdef);
        if (!skip_csc)
            pcsc->add(lay);

        return pcsc;
    }

    void Csc::print() const {
        MsgStream log(Athena::getMessageSvc(), "MuonGM::Csc");
        log << MSG::INFO << " Csc:: Csc " << name << " : " << endmsg;
    }

} // namespace MuonGM
