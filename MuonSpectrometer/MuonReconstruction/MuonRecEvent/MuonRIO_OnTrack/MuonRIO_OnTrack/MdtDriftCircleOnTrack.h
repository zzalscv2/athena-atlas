/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRIO_ONTRACK_MDTDRIFTCIRCLEONTRACK_H
#define MUONRIO_ONTRACK_MDTDRIFTCIRCLEONTRACK_H

// Base classes
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "MuonPrepRawData/MdtPrepData.h"


#include "TrkEventPrimitives/DriftCircleSide.h"
#include "TrkEventPrimitives/DriftCircleStatus.h"

#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "AthLinks/ElementLink.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "CxxUtils/CachedUniquePtr.h"


#include <cassert>

using ElementLinkToIDC_MDT_Container = ElementLink<Muon::MdtPrepDataContainer>;

namespace Trk{
    class LocalParameters;
    class ITrkEventCnvTool;
}

namespace Muon
{
    class MuonEventCnvTool;
    class MdtDriftCircleOnTrackCreator;

/** @brief This class represents the corrected MDT measurements,
    where the corrections include the effects of wire sag etc.*/
class MdtDriftCircleOnTrack final: public Trk::RIO_OnTrack {
public:

    friend class  Trk::ITrkEventCnvTool;
    friend class Muon::MuonEventCnvTool;
    friend class Muon::MdtDriftCircleOnTrackCreator;

    /** @brief Default ctor - for use by POOL only. Do not use yourself!*/
    MdtDriftCircleOnTrack() = default;

    MdtDriftCircleOnTrack(const MdtDriftCircleOnTrack &);
    MdtDriftCircleOnTrack &operator=(const MdtDriftCircleOnTrack &);

    MdtDriftCircleOnTrack(MdtDriftCircleOnTrack&&) = default;
    MdtDriftCircleOnTrack &operator=(MdtDriftCircleOnTrack&&) = default;



    /** @brief Constructor for both non-sagged and sagged wire (default is non-sagged, since saggedSurface=0).
        Using the constructor implies the sign solution of the MDT is resolved.

        The identifier hash, det element pointer etc are taken from RIO (the MdtPrepData pointer)
        so it is <b>vital</b> that this pointer is not zero.

        @param [in] RIO                  pointer to parent MdtPrepData used to create this RIO_OnTrack
        @param [in] locPos               local position (i.e. drift radius) of the measurement.
                                         This drift radius includes all corrections
        @param [in] errDriftRadius       the Amg::MatrixX (should be 1d, and contains the error on the drift radius measurement)
        @param [in] driftTime            drift time used to obtain the drift radius
        @param [in] status               status of drift circle. See Trk::DriftCircleStatus for definitions.
        @param [in] globDir              the predicted/extrapolated track direction (used to calculate global position)
        @param [in] positionAlongWire    this is the extrapolated position along the wire. i.e. it is NOT a measurement, and so should not
                                         be included in the Amg::MatrixX
        @param [in] saggedSurface        pointer to a Trk::StraightLineSurface created at the sagged position of the wire. It is not mandatory
                                         (i.e. if nothing is passed, or a zero pointer then the non-sagged wire is used). If something is passed,
                                         this object will own it (i.e. it will be deleted by this object)
        @param [in] creationParameters   A bitword containing information about the construction of the ROT.
                                         See m_rotCreationParameters for details.
        */
    MdtDriftCircleOnTrack(const MdtPrepData* RIO,
                          Trk::LocalParameters&& locPos,
                          Amg::MatrixX&& errDriftRadius,
                          const double driftTime,
                          const Trk::DriftCircleStatus status,
                          const Amg::Vector3D& globDir,
                          const double positionAlongWire,
                          const MuonDriftCircleErrorStrategy& errorStrategy);

    /** @brief Constructor without global direction for both non-sagged and sagged wire (default is non-sagged, since saggedSurface=0).
        This necessarily implies that the DriftCircleStatus is UNDECIDED, since without the GlobalDirection it cannot be worked out.
        In order to have a fully defined MdtDriftCircleOnTrack it is necessary to use the complete constructor (above)

        The identifier hash, det element pointer etc are taken from RIO (the MdtPrepData pointer)
        so it is vital that this pointer is not zero.

        @param [in] RIO                  pointer to parent MdtPrepData used to create this RIO_OnTrack
        @param [in] locPos               local position (i.e. drift radius) of the measurement. This drift radius includes
        all corrections
        @param [in] errDriftRadius       the Amg::MatrixX (should be 1d, and contains the error on the drift radius measurement)
        @param [in] driftTime            drift time used to obtain the drift radius
        @param [in] status               status of drift circle. See Trk::DriftCircleStatus for definitions.
        @param [in] positionAlongWire    this is the extrapolated position along the wire. i.e. it is NOT a measurement, and so should not
                                         be included in the Amg::MatrixX
        @param [in] saggedSurface        pointer to a Trk::StraightLineSurface created at the sagged position of the wire. It is not mandatory
                                         (i.e. if nothing is passed, or a zero pointer then the non-sagged wire is used). If something is passed,
                                         this object will own it (i.e. it will be deleted by this object)
        @param [in] creationParameters   A bitword containing information about the construction of the ROT.
                                         See m_rotCreationParameters for details.
        */
    MdtDriftCircleOnTrack(const MdtPrepData* RIO,
                          Trk::LocalParameters&& locPos,
                          Amg::MatrixX&& errDriftRadius,
                          const double driftTime,
                          const Trk::DriftCircleStatus status,
                          const double positionAlongWire,
                          const MuonDriftCircleErrorStrategy& errorStrategy);


