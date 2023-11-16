/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDetDescrUtils/BuildNSWReadoutGeometry.h"

#include <TString.h>  // for Form

#include <fstream>

#include "AGDDControl/AGDDController.h"
#include "AGDDControl/IAGDD2GeoSvc.h"
#include "AGDDKernel/AGDDDetector.h"
#include "AGDDKernel/AGDDDetectorStore.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

using namespace MuonGM;

BuildNSWReadoutGeometry::BuildNSWReadoutGeometry() = default;

bool BuildNSWReadoutGeometry::BuildReadoutGeometry(MuonGM::MuonDetectorManager* mgr, const NswPassivationDbData* passivData) const {
    bool geoBuilt = true;

    ServiceHandle<IAGDDtoGeoSvc> svc("AGDDtoGeoSvc", "MMDetectorHelper");
    if (svc.retrieve().isFailure()) { std::abort(); }
    IAGDDtoGeoSvc::LockedController c = svc->getController();
    detectorList& dList = c->GetDetectorStore().GetDetectorList();
    detectorList::const_iterator it;
    for (it = dList.begin(); it != dList.end(); ++it) {
        std::vector<AGDDDetectorPositioner*>& dPos = ((*it).second)->GetDetectorPositioners();
        for (unsigned int i = 0; i < dPos.size(); i++) {
            std::string chTag = dPos[i]->ID.detectorAddress;
            GeoFullPhysVol* vol = dPos[i]->theVolume;

            std::string stName = chTag.substr(0, 4);

            int etaIndex{999}, phiIndex{999}, mLayer{999}, iSide{0};
            int iLS = atoi((chTag.substr(3, 1)).c_str());  // sTG3 and sMD3 are small chambers for small sectors
            if (iLS == 3)
                iLS = 1;  // small
            else
                iLS = 0;  // large
            if (chTag.substr(13, 1) == "A")
                iSide = 1;
            else if (chTag.substr(13, 1) == "C")
                iSide = -1;
            etaIndex = iSide * atoi((chTag.substr(5, 1)).c_str());
            phiIndex = atoi((chTag.substr(12, 1)).c_str());
            mLayer = atoi((chTag.substr(7, 1)).c_str());
            std::string vName = vol->getLogVol()->getName();
            std::string sName = vName.substr(vName.find('-') + 1);
                
            if (chTag.substr(0, 3) == "sMD") {
                std::unique_ptr<MMReadoutElement> re = std::make_unique<MMReadoutElement>(vol, sName, etaIndex, phiIndex, mLayer, mgr, passivData);
                re->initDesign();
                re->fillCache();
                mgr->addMMReadoutElement(std::move(re));
            } else if (chTag.substr(0, 3) == "sTG") {
                std::unique_ptr<sTgcReadoutElement> re = std::make_unique<sTgcReadoutElement>(vol, sName, etaIndex, phiIndex, mLayer, mgr);
                std::string myVolName = (chTag.substr(0, 8)).c_str();
                re->initDesign(-999., -999., -999., 3.2, -999., 2.7, -999., 2.6);
                re->fillCache();
                mgr->addsTgcReadoutElement(std::move(re));
            }
        }
    }
    return geoBuilt;
}
