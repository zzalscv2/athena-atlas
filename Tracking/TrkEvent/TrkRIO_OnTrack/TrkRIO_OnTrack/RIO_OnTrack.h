/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RIO_OnTrack.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKRIO_ONTRACK_RIO_ONTRACK_H
#define TRKRIO_ONTRACK_RIO_ONTRACK_H

// Amg
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
// Identifier
#include "Identifier/Identifier.h"

#include <iosfwd>
#include <atomic>
#include <memory>

class IdentifierHash;
class MsgStream;
class RIO_OnTrackCnv_p1;
class RIO_OnTrackCnv_p2;


namespace Trk {

  class PrepRawData;
  class TrkDetElementBase;
  class ITrkEventCnvTool;

/** @class RIO_OnTrack

  Class to handle RIO On Tracks  ROT) for InDet and Muons,
  it inherits from the common MeasurementBase.

  The Track holds a vector of TrackStateOnSurface
  that carry the MeasurmentBase class pointers.

  Need a multimap{RIO, ROT} to go from RIO to ROT
  and a map{ROT, Track} to go from ROT to Track

  @author Veronique.Boisvert@cern.ch
  @author Edward.Moyse@cern.ch
  @author Andreas.Salzburger@cern.ch
  @author Christos Anastopoulos (Athena MT)
 */

  namespace RIO_OnTrackType{
    enum Type{
        PixelCluster=0,
        SCTCluster=1,
        TRT_DriftCircle=2,
        MdtDriftCircle=3,
        MuonCluster=4,
        PlanarCluster=5,
        HGTD_Cluster=6
    };
  }

  class RIO_OnTrack
    : public MeasurementBase
    , public Trk::ObjectCounter<Trk::RIO_OnTrack>
  {

    friend class ITrkEventCnvTool;

    public:
      /** Constructor with parameters and without externalPrediction*/
     RIO_OnTrack(LocalParameters&& locpars,
                 Amg::MatrixX&& loccov,
                 const Identifier& id);

     /** Default Constructor for POOL */
     RIO_OnTrack() = default;
     /** Copy Constructor */
     RIO_OnTrack(const RIO_OnTrack& rot) = default;
     RIO_OnTrack(RIO_OnTrack&& rot) = default;

     /** Assignment operator */
     RIO_OnTrack& operator=(const RIO_OnTrack& rot) = default;
     RIO_OnTrack& operator=(RIO_OnTrack&& rot) = default;

     /** Destructor */
     virtual ~RIO_OnTrack() = default;

     /** Pseudo-constructor, needed to avoid excessive RTTI*/
     virtual RIO_OnTrack* clone() const override = 0;

     /** NVI clone returning unique_ptr*/
     std::unique_ptr<RIO_OnTrack> uniqueClone() const {
       return std::unique_ptr<RIO_OnTrack>(clone());
      };

     /** returns the surface for the local to global transformation
      - interface from MeasurementBase */
      virtual const Surface& associatedSurface() const override = 0;

     /**Interface method to get the global Position
      - interface from MeasurementBase */
      virtual const Amg::Vector3D& globalPosition() const override = 0;

      /** Extended method checking the type*/
      virtual bool type(MeasurementBaseType::Type type) const override final
      {
        return (type == MeasurementBaseType::RIO_OnTrack);
      }

      /** Method checking the Rio On Track type*/
      virtual bool rioType(RIO_OnTrackType::Type type) const = 0;

      /**returns the some information about this RIO_OnTrack. */
      virtual MsgStream&    dump( MsgStream& out ) const override;

      /**returns the some information about this RIO_OnTrack. */
      virtual std::ostream& dump( std::ostream& out ) const override;

     /** returns the PrepRawData (also known as  RIO) object to which this RIO_OnTrack is associated.
      Can be null (in case where the Trk::PrepRawData is not persistified).
      Use Detector Element if possible (this is always there).
      - extends MeasurementBase */
      virtual const Trk::PrepRawData* prepRawData() const = 0;

       /** returns the DE hashID
      - extends MeasurementBase */
      virtual IdentifierHash idDE() const = 0;

       /** returns the detector element, assoicated with the PRD of this class
      - extends MeasurementBase */
      virtual const TrkDetElementBase* detectorElement() const = 0;

       /** return the identifier
      -extends MeasurementBase */
      virtual Identifier identify() const final;

    protected:
      friend class ::RIO_OnTrackCnv_p1;
      friend class ::RIO_OnTrackCnv_p2;
      /** ONLY for use in custom convertor
      Allows the custom convertor to reset values when persistying/reading back RoTs*/
      virtual void setValues(
          const Trk::TrkDetElementBase* detEl,
          const Trk::PrepRawData* prd)=0;

      /**Identifier of the RIO_OnTrack (comes from the associated Trk::PrepRawData)*/
      Identifier m_identifier{};
  };

  inline Identifier RIO_OnTrack::identify() const
  { return m_identifier; }
}

#endif // TRKRIO_ONTRACK_RIO_ONTRACK_H

