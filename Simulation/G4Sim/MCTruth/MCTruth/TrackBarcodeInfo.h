/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrackBarcodeInfo_H
#define TrackBarcodeInfo_H

#include "VTrackInformation.h"

namespace ISF {
  class ISFParticle;
}

class TrackBarcodeInfo: public VTrackInformation {
public:
	TrackBarcodeInfo(int bc, ISF::ISFParticle* baseIsp=0);
	virtual int GetParticleBarcode() const override {return m_barcode;}
	virtual const ISF::ISFParticle *GetBaseISFParticle() const override {return m_theBaseISFParticle;}
	virtual ISF::ISFParticle *GetBaseISFParticle() override {return m_theBaseISFParticle;}

	virtual void SetBaseISFParticle(ISF::ISFParticle*) override;
	virtual void SetReturnedToISF(bool returned) override;
	virtual bool GetReturnedToISF() const override {return m_returnedToISF;}

private:
	ISF::ISFParticle *m_theBaseISFParticle;
	int m_barcode;
	bool m_returnedToISF;
};


#endif // TrackBarcodeInfo_H
