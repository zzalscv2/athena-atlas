/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiDetElementBoundaryLink_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for detector elements links
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiDetElementBoundaryLink_xk_H
#define SiDetElementBoundaryLink_xk_H

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TrkPatternParameters/PatternTrackParameters.h"

namespace InDet{


  class SiDetElementBoundaryLink_xk final
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      SiDetElementBoundaryLink_xk(const InDetDD::SiDetectorElement*&, bool isITk = false);
      SiDetElementBoundaryLink_xk(const SiDetElementBoundaryLink_xk&) = default;
      ~SiDetElementBoundaryLink_xk() = default;
      SiDetElementBoundaryLink_xk& operator  = (const SiDetElementBoundaryLink_xk&) = default;

      ///////////////////////////////////////////////////////////////////
      // Main methods
      ///////////////////////////////////////////////////////////////////

      const InDetDD::SiDetectorElement* detElement() const {return m_detelement;}
      int intersect(const Trk::PatternTrackParameters&,double&) const;

    protected:

      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      const InDetDD::SiDetectorElement*   m_detelement ;
      double                              m_bound[4][3]{};
      bool                                m_ITkGeometry;
      double                              m_dR         ;

      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

    private:
      enum AxisDirection {
        PositiveX=0,
        NegativeY,
        NegativeX,
        PositiveY
      };

      enum IntersectionStatus {
        Inside=-1,
        NotInsideNorOutside=0,
        Outside=1
      };

    };
  
  /////////////////////////////////////////////////////////////////////////////////
  // Inline methods
  /////////////////////////////////////////////////////////////////////////////////


} // end of name space

#endif // SiDetElementBoundaryLink_xk
