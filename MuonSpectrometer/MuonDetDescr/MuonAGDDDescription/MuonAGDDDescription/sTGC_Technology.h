/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGC_Technology_H
#define sTGC_Technology_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "AGDDKernel/AGDDTechnology.h"
#include <vector>
namespace MuonGM {

// Description class to build sTGC chambers

class sTGC_Technology: public AGDDTechnology {
public:
	double thickness{0.};
	int nlayers{0};
	double gasThickness{0.};
	double pcbThickness{0.}; // Included for Backwards Compatibility
	double pcbThickness150{0.};
	double pcbThickness200{0.};
	double coverThickness{0.};
	double f4Thickness{0.};
  	double f5Thickness{0.};
  	double f6Thickness{0.};



	int geoLevel{0};

	// inner structure parameters (to be defined)

	// constructor
	inline sTGC_Technology(const std::string& s,
                               AGDDDetectorStore& ds);
	inline double Thickness() const;
};

sTGC_Technology::sTGC_Technology(const std::string& s,
                                 AGDDDetectorStore& ds): AGDDTechnology(s, ds)
{
}

double sTGC_Technology::Thickness() const
{
	//thickness=nlayers*(gasThickness+pcbThickness) + pcbThickness;
	return thickness;
}

} // namespace MuonGM

#endif
