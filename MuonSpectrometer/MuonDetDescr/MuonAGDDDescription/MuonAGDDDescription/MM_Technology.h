/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_Technology_H
#define MM_Technology_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "AGDDKernel/AGDDTechnology.h"
#include <vector>
namespace MuonGM {

// Description class to build MicroMegas chambers

class MM_Technology: public AGDDTechnology {
public:
	double thickness{0.};
	int nlayers{0};
	double gasThickness{0.};
	double pcbThickness{0.};
	double roThickness{0.};
	double f1Thickness{0.};
	double f2Thickness{0.};
	double f3Thickness{0.};

	int geoLevel{0};


	// inner structure parameters (to be defined)

	// constructor
	inline MM_Technology(const std::string& s,
                             AGDDDetectorStore& ds);
	inline double Thickness() const;
};

MM_Technology::MM_Technology(const std::string& s,
                             AGDDDetectorStore& ds): AGDDTechnology(s, ds){}


double MM_Technology::Thickness() const
{
	//thickness=nlayers*(gasThickness+pcbThickness) + 2.*pcbThickness;
	return thickness;
}

} // namespace MuonGM

#endif
