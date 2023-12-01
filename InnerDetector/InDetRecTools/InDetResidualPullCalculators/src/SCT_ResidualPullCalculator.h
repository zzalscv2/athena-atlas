/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// SCT_ResidualPullCalculator.h
//   Header file for SCT_ResidualPullCalculator tool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef INDET_SCT_RESIDUALPULLCALCULATOR_H
#define INDET_SCT_RESIDUALPULLCALCULATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "TrkEventPrimitives/ResidualPull.h"

namespace InDet {


/**
 * The InDet::SCT_ResidualPullCalculator is an AlgTool to calculate the residual and pull
 * of a measurement and the related track state for the SCT. It is called by the
 * Trk::ResidualPullCalculator in the case of SCT measurements.
 * This tools assumes in any cases that the given measurement belongs to the SCT!
 * Normaly it should not be called directly, but through the Trk::ResidualPullCalculator.
 */

class SCT_ResidualPullCalculator final : virtual public Trk::IResidualPullCalculator, public AthAlgTool {
public:
    using Trk::IResidualPullCalculator::residualPull;

    SCT_ResidualPullCalculator(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~SCT_ResidualPullCalculator() = default;
    virtual StatusCode initialize() override;


    /** This function returns (creates!) a Trk::ResidualPull object, which contains the values
     * of residual and pull for the given measurement and track state.
     * The track state can be an unbiased one (which can be retrieved by the Trk::IUpdator),
     * a biased one (which contains the measurement),
     * or a truth state.
     * The enum ResidualType must be set according to this, otherwise the pulls will be wrong.
     * Residuals differ in all three cases; please be aware of this!
     */
    virtual std::optional<Trk::ResidualPull> residualPull(
        const Trk::MeasurementBase* measurement,
        const Trk::TrackParameters* trkPar,
        const Trk::ResidualPull::ResidualType resType,
        const Trk::TrackState::MeasurementType) const override;

    /** This function is a light-weight version of the function above, designed for track fitters
     * where speed is critical.
     */
    virtual std::array<double,5> residuals(
        const Trk::MeasurementBase* measurement,
        const Trk::TrackParameters* trkPar,
        const Trk::ResidualPull::ResidualType resType,
        const Trk::TrackState::MeasurementType) const override;

private:
    static double calcPull(
        const double residual,
        const double locMesCov,
        const double locTrkCov,
        const Trk::ResidualPull::ResidualType&,
        bool& pullIsValid ) ;
}
; // end class
} // end namespace InDet

#endif //INDET_SCT_RESIDUALPULLCALCULATOR_H
