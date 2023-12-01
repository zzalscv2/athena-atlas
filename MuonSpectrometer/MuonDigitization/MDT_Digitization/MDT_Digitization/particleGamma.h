/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_DIGITIZATION_PARTICLEGAMMA_H
#define MDT_DIGITIZATION_PARTICLEGAMMA_H
/*-----------------------------------------------

Created 07-10-2011 by Oleg.Bulekov@cern.ch
Function particleGamma returns the value of gamma factor for Qball particle.
-----------------------------------------------*/

#include <iostream>
#include <sstream>
#include <vector>

#include "CLHEP/Units/PhysicalConstants.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtTubeCalibContainer.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonSimData/MuonSimData.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimEvent/MDTSimHitCollection.h"
#include "PathResolver/PathResolver.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "TrkDetDescrUtils/GeometryStatics.h"

// SB
#include "AtlasHepMC/GenParticle.h"
//

double particleGamma(const MDTSimHit& hit, unsigned short eventId = 0) {
    double QGamma = -9999.;
    const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
    const HepMcParticleLink::PositionFlag idxFlag = (eventId == 0) ? HepMcParticleLink::IS_POSITION : HepMcParticleLink::IS_EVENTNUM;
    const HepMcParticleLink trkParticle(hit.trackNumber(), eventId, evColl, idxFlag);
    HepMC::ConstGenParticlePtr genParticle = trkParticle.cptr();
    if (genParticle) {
        int particleEncoding = genParticle->pdg_id();
        //      std::cout << "SB: pdgId=" << particleEncoding <<std::endl;
        if ((((int)(abs(particleEncoding) / 10000000) == 1) && ((int)(abs(particleEncoding) / 100000) == 100)) ||
            (((int)(abs(particleEncoding) / 10000000) == 2) && ((int)(abs(particleEncoding) / 100000) == 200))) {
            double QPx = genParticle->momentum().px();
            double QPy = genParticle->momentum().py();
            double QPz = genParticle->momentum().pz();
            double QE = genParticle->momentum().e();
            double QM2 = pow(QE, 2) - pow(QPx, 2) - pow(QPy, 2) - pow(QPz, 2);
            double QM;

            if (QM2 >= 0.) {
                QM = sqrt(QM2);
            } else {
                QM = -9999.;
            }
            if (QM > 0.) {
                QGamma = QE / QM;
            } else {
                QGamma = -9999.;
            }
        }
    } else {
        QGamma = -9999.;
    }

    return QGamma;
}

#endif
