/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SEGMENT_COLLECTION_TLP4_TRK_H
#define SEGMENT_COLLECTION_TLP4_TRK_H

#include <memory>

//-----------------------------------------------------------------------------
// TrkSegment
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkSegment/SegmentCollection_p1.h"
#include "TrkEventTPCnv/TrkSegment/Segment_p1.h"

//-----------------------------------------------------------------------------
// TrkEventPrimitives
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkEventPrimitives/FitQuality_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/LocalParameters_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/HepSymMatrix_p1.h"

//-----------------------------------------------------------------------------
// TrkRIO_OnTrack
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack_p2.h"
#include "TrkEventTPCnv/TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack_p1.h"

//-----------------------------------------------------------------------------
// TrkSurfaces
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkSurfaces/Surface_p2.h"   
#include "TrkEventTPCnv/TrkDetElementSurface/DetElementSurface_p1.h"


//-----------------------------------------------------------------------------
// Top Level Pers Objects from InnerDetector and MuonSpectrometer
// Previously stored as separate Extening TP objects, now integrated
// including full declarations for dictionary's sake (and for unique_ptr)

#include "InDetEventTPCnv/InDetTrack_tlp2.h"
#include "MuonEventTPCnv/MuonMeasurements_tlp2.h"

namespace Trk
{
   class SegmentCollection_tlp4 
   {
    public:
     SegmentCollection_tlp4() {}
     
     // This object should not be copied
     SegmentCollection_tlp4 (const SegmentCollection_tlp4&) = delete;
     SegmentCollection_tlp4& operator= (const SegmentCollection_tlp4&) = delete;

     // Storage vectors
     std::vector< Trk::SegmentCollection_p1 >           m_segmentCollections;
     std::vector< Trk::Segment_p1 >                     m_segments;

     std::vector< Trk::Surface_p2 >                     m_surfaces;

     std::vector< Trk::FitQuality_p1 >                  m_fitQualities;
     std::vector< Trk::LocalParameters_p1 >             m_localParameters;
     std::vector< Trk::HepSymMatrix_p1 >                m_hepSymMatrices;

     std::vector< Trk::PseudoMeasurementOnTrack_p2 >    m_pseudoMeasurementOnTrack;
     std::vector< Trk::CompetingRIOsOnTrack_p1 >        m_competingRotsOnTrack; 

     // TLP objects for Inner and Muon subdetector data
     // for derived object types found in Tracking polymorphic collections
     std::unique_ptr<InDet::Track_tlp2>                 m_inDetTrackExt;
     std::unique_ptr<TPCnv::MuonMeasurements_tlp2>      m_muonMeasurementsExt;
  };
}

#endif 

