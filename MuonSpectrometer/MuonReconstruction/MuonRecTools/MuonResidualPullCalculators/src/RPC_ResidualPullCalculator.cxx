/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RPC_ResidualPullCalculator.h"

#include "TrkEventUtils/IdentifierExtractor.h"
#include "TrkEventPrimitives/LocalParameters.h"

//================ Constructor =================================================

Muon::RPC_ResidualPullCalculator::RPC_ResidualPullCalculator(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t,n,p)
{
  declareInterface<IResidualPullCalculator>(this);
}

//================ Initialisation =================================================

StatusCode Muon::RPC_ResidualPullCalculator::initialize()
{
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_MSG_DEBUG ("initialize() successful in " << name());
  return StatusCode::SUCCESS;
}

//================ calculate residuals for RPC ==================================
std::array<double,5>
Muon::RPC_ResidualPullCalculator::residuals(
    const Trk::MeasurementBase* measurement,
    const Trk::TrackParameters* trkPar,
    const Trk::ResidualPull::ResidualType /*resType*/,
    const Trk::TrackState::MeasurementType) const {
  std::array<double, 5> residuals{};
  if (!trkPar || !measurement) return residuals;
  Identifier ID = Trk::IdentifierExtractor::extract(measurement);

  if( ID.is_valid() && m_idHelperSvc->isRpc(ID) ) {

    if (measurement->localParameters().parameterKey() == 1) {
      // convention to be interpreted by TrkValTools: 2nd coordinate codes orientation of RPC
        residuals[Trk::loc1] = measurement->localParameters()[Trk::loc1]
          - trkPar->parameters()[Trk::loc1];
    } else {
      ATH_MSG_WARNING ( "RPC ClusterOnTrack does not carry the expected "
                        << "LocalParameters structure!" );
      return residuals;
    }

  } else {
    ATH_MSG_DEBUG ( "Input problem measurement is not RPC." );
    return residuals;
  }
  return residuals;
}

//================ calculate residuals and pulls for RPC ==================================
std::optional<Trk::ResidualPull> Muon::RPC_ResidualPullCalculator::residualPull(
    const Trk::MeasurementBase* measurement,
    const Trk::TrackParameters* trkPar,
    const Trk::ResidualPull::ResidualType resType,
    const Trk::TrackState::MeasurementType) const {

  if (!trkPar || !measurement) return std::nullopt;

  Identifier ID = Trk::IdentifierExtractor::extract(measurement);
  if( ID.is_valid() && m_idHelperSvc->isRpc(ID) ) {

    // if no covariance for the track parameters is given the pull calculation is not valid
    const AmgSymMatrix(5)* trkCov = trkPar->covariance();
    bool pullIsValid = (trkCov);

    // calculate residual
    std::vector<double> residual(1);
    std::vector<double> pull(1);
    if (measurement->localParameters().parameterKey() == 1) {
      // convention to be interpreted by TrkValTools: 2nd coordinate codes orientation of RPC
        residual[Trk::loc1] = measurement->localParameters()[Trk::loc1]
          - trkPar->parameters()[Trk::loc1];
    } else {
      ATH_MSG_WARNING ( "RPC ClusterOnTrack does not carry the expected "
                        << "LocalParameters structure!" );
      return std::nullopt;
    }

    // calculate pull
    if (pullIsValid)
      pull[Trk::loc1] = calcPull(residual[Trk::loc1],
                                 measurement->localCovariance()(Trk::loc1,Trk::loc1),
                                 (*trkCov)(Trk::loc1,Trk::loc1),
                                 resType);
    else
      pull[Trk::loc1] = calcPull(residual[Trk::loc1],
                                 measurement->localCovariance()(Trk::loc1,Trk::loc1),
                                 0,
                                 resType);

    // create the Trk::ResidualPull.
    ATH_MSG_DEBUG ( "Calculating Pull for channel " << m_idHelperSvc->toString(ID) << " residual " << residual[Trk::loc1] << " pull " << pull[Trk::loc1] );
    return std::make_optional<Trk::ResidualPull>(
        std::move(residual), std::move(pull), pullIsValid, resType, 1);

  } else {
    ATH_MSG_DEBUG ( "Input problem measurement is not RPC." );
    return std::nullopt;
  }
}


/////////////////////////////////////////////////////////////////////////////
/// calc pull in 1 dimension
/////////////////////////////////////////////////////////////////////////////
double Muon::RPC_ResidualPullCalculator::calcPull(
    const double residual,
    const double locMesCov,
    const double locTrkCov,
    const Trk::ResidualPull::ResidualType& resType ) {

    double ErrorSum(0.0);
    if (resType == Trk::ResidualPull::Unbiased) {
      if( locMesCov + locTrkCov > 0 ) ErrorSum = sqrt(locMesCov + locTrkCov);
    } else if (resType == Trk::ResidualPull::Biased) {
      if ((locMesCov - locTrkCov) < 0.) {
            return 0;
        }
        ErrorSum = sqrt(locMesCov - locTrkCov);
    } else ErrorSum = sqrt(locMesCov);
    if (ErrorSum != 0) return residual/ErrorSum;
    return 0;
}
