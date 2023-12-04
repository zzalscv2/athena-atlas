/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_DriftCircleOnTrack.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETRIO_ONTRACK_TRT_DRIFTCIRCLEONTRACK_H
#define INDETRIO_ONTRACK_TRT_DRIFTCIRCLEONTRACK_H

// Base classes
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "CxxUtils/CachedUniquePtr.h"
#include "TrkEventPrimitives/DriftCircleStatus.h"
#include "TrkEventPrimitives/DriftCircleSide.h"

// InDet


// for ElementLink to IdentifiableContainer PixelClusterContainer
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "AthLinks/ElementLink.h"

#include "GaudiKernel/GaudiException.h"

#include <atomic>

namespace Trk {
  class Surface;
  class LocalParameters;
  class ITrkEventCnvTool;
}

namespace InDetDD {
  class TRT_BaseElement;
}

typedef ElementLink<InDet::TRT_DriftCircleContainer> ElementLinkToIDCTRT_DriftCircleContainer;

namespace InDet{
  class TRT_DriftCircle;

  /**@class TRT_DriftCircleOnTrack
  Represents 'corrected' measurements from the TRT (for example, corrected for wire sag). The
  equivalent 'uncorrected' class is InDet::TRT_DriftCircle*

  @author Edward.Moyse@cern.ch
  @author Andreas.Salzburger@cern.ch
  @author Christos.Anastopoulos@cern.ch (Athena MT)
   */

  class TRT_DriftCircleOnTrack final: public Trk::RIO_OnTrack {

    public:
      friend class  Trk::ITrkEventCnvTool;
      /**Default constructor - needed for POOL*/
      TRT_DriftCircleOnTrack();
      /**Copy constructor */
      TRT_DriftCircleOnTrack(const TRT_DriftCircleOnTrack &);
      /**Assignment operator*/
      TRT_DriftCircleOnTrack &operator=(const TRT_DriftCircleOnTrack &);
      /**Move assignment operator*/
      TRT_DriftCircleOnTrack &operator=(TRT_DriftCircleOnTrack &&) noexcept;

     /** Constructor with parameters and without externalPrediction*/
      TRT_DriftCircleOnTrack(const InDet::TRT_DriftCircle* RIO,
                             Trk::LocalParameters&& driftRadius,
                             Amg::MatrixX&& errDriftRadius,
                             const IdentifierHash& idDE, double predictedLocZ,
                             const Amg::Vector3D& predictedTrackDirection,
                             const Trk::DriftCircleStatus status);
      /**
       * Constructor used by the TP converters
       */
      TRT_DriftCircleOnTrack( const ElementLinkToIDCTRT_DriftCircleContainer& RIO,
                              const Trk::LocalParameters& driftRadius,
                              const Amg::MatrixX& errDriftRadius,
                              IdentifierHash idDE,
                              const Identifier& id,
                              double predictedLocZ,
                              float localAngle,
                              const Trk::DriftCircleStatus status,
                              bool highLevel,
                              double timeOverThreshold);

      /**Destructor */
      virtual ~TRT_DriftCircleOnTrack();

      /**allows copying without losing the type information. Used in Trk::Track*/
      virtual TRT_DriftCircleOnTrack* clone() const override final;

     /** return the global position of this RIO_OnTrack
      @todo convention about z coordinate
      - fullfills Trk::MeasurementBase interface
      */
      virtual const Amg::Vector3D& globalPosition() const override final;

     /** returns the surface for the local to global transformation
      - fullfills Trk::MeasurementBase interface
      */
      virtual const Trk::Surface& associatedSurface() const override final;

      virtual bool rioType(Trk::RIO_OnTrackType::Type type) const override final
      {
        return (type==Trk::RIO_OnTrackType::TRT_DriftCircle);
      }

     /** returns the PrepRawData - is a TRT_DriftCircle in this scope
      - fullfills Trk::RIO_OnTrack interface
      */
      virtual const TRT_DriftCircle* prepRawData() const override final;
      const ElementLinkToIDCTRT_DriftCircleContainer& prepRawDataLink() const;