     // Alternate constructor that doesn't dereference the RIO link.
    MdtDriftCircleOnTrack(const ElementLinkToIDC_MDT_Container& RIO,
                          Trk::LocalParameters&& locPos,
                          Amg::MatrixX&& errDriftRadius,
                          const Identifier& id,
                          const MuonGM::MdtReadoutElement* detEl,
                          const double  driftTime,
                          const Trk::DriftCircleStatus status,
                          const double positionAlongWire,
                          const double localAngle,
                          const MuonDriftCircleErrorStrategy& errorStrategy);

    /** @brief Destructor: */
    virtual ~MdtDriftCircleOnTrack() = default;

    /** @brief Returns the side on which the drift radius is wrt to the track */
    Trk::DriftCircleSide side() const;

     /** @brief Returns the status of the  drift radius calibration.
        (for more information see the definition of TrkEventPrimitives/DriftCircleStatus)
         - extends Trk::RIO_OnTrack interface*/
    Trk::DriftCircleStatus status() const;

    /** @copydoc Trk::RIO_OnTrack::clone()  */
    virtual MdtDriftCircleOnTrack* clone() const override final;

    /** @brief Returns the PrepRawData used to create this corrected measurement */
    virtual const MdtPrepData* prepRawData() const override final;
    const ElementLinkToIDC_MDT_Container& prepRawDataLink() const;

    /** @brief Returns the hashID of the PRD collection */
    IdentifierHash collectionHash() const;

    /** @brief Returns an invalid hash @todo Remove*/
    virtual IdentifierHash idDE() const override final{ return IdentifierHash(); }

    /** @brief Returns the detector element, assoicated with the PRD of this class*/
    virtual const MuonGM::MdtReadoutElement* detectorElement() const override final;

    /** @brief Returns the surface on which this measurement was taken.
        - If hasSaggedSurface()==false, then the surface will be that of the matching Detector Element
        - If hasSaggedSurface()==true, then the surface will be a special surface, representing the sagged position
        of the wire at the coords of this measurement.*/
    virtual const Trk::StraightLineSurface& associatedSurface() const override final;

    /** @brief Returns the global Position.
    Be aware that this is calculated from the predicted position along
    the tube, and the drift radius. i.e. it is partly inferred from other data,
    and so is not a 'true' measurement.*/
    virtual const Amg::Vector3D& globalPosition() const override final;

    virtual bool rioType(Trk::RIO_OnTrackType::Type type) const override final {
      return (type == Trk::RIO_OnTrackType::MdtDriftCircle);
    }

    /** @brief Returns the value of the drift radius.
    Obviously to use this method you need to cast to a MdtDriftCircleOnTrack if you have a pointer or reference
    to the base class (Trk::RIO_OnTrack). An alternative is to use:
    @code localParameters().get(Trk::driftRadius) @endcode
    @warning This method assumes that the MdtDriftCircleOnTrack has been filled correctly. There are no checks!
    */
    double driftRadius() const;

    /** @brief Returns the value of the drift time used to obtain the drift radius.
    Obviously to use this method you need to cast to a MdtDriftCircleOnTrack if you have a pointer or reference
    to the base class (Trk::RIO_OnTrack).
    */
    double driftTime() const;

    /** @brief Returns the position along the wire, as determined by the extrapolation used when creating this Trk::RIO_OnTrack.*/
    double positionAlongWire() const;

    /** @brief Returns the local angle, as determined by the extrapolation used when creating this Trk::RIO_OnTrack.*/
    double localAngle() const;

    /** @brief Get information about the creation strategy used by Muon::MdtDriftCircleOnTrackCreator when making this object.*/
    const MuonDriftCircleErrorStrategy& errorStrategy() const;

    /** @brief Dumps information about the PRD*/
    virtual MsgStream&    dump( MsgStream&    stream) const override final;

    /** @brief Dumps information about the PRD*/
    virtual std::ostream& dump( std::ostream& stream) const override final;

    // /////////////////////////////////////////////////////////////////
    // Private data:
    // /////////////////////////////////////////////////////////////////
private:
    /**@brief Sets the local parameters.
    @warning Only intended for use by the Muon::MdtDriftCircleOnTrackCreator*/
    virtual void setLocalParameters( const Trk::LocalParameters& locPos);

    /**@brief Sets the DetElement and Trk::PrepRawData pointers after reading from disk.
    @warning Only intended for use by persistency convertors*/
    virtual void setValues(const Trk::TrkDetElementBase*,
                           const Trk::PrepRawData*) override final;

    /** @brief Uses the passed loc3Dframe to calculate and set the global coord of this hit.
    If there is a sagged wire defined, this will be used for the transformation, otherwise the detector element surface is used*/
    void setGlobalPosition(Amg::Vector3D&& loc3Dframe) const;

    //Sets the error strategy, only used by the Muon::MdtDriftCircleOnTrackCreator
    void setErrorStrategy(const MuonDriftCircleErrorStrategy& strategy);

    /**information on the status of the Mdt measurement - see Trk::DriftCircleStatus for definitions*/
    Trk::DriftCircleStatus m_status{Trk::DriftCircleStatus::UNDECIDED};

    /** the pointer to the MdtPrepData object (mutable because it might need to be recreated when reading tracks)*/
    ElementLinkToIDC_MDT_Container m_rio{};

    /** global position of the measurement. */
    CxxUtils::CachedUniquePtr<Amg::Vector3D> m_globalPosition{};

    /** This angle is the position of the point of closest approach in cylindrical coordinates, and is needed to construct the global position*/
    double m_localAngle{0.};

    /** This is the position of the point of closest approach, in the local z coord (i.e. along the wire), and is needed to construct the global position*/
    double m_positionAlongWire{0.};

    /** This is the drift time used to obtain the drift radius */
    double m_driftTime{0.};

    /** Records information about the 'strategy' used by Muon::MdtDriftCircleOnTrackCreator to make this object.*/
    MuonDriftCircleErrorStrategy m_errorStrategy{};

    /*** Pointer to the detector element. Needed if no prepData is present*/
    const MuonGM::MdtReadoutElement* m_detEl{nullptr};

};

// /////////////////////////////////////////////////////////////////
// Inline methods:
// /////////////////////////////////////////////////////////////////:

inline Trk::DriftCircleSide MdtDriftCircleOnTrack::side() const {
    if (m_status == Trk::UNDECIDED) return Trk::NONE;
    if (localParameters()[Trk::driftRadius] < 0. ) return Trk::LEFT;
    return Trk::RIGHT;
}

inline Trk::DriftCircleStatus MdtDriftCircleOnTrack::status() const { return m_status; }
inline MdtDriftCircleOnTrack* MdtDriftCircleOnTrack::clone() const { return new MdtDriftCircleOnTrack(*this); }
inline const MdtPrepData* MdtDriftCircleOnTrack::prepRawData() const {
    if (m_rio.isValid()) return m_rio.cachedElement();
    return nullptr;
}

inline const ElementLinkToIDC_MDT_Container& MdtDriftCircleOnTrack::prepRawDataLink() const { return m_rio; }

inline IdentifierHash MdtDriftCircleOnTrack::collectionHash() const {
    return prepRawData()->collectionHash();
}

inline const MuonGM::MdtReadoutElement* MdtDriftCircleOnTrack::detectorElement() const {
    return prepRawData() ? prepRawData()->detectorElement() : m_detEl;
}
inline const Trk::StraightLineSurface& MdtDriftCircleOnTrack::associatedSurface() const {
    return detectorElement()->surface(identify());
}

inline void MdtDriftCircleOnTrack::setLocalParameters(const Trk::LocalParameters& locParams) { m_localParams = locParams; }
inline void MdtDriftCircleOnTrack::setErrorStrategy(const MuonDriftCircleErrorStrategy& strategy) { m_errorStrategy= strategy; }
inline double MdtDriftCircleOnTrack::driftRadius() const {
    return localParameters().get(Trk::driftRadius);
}
inline double MdtDriftCircleOnTrack::driftTime() const { return m_driftTime; }
inline double MdtDriftCircleOnTrack::positionAlongWire() const { return m_positionAlongWire; }
inline double MdtDriftCircleOnTrack::localAngle() const { return m_localAngle; }
inline const MuonDriftCircleErrorStrategy& MdtDriftCircleOnTrack::errorStrategy() const {
    return m_errorStrategy;
}
inline void MdtDriftCircleOnTrack::setValues(const Trk::TrkDetElementBase* detEl,
                                             const Trk::PrepRawData* prd) {

    m_detEl = dynamic_cast<const MuonGM::MdtReadoutElement*>(detEl);
    if (!prd) return;
    if (!prd->type(Trk::PrepRawDataType::MdtPrepData)) {
        throw std::runtime_error("No Mdt prd given to MdtDriftCircleOnTrack");
    }
    m_rio.setElement(static_cast<const MdtPrepData*>(prd));
}

}

#endif // MUONRIOONTRACK_MUONDRIFTCIRCLEONTRACK_H


