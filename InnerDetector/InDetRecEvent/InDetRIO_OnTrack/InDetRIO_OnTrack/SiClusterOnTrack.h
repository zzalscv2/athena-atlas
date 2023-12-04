/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SiClusterOnTrack.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKRIO_ONTRACK_SICLUSTERONTRACK_H
#define TRKRIO_ONTRACK_SICLUSTERONTRACK_H

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "Identifier/IdentifierHash.h"

class SiClusterOnTrackCnv_p1;

namespace Trk
{
  class LocalParameters;
}

namespace InDetDD
{
  class SiDetectorElement;
}

namespace InDet {

  class SiCluster;

  /**@class SiClusterOnTrack
  RIO_OnTrack base class for Silicon detector in the InnerDetector.
  It combines the common members / structures from SCT and
  Pixel clusters on tracks.

  @author Veronique.Boisvert@cern.ch, Edward.Moyse@cern.ch, Andreas.Salzburger@cern.ch
  @author  Christos Anastopoulos (Athena MT)
   */
  class SiClusterOnTrack :   public Trk::RIO_OnTrack{


    public:
      /** Default Constructor - needed for POOL */
      SiClusterOnTrack();
      // copy constructor
      SiClusterOnTrack( const SiClusterOnTrack& rot) = default;
      // assignment operator:
      SiClusterOnTrack& operator=( const SiClusterOnTrack& rot) = default;
      // move constructor
      SiClusterOnTrack( SiClusterOnTrack&& rot) = default;
      // move assignment operator:
      SiClusterOnTrack& operator=(SiClusterOnTrack&& rot) = default;
      /** Destructor:*/
      virtual ~SiClusterOnTrack() = default;

      /** Constructors with parameters :
      LocalParameters,
      ErrorMatrix,
      idDE */
      SiClusterOnTrack(Trk::LocalParameters&& locpos,
                       Amg::MatrixX&& locerr,
                       const IdentifierHash& idDE,
                       const Identifier& id,
                       bool isbroad=false);

      /** Constructors with parameters :
      LocalParameters,
      ErrorMatrix,
      idDE,
      GlobalPosition,
      */
      SiClusterOnTrack( Trk::LocalParameters&& locpos,
                        Amg::MatrixX&& locerr,
                        const IdentifierHash& idDE,
                        const Identifier& id,
                        const Amg::Vector3D& globalPosition,
                        bool isbroad=false);


      /** returns global position (gathered through Surface constraint)
      - fullfills Trk::MeasurementBase interface */
      virtual const Amg::Vector3D& globalPosition() const override;

      virtual bool rioType(Trk::RIO_OnTrackType::Type type) const override = 0;

      /** returns the DE hashID*
      - fullfills Trk::RIO_OnTrack interface*/
      virtual IdentifierHash idDE() const override;

      bool isBroadCluster() const;

	    /**returns some information about this RIO_OnTrack.
      - fullfills Trk::RIO_OnTrack interface*/
      virtual MsgStream&    dump( MsgStream& out ) const override;

	    /**returns some information about this RIO_OnTrack.
      - fullfills Trk::RIO_OnTrack interface*/
      virtual std::ostream& dump( std::ostream& out ) const override;

    protected:
      friend class ::SiClusterOnTrackCnv_p1;

      /** ONLY for use in custom convertor
      Allows the custom convertor to reset values when persistying/reading back RoTs*/
      virtual void setValues(const Trk::TrkDetElementBase* detEl,
                             const Trk::PrepRawData* prd) override = 0;

      /** The IdentifierHash - probably not used*/
      IdentifierHash  m_idDE;
      /** The global position */
      Amg::Vector3D m_globalPosition;
      bool m_isbroad;
  };

  inline const Amg::Vector3D& SiClusterOnTrack::globalPosition() const {
      return m_globalPosition;
  }

  inline  IdentifierHash SiClusterOnTrack::idDE() const
  {
    return m_idDE;
  }

  inline bool SiClusterOnTrack::isBroadCluster() const
  {
    return m_isbroad;
  }
}

#endif // TRKRIO_ONTRACK_SICLUSTERONTRACK_H