     /** returns the DE hashID
      - fullfills Trk::RIO_OnTrack interface
      */
      virtual IdentifierHash idDE() const override final;

     /** returns the detector element, assoicated with the PRD of this class
      - fullfills Trk::RIO_OnTrack interface
      */
      virtual const InDetDD::TRT_BaseElement* detectorElement() const override final;

     /** returns the side on which the drift radius is.
      (for more information see the definition of
      TrkEventPrimitives/DriftCirlceSide)
      - extends Trk::RIO_OnTrack interface*/
      Trk::DriftCircleSide side() const;

     /** returns the status of the  drift radius calibration.
      (for more information see the definition of
      TrkEventPrimitives/DriftCirlceStatus)
      - extends Trk::RIO_OnTrack interface*/
      Trk::DriftCircleStatus status() const;


     /** returns true if the high level threshold was passed
      - extends Trk::RIO_OnTrack interface*/
      bool highLevel() const ;

     /** returns time over threshold in ns for valid digits; zero otherwise
      - extends Trk::RIO_OnTrack interface*/
      double timeOverThreshold() const;

      /**returns some information about this RIO_OnTrack.*/
      virtual MsgStream&    dump( MsgStream& out ) const override final;
      /**returns some information about this RIO_OnTrack.*/
      virtual std::ostream& dump( std::ostream& out ) const override final;

      float localAngle() const;
      float positionAlongWire() const;


    private:
    /** ONLY for use in custom convertor
      Allows the custom convertor to reset values when persistying/reading back RoTs*/
      virtual void setValues(const Trk::TrkDetElementBase* detEl,
                             const Trk::PrepRawData* prd) override final;

      /** @calculate and set the global coord of this hit.
       The detector element surface is used. Can be used from the convertor
       after setValues if the element is constructed without a detEl*/
      void setGlobalPositionHelper();

      /** global position to be cached */
      Amg::Vector3D m_globalPosition;

      /**local angle to be written out */
      float m_localAngle;

      /**local position along wire to be written out*/
      float m_positionAlongWire;

      ElementLinkToIDCTRT_DriftCircleContainer m_rio;

      /** the IdentifierHash - probably not used*/
      IdentifierHash                         m_idDE;

      /** the information how the (possibly signed) drift radius should
          be understood */
      Trk::DriftCircleStatus                 m_status;

      /** the high level flag */
      bool                                   m_highLevel;

      /** the time over threshold in nsec */
      double                                 m_timeOverThreshold;

      /** corresponding detector element*/
      const InDetDD::TRT_BaseElement*        m_detEl;

  };

  inline TRT_DriftCircleOnTrack*
  TRT_DriftCircleOnTrack::clone() const
  {
    return new TRT_DriftCircleOnTrack(*this);
  }

  inline const TRT_DriftCircle*
  TRT_DriftCircleOnTrack::prepRawData() const
  {
    // ask first if it is valid ...
    // otherwise it always returns nullptr ...
    if (m_rio.isValid()) {
      return m_rio.cachedElement();
    }
    return nullptr;
    }

  inline
  const ElementLinkToIDCTRT_DriftCircleContainer&
  TRT_DriftCircleOnTrack::prepRawDataLink() const
  {
    return m_rio;
  }

  inline IdentifierHash TRT_DriftCircleOnTrack::idDE() const
  {
    return m_idDE;
  }

  inline const InDetDD::TRT_BaseElement* TRT_DriftCircleOnTrack::detectorElement() const
  {
    return m_detEl;
  }

  inline  Trk::DriftCircleStatus TRT_DriftCircleOnTrack::status() const
  {
    return m_status;
  }

  inline bool TRT_DriftCircleOnTrack::highLevel() const
  {
    return m_highLevel;
  }

  inline double TRT_DriftCircleOnTrack::timeOverThreshold() const
  {
    return m_timeOverThreshold;
  }

  inline float TRT_DriftCircleOnTrack::localAngle() const
  {
    return m_localAngle;
  }

  inline float TRT_DriftCircleOnTrack::positionAlongWire() const
  {
    return m_positionAlongWire;
  }
}

#endif // TRKRIO_ONTRACK_TRT_DRIFTCIRCLEONTRACK_H

