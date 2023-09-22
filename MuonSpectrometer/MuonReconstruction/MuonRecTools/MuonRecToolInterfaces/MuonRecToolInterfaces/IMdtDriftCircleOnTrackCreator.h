/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_IMDTDRIFTCIRCLEONTRACKCREATOR_H
#define MUON_IMDTDRIFTCIRCLEONTRACKCREATOR_H

#include "GaudiKernel/IAlgTool.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonPrepRawData/MdtDriftCircleStatus.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "TrkEventPrimitives/DriftCircleSide.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

#include <memory>
namespace Muon {

    class MdtPrepData;

    /** @brief Interface for tools calibrating MdtPrepData, turning them into Muon::MdtDriftCircleOnTrack object.
               The interface inherits from Trk::IRIO_OnTrackCreator.
    */
    class IMdtDriftCircleOnTrackCreator : public Trk::IRIO_OnTrackCreator {
    public:
        using MdtRotPtr = MdtDriftCircleOnTrack*;
        /** @brief Default desructor*/
        virtual ~IMdtDriftCircleOnTrackCreator() = default;
        DeclareInterfaceID(Muon::IMdtDriftCircleOnTrackCreator, 1, 0);
        /** @brief Calibrate a MdtPrepData object. The result is stored in a new MdtDriftCircleOnTrack object.
            Included calibrations:
                - Conversion t->r using MdtCalibrationSvc
                - Wire sag + chamber deformations (if available)
                - Special treatment for cosmics if switched on
            @param prd  MdtPrepData object.
            @param globalPos GlobalPosition (including second coordinate along the tube).
            @param strategy optional drift circle error strategy to override the default
            @return Fully calibrated MdtDriftCircleOnTrack (the user must delete this object when it is no longer needed).
        */
        virtual MdtRotPtr createRIO_OnTrack(const MdtPrepData& DC, 
                                            const Amg::Vector3D& GP, 
                                            const Amg::Vector3D* GD = nullptr,
                                            const double t0Shift = 0, 
                                            const MuonDriftCircleErrorStrategy* strategy = nullptr,
                                            const double beta = 1, 
                                            const double tTrack = 0) const = 0;

        /** @brief Update of the sign of the drift radius.
        @param DCT reference to the Muon::MdtDriftCircleOnTrack of which the sign should be updated.
        @param si  Trk::DriftCircleSide indicating whether the muon passed on the left or right side of the wire.
        */
        virtual void updateSign(MdtDriftCircleOnTrack& DCT, const Trk::DriftCircleSide si) const = 0;

        /** @brief Update error of a ROT without changing the drift radius
        @param DCT reference to the Muon::MdtDriftCircleOnTrack of which the sign should be updated.
        @param tp Reference to the extrapolated/predicted TrackParameters at this MdtPrepData
        @param strategy optional drift circle error strategy to override the default
        @return New ROT with updated error. (the user must delete this object when it is no longer needed).
        */
        virtual MdtRotPtr updateError(const MdtDriftCircleOnTrack& DCT, 
                                      const Trk::TrackParameters* pars = nullptr,
                                      const MuonDriftCircleErrorStrategy* strategy = nullptr) const = 0;

        /** @brief Returns calibrated MdtDriftCircleOnTrack.
        Overrides the IRIO_OnTrackCreator method to add an error strategy object.
        @param prd Reference to a Trk::PrepRawData object (which should always be a Muon::MdtPrepData in this case)
        @param tp Reference to the extrapolated/predicted TrackParameters at this MdtPrepData
        @return calibrated MdtDriftCircleOnTrack. Memory management is passed to user.
        */
        using Trk::IRIO_OnTrackCreator::correct;
        virtual MdtRotPtr correct(const MdtPrepData& prd, 
                                  const Trk::TrackParameters& tp,
                                  const MuonDriftCircleErrorStrategy* strategy, 
                                  const double beta = 1,
                                  const double tTrack = 0) const = 0;

        /** @brief Returns the default error strategy object */
        virtual const MuonDriftCircleErrorStrategy& errorStrategy() const = 0;
    };
}  // namespace Muon

#endif  // MUON_IMDTDRIFTCIRCLEONTRACKCREATOR_H
