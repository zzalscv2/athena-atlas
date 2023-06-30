// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    RoiUtil.h        
//
//            non-member, non friend RoiDescriptor utility functions
//            to improve encapsulation
//                   
//  
//


#ifndef  IREGIONSELECTOR_ROIUTIL_H
#define  IREGIONSELECTOR_ROIUTIL_H

class IRoiDescriptor;

namespace RoiUtil { 

/// see whether a segment is contained within the roi in r-z
bool contains( const IRoiDescriptor& roi, double z0, double dzdr );

bool contains_zrange( const IRoiDescriptor& roi, double z0, double dzdr, double zmin, double zmax );

/// see whether a point is contained within the roi (in phi and r-z)
bool contains( const IRoiDescriptor& roi, double z, double r, double phi );
bool containsPhi( const IRoiDescriptor& roi, double phi );
bool containsZed( const IRoiDescriptor& roi, double z, double r );

/// basic range checkers
double phicheck(double phi);
double etacheck(double eta);
double zedcheck(double zed);

}

bool operator==( const IRoiDescriptor& roi0, const IRoiDescriptor& roi1 );
bool operator!=( const IRoiDescriptor& roi0, const IRoiDescriptor& roi1 );




#endif  // IREGIONSELECTOR_ROIUTIL_H
