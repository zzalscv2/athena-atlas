/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 CLASS_DEF for ObservedTracksMap used in TrkObserverTool
 ------------------------------
 ATLAS Collaboration
 ***************************************************************************/

#ifndef OBSERVEDTRACKsMAP_H
#define OBSERVEDTRACKsMAP_H

#include "xAODCore/CLASS_DEF.h"
#include "xAODTracking/TrackingPrimitives.h"
#include <map>

typedef std::map< int, std::tuple< Trk::Track*, // unique ID, track object
                        double, // score
                        xAOD::RejectionStep, // rejection step
                        xAOD::RejectionReason, // rejection reason
                        int, // unique parentId
                        int, // numPixelHoles
                        int, // numSCTHoles
                        int, // numSplitSharedPixel
                        int, // numSplitSharedSCT
                        int, // numSharedOrSplit
                        int, // numSharedOrSplitPixels
                        int, // numShared
                        int, // isPatternTrack
                        int, // totalSiHits
                        int, // inROI
                        int, // thishasblayer
                        int, // hassharedblayer
                        int, // hassharedpixel
                        int, // firstisshared
                        int, // numPixelDeadSensor
                        int, // numSCTDeadSensor
                        int, // numPixelHits
                        int, // numSCTHits
                        int, // numUnused
                        int, // numTRT_Unused
                        int, // numSCT_Unused
                        int, // numPseudo
                        float, // averageSplit1
                        float, // averageSplit2
                        int, // numWeightedShared
                        std::vector<xAOD::RejectionStep>, // cumulative rejection steps
                        std::vector<xAOD::RejectionReason>, // cumulative rejection reasons
                        float, // dR of closest hadronic ROI
                        float // Et of closest hadronic ROI
                        > > ObservedTracksMap;

CLASS_DEF( ObservedTracksMap , 238473855 , 1 )

#endif // OBSERVEDTRACKsMAP_H