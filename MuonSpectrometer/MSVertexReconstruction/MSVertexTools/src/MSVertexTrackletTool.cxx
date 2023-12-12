/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexTrackletTool.h"

#include "AthContainers/DataVector.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPrepRawData/CscPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "TMath.h"  // for TMath::Prob()
#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "CxxUtils/trapping_fp.h"

/*
  Tracklet reconstruction tool
  See documentation at https://cds.cern.ch/record/1455664 and https://cds.cern.ch/record/1520894

  station name reference:
         Barrel         |           Endcap
  0 == BIL   7 == BIR   |   13 == EIL  21 == EOS
  1 == BIS   8 == BMF   |   14 == EEL  49 == EIS
  2 == BML   9 == BOF   |   15 == EES
  3 == BMS  10 == BOG   |   17 == EML
  4 == BOL  52 == BIM   |   18 == EMS
  5 == BOS  53 == BME   |   20 == EOL
  6 == BEE  54 == BMG

*/

namespace Muon {

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // Delta Alpha Constants -- p = k/(delta_alpha)
    constexpr float c_BIL = 28.4366;  // MeV*mrad
    constexpr float c_BML = 62.8267;  // MeV*mrad
    constexpr float c_BMS = 53.1259;  // MeV*mrad
    constexpr float c_BOL = 29.7554;  // MeV*mrad
    constexpr float sq(float x) { return (x) * (x); }
    //** ----------------------------------------------------------------------------------------------------------------- **//

    MSVertexTrackletTool::MSVertexTrackletTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMSVertexTrackletTool>(this);

        // max residual for tracklet seeds
        declareProperty("SeedResidual", m_SeedResidual = 5);
        // segment fitter chi^2 cut
        declareProperty("MinSegFinderChi2Prob", m_minSegFinderChi2 = 0.05);
        // maximum delta_alpha allowed in barrel MS chambers
        declareProperty("BarrelDeltaAlpha", m_BarrelDeltaAlphaCut = 0.100);
        // maximum delta_b allowed
        declareProperty("maxDeltab", m_maxDeltabCut = 3);
        // maximum delta_alpha allowed in the endcap MS chambers
        declareProperty("EndcapDeltaAlpha", m_EndcapDeltaAlphaCut = 0.015);
        // tight tracklet requirement (affects efficiency - disabled by default)
        declareProperty("TightTrackletRequirement", m_tightTrackletRequirement = false);
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexTrackletTool::initialize() {
        ATH_CHECK(m_mdtTESKey.initialize());
        ATH_CHECK(m_TPContainer.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexTrackletTool::findTracklets(std::vector<Tracklet>& tracklets, const EventContext& ctx) const {
        SG::WriteHandle<xAOD::TrackParticleContainer> container(m_TPContainer, ctx);
        // record TrackParticle container in StoreGate

        ATH_CHECK(container.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));

        // sort the MDT hits into chambers & MLs
        std::vector<std::vector<const Muon::MdtPrepData*> > SortedMdt;

        int nMDT = SortMDThits(SortedMdt, ctx);

        if (nMDT <= 0) { return StatusCode::SUCCESS; }

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << nMDT << " MDT hits are selected and sorted" << endmsg;

        // loop over the MDT hits and find segments
        // select the tube combinations to be fit
        /*Select hits in at least 2 layers and require hits be ordered by increasing tube number (see diagrams below).
           ( )( )(3)( )     ( )(3)( )( )     ( )( )( )( )     ( )( )(3)( )     ( )(2)(3)( )
          ( )(2)( )( )     ( )(2)( )( )     ( )(2)(3)( )     ( )(1)(2)( )     ( )(1)( )( )
           (1)( )( )( )     (1)( )( )( )     (1)( )( )( )     ( )( )( )( )     ( )( )( )( )
         Barrel selection criteria: |z_mdt1 - z_mdt2| < 50 mm, |z_mdt1 - z_mdt3| < 80 mm
         Endcap selection criteria: |r_mdt1 - r_mdt2| < 50 mm, |r_mdt1 - r_mdt3| < 80 mm
        */
        double d12_max = 50.;
        double d13_max = 80.;

        constexpr double errorCutOff = 0.001;
        std::vector<TrackletSegment> segs[6][2][16];  // single ML segment array (indicies [station type][ML][sector])
        std::vector<std::vector<const Muon::MdtPrepData*> >::const_iterator ChamberItr = SortedMdt.begin();
        for (; ChamberItr != SortedMdt.end(); ++ChamberItr) {
            std::vector<TrackletSegment> mlsegments;
            // loop on hits inside the chamber
            std::vector<const Muon::MdtPrepData*>::const_iterator mdt1 = ChamberItr->begin();
            std::vector<const Muon::MdtPrepData*>::const_iterator mdtEnd = ChamberItr->end();
            int stName = m_idHelperSvc->mdtIdHelper().stationName((*mdt1)->identify());
            int stEta = m_idHelperSvc->mdtIdHelper().stationEta((*mdt1)->identify());
            if (stName == 6 || stName == 14 || stName == 15) continue;  // ignore hits from BEE, EEL and EES
            if (stName == 1 && std::abs(stEta) >= 7) continue;          // ignore hits from BIS7/8
            if (stName == 53 || stName == 54) continue;                 // ignore hits from BME and BMG

            // convert to the hardware sector [1-16]
            int sector = 2 * (m_idHelperSvc->mdtIdHelper().stationPhi((*mdt1)->identify()));
            if (stName == 0 || stName == 2 || stName == 4 || stName == 13 || stName == 17 || stName == 20) sector -= 1;
            // information about the chamber we are looking at
            int maxLayer = m_idHelperSvc->mdtIdHelper().tubeLayerMax((*mdt1)->identify());
            int ML = m_idHelperSvc->mdtIdHelper().multilayer((*mdt1)->identify());
            for (; mdt1 != mdtEnd; ++mdt1) {
                if (Amg::error((*mdt1)->localCovariance(), Trk::locR) < errorCutOff) {
                    ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string((*mdt1)->identify()) << " with too small error "
                                         << Amg::error((*mdt1)->localCovariance(), Trk::locR));
                    continue;
                }
                int tl1 = m_idHelperSvc->mdtIdHelper().tubeLayer((*mdt1)->identify());
                if (tl1 == maxLayer) break;  // require hits in at least 2 layers
                std::vector<const Muon::MdtPrepData*>::const_iterator mdt2 = (mdt1 + 1);
                for (; mdt2 != mdtEnd; ++mdt2) {
                    if (Amg::error((*mdt2)->localCovariance(), Trk::locR) < errorCutOff) {
                        ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string((*mdt2)->identify())
                                             << " with too small error " << Amg::error((*mdt2)->localCovariance(), Trk::locR));
                        continue;
                    }

                    // select the correct combinations
                    int tl2 = m_idHelperSvc->mdtIdHelper().tubeLayer((*mdt2)->identify());
                    if (mdt1 == mdt2 || (tl2 - tl1) > 1 || (tl2 - tl1) < 0) continue;
                    if (std::abs((*mdt2)->globalPosition().z() - (*mdt1)->globalPosition().z()) > d12_max && (stName <= 11 || stName == 52))
                        continue;
                    // if chamber is endcap, use distance in r
                    if ((stName > 11 && stName <= 21) || stName == 49) {
                        float mdt1R = (*mdt1)->globalPosition().perp();
                        float mdt2R = (*mdt2)->globalPosition().perp();
                        if (std::abs(mdt1R - mdt2R) > d12_max) continue;
                    }
                    if ((tl2 - tl1) == 0 && (m_idHelperSvc->mdtIdHelper().tube((*mdt2)->identify()) -
                                             m_idHelperSvc->mdtIdHelper().tube((*mdt1)->identify())) < 0)
                        continue;
                    // find the third hit
                    std::vector<const Muon::MdtPrepData*>::const_iterator mdt3 = (mdt2 + 1);
                    for (; mdt3 != mdtEnd; ++mdt3) {
                        if (Amg::error((*mdt3)->localCovariance(), Trk::locR) < errorCutOff) {
                            ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string((*mdt3)->identify())
                                                 << " with too small error " << Amg::error((*mdt3)->localCovariance(), Trk::locR));
                            continue;
                        }

                        // reject the bad tube combinations
                        int tl3 = m_idHelperSvc->mdtIdHelper().tubeLayer((*mdt3)->identify());
                        if (mdt1 == mdt3 || mdt2 == mdt3) continue;
                        if ((tl3 - tl2) > 1 || (tl3 - tl2) < 0 || (tl3 - tl1) <= 0) continue;
                        if ((tl3 - tl2) == 0 && (m_idHelperSvc->mdtIdHelper().tube((*mdt3)->identify()) -
                                                 m_idHelperSvc->mdtIdHelper().tube((*mdt2)->identify())) < 0)
                            continue;
                        if (std::abs((*mdt3)->globalPosition().z() - (*mdt1)->globalPosition().z()) > d13_max &&
                            (stName <= 11 || stName == 52))
                            continue;
                        // if chamber is endcap, use distance in r
                        if ((stName > 11 && stName <= 21) || stName == 49) {
                            float mdt1R = (*mdt1)->globalPosition().perp();
                            float mdt3R = (*mdt3)->globalPosition().perp();
                            if (std::abs(mdt1R - mdt3R) > d13_max) continue;
                        }
                        // store and fit the good combinations
                        std::vector<const Muon::MdtPrepData*> mdts;
                        mdts.push_back((*mdt1));
                        mdts.push_back((*mdt2));
                        mdts.push_back((*mdt3));
                        std::vector<TrackletSegment> tmpSegs = TrackletSegmentFitter(mdts);
                        for (unsigned int i = 0; i < tmpSegs.size(); ++i) mlsegments.push_back(tmpSegs.at(i));
                    }  // end loop on mdt3
                }      // end loop on mdt2
            }          // end loop on mdt1

            // store the reconstructed segments according to station, ML and sector
            // station identifiers:
            // 0 == BI  1 == BM  2 == BO
            // 3 == EI  4 == EM  5 == EO
            if (stName == 0 || stName == 1 || stName == 7 || stName == 52)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[0][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else if (stName == 2 || stName == 3 || stName == 8)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[1][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else if (stName == 4 || stName == 5 || stName == 9 || stName == 10)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[2][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else if (stName == 13 || stName == 49)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[3][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else if (stName == 17 || stName == 18)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[4][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else if (stName == 20 || stName == 21)
                for (unsigned int k = 0; k < mlsegments.size(); ++k) segs[5][ML - 1][sector - 1].push_back(mlsegments.at(k));
            else
                ATH_MSG_WARNING("Found segments belonging to chamber " << stName << " that have not been stored");
        }  // end loop on mdt chambers

        // Combine/remove duplicate segments
        std::vector<TrackletSegment> CleanSegs[6][2][16];
        for (int st = 0; st < 6; ++st) {
            for (int ml = 0; ml < 2; ++ml) {
                for (int sector = 0; sector < 16; ++sector) {
                    if (!segs[st][ml][sector].empty()) {
                        CleanSegs[st][ml][sector] = CleanSegments(segs[st][ml][sector]);
                    }
                }
            }
        }

        for (int st = 0; st < 6; ++st) {
            float DeltaAlphaCut = m_BarrelDeltaAlphaCut;
            if (st >= 3) DeltaAlphaCut = m_EndcapDeltaAlphaCut;
            for (int sector = 0; sector < 16; ++sector) {
                for (unsigned int i1 = 0; i1 < CleanSegs[st][0][sector].size(); ++i1) {
                    // Set the delta alpha cut depending on station type
                    int stName = CleanSegs[st][0][sector].at(i1).mdtChamber();
                    if (stName == 0)
                        DeltaAlphaCut = c_BIL / 750.0;
                    else if (stName == 2)
                        DeltaAlphaCut = c_BML / 750.0;
                    else if (stName == 3)
                        DeltaAlphaCut = c_BMS / 750.0;
                    else if (stName == 4)
                        DeltaAlphaCut = c_BOL / 750.0;
                    else if (stName == 7)
                        DeltaAlphaCut = c_BIL / 750.0;
                    else if (stName == 8)
                        DeltaAlphaCut = c_BML / 750.0;
                    else if (stName == 9)
                        DeltaAlphaCut = c_BOL / 750.0;
                    else if (stName == 10)
                        DeltaAlphaCut = c_BOL / 750.0;
                    else if (stName == 52)
                        DeltaAlphaCut = c_BIL / 750.0;
                    else if (stName <= 11)
                        DeltaAlphaCut = 0.02;
                    else
                        DeltaAlphaCut = m_EndcapDeltaAlphaCut;
                    // loop on ML2 segments from same sector
                    for (unsigned int i2 = 0; i2 < CleanSegs[st][1][sector].size(); ++i2) {
                        if (CleanSegs[st][0][sector].at(i1).mdtChamber() != CleanSegs[st][1][sector].at(i2).mdtChamber() ||
                            CleanSegs[st][0][sector].at(i1).mdtChEta() != CleanSegs[st][1][sector].at(i2).mdtChEta())
                            continue;
                        float deltaAlpha = CleanSegs[st][0][sector].at(i1).alpha() - CleanSegs[st][1][sector].at(i2).alpha();
                        bool goodDeltab = DeltabCalc(CleanSegs[st][0][sector].at(i1), CleanSegs[st][1][sector].at(i2));
                        // select the good combinations
                        if (std::abs(deltaAlpha) < DeltaAlphaCut && goodDeltab) {
                            if (st < 3) {  // barrel chambers
                                float charge = 1;
                                if (deltaAlpha * CleanSegs[st][0][sector].at(i1).globalPosition().z() *
                                        std::tan(CleanSegs[st][0][sector].at(i1).alpha()) <
                                    0)
                                    charge = -1;
                                float pTot = TrackMomentum(CleanSegs[st][0][sector].at(i1).mdtChamber(), deltaAlpha);
                                if (pTot < 800.) continue;
                                if (pTot >= 9999.) {
                                    // if we find a straight track, try to do a global refit to minimize the number of duplicates
                                    charge = 0;
                                    std::vector<const Muon::MdtPrepData*> mdts = CleanSegs[st][0][sector].at(i1).mdtHitsOnTrack();
                                    std::vector<const Muon::MdtPrepData*> mdts2 = CleanSegs[st][1][sector].at(i2).mdtHitsOnTrack();
                                    for (unsigned int k = 0; k < mdts2.size(); ++k) mdts.push_back(mdts2.at(k));
                                    std::vector<TrackletSegment> CombinedSeg = TrackletSegmentFitter(mdts);
                                    if (!CombinedSeg.empty()) {
                                        // calculate momentum components & uncertainty
                                        float Trk1overPErr = TrackMomentumError(CombinedSeg[0]);
                                        float pT = pTot * std::sin(CombinedSeg[0].alpha());
                                        float pz = pTot * std::cos(CombinedSeg[0].alpha());
                                        Amg::Vector3D momentum(pT * std::cos(CombinedSeg[0].globalPosition().phi()),
                                                               pT * std::sin(CombinedSeg[0].globalPosition().phi()), pz);
                                        // create the error matrix
                                        AmgSymMatrix(5) matrix;
                                        matrix.setIdentity();
                                        matrix(0, 0) = sq(CombinedSeg[0].rError());  // delta locR
                                        matrix(1, 1) = sq(CombinedSeg[0].zError());  // delta locz
                                        matrix(2, 2) = sq(0.00000000001);  // delta phi (~0 because we explicitly rotate all tracklets into
                                                                           // the middle of the chamber)
                                        matrix(3, 3) = sq(CombinedSeg[0].alphaError());  // delta theta
                                        matrix(4, 4) = sq(Trk1overPErr);                 // delta 1/p
                                        Tracklet tmpTrk(CombinedSeg[0], momentum, matrix, charge);
                                        ATH_MSG_DEBUG("Track " << tracklets.size() << " found with p = (" << momentum.x() << ", "
                                                               << momentum.y() << ", " << momentum.z()
                                                               << ") and |p| = " << tmpTrk.momentum().mag() << " MeV");
                                        tracklets.push_back(tmpTrk);
                                    }
                                } else {
                                    // tracklet has a measurable momentum
                                    float Trk1overPErr =
                                        TrackMomentumError(CleanSegs[st][0][sector].at(i1), CleanSegs[st][1][sector].at(i2));
                                    float pT = pTot * std::sin(CleanSegs[st][0][sector].at(i1).alpha());
                                    float pz = pTot * std::cos(CleanSegs[st][0][sector].at(i1).alpha());
                                    Amg::Vector3D momentum(pT * std::cos(CleanSegs[st][0][sector].at(i1).globalPosition().phi()),
                                                           pT * std::sin(CleanSegs[st][0][sector].at(i1).globalPosition().phi()), pz);
                                    // create the error matrix
                                    AmgSymMatrix(5) matrix;
                                    matrix.setIdentity();
                                    matrix(0, 0) = sq(CleanSegs[st][0][sector].at(i1).rError());  // delta locR
                                    matrix(1, 1) = sq(CleanSegs[st][0][sector].at(i1).zError());  // delta locz
                                    matrix(2, 2) = sq(0.00000000001);  // delta phi (~0 because we explicitly rotate all tracks into the
                                                                       // middle of the chamber)
                                    matrix(3, 3) = sq(CleanSegs[st][0][sector].at(i1).alphaError());  // delta theta
                                    matrix(4, 4) = sq(Trk1overPErr);                                  // delta 1/p
                                    Tracklet tmpTrk(CleanSegs[st][0][sector].at(i1), CleanSegs[st][1][sector].at(i2), momentum, matrix,
                                                    charge);
                                    ATH_MSG_DEBUG("Track " << tracklets.size() << " found with p = (" << momentum.x() << ", "
                                                           << momentum.y() << ", " << momentum.z()
                                                           << ") and |p| = " << tmpTrk.momentum().mag() << " MeV");
                                    tracklets.push_back(tmpTrk);
                                }
                            }                    // end barrel chamber selection
                            else if (st >= 3) {  // endcap tracklets
                                // always straight tracklets (no momentum measurement possible)
                                std::vector<const Muon::MdtPrepData*> mdts = CleanSegs[st][0][sector].at(i1).mdtHitsOnTrack();
                                std::vector<const Muon::MdtPrepData*> mdts2 = CleanSegs[st][1][sector].at(i2).mdtHitsOnTrack();
                                for (unsigned int k = 0; k < mdts2.size(); ++k) mdts.push_back(mdts2.at(k));
                                std::vector<TrackletSegment> CombinedSeg = TrackletSegmentFitter(mdts);
                                if (!CombinedSeg.empty()) {
                                    float charge = 0;
                                    float pT = 100000.0 * std::sin(CombinedSeg[0].alpha());
                                    float pz = 100000.0 * std::cos(CombinedSeg[0].alpha());
                                    // create the error matrix
                                    AmgSymMatrix(5) matrix;
                                    matrix.setIdentity();
                                    matrix(0, 0) = sq(CombinedSeg[0].rError());  // delta locR
                                    matrix(1, 1) = sq(CombinedSeg[0].zError());  // delta locz
                                    matrix(2, 2) = sq(0.0000001);  // delta phi (~0 because we explicitly rotate all tracks into the middle
                                                                   // of the chamber)
                                    matrix(3, 3) = sq(CombinedSeg[0].alphaError());  // delta theta
                                    matrix(4, 4) = sq(
                                        0.00005);  // delta 1/p (endcap tracks are straight lines with no momentum that we can measure ...)
                                    Amg::Vector3D momentum(pT * std::cos(CombinedSeg[0].globalPosition().phi()),
                                                           pT * std::sin(CombinedSeg[0].globalPosition().phi()), pz);
                                    Tracklet tmpTrk(CombinedSeg[0], momentum, matrix, charge);
                                    tracklets.push_back(tmpTrk);
                                }
                            }  // end endcap tracklet selection

                        }  // end tracklet selection (delta alpha & delta b)

                    }  // end loop on ML2 segments
                }      // end loop on ML1 segments
            }          // end loop on sectors
        }              // end loop on stations

        // Resolve any ambiguous tracklets
        tracklets = ResolveAmbiguousTracklets(tracklets);

        // convert from tracklets to Trk::Tracks
        convertToTrackParticles(tracklets, container);

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // convert tracklets to Trk::Track and store in a TrackCollection
    void MSVertexTrackletTool::convertToTrackParticles(std::vector<Tracklet>& tracklets,
                                                       SG::WriteHandle<xAOD::TrackParticleContainer>& container) {
        for (std::vector<Tracklet>::iterator trkItr = tracklets.begin(); trkItr != tracklets.end(); ++trkItr) {
            // create the Trk::Perigee for the tracklet
            AmgSymMatrix(5) covariance = AmgSymMatrix(5)(trkItr->errorMatrix());
            Trk::Perigee* myPerigee =
                new Trk::Perigee(0., 0., trkItr->momentum().phi(), trkItr->momentum().theta(), trkItr->charge() / trkItr->momentum().mag(),
                                 Trk::PerigeeSurface(trkItr->globalPosition()), covariance);

            // create, store & define the xAOD::TrackParticle
            xAOD::TrackParticle* trackparticle = new xAOD::TrackParticle();

            container->push_back(trackparticle);

            // trackparticle->setFitQuality(1.,(float)trkItr->mdtHitsOnTrack().size());
            trackparticle->setTrackProperties(xAOD::TrackProperties::LowPtTrack);
            trackparticle->setDefiningParameters(myPerigee->parameters()[Trk::d0], myPerigee->parameters()[Trk::z0],
                                                 myPerigee->parameters()[Trk::phi0], myPerigee->parameters()[Trk::theta],
                                                 myPerigee->parameters()[Trk::qOverP]);

            std::vector<float> covMatrixVec;
            Amg::compress(covariance, covMatrixVec);
            trackparticle->setDefiningParametersCovMatrixVec(covMatrixVec);

            // cleanup memory
            delete myPerigee;
        }
           }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    int MSVertexTrackletTool::SortMDThits(std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt, const EventContext& ctx) const {
        SortedMdt.clear();
        int nMDT(0);

        SG::ReadHandle<Muon::MdtPrepDataContainer> mdtTES(m_mdtTESKey, ctx);
        if (!mdtTES.isValid()) {
            if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Muon::MdtPrepDataContainer with key MDT_DriftCircles was not retrieved" << endmsg;
            return 0;
        } else {
            if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Muon::MdtPrepDataContainer with key MDT_DriftCircles retrieved" << endmsg;
        }

        Muon::MdtPrepDataContainer::const_iterator MDTItr = mdtTES->begin();
        Muon::MdtPrepDataContainer::const_iterator MDTItrE = mdtTES->end();

        for (; MDTItr != MDTItrE; ++MDTItr) {
            // iterators over collections, a collection corresponds to a chamber
            Muon::MdtPrepDataCollection::const_iterator mpdc = (*MDTItr)->begin();
            Muon::MdtPrepDataCollection::const_iterator mpdcE = (*MDTItr)->end();

            if ((*MDTItr)->empty()) continue;

            int stName = m_idHelperSvc->mdtIdHelper().stationName((*mpdc)->identify());

            // Doesn't consider hits belonging to chambers BEE, EEL and EES
            if (stName == 6 || stName == 14 || stName == 15) continue;

            // Doesn't consider hits belonging to chambers BIS7 and BIS8
            if (stName == 1 && std::abs(m_idHelperSvc->mdtIdHelper().stationEta((*mpdc)->identify())) >= 7) continue;

            // Doesn't consider hits belonging to BME or BMG chambers
            if (stName == 53 || stName == 54) continue;

            // sort per multi layer
            std::vector<const Muon::MdtPrepData*> hitsML1;
            std::vector<const Muon::MdtPrepData*> hitsML2;

            for (; mpdc != mpdcE; ++mpdc) {
                // Removes noisy hits
                if ((*mpdc)->adc() < 50) continue;

                // Removes dead modules or out of time hits
                if ((*mpdc)->status() != Muon::MdtStatusDriftTime) continue;

                // Removes tubes out of readout during drift time or with unphysical errors
                if ((*mpdc)->localPosition()[Trk::locR] == 0.) continue;

                if ((*mpdc)->localCovariance()(Trk::locR, Trk::locR) < 1e-6) {
                    ATH_MSG_WARNING("Found MDT with unphysical error " << m_idHelperSvc->mdtIdHelper().print_to_string((*mpdc)->identify())
                                                                       << " cov " << (*mpdc)->localCovariance()(Trk::locR, Trk::locR));
                    continue;
                }

                ++nMDT;

                // sort per multi layer
                if (m_idHelperSvc->mdtIdHelper().multilayer((*mpdc)->identify()) == 1)
                    hitsML1.push_back(*mpdc);
                else
                    hitsML2.push_back(*mpdc);

            }  // end MdtPrepDataCollection

            // add
            addMDTHits(hitsML1, SortedMdt);
            addMDTHits(hitsML2, SortedMdt);
        }  // end MdtPrepDataContaier

        return nMDT;
    }

    void MSVertexTrackletTool::addMDTHits(std::vector<const Muon::MdtPrepData*>& hits,
                                          std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt) const {
        if (hits.empty()) return;

        // calculate number of hits in ML
        int ntubes = hits.front()->detectorElement()->getNLayers() * hits.front()->detectorElement()->getNtubesperlayer();
        if (hits.size() > 0.75 * ntubes) return;
        std::sort(hits.begin(), hits.end(), [this](const Muon::MdtPrepData* mprd1, const Muon::MdtPrepData* mprd2) -> bool {
            if (m_idHelperSvc->mdtIdHelper().tubeLayer(mprd1->identify()) > m_idHelperSvc->mdtIdHelper().tubeLayer(mprd2->identify()))
                return false;
            if (m_idHelperSvc->mdtIdHelper().tubeLayer(mprd1->identify()) < m_idHelperSvc->mdtIdHelper().tubeLayer(mprd2->identify()))
                return true;
            if (m_idHelperSvc->mdtIdHelper().tube(mprd1->identify()) < m_idHelperSvc->mdtIdHelper().tube(mprd2->identify())) return true;
            return false;
        });  // sort the MDTs by layer and tube number

        SortedMdt.push_back(hits);
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    bool MSVertexTrackletTool::SortMDT(Identifier& i1, Identifier& i2) const {
        if (m_idHelperSvc->mdtIdHelper().stationName(i1) != m_idHelperSvc->mdtIdHelper().stationName(i2)) return false;
        if (m_idHelperSvc->mdtIdHelper().stationEta(i1) != m_idHelperSvc->mdtIdHelper().stationEta(i2)) return false;
        if (m_idHelperSvc->mdtIdHelper().stationPhi(i1) != m_idHelperSvc->mdtIdHelper().stationPhi(i2)) return false;
        if (m_idHelperSvc->mdtIdHelper().multilayer(i1) != m_idHelperSvc->mdtIdHelper().multilayer(i2)) return false;
        return true;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::TrackletSegmentFitter(std::vector<const Muon::MdtPrepData*>& mdts) const {
        // create the segment seeds
        std::vector<std::pair<float, float> > SeedParams = SegSeeds(mdts);
        // fit the segments
        std::vector<TrackletSegment> segs = TrackletSegmentFitterCore(mdts, SeedParams);

        return segs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<std::pair<float, float> > MSVertexTrackletTool::SegSeeds(std::vector<const Muon::MdtPrepData*>& mdts) const {
        std::vector<std::pair<float, float> > SeedParams;
        // create seeds by drawing the 4 possible lines tangent to the two outermost drift circles
        // see http://cds.cern.ch/record/620198 (section 4.3) for description of the algorithm
        // keep all seeds which satisfy the criterion: residual(mdt 2) < m_SeedResidual
        // NOTE: here there is an assumption that each MDT has a radius of 30mm
        //      -- needs to be revisited when the small tubes in sectors 12 & 14 are installed
        float x1 = mdts.front()->globalPosition().z();
        float y1 = mdts.front()->globalPosition().perp();
        float r1 = std::abs(mdts.front()->localPosition()[Trk::locR]);

        float x2 = mdts.back()->globalPosition().z();
        float y2 = mdts.back()->globalPosition().perp();
        float r2 = std::abs(mdts.back()->localPosition()[Trk::locR]);

        float DeltaX = x2 - x1;
        float DeltaY = y2 - y1;
        float DistanceOfCenters = std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
        if (DistanceOfCenters < 30) return SeedParams;
        float Alpha0 = std::acos(DeltaX / DistanceOfCenters);

        // First seed
        float phi = mdts.front()->globalPosition().phi();
        float RSum = r1 + r2;
        if (RSum > DistanceOfCenters) return SeedParams;
        float Alpha1 = std::asin(RSum / DistanceOfCenters);
        float line_theta = Alpha0 + Alpha1;
        float z_line = x1 + r1 * std::sin(line_theta);
        float rho_line = y1 - r1 * std::cos(line_theta);

        Amg::Vector3D gPos1(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
        Amg::Vector3D gDir(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        Amg::Vector3D globalDir1(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        float gSlope1 = (globalDir1.perp() / globalDir1.z());
        float gInter1 = gPos1.perp() - gSlope1 * gPos1.z();
        float resid = SeedResiduals(mdts, gSlope1, gInter1);
        if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope1, gInter1);
        // Second seed
        line_theta = Alpha0 - Alpha1;
        z_line = x1 - r1 * std::sin(line_theta);
        rho_line = y1 + r1 * std::cos(line_theta);
        Amg::Vector3D gPos2(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
        Amg::Vector3D globalDir2(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        float gSlope2 = (globalDir2.perp() / globalDir2.z());
        float gInter2 = gPos2.perp() - gSlope2 * gPos2.z();
        resid = SeedResiduals(mdts, gSlope2, gInter2);
        if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope2, gInter2);

        float Alpha2 = std::asin(std::abs(r2 - r1) / DistanceOfCenters);
        if (r1 < r2) {
            // Third seed
            line_theta = Alpha0 + Alpha2;
            z_line = x1 - r1 * std::sin(line_theta);
            rho_line = y1 + r1 * std::cos(line_theta);

            Amg::Vector3D gPos3(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir3(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            float gSlope3 = (globalDir3.perp() / globalDir3.z());
            float gInter3 = gPos3.perp() - gSlope3 * gPos3.z();
            resid = SeedResiduals(mdts, gSlope3, gInter3);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope3, gInter3);

            // Fourth seed
            line_theta = Alpha0 - Alpha2;
            z_line = x1 + r1 * std::sin(line_theta);
            rho_line = y1 - r1 * std::cos(line_theta);

            Amg::Vector3D gPos4(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir4(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            float gSlope4 = (globalDir4.perp() / globalDir4.z());
            float gInter4 = gPos4.perp() - gSlope4 * gPos4.z();
            resid = SeedResiduals(mdts, gSlope4, gInter4);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope4, gInter4);
        } else {
            // Third seed
            line_theta = Alpha0 + Alpha2;
            z_line = x1 + r1 * std::sin(line_theta);
            rho_line = y1 - r1 * std::cos(line_theta);

            Amg::Vector3D gPos3(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir3(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            float gSlope3 = (globalDir3.perp() / globalDir3.z());
            float gInter3 = gPos3.perp() - gSlope3 * gPos3.z();
            resid = SeedResiduals(mdts, gSlope3, gInter3);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope3, gInter3);

            // Fourth seed
            line_theta = Alpha0 - Alpha2;
            z_line = x1 - r1 * std::sin(line_theta);
            rho_line = y1 + r1 * std::cos(line_theta);

            Amg::Vector3D gPos4(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir4(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            float gSlope4 = (globalDir4.perp() / globalDir4.z());
            float gInter4 = gPos4.perp() - gSlope4 * gPos4.z();
            resid = SeedResiduals(mdts, gSlope4, gInter4);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope4, gInter4);
        }

        return SeedParams;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    float MSVertexTrackletTool::SeedResiduals(std::vector<const Muon::MdtPrepData*>& mdts, float slope, float inter) {
        // calculate the residual of the MDTs not used to create the seed
        float resid = 0;
        for (unsigned int i = 1; i < (mdts.size() - 1); ++i) {
            float mdtR = mdts.at(i)->globalPosition().perp();
            float mdtZ = mdts.at(i)->globalPosition().z();
            float res =
                std::abs((mdts.at(i)->localPosition()[Trk::locR] - std::abs((mdtR - inter - slope * mdtZ) / std::sqrt(sq(slope) + 1))) /
                         (Amg::error(mdts.at(i)->localCovariance(), Trk::locR)));
            if (res > resid) resid = res;
        }
        return resid;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::TrackletSegmentFitterCore(std::vector<const Muon::MdtPrepData*>& mdts,
                                                                                 std::vector<std::pair<float, float> >& SeedParams) const {
        std::vector<TrackletSegment> segs;
        int stName = m_idHelperSvc->mdtIdHelper().stationName(mdts.at(0)->identify());
        float mlmidpt = 0;
        if (stName <= 11 || stName == 52)
            mlmidpt = std::sqrt(sq(mdts.at(0)->detectorElement()->center().x()) + sq(mdts.at(0)->detectorElement()->center().y()));
        else if (stName <= 21 || stName == 49)
            mlmidpt = mdts.at(0)->detectorElement()->center().z();
        else
            return segs;
        for (unsigned int i_p = 0; i_p < SeedParams.size(); ++i_p) {
            // Min chi^2 fit from "Precision of the ATLAS Muon Spectrometer" -- M. Woudstra
            // http://cds.cern.ch/record/620198?ln=en (section 4.3)
            float chi2(0);
            float s(0), sz(0), sy(0);
            // loop on the mdt hits, find the weighted center
            for (unsigned int i = 0; i < mdts.size(); ++i) {
                // Tell clang to optimize assuming that FP exceptions can trap.
                // Otherwise, it can vectorize the division, which can lead to
                // spurious division-by-zero traps from unused vector lanes.
                CXXUTILS_TRAPPING_FP;
                const Muon::MdtPrepData* prd = mdts.at(i);
                const float mdt_y = std::hypot(prd->globalPosition().x(), prd->globalPosition().y());
                const float mdt_z = prd->globalPosition().z();
                const float sigma2 = sq(Amg::error(prd->localCovariance(), Trk::locR));
                s += 1 / sigma2;
                sz += mdt_z / sigma2;
                sy += mdt_y / sigma2;
            }
            const float yc = sy / s;
            const float zc = sz / s;

            // Find the initial parameters of the fit
            float alpha = std::atan2(SeedParams.at(i_p).first, 1.0);
            if (alpha < 0) alpha += M_PI;
            float dalpha = 0;
            float d = (SeedParams.at(i_p).second - yc + zc * SeedParams.at(i_p).first) * std::cos(alpha);
            float dd = 0;

            // require segments to point to the second ML
            if (std::abs(std::cos(alpha)) > 0.97 && (stName <= 11 || stName == 52)) continue;
            if (std::abs(std::cos(alpha)) < 0.03 && ((stName > 11 && stName < 22) || stName == 49)) continue;

            // calculate constants used in the fit
            float sPyy(0), sPyz(0), sPyyzz(0);
            for (unsigned int i = 0; i < mdts.size(); ++i) {
                const Muon::MdtPrepData* prd = mdts.at(i);
                float mdt_y = std::hypot(prd->globalPosition().x(), prd->globalPosition().y());
                float mdt_z = prd->globalPosition().z();
                float sigma2 = sq(Amg::error(prd->localCovariance(), Trk::locR));
                sPyy += sq(mdt_y - yc) / sigma2;
                sPyz += (mdt_y - yc) * (mdt_z - zc) / sigma2;
                sPyyzz += ((mdt_y - yc) - (mdt_z - zc)) * ((mdt_y - yc) + (mdt_z - zc)) / sigma2;
            }

            // iterative fit
            int Nitr = 0;
            float deltaAlpha = 0;
            float deltad = 0;
            while (true) {
                float sumRyi(0), sumRzi(0), sumRi(0);
                chi2 = 0;
                Nitr++;
                const float cos_a = std::cos(alpha);
                const float sin_a = std::sin(alpha);
                for (unsigned int i = 0; i < mdts.size(); ++i) {
                    const Muon::MdtPrepData* prd = mdts.at(i);
                    float mdt_y = std::hypot(prd->globalPosition().x(), prd->globalPosition().y());
                    float mdt_z = prd->globalPosition().z();
                    float yPi = -(mdt_z - zc) * sin_a + (mdt_y - yc) * cos_a - d;
                    float signR = yPi >= 0 ? 1. : -1;
                    float sigma2 = sq(Amg::error(prd->localCovariance(), Trk::locR));
                    float ri = signR * prd->localPosition()[Trk::locR];
                    ////
                    sumRyi += ri * (mdt_y - yc) / sigma2;
                    sumRzi += ri * (mdt_z - zc) / sigma2;
                    sumRi += ri / sigma2;
                    //
                    chi2 += sq(yPi + ri) / sigma2;
                }
                float bAlpha = -1 * sPyz + cos_a * (sin_a * sPyyzz + 2 * cos_a * sPyz + sumRzi) + sin_a * sumRyi;
                float AThTh = sPyy + cos_a * (2 * sin_a * sPyz - cos_a * sPyyzz);
                /// The AThTh represent the derivative fed in to the Newton
                /// minimaztion. If the value is too small the procedure will
                /// diverge anyway let's break the loop
                if (std::abs(AThTh) < 1.e-7) break;
                // the new alpha & d parameters
                float alphaNew = alpha + bAlpha / AThTh;
                float dNew = sumRi / s;
                // the errors
                dalpha = std::sqrt(1 / std::abs(AThTh));
                dd = std::sqrt(1 / s);
                deltaAlpha = std::abs(alphaNew - alpha);
                deltad = std::abs(d - dNew);
                // test if the new segment is different than the previous
                if (deltaAlpha < 0.0000005 && deltad < 0.000005) break;
                alpha = alphaNew;
                d = dNew;
                // Guard against infinite loops
                if (Nitr > 10) break;
            }  // end while loop

            // find the chi^2 probability of the segment
            float chi2Prob = TMath::Prob(chi2, mdts.size() - 2);
            // keep only "good" segments
            if (chi2Prob > m_minSegFinderChi2) {
                float z0 = zc - d * std::sin(alpha);
                float dz0 = std::sqrt(sq(dd * std::sin(alpha)) + sq(d * dalpha * std::cos(alpha)));
                float y0 = yc + d * std::cos(alpha);
                float dy0 = std::sqrt(sq(dd * std::cos(alpha)) + sq(d * dalpha * std::sin(alpha)));
                // find the hit pattern, which side of the wire did the particle pass? (1==Left, 2==Right)
                /*
                    (  )(/O)(  )
                     (./)(  )(  ) == RRL == 221
                    (O/)(  )(  )
                */
                int pattern(0);
                if (mdts.size() > 8)
                    pattern = -1;  // with more then 8 MDTs the pattern is unique
                else {
                    for (unsigned int k = 0; k < mdts.size(); ++k) {
                        int base = std::pow(10, k);
                        float mdtR = std::sqrt(sq(mdts.at(k)->globalPosition().x()) + sq(mdts.at(k)->globalPosition().y()));
                        float mdtZ = mdts.at(k)->globalPosition().z();
                        float zTest = (mdtR - y0) / std::tan(alpha) + z0 - mdtZ;
                        if (zTest > 0)
                            pattern += 2 * base;
                        else
                            pattern += base;
                    }
                }

                // information about the MDT chamber containing the tracklet
                int chEta = m_idHelperSvc->mdtIdHelper().stationEta(mdts.at(0)->identify());
                int chPhi = m_idHelperSvc->mdtIdHelper().stationPhi(mdts.at(0)->identify());

                // find the position of the tracklet in the global frame
                float mdtPhi = mdts.at(0)->globalPosition().phi();
                Amg::Vector3D segpos(y0 * std::cos(mdtPhi), y0 * std::sin(mdtPhi), z0);
                // create the tracklet
                TrackletSegment MyTrackletSegment(stName, chEta, chPhi, mlmidpt, alpha, dalpha, segpos, dy0, dz0, mdts, pattern);
                segs.push_back(MyTrackletSegment);
                if (pattern == -1) break;  // stop if we find a segment with more than 8 hits (guaranteed to be unique!)
            }
        }  // end loop on segment seeds

        // in case more than 1 segment is reconstructed, check if there are duplicates using the hit patterns
        if (segs.size() > 1) {
            std::vector<TrackletSegment> tmpSegs;
            for (unsigned int i1 = 0; i1 < segs.size(); ++i1) {
                bool isUnique = true;
                int pattern1 = segs.at(i1).getHitPattern();
                for (unsigned int i2 = (i1 + 1); i2 < segs.size(); ++i2) {
                    if (pattern1 == -1) break;
                    int pattern2 = segs.at(i2).getHitPattern();
                    if (pattern1 == pattern2) isUnique = false;
                }
                if (isUnique) tmpSegs.push_back(segs.at(i1));
            }
            segs = tmpSegs;
        }

        // return the unique segments
        return segs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::CleanSegments(std::vector<TrackletSegment>& Segs) const {
        std::vector<TrackletSegment> CleanSegs;
        std::vector<TrackletSegment> segs = Segs;
        bool keepCleaning(true);
        int nItr(0);
        while (keepCleaning) {
            nItr++;
            keepCleaning = false;
            for (std::vector<TrackletSegment>::iterator it = segs.begin(); it != segs.end(); ++it) {
                if (it->isCombined()) continue;
                std::vector<TrackletSegment> segsToCombine;
                float tanTh1 = std::tan(it->alpha());
                float r1 = it->globalPosition().perp();
                float zi1 = it->globalPosition().z() - r1 / tanTh1;
                // find all segments with similar parameters & attempt to combine
                for (std::vector<TrackletSegment>::iterator sit = (it + 1); sit != segs.end(); ++sit) {
                    if (sit->isCombined()) continue;
                    if (it->mdtChamber() != sit->mdtChamber()) continue;         // require the segments are in the same chamber
                    if ((it->mdtChEta()) * (sit->mdtChEta()) < 0) continue;      // check both segments are on the same side of the detector
                    if (it->mdtChPhi() != sit->mdtChPhi()) continue;             // in the same sector
                    if (std::abs(it->alpha() - sit->alpha()) > 0.005) continue;  // same trajectory
                    float tanTh2 = std::tan(sit->alpha());
                    float r2 = sit->globalPosition().perp();
                    float zi2 = sit->globalPosition().z() - r2 / tanTh2;
                    // find the distance at the midpoint between the two segments
                    float rmid = (r1 + r2) / 2.;
                    float z1 = rmid / tanTh1 + zi1;
                    float z2 = rmid / tanTh2 + zi2;
                    float zdist = std::abs(z1 - z2);
                    if (zdist < 0.5) {
                        segsToCombine.push_back(*sit);
                        sit->isCombined(true);
                    }
                }  // end sit loop

                // if the segment is unique, keep it
                if (segsToCombine.empty()) {
                    CleanSegs.push_back(*it);
                }
                // else, combine all like segments & refit
                else if (!segsToCombine.empty()) {
                    // create a vector of all unique MDT hits in the segments
                    std::vector<const Muon::MdtPrepData*> mdts = it->mdtHitsOnTrack();
                    for (unsigned int i = 0; i < segsToCombine.size(); ++i) {
                        std::vector<const Muon::MdtPrepData*> tmpmdts = segsToCombine[i].mdtHitsOnTrack();
                        for (std::vector<const Muon::MdtPrepData*>::iterator mit = tmpmdts.begin(); mit != tmpmdts.end(); ++mit) {
                            bool isNewHit(true);
                            for (std::vector<const Muon::MdtPrepData*>::iterator msit = mdts.begin(); msit != mdts.end(); ++msit) {
                                if ((*mit)->identify() == (*msit)->identify()) {
                                    isNewHit = false;
                                    break;
                                }
                            }
                            if (isNewHit && Amg::error((*mit)->localCovariance(), Trk::locR) > 0.001) mdts.push_back(*mit);
                        }
                    }  // end segsToCombine loop

                    // only need to combine if there are extra hits added to the first segment
                    if (mdts.size() > it->mdtHitsOnTrack().size()) {
                        std::vector<TrackletSegment> refitsegs = TrackletSegmentFitter(mdts);
                        // if the refit fails, what to do?
                        if (refitsegs.empty()) {
                            if (segsToCombine.size() == 1) {
                                segsToCombine[0].isCombined(false);
                                CleanSegs.push_back(*it);
                                CleanSegs.push_back(segsToCombine[0]);
                            } else {
                                // loop on the mdts and count the number of segments that share that hit
                                std::vector<int> nSeg;
                                for (unsigned int i = 0; i < mdts.size(); ++i) {
                                    nSeg.push_back(0);
                                    // hit belongs to the first segment
                                    for (unsigned int k = 0; k < it->mdtHitsOnTrack().size(); ++k) {
                                        if (it->mdtHitsOnTrack()[k]->identify() == mdts[i]->identify()) {
                                            nSeg[i]++;
                                            break;
                                        }
                                    }
                                    // hit belongs to one of the duplicate segments
                                    for (unsigned int k = 0; k < segsToCombine.size(); ++k) {
                                        for (unsigned int m = 0; m < segsToCombine[k].mdtHitsOnTrack().size(); ++m) {
                                            if (segsToCombine[k].mdtHitsOnTrack()[m]->identify() == mdts[i]->identify()) {
                                                nSeg[i]++;
                                                break;
                                            }
                                        }  // end loop on mdtHitsOnTrack
                                    }      // end loop on segsToCombine
                                }          // end loop on mdts

                                // loop over the duplicates and remove the MDT used by the fewest segments until the fit converges
                                bool keeprefitting(true);
                                int nItr2(0);
                                while (keeprefitting) {
                                    nItr2++;
                                    int nMinSeg(nSeg[0]);
                                    const Muon::MdtPrepData* minmdt = mdts[0];
                                    std::vector<int> nrfsegs;
                                    std::vector<const Muon::MdtPrepData*> refitmdts;
                                    // loop on MDTs, identify the overlapping set of hits
                                    for (unsigned int i = 1; i < mdts.size(); ++i) {
                                        if (nSeg[i] < nMinSeg) {
                                            refitmdts.push_back(minmdt);
                                            nrfsegs.push_back(nMinSeg);
                                            minmdt = mdts[i];
                                            nMinSeg = nSeg[i];
                                        } else {
                                            refitmdts.push_back(mdts[i]);
                                            nrfsegs.push_back(nSeg[i]);
                                        }
                                    }
                                    // reset the list of MDTs & the minimum number of segments an MDT must belong to
                                    mdts = refitmdts;
                                    nSeg = nrfsegs;
                                    // try to fit the new set of MDTs
                                    refitsegs = TrackletSegmentFitter(mdts);
                                    if (!refitsegs.empty()) {
                                        for (std::vector<TrackletSegment>::iterator rfit = refitsegs.begin(); rfit != refitsegs.end();
                                             ++rfit) {
                                            CleanSegs.push_back(*rfit);
                                        }
                                        keeprefitting = false;  // stop refitting if segments are found
                                    } else if (mdts.size() <= 3) {
                                        CleanSegs.push_back(*it);
                                        keeprefitting = false;
                                    }
                                    if (nItr2 > 10) break;
                                }  // end while
                            }
                        } else {
                            keepCleaning = true;
                            for (std::vector<TrackletSegment>::iterator rfit = refitsegs.begin(); rfit != refitsegs.end(); ++rfit) {
                                CleanSegs.push_back(*rfit);
                            }
                        }
                    }
                    // if there are no extra MDT hits, keep only the first segment as unique
                    else
                        CleanSegs.push_back(*it);
                }
            }  // end it loop
            if (keepCleaning) {
                segs = CleanSegs;
                CleanSegs.clear();
            }
            if (nItr > 10) break;
        }  // end while

        return CleanSegs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    bool MSVertexTrackletTool::DeltabCalc(TrackletSegment& ML1seg, TrackletSegment& ML2seg) const {
        float ChMid = (ML1seg.getChMidPoint() + ML2seg.getChMidPoint()) / 2.0;
        // Calculate the Delta b (see http://inspirehep.net/record/1266438)
        float mid1(100), mid2(1000);
        float deltab(100);
        if (ML1seg.mdtChamber() <= 11 || ML1seg.mdtChamber() == 52) {
            // delta b in the barrel
            mid1 = (ChMid - ML1seg.globalPosition().perp()) / std::tan(ML1seg.alpha()) + ML1seg.globalPosition().z();
            mid2 = (ChMid - ML2seg.globalPosition().perp()) / std::tan(ML2seg.alpha()) + ML2seg.globalPosition().z();
            float r01 = ML1seg.globalPosition().perp() - ML1seg.globalPosition().z() * std::tan(ML1seg.alpha());
            float r02 = ML2seg.globalPosition().perp() - ML2seg.globalPosition().z() * std::tan(ML2seg.alpha());
            deltab = (mid2 * std::tan(ML1seg.alpha()) - ChMid + r01) / (std::sqrt(1 + sq(std::tan(ML1seg.alpha()))));
            float deltab2 = (mid1 * std::tan(ML2seg.alpha()) - ChMid + r02) / (std::sqrt(1 + sq(std::tan(ML2seg.alpha()))));
            if (std::abs(deltab2) < std::abs(deltab)) deltab = deltab2;
        } else {
            // delta b in the endcap
            mid1 = ML1seg.globalPosition().perp() + std::tan(ML1seg.alpha()) * (ChMid - ML1seg.globalPosition().z());
            mid2 = ML2seg.globalPosition().perp() + std::tan(ML2seg.alpha()) * (ChMid - ML2seg.globalPosition().z());
            float z01 = ML1seg.globalPosition().z() - ML1seg.globalPosition().perp() / std::tan(ML1seg.alpha());
            float z02 = ML1seg.globalPosition().z() - ML1seg.globalPosition().perp() / std::tan(ML1seg.alpha());
            deltab = (mid2 / std::tan(ML1seg.alpha()) - ChMid + z01) / (std::sqrt(1 + sq(1 / std::tan(ML1seg.alpha()))));
            float deltab2 = (mid1 / std::tan(ML2seg.alpha()) - ChMid + z02) / (std::sqrt(1 + sq(1 / std::tan(ML2seg.alpha()))));
            if (std::abs(deltab2) < std::abs(deltab)) deltab = deltab2;
        }

        // calculate the maximum allowed Delta b based on delta alpha uncertainties and ML spacing
        double dbmax = 5 * std::abs(ChMid - ML1seg.getChMidPoint()) * std::sqrt(sq(ML1seg.alphaError()) + sq(ML2seg.alphaError()));
        if (dbmax > m_maxDeltabCut) dbmax = m_maxDeltabCut;
        return std::abs(deltab) < dbmax;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    float MSVertexTrackletTool::TrackMomentum(int chamber, float deltaAlpha) {
        float pTot = 100000.;
        // p = k/delta_alpha
        if (chamber == 0)
            pTot = c_BIL / std::abs(deltaAlpha);
        else if (chamber == 2)
            pTot = c_BML / std::abs(deltaAlpha);
        else if (chamber == 3)
            pTot = c_BMS / std::abs(deltaAlpha);
        else if (chamber == 54)
            pTot = c_BMS / std::abs(deltaAlpha);
        else if (chamber == 4)
            pTot = c_BOL / std::abs(deltaAlpha);
        else if (chamber == 7)
            pTot = c_BIL / std::abs(deltaAlpha);
        else if (chamber == 8)
            pTot = c_BML / std::abs(deltaAlpha);
        else if (chamber == 9)
            pTot = c_BOL / std::abs(deltaAlpha);
        else if (chamber == 10)
            pTot = c_BOL / std::abs(deltaAlpha);
        else if (chamber == 52)
            pTot = c_BIL / std::abs(deltaAlpha);
        if (pTot > 10000.) pTot = 100000.;

        return pTot;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    float MSVertexTrackletTool::TrackMomentumError(TrackletSegment& ml1, TrackletSegment& ml2) {
        // uncertainty on 1/p
        int ChType = ml1.mdtChamber();
        float dalpha = std::sqrt(sq(ml1.alphaError()) + sq(ml2.alphaError()));
        float pErr = dalpha / c_BML;
        if (ChType == 0)
            pErr = dalpha / c_BIL;
        else if (ChType == 2)
            pErr = dalpha / c_BML;
        else if (ChType == 3)
            pErr = dalpha / c_BMS;
        else if (ChType == 54)
            pErr = dalpha / c_BMS;
        else if (ChType == 4)
            pErr = dalpha / c_BOL;
        else if (ChType == 7)
            pErr = dalpha / c_BIL;
        else if (ChType == 8)
            pErr = dalpha / c_BML;
        else if (ChType == 9)
            pErr = dalpha / c_BOL;
        else if (ChType == 10)
            pErr = dalpha / c_BOL;
        else if (ChType == 52)
            pErr = dalpha / c_BIL;

        return pErr;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    float MSVertexTrackletTool::TrackMomentumError(TrackletSegment& ml1) {
        // uncertainty in 1/p
        int ChType = ml1.mdtChamber();
        float dalpha = std::abs(ml1.alphaError());
        float pErr = dalpha / c_BML;
        if (ChType == 0)
            pErr = dalpha / c_BIL;
        else if (ChType == 2)
            pErr = dalpha / c_BML;
        else if (ChType == 3)
            pErr = dalpha / c_BMS;
        else if (ChType == 54)
            pErr = dalpha / c_BMS;
        else if (ChType == 4)
            pErr = dalpha / c_BOL;
        else if (ChType == 7)
            pErr = dalpha / c_BIL;
        else if (ChType == 8)
            pErr = dalpha / c_BML;
        else if (ChType == 9)
            pErr = dalpha / c_BOL;
        else if (ChType == 10)
            pErr = dalpha / c_BOL;
        else if (ChType == 52)
            pErr = dalpha / c_BIL;

        return pErr;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<Tracklet> MSVertexTrackletTool::ResolveAmbiguousTracklets(std::vector<Tracklet>& tracks) const {
        ATH_MSG_DEBUG("In ResolveAmbiguousTracks");

        //////////////////////////////
        // considering only tracklets with the number of associated hits
        // being more than 3/4 the number of layers in the MS chamber

        if (m_tightTrackletRequirement) {
            std::vector<Tracklet> myTracks = tracks;
            tracks.clear();

            for (unsigned int tk1 = 0; tk1 < myTracks.size(); ++tk1) {
                Identifier id1 = myTracks.at(tk1).getML1seg().mdtHitsOnTrack().at(0)->identify();
                Identifier id2 = myTracks.at(tk1).getML2seg().mdtHitsOnTrack().at(0)->identify();
                int nLayerML1 = m_idHelperSvc->mdtIdHelper().tubeLayerMax(id1);
                int nLayerML2 = m_idHelperSvc->mdtIdHelper().tubeLayerMax(id2);
                float ratio = (float)(myTracks.at(tk1).mdtHitsOnTrack().size()) / (nLayerML1 + nLayerML2);
                if (ratio > 0.75) tracks.push_back(myTracks.at(tk1));
            }
        }

        std::vector<Tracklet> UniqueTracks;
        std::vector<unsigned int> AmbigTrks;
        for (unsigned int tk1 = 0; tk1 < tracks.size(); ++tk1) {
            int nShared = 0;
            // check if any Ambiguity has been broken
            bool isResolved = false;
            for (unsigned int ck = 0; ck < AmbigTrks.size(); ++ck) {
                if (tk1 == AmbigTrks.at(ck)) {
                    isResolved = true;
                    break;
                }
            }
            if (isResolved) continue;
            std::vector<Tracklet> AmbigTracks;
            AmbigTracks.push_back(tracks.at(tk1));
            // get a point on the track
            float Trk1ML1R = tracks.at(tk1).getML1seg().globalPosition().perp();
            float Trk1ML1Z = tracks.at(tk1).getML1seg().globalPosition().z();
            float Trk1ML2R = tracks.at(tk1).getML2seg().globalPosition().perp();
            float Trk1ML2Z = tracks.at(tk1).getML2seg().globalPosition().z();

            // loop over the rest of the tracks and find any amibuities
            for (unsigned int tk2 = (tk1 + 1); tk2 < tracks.size(); ++tk2) {
                if (tracks.at(tk1).mdtChamber() == tracks.at(tk2).mdtChamber() && tracks.at(tk1).mdtChPhi() == tracks.at(tk2).mdtChPhi() &&
                    (tracks.at(tk1).mdtChEta()) * (tracks.at(tk2).mdtChEta()) > 0) {
                    // check if any Ambiguity has been broken
                    for (unsigned int ck = 0; ck < AmbigTrks.size(); ++ck) {
                        if (tk2 == AmbigTrks.at(ck)) {
                            isResolved = true;
                            break;
                        }
                    }
                    if (isResolved) continue;
                    // get a point on the track
                    float Trk2ML1R = tracks.at(tk2).getML1seg().globalPosition().perp();
                    float Trk2ML1Z = tracks.at(tk2).getML1seg().globalPosition().z();
                    float Trk2ML2R = tracks.at(tk2).getML2seg().globalPosition().perp();
                    float Trk2ML2Z = tracks.at(tk2).getML2seg().globalPosition().z();

                    // find the distance between the tracks
                    float DistML1(1000), DistML2(1000);
                    if (tracks.at(tk1).mdtChamber() <= 11 || tracks.at(tk1).mdtChamber() == 52) {
                        DistML1 = std::abs(Trk1ML1Z - Trk2ML1Z);
                        DistML2 = std::abs(Trk1ML2Z - Trk2ML2Z);
                    } else if (tracks.at(tk1).mdtChamber() <= 21 || tracks.at(tk1).mdtChamber() == 49) {
                        DistML1 = std::abs(Trk1ML1R - Trk2ML1R);
                        DistML2 = std::abs(Trk1ML2R - Trk2ML2R);
                    }
                    if (DistML1 < 40 || DistML2 < 40) {
                        // find how many MDTs the tracks share
                        std::vector<const Muon::MdtPrepData*> mdt1 = tracks.at(tk1).mdtHitsOnTrack();
                        std::vector<const Muon::MdtPrepData*> mdt2 = tracks.at(tk2).mdtHitsOnTrack();
                        nShared = 0;
                        for (unsigned int m1 = 0; m1 < mdt1.size(); ++m1) {
                            for (unsigned int m2 = 0; m2 < mdt2.size(); ++m2) {
                                if (mdt1.at(m1)->identify() == mdt2.at(m2)->identify()) {
                                    nShared++;
                                    break;
                                }
                            }
                        }

                        if (nShared <= 1) continue;  // if the tracks share only 1 hits move to next track
                        // store the track as ambiguous
                        AmbigTracks.push_back(tracks.at(tk2));
                        AmbigTrks.push_back(tk2);
                    }
                }  // end chamber match
            }      // end tk2 loop

            if (AmbigTracks.size() == 1) {
                UniqueTracks.push_back(tracks.at(tk1));
                continue;
            }
            // Deal with any ambiguities
            // Barrel tracks
            if (tracks.at(tk1).mdtChamber() <= 11 || tracks.at(tk1).mdtChamber() == 52) {
                bool hasMomentum = true;
                if (tracks.at(tk1).charge() == 0) hasMomentum = false;
                float aveX(0), aveY(0), aveZ(0), aveAlpha(0);
                float aveP(0), nAmbigP(0), TrkCharge(tracks.at(tk1).charge());
                bool allSameSign(true);
                for (unsigned int i = 0; i < AmbigTracks.size(); ++i) {
                    if (!hasMomentum) {
                        aveX += AmbigTracks.at(i).globalPosition().x();
                        aveY += AmbigTracks.at(i).globalPosition().y();
                        aveZ += AmbigTracks.at(i).globalPosition().z();
                        aveAlpha += AmbigTracks.at(i).getML1seg().alpha();
                    } else {
                        // check the charge is the same
                        if (std::abs(AmbigTracks.at(i).charge() - TrkCharge) > 0.1) allSameSign = false;
                        // find the average momentum
                        aveP += AmbigTracks.at(i).momentum().mag();
                        nAmbigP++;
                        aveAlpha += AmbigTracks.at(i).alpha();
                        aveX += AmbigTracks.at(i).globalPosition().x();
                        aveY += AmbigTracks.at(i).globalPosition().y();
                        aveZ += AmbigTracks.at(i).globalPosition().z();
                    }
                }  // end loop on ambiguous tracks
                if (!hasMomentum) {
                    aveX = aveX / (float)AmbigTracks.size();
                    aveY = aveY / (float)AmbigTracks.size();
                    aveZ = aveZ / (float)AmbigTracks.size();
                    Amg::Vector3D gpos(aveX, aveY, aveZ);
                    aveAlpha = aveAlpha / (float)AmbigTracks.size();
                    float alphaErr = tracks.at(tk1).getML1seg().alphaError();
                    float rErr = tracks.at(tk1).getML1seg().rError();
                    float zErr = tracks.at(tk1).getML1seg().zError();
                    int Chamber = tracks.at(tk1).mdtChamber();
                    int ChEta = tracks.at(tk1).mdtChEta();
                    int ChPhi = tracks.at(tk1).mdtChPhi();
                    //
                    TrackletSegment aveSegML1(Chamber, ChEta, ChPhi, tracks.at(tk1).getML1seg().getChMidPoint(), aveAlpha, alphaErr, gpos,
                                              rErr, zErr, tracks.at(tk1).getML1seg().mdtHitsOnTrack(), 0);
                    float pT = 10000.0 * std::sin(aveSegML1.alpha());
                    float pz = 10000.0 * std::cos(aveSegML1.alpha());
                    Amg::Vector3D momentum(pT * std::cos(aveSegML1.globalPosition().phi()), pT * std::sin(aveSegML1.globalPosition().phi()),
                                           pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = sq(tracks.at(tk1).getML1seg().rError());  // delta R
                    matrix(1, 1) = sq(tracks.at(tk1).getML1seg().zError());  // delta z
                    matrix(2, 2) = sq(0.0000001);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = sq(tracks.at(tk1).getML1seg().alphaError());  // delta theta
                    matrix(4, 4) = sq(0.00005);                                  // delta 1/p
                    Tracklet aveTrack(aveSegML1, momentum, matrix, 0);
                    UniqueTracks.push_back(aveTrack);
                } else if (allSameSign) {
                    aveP = aveP / nAmbigP;
                    float pT = aveP * std::sin(tracks.at(tk1).getML1seg().alpha());
                    float pz = aveP * std::cos(tracks.at(tk1).getML1seg().alpha());
                    Amg::Vector3D momentum(pT * std::cos(tracks.at(tk1).globalPosition().phi()),
                                           pT * std::sin(tracks.at(tk1).globalPosition().phi()), pz);
                    Tracklet MyTrack = tracks.at(tk1);
                    MyTrack.momentum(momentum);
                    MyTrack.charge(tracks.at(tk1).charge());
                    UniqueTracks.push_back(MyTrack);
                } else {
                    aveX = aveX / (float)AmbigTracks.size();
                    aveY = aveY / (float)AmbigTracks.size();
                    aveZ = aveZ / (float)AmbigTracks.size();
                    Amg::Vector3D gpos(aveX, aveY, aveZ);
                    aveAlpha = aveAlpha / (float)AmbigTracks.size();
                    float alphaErr = tracks.at(tk1).getML1seg().alphaError();
                    float rErr = tracks.at(tk1).getML1seg().rError();
                    float zErr = tracks.at(tk1).getML1seg().zError();
                    int Chamber = tracks.at(tk1).mdtChamber();
                    int ChEta = tracks.at(tk1).mdtChEta();
                    int ChPhi = tracks.at(tk1).mdtChPhi();
                    TrackletSegment aveSegML1(Chamber, ChEta, ChPhi, tracks.at(tk1).getML1seg().getChMidPoint(), aveAlpha, alphaErr, gpos,
                                              rErr, zErr, tracks.at(tk1).getML1seg().mdtHitsOnTrack(), 0);
                    float pT = 10000.0 * std::sin(aveSegML1.alpha());
                    float pz = 10000.0 * std::cos(aveSegML1.alpha());
                    Amg::Vector3D momentum(pT * std::cos(aveSegML1.globalPosition().phi()), pT * std::sin(aveSegML1.globalPosition().phi()),
                                           pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = sq(tracks.at(tk1).getML1seg().rError());  // delta R
                    matrix(1, 1) = sq(tracks.at(tk1).getML1seg().zError());  // delta z
                    matrix(2, 2) = sq(0.0000001);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = sq(tracks.at(tk1).getML1seg().alphaError());  // delta theta
                    matrix(4, 4) = sq(0.00005);                                  // delta 1/p
                    Tracklet aveTrack(aveSegML1, momentum, matrix, 0);
                    UniqueTracks.push_back(aveTrack);
                }
            }  // end barrel Tracks

            // Endcap tracks
            else if ((tracks.at(tk1).mdtChamber() > 11 && tracks.at(tk1).mdtChamber() <= 21) || tracks.at(tk1).mdtChamber() == 49) {
                std::vector<const Muon::MdtPrepData*> AllMdts;
                for (unsigned int i = 0; i < AmbigTracks.size(); ++i) {
                    std::vector<const Muon::MdtPrepData*> mdts = AmbigTracks.at(i).mdtHitsOnTrack();
                    std::vector<const Muon::MdtPrepData*> tmpAllMdt = AllMdts;
                    for (unsigned int m1 = 0; m1 < mdts.size(); ++m1) {
                        bool isNewHit = true;
                        for (unsigned int m2 = 0; m2 < tmpAllMdt.size(); ++m2) {
                            if (mdts.at(m1)->identify() == tmpAllMdt.at(m2)->identify()) {
                                isNewHit = false;
                                break;
                            }
                        }
                        if (isNewHit) AllMdts.push_back(mdts.at(m1));
                    }  // end loop on mdts
                }      // end loop on ambiguous tracks

                std::vector<TrackletSegment> MyECsegs = TrackletSegmentFitter(AllMdts);
                if (!MyECsegs.empty()) {
                    TrackletSegment ECseg = MyECsegs.at(0);
                    ECseg.clearMdt();
                    float pT = 10000.0 * std::sin(ECseg.alpha());
                    float pz = 10000.0 * std::cos(ECseg.alpha());
                    Amg::Vector3D momentum(pT * std::cos(ECseg.globalPosition().phi()), pT * std::sin(ECseg.globalPosition().phi()), pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = sq(ECseg.rError());  // delta R
                    matrix(1, 1) = sq(ECseg.zError());  // delta z
                    matrix(2, 2) = sq(0.0000001);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = sq(ECseg.alphaError());  // delta theta
                    matrix(4, 4) = sq(0.00005);  // delta 1/p (endcap tracks are straight lines with no momentum that we can measure ...)
                    Tracklet MyCombTrack(MyECsegs.at(0), ECseg, momentum, matrix, 0);
                    UniqueTracks.push_back(MyCombTrack);
                } else
                    UniqueTracks.push_back(tracks.at(tk1));
            }  // end endcap tracks

        }  // end loop on tracks -- tk1

        return UniqueTracks;
    }

}  // namespace Muon
