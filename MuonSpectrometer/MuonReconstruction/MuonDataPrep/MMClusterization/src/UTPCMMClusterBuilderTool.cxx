/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "UTPCMMClusterBuilderTool.h"

#include <cmath>
#include <numeric>

#include "GaudiKernel/SystemOfUnits.h"

#define DEFINE_VECTOR(dType, varName, nEles)  \
    std::vector<dType> varName{};             \
    varName.reserve(nEles);   

namespace {

    bool sortTracks(std::tuple<int, double>& a, std::tuple<int, double>& b) {
        if (std::get<0>(a) != std::get<0>(b)) {
            return std::get<0>(a) < std::get<0>(b);
        }
        return std::get<1>(a) < std::get<1>(b);
    }
}  // namespace

namespace Muon{

using RIO_Author = IMMClusterBuilderTool::RIO_Author;

UTPCMMClusterBuilderTool::UTPCMMClusterBuilderTool(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t, n, p) {
    declareInterface<IMMClusterBuilderTool>(this);
}

StatusCode UTPCMMClusterBuilderTool::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::getClusters(const EventContext& /*ctx*/,
                                                       std::vector<MMPrepData>&& MMprds,
                                                       std::vector<std::unique_ptr<MMPrepData>>& clustersVect) const  {
    
    LaySortedPrds MMprdsPerLayer{};
    for (MMPrepData& MMprd : MMprds) {
        Identifier id = MMprd.identify();
        int layer = 4 * (m_idHelperSvc->mmIdHelper().multilayer(id) - 1) + (m_idHelperSvc->mmIdHelper().gasGap(id) - 1);
        ATH_MSG_DEBUG("Sorting PRD into layer layer: " << layer << " gas_gap " << m_idHelperSvc->mmIdHelper().gasGap(id) << "multilayer "
                                                       << m_idHelperSvc->mmIdHelper().multilayer(id));
        MMprdsPerLayer.at(layer).push_back(std::move(MMprd));
    }

    // sort MMPrds by channel
    for (std::vector<MMPrepData>& prdInLay :  MMprdsPerLayer) {
        std::sort(prdInLay.begin(), prdInLay.end(), [this](const MMPrepData& a, const MMPrepData& b) {
            return m_idHelperSvc->mmIdHelper().channel(a.identify()) < m_idHelperSvc->mmIdHelper().channel(b.identify());
        });
    }

    for (std::vector<MMPrepData>& MMprdsOfLayer : MMprdsPerLayer) {
        std::vector<double> stripsLocalPosX;
        stripsLocalPosX.reserve(MMprdsOfLayer.size());

        for (MMPrepData& MMprd : MMprdsOfLayer) {
            Identifier id_prd = MMprd.identify();
            int gasGap = m_idHelperSvc->mmIdHelper().gasGap(id_prd);
            stripsLocalPosX.push_back(
                ((gasGap % 2 == 0 && m_digiHasNegativeAngles) ? -1 : 1) *
                MMprd.localPosition().x());  // flip local positions for even gas gaps to reduce hough space to positiv angles

            // determine angle to IP for debug reasons
            double globalR = MMprd.globalPosition().perp();
            double Tan = globalR / MMprd.globalPosition().z();
            double angleToIp = std::atan(Tan) / Gaudi::Units::degree;

            ATH_MSG_DEBUG("Hit channel: " << m_idHelperSvc->toString(id_prd) << " time " << MMprd.time() << " drift dist "
                                          << MMprd.driftDist() << " localPosX " <<Amg::toString(MMprd.localPosition()) 
                                          << " angleToIp " << angleToIp);
        }
        if (MMprdsOfLayer.size() < 2) continue;

        // move hits to the origin of the coordinate system to exclude extrapolation error and to keep the size of the hough space small
        double meanX = std::accumulate(stripsLocalPosX.begin(), stripsLocalPosX.end(), 0.0) / stripsLocalPosX.size();
        ATH_MSG_DEBUG("Got mean element " << meanX);
        std::for_each(stripsLocalPosX.begin(), stripsLocalPosX.end(), [meanX](double& d) { d -= meanX; });
        ATH_MSG_VERBOSE(stripsLocalPosX);

        // vector of 0s, ones and twos to indicate if strip was already used (0=unused;1=used;2=rejected as cross talk)
        std::vector<int> flag = std::vector<int>(MMprdsOfLayer.size(), 0);

        while (true) {
            ATH_MSG_DEBUG("while true");
            std::vector<int> idx_goodStrips;
            StatusCode sc = runHoughTrafo(MMprdsOfLayer, stripsLocalPosX, flag, idx_goodStrips);

            ATH_MSG_DEBUG("Hough done");
            // if(sc.isFailure()) break;
            if (idx_goodStrips.size() < 3 || sc.isFailure()) {
                // break; // should be already catched in runHough but for safety once more here
                idx_goodStrips.clear();
                for (size_t i_hit = 0; i_hit < MMprdsOfLayer.size(); i_hit++) {
                    if (flag.at(i_hit) == 0) idx_goodStrips.push_back(i_hit);
                }
                ATH_MSG_DEBUG("Hough Trafo failed, try to fit everything");
            }

            // remove strips induced by crosstalk till no more strip was removed
            int nStripsCutByCrosstalk = 0;
            while (!applyCrossTalkCut(idx_goodStrips, MMprdsOfLayer, flag, nStripsCutByCrosstalk).isFailure()) {}
            ATH_MSG_DEBUG("After cutting CT idx_goddStrips " << idx_goodStrips << " flag " << flag);

            std::vector<Identifier> stripsOfCluster;
            std::vector<uint16_t> stripsOfClusterChannels;
            std::vector<short int> stripsOfClusterTimes;
            std::vector<int> stripsOfClusterCharges;
            std::vector<float> stripsOfClusterDriftDists;
            std::vector<Amg::MatrixX> stripsOfClusterDriftDistErrors;
            std::vector<float> stripsOfClusterLocalPos;  // needed for the final fit function
            stripsOfCluster.reserve(idx_goodStrips.size());
            if (m_writeStripProperties) { stripsOfClusterChannels.reserve(idx_goodStrips.size()); }
            stripsOfClusterTimes.reserve(idx_goodStrips.size());
            stripsOfClusterCharges.reserve(idx_goodStrips.size());
            stripsOfClusterDriftDists.reserve(idx_goodStrips.size());
            stripsOfClusterDriftDistErrors.reserve(idx_goodStrips.size());
            stripsOfClusterLocalPos.reserve(idx_goodStrips.size());

            ATH_MSG_DEBUG("Found good Strips: " << idx_goodStrips.size());

            for (int idx : idx_goodStrips) {
                ATH_MSG_DEBUG("Setting good strip: " << idx << " size of strips flag: " << flag.size());
                flag.at(idx) = 1;
                ATH_MSG_DEBUG("Set Good strips");
                stripsOfCluster.push_back(MMprdsOfLayer.at(idx).identify());
                if (m_writeStripProperties) {
                    stripsOfClusterChannels.push_back(m_idHelperSvc->mmIdHelper().channel(MMprdsOfLayer.at(idx).identify()));
                    stripsOfClusterTimes.push_back(MMprdsOfLayer.at(idx).time());
                    stripsOfClusterCharges.push_back(MMprdsOfLayer.at(idx).charge());
                }
                stripsOfClusterDriftDists.push_back(MMprdsOfLayer.at(idx).driftDist());
                stripsOfClusterDriftDistErrors.push_back(MMprdsOfLayer.at(idx).localCovariance());
                stripsOfClusterLocalPos.push_back(MMprdsOfLayer.at(idx).localPosition().x());
            }

            double localClusterPosition = -9999;
            double sigmaLocalClusterPosition = 0;
            double finalFitAngle, finalFitChiSqProb;

            sc = finalFit(stripsOfCluster, stripsOfClusterLocalPos, stripsOfClusterDriftDists, stripsOfClusterDriftDistErrors,
                          localClusterPosition, sigmaLocalClusterPosition, finalFitAngle, finalFitChiSqProb);

            ATH_MSG_DEBUG("final fit done");

            if (sc.isFailure()) break;
            ATH_MSG_DEBUG("Did final fit successfull");

            auto covN = Amg::MatrixX(1, 1);
            covN.coeffRef(0, 0) = sigmaLocalClusterPosition;
            ATH_MSG_DEBUG("Did set covN Matrix");
            int idx = idx_goodStrips[0];
            ATH_MSG_DEBUG("idx_goodStrips[0]: " << idx << " size: " << MMprdsOfLayer.size());
            Amg::Vector2D localClusterPositionV(localClusterPosition,
                                                MMprdsOfLayer.at(idx).localPosition().y());  // y position is the same for all strips
            ATH_MSG_DEBUG("Did set local position");

            float driftDist = 0.0;

            std::unique_ptr<MMPrepData> prdN = std::make_unique<MMPrepData>(MMprdsOfLayer.at(idx).identify(), 
                                                                            MMprdsOfLayer.at(idx).collectionHash(), 
                                                                            std::move(localClusterPositionV), 
                                                                            std::move(stripsOfCluster),
                                                                            std::move(covN), 
                                                                            MMprdsOfLayer.at(idx).detectorElement(), 
                                                                            0,
                                                                            std::accumulate(stripsOfClusterCharges.begin(), stripsOfClusterCharges.end(), 0), 
                                                                            driftDist, 
                                                                            std::move(stripsOfClusterChannels),
                                                                            std::move(stripsOfClusterTimes), 
                                                                            std::move(stripsOfClusterCharges));

            ATH_MSG_DEBUG("Did create new prd");

            ATH_MSG_DEBUG("Setting prd angle: " << finalFitAngle << " chi2 Prob: " << finalFitChiSqProb);

            prdN->setAuthor(MMPrepData::Author::uTPCClusterBuilder);
            prdN->setDriftDist(std::move(stripsOfClusterDriftDists), std::move(stripsOfClusterDriftDistErrors));

            prdN->setMicroTPC(finalFitAngle, finalFitChiSqProb);
            ATH_MSG_DEBUG("Reading back prd angle: " << prdN->angle() << " chi2 Prob: " << prdN->chisqProb());
            clustersVect.push_back(std::move(prdN));
            ATH_MSG_DEBUG("pushedBack  prdN");
            int leftOverStrips = 0;
            for (auto f : flag) {
                if (f == 0) leftOverStrips++;
            }
            if (leftOverStrips < 3) break;
        }
        ATH_MSG_DEBUG("End of get clusters layer");
    }
    ATH_MSG_DEBUG("End of get clusters");
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::runHoughTrafo(const std::vector<MMPrepData>& mmPrd, std::vector<double>& xpos,
                                                         std::vector<int>& flag, std::vector<int>& idx_selected) const {
    ATH_MSG_DEBUG("beginning of runHoughTrafo() with: " << xpos.size() << " hits");
    double maxX = *std::max_element(xpos.begin(), xpos.end());
    double minX = *std::min_element(xpos.begin(), xpos.end());
    std::unique_ptr<TH2D> h_hough = nullptr;
    StatusCode sc = houghInitCummulator(h_hough, maxX, minX);

    if (sc.isFailure()) return sc;
    sc = fillHoughTrafo(mmPrd, xpos, flag, h_hough);
    ATH_MSG_DEBUG("filled Hough");
    if (sc.isFailure()) return sc;

    std::vector<std::tuple<double, double>> houghPeaks;
    sc = findAlphaMax(h_hough, houghPeaks);
    if (sc.isFailure()) return sc;
    ATH_MSG_DEBUG("Found HoughPeaks");
    sc = selectTrack(mmPrd, xpos, flag, houghPeaks, idx_selected);
    if (sc.isFailure()) return sc;
    return StatusCode::SUCCESS;
}
StatusCode UTPCMMClusterBuilderTool::houghInitCummulator(std::unique_ptr<TH2D>& h_hough, double xmax, double xmin) const {
    ATH_MSG_VERBOSE("xmax: " << xmax << " xmin: " << xmin << " m_dResolution " << m_dResolution << " m_alphaMin " << m_alphaMin
                             << " m_alphaMax: " << m_alphaMax << " m_alphaResolution: " << m_alphaResolution);
    double dmax = std::max(std::fabs(xmin), std::fabs(xmax));
    dmax = std::sqrt(std::pow(dmax, 2) +
                     std::pow(m_driftRange, 2));  // rspace =sqrt(xmax*xmax+driftrange*driftrange) where driftrange is assumed to be 6mm
    int nbinsd = static_cast<int>(1.0 * dmax * 2 / m_dResolution);
    int nbinsa = static_cast<int>((1.0 * m_alphaMax - m_alphaMin) / m_alphaResolution);

    ATH_MSG_VERBOSE("Hough Using nBinsA " << nbinsa << " nBinsd " << nbinsd << " dmax " << dmax);

    h_hough = std::make_unique<TH2D>("h_hough", "h_hough", nbinsa, m_alphaMin * Gaudi::Units::degree, m_alphaMax * Gaudi::Units::degree,
                                     nbinsd, -dmax, dmax);

    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::fillHoughTrafo(const std::vector<MMPrepData>& mmPrd, std::vector<double>& xpos,
                                                          std::vector<int>& flag, std::unique_ptr<TH2D>& h_hough) const {
    for (size_t i_hit = 0; i_hit < mmPrd.size(); i_hit++) {
        if (flag.at(i_hit) != 0) continue;  // skip hits which have been already used or been rejected as cross talk
        for (int i_alpha = 1; i_alpha < h_hough->GetNbinsX(); i_alpha++) {
            double alpha = h_hough->GetXaxis()->GetBinCenter(i_alpha);
            double d = xpos.at(i_hit) * std::cos(alpha) - mmPrd.at(i_hit).driftDist() * std::sin(alpha);
            ATH_MSG_VERBOSE("Fill Hough alpha: " << alpha << " d " << d << " x " << xpos.at(i_hit) << " y " << mmPrd.at(i_hit).driftDist());
            h_hough->Fill(alpha, d);
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::findAlphaMax(std::unique_ptr<TH2D>& h_hough,
                                                        std::vector<std::tuple<double, double>>& maxPos) const {
    unsigned int cmax = h_hough->GetMaximum();
    if (cmax < m_houghMinCounts) {
        ATH_MSG_DEBUG("cmax " << cmax << "smaller then m_houghMinCounts " << m_houghMinCounts);
        return StatusCode::FAILURE;
    }

    for (int i_binX = 0; i_binX <= h_hough->GetNbinsX(); i_binX++) {
        for (int i_binY = 0; i_binY <= h_hough->GetNbinsY(); i_binY++) {
            if (h_hough->GetBinContent(i_binX, i_binY) == cmax) {
                ATH_MSG_DEBUG("Find Max Alpha: BinX "
                              << i_binX << " Alpha: " << h_hough->GetXaxis()->GetBinCenter(i_binX) / Gaudi::Units::degree
                              << " BinY: " << i_binY << " over threshold: " << h_hough->GetBinContent(i_binX, i_binY));
                maxPos.emplace_back(h_hough->GetXaxis()->GetBinCenter(i_binX), h_hough->GetYaxis()->GetBinCenter(i_binY));
            }
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::selectTrack(const std::vector<MMPrepData>& mmPrd, std::vector<double>& xpos,
                                                       std::vector<int>& flag, std::vector<std::tuple<double, double>>& tracks,
                                                       std::vector<int>& idxGoodStrips) const {
    std::vector<double> chi2;
    std::vector<std::vector<int>> allGoodStrips;
    std::vector<int> ngoodStrips;
    std::vector<std::tuple<int, double>> trackQuality;  // touble holding the number of good strips and the chi2 for each track
    for (auto track : tracks) {
        double houghAngle = std::get<0>(track);
        double houghD = std::get<1>(track);
        double slope, intercept;
        double interceptRMS;
        double aRMS = 0;
        StatusCode sc = transformParameters(houghAngle, houghD, aRMS, slope, intercept, interceptRMS);
        if (sc.isFailure()) { return sc; }
        std::vector<double> dist;
        std::vector<int> indexUsedForDist, goodStrips;
        for (size_t i_hit = 0; i_hit < xpos.size(); i_hit++) {
            if (flag.at(i_hit) != 1) {
                dist.push_back(mmPrd.at(i_hit).driftDist() - slope * xpos.at(i_hit) - intercept);
                indexUsedForDist.push_back(i_hit);
            }
        }
        ATH_MSG_VERBOSE("indexUsedForDist " << indexUsedForDist);
        ATH_MSG_VERBOSE("distVec: " << dist);
        dist.clear();
        for (size_t i_hit : indexUsedForDist) {
            double d = mmPrd.at(i_hit).driftDist() - slope * xpos.at(i_hit) - intercept;
            ATH_MSG_VERBOSE("dist: " << d << " for index " << i_hit);
            if (std::fabs(d) < m_selectionCut) {
                dist.push_back(d * d / mmPrd.at(i_hit).localCovariance()(1, 1));  // determine chi2
                goodStrips.push_back(i_hit);
            }
        }
        {
            ATH_MSG_DEBUG("Angle estimate         =" << std::get<0>(track) << " " << std::get<0>(track) / Gaudi::Units::degree);

            ATH_MSG_DEBUG("restimate              =" << std::get<1>(track));

            ATH_MSG_DEBUG("slope estimate         =" << slope);

            ATH_MSG_DEBUG("intercept estimate     =" << intercept);

            ATH_MSG_DEBUG("good strips            =" << goodStrips);

            ATH_MSG_DEBUG("n good points          =" << goodStrips.size());

            ATH_MSG_DEBUG("chi2                   =" << std::accumulate(dist.begin(), dist.end(), 0.0) / (dist.size() - 2));
        }
        if (!goodStrips.empty()) {
            allGoodStrips.push_back(goodStrips);
            chi2.push_back(std::accumulate(dist.begin(), dist.end(), 0.0) / (dist.size() - 2));
            trackQuality.emplace_back(goodStrips.size(), std::accumulate(dist.begin(), dist.end(), 0.0) / (dist.size() - 2));
        }
    }
    if (chi2.empty()) return StatusCode::FAILURE;
    int trackIndex = std::min_element(trackQuality.begin(), trackQuality.end(), sortTracks) - trackQuality.begin();
    idxGoodStrips = allGoodStrips.at(trackIndex);
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::transformParameters(double alpha, double d, double dRMS, double& slope, double& intercept,
                                                               double& interceptRMS) const {
    slope = 1. / std::tan(alpha);
    intercept = -d / std::sin(alpha);
    interceptRMS = std::fabs(dRMS);
    ATH_MSG_VERBOSE("transformParameters alpha: " << alpha << " d: " << d << " dRMS: " << dRMS << " slope: " << slope
                                                  << " intercept: " << intercept);
    return StatusCode::SUCCESS;
}

StatusCode UTPCMMClusterBuilderTool::applyCrossTalkCut(std::vector<int>& idxSelected, const std::vector<MMPrepData>& MMPrdsOfLayer,
                                                             std::vector<int>& flag, int& nStripsCut) const {
    int N = idxSelected.size();
    bool removedStrip = false;
    ATH_MSG_DEBUG("starting to remove cross talk strips " << idxSelected << " flag " << flag);
    if (N < 3 || nStripsCut >= m_maxStripsCut) {
        ATH_MSG_DEBUG("first cut failed");
        return StatusCode::FAILURE;
    }
    // reject outer strip for fit if charge ratio to second outer strip indicates strip charge is created by crosstalk only
    double ratioLastStrip = 1.0 * MMPrdsOfLayer.at(idxSelected.at(N - 1)).charge() / MMPrdsOfLayer.at(idxSelected.at(N - 2)).charge();
    double ratioFirstStrip = 1.0 * MMPrdsOfLayer.at(idxSelected.at(0)).charge() / MMPrdsOfLayer.at(idxSelected.at(1)).charge();
    ATH_MSG_DEBUG("ratioLastStrip " << ratioLastStrip << " ratioFirstStrip " << ratioFirstStrip);
    if (ratioLastStrip < m_outerChargeRatioCut) {
        flag.at(idxSelected.at(N - 1)) = 2;
        idxSelected.erase(idxSelected.begin() + (N - 1));
        removedStrip = true;
        nStripsCut++;
        ATH_MSG_DEBUG("cutting last strip nStripCuts: " << nStripsCut << " flag " << flag << " idxSelected " << idxSelected);
    }
    if (idxSelected.size() < 3 || nStripsCut >= m_maxStripsCut) {
        ATH_MSG_DEBUG("first cut failed");
        return StatusCode::FAILURE;
    }

    if (ratioFirstStrip < m_outerChargeRatioCut) {
        flag.at(idxSelected.at(0)) = 2;
        idxSelected.erase(idxSelected.begin() + 0);
        removedStrip = true;
        nStripsCut++;
        ATH_MSG_DEBUG("cutting first strip nStripCuts: " << nStripsCut << " flag " << flag << " idxSelected " << idxSelected);
    }
    if (nStripsCut >= m_maxStripsCut) {
        ATH_MSG_DEBUG("first cut failed");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("return value " << (!removedStrip ? "FAILURE" : "SUCCESS"));
    return (!removedStrip ? StatusCode::FAILURE : StatusCode::SUCCESS);  // return success if at least one strip was removed
}

StatusCode UTPCMMClusterBuilderTool::finalFit(const std::vector<Identifier>& ids, const std::vector<float>& stripsPos,
                                                    const std::vector<float>& driftDists, const std::vector<Amg::MatrixX>& driftDistErrors,
                                                    double& x0, double& sigmaX0, double& fitAngle, double& chiSqProb) const {
    std::unique_ptr<TGraphErrors> fitGraph = std::make_unique<TGraphErrors>();
    std::unique_ptr<TF1> ffit = std::make_unique<TF1>("ffit", "pol1");

    double xmin = 5000;
    double xmax = -5000;
    double xmean = std::accumulate(stripsPos.begin(), stripsPos.end(), 0.0) / stripsPos.size();  // get mean x value

    std::unique_ptr<TLinearFitter> lf = std::make_unique<TLinearFitter>(1, "1++x");

    for (size_t idx = 0; idx < stripsPos.size(); idx++) {
        double xpos = stripsPos.at(idx) - xmean;  // substract mean value from x to center fit around 0
        if (xmin > xpos)
            xmin = xpos;
        else if (xmax < xpos)
            xmax = xpos;
        lf->AddPoint(&xpos, driftDists.at(idx));
        fitGraph->SetPoint(fitGraph->GetN(), xpos, driftDists.at(idx));
        fitGraph->SetPointError(fitGraph->GetN() - 1, std::sqrt(driftDistErrors.at(idx)(0, 0)), std::sqrt(driftDistErrors.at(idx)(1, 1)));
    }
    lf->Eval();

    if (m_idHelperSvc->mmIdHelper().gasGap(ids.at(0)) % 2 == 1 || !m_digiHasNegativeAngles) {
        ffit->SetParLimits(1, -11.5, -0.15);  // 5 to 81 degree
    } else {
        ffit->SetParLimits(1, 0.15, 11.5);  // 5 to 81 degree
    }
    ffit->SetParameters(lf->GetParameter(0), lf->GetParameter(1));
    TFitResultPtr s = fitGraph->Fit("ffit", "SMQ", "", xmin - .5, xmax + .5);

    double cov = (s->GetCovarianceMatrix())(0, 1);
    ATH_MSG_DEBUG("covariance is: " << cov);
    double fitSlope = ffit->GetParameter(1);
    double dHalfGap = 2.52;  // half the thickness of the gas gap
    double interceptCorr = dHalfGap - ffit->GetParameter(0);
    x0 = interceptCorr / fitSlope;
    sigmaX0 =       
        (std::pow(ffit->GetParError(0) / interceptCorr, 2) + std::pow(ffit->GetParError(1) / fitSlope, 2) - 2. / (fitSlope * interceptCorr) * cov) *
        std::pow(x0, 2);
    x0 += xmean;  // adding back mean value after error was calculated
    fitAngle = std::tan(-1 / fitSlope);
    chiSqProb = ffit->GetProb();

    {
        ATH_MSG_DEBUG("==========uTPC fit Summary==============");

        ATH_MSG_DEBUG("Fit slope: " << ffit->GetParameter(1));

        ATH_MSG_DEBUG("Fit intercept:" << ffit->GetParameter(0));

        ATH_MSG_DEBUG("Fit status: " << s);

        ATH_MSG_DEBUG("Cluster position " << x0 << " +- " << sigmaX0);

        ATH_MSG_DEBUG("Fit angle: " << fitAngle << " " << fitAngle * 180. / M_PI);

        ATH_MSG_DEBUG("ChisSqProb" << chiSqProb);
        ATH_MSG_DEBUG("nStrips:" << stripsPos.size());
        ATH_MSG_DEBUG("gas gap:" << m_idHelperSvc->mmIdHelper().gasGap(ids.at(0)));
    }
    if (s != 0 && s != 4000) {  // 4000 means fit succesfull but error optimization by minos failed; fit is still usable.
        ATH_MSG_DEBUG("Final fit failed with error code " << s);
        return StatusCode::FAILURE;
    }
    if ((m_idHelperSvc->mmIdHelper().gasGap(ids.at(0)) % 2 == 1 || !m_digiHasNegativeAngles) &&
        (ffit->GetParameter(1) <= -11.49 || ffit->GetParameter(1) >= -0.151))
        return StatusCode::FAILURE;  // fit should have negativ slope for odd gas gaps
    if (m_idHelperSvc->mmIdHelper().gasGap(ids.at(0)) % 2 == 0 && m_digiHasNegativeAngles &&
        (ffit->GetParameter(1) >= 11.49 || ffit->GetParameter(1) <= 0.151))
        return StatusCode::FAILURE;  // fit should have positiv slope for even gas gaps
    return StatusCode::SUCCESS;
}

RIO_Author UTPCMMClusterBuilderTool::getCalibratedClusterPosition(const EventContext& /*ctx*/, 
                                                                  const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                                  const Amg::Vector3D& /*directionEstimate*/, 
                                                                  Amg::Vector2D& clusterLocalPosition,
                                                                  Amg::MatrixX& covMatrix) const {
 
    std::vector<Identifier> ids;
    std::vector<float> stripsPos;
    std::vector<float> driftDists;
    std::vector<Amg::MatrixX> driftDistErrors;

    for (const NSWCalib::CalibratedStrip& strip : calibratedStrips) {
        ids.push_back(strip.identifier);
        stripsPos.push_back(strip.locPos.x());
        driftDists.push_back(strip.distDrift);

        Amg::MatrixX cov(2, 2);
        cov.setIdentity();
        cov(0, 0) = strip.resTransDistDrift;
        cov(1, 1) = strip.resLongDistDrift;
        driftDistErrors.push_back(cov);
    }

    double localClusterPosition = -9999;
    double sigmaLocalClusterPosition = 0;
    double finalFitAngle, finalFitChiSqProb;

    StatusCode sc = finalFit(ids, stripsPos, driftDists, driftDistErrors, localClusterPosition, sigmaLocalClusterPosition, finalFitAngle,
                             finalFitChiSqProb);
    if (sc == StatusCode::FAILURE) return RIO_Author::unKnownAuthor;

    clusterLocalPosition[Trk::locX] = localClusterPosition;

    Amg::MatrixX covN(1, 1);
    covN.coeffRef(0, 0) = sigmaLocalClusterPosition;
    covMatrix = covN;
    return RIO_Author::uTPCClusterBuilder;
}
}