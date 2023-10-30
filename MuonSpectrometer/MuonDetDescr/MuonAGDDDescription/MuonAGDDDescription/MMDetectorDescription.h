/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMDetectorDescription_H
#define MMDetectorDescription_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "AGDDKernel/AGDDDetector.h"
#include "MuonAGDDDescription/MM_Technology.h"
#include <string>
#include <vector>
#include <iostream>

class AGDDDetectorStore;

struct MMReadoutParameters {
    double stripPitch{0.};
    double gasThickness{0.};
    double pcbThickness{0.};
    double driftThickness{0.};
    std::vector<double> stereoAngle{};
    std::vector<int> readoutSide{};
    double zpos{0.};
    double distanceFromZAxis{0.};  //inner radius: distance from the IP till the bottom part of the module
    double roLength{0.};   //module's radial size
    double activeBottomLength{0.};  // active area: bottom length
    double activeTopLength{0.};    //active area: top length
    double activeH{0.};            //active area: radial size
    double minYPhiR{0.};          //active area: the distance between the first eta and stereo strips (active) [R:right, L:left, min:bottom part, max:top part]. LM1 is the special case as there is no space to route any strips at the bottom, we use two distances (left and right).
    double minYPhiL{0.};
    double maxYPhi{0.};
    int nMissedTopEta{0};      //number of strips that are not connected at any FE board (eta layer)
    int nMissedBottomEta{0};
    int nMissedTopStereo{0};   //number of strips that are not connected at any FE board (stereo layer)
    int nMissedBottomStereo{0};
    int nRoutedTop{0};     // number of strips needed to cover the low efficient parts of the module (these strips are shorter in lenght than the normal ones)
    int nRoutedBottom{0};
    double dlStereoTop{0.};
    double dlStereoBottom{0.};
    int tStrips{0}; //total strips per module (disconnected strips are included)
};

class MMDetectorDescription: public AGDDDetector {
public:
	MMDetectorDescription(const std::string& s,
                              AGDDDetectorStore& ds);
	void Register();

	double sWidth() const {return small_x();}
	double lWidth() const {return large_x();}
	double Length() const {return y();}
	double Tck()    const {return z();}

	void xFrame(double y) {m_xFrame=y;}
	double xFrame() const {return m_xFrame;}

	void ysFrame(double y) {m_ysFrame=y;}
	double ysFrame() const {return m_ysFrame;}

	void ylFrame(double y) {m_ylFrame=y;}
	double ylFrame() const {return m_ylFrame;}

        MuonGM::MM_Technology* GetTechnology();

	MMReadoutParameters roParameters;
	
	MMReadoutParameters& GetReadoutParameters() {return roParameters;}

protected:
	double m_xFrame = 0.0;
	double m_ysFrame = 0.0;
	double m_ylFrame = 0.0;

        AGDDDetectorStore& m_ds;

	void SetDetectorAddress(AGDDDetectorPositioner*);
};

#endif
