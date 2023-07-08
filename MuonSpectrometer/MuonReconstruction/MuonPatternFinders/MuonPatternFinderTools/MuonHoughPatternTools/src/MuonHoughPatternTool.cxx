/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternTools/MuonHoughPatternTool.h"

#include "CxxUtils/sincos.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonHoughPatternEvent/MuonHoughPattern.h"
#include "MuonHoughPatternEvent/MuonHoughTransformSteering.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer_CurvedAtACylinder.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer_rz.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer_rzcosmics.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer_xy.h"
#include "MuonHoughPatternEvent/MuonHoughTransformer_yz.h"
#include "MuonPrepRawData/MdtPrepData.h"
#include "MuonPrepRawData/MuonCluster.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH2F.h"
#include "TString.h"
#include "TrkSurfaces/Surface.h"

MuonHoughPatternTool::MuonHoughPatternTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<IMuonHoughPatternTool>(this);

    m_detectorsize_xy = m_detectorsize_xy_full;
    m_detectorsize_yz = m_detectorsize_yz_full;
    m_detectorsize_rz = m_detectorsize_rz_full;
}

void MuonHoughPatternTool::useIPMuons() {
    m_detectorsize_xy = m_detectorsize_xy_ip;
    m_detectorsize_yz = m_detectorsize_yz_ip;
    m_detectorsize_rz = m_detectorsize_rz_ip;
    m_use_ip = true;
}

void MuonHoughPatternTool::makePatterns(const MuonHoughHitContainer& hitcontainer, MuonHoughPatternContainerShip& houghpattern) const {
    /** skip cosmic events that have more than 1000 phi hits */
    if (m_use_cosmics && m_maxNumberOfPhiHits >= 0) {
        int phihits{0};
        for (unsigned int hitid = 0; hitid < hitcontainer.size(); ++hitid) { phihits += hitcontainer.getMeasuresPhi(hitid); }
        if (phihits > m_maxNumberOfPhiHits) {
            ATH_MSG_DEBUG("Cosmic event has more than 1000 phi hits: " << phihits << " event is not reconstructed!");
            return;
        }
    }

    /** value of mdt weight cut, dependent on # hits in event */
    double weightmdt{0.};
    if (m_weightcutmdt) { setWeightMdtCutValue(hitcontainer, weightmdt); }

    ATH_MSG_DEBUG("Mdt Cut Value: " << weightmdt);

    // reset weights, based on rejection factor and weightmdt
    calculateWeights(hitcontainer, weightmdt);

    if (msgLevel(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("Event Info");
        ATH_MSG_VERBOSE("Size: " << hitcontainer.size());

        for (unsigned int i = 0; i < hitcontainer.size(); ++i) {
            const std::shared_ptr<MuonHoughHit> hit = hitcontainer.getHit(i);
            ATH_MSG_VERBOSE(hit->getHitx() << " " << hit->getHity() << " " << hit->getHitz() << " " << hit->getMeasuresPhi() << " "
                                           << hit->getWhichDetector() << " " << hit->getProbability() << " " << hit->getWeight() << " "
                                           << hit->getAssociated());
        }
    }
    makePatterns(MuonHough::hough_xy, weightmdt, hitcontainer, houghpattern);

    if (m_use_cosmics) {
        makePatterns(MuonHough::hough_rzcosmics, weightmdt, hitcontainer, houghpattern);
    } else if (m_use_curvedhough) {
        makePatterns(MuonHough::hough_curved_at_a_cylinder, weightmdt, hitcontainer, houghpattern);
    } else {
        makePatterns(MuonHough::hough_rz, weightmdt, hitcontainer, houghpattern);
    }

    ATH_MSG_VERBOSE("End makePatterns ");
}

void MuonHoughPatternTool::makePatterns(int id_number, double weightmdt, const MuonHoughHitContainer& event,
                                        MuonHoughPatternContainerShip& houghpattern) const {
    ATH_MSG_DEBUG("makePatterns");

    resetAssociation(event);  // resets association, for hits that are already assigned to pattern in a previous hough

    std::unique_ptr<MuonHoughHitContainer> event_for_hough{whichEventHough(id_number, event, weightmdt)};
    std::unique_ptr<MuonHoughHitContainer> event_for_association{whichEventAssociation(id_number, event)};

    if (msgLevel(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("Size event fill: " << event_for_hough->size());
        for (unsigned int i = 0; i < event_for_hough->size(); ++i) {
            const std::shared_ptr<MuonHoughHit> hit = event_for_hough->getHit(i);
            ATH_MSG_VERBOSE(hit->getHitx() << " " << hit->getHity() << " " << hit->getHitz() << " " << hit->getMeasuresPhi() << " "
                                           << hit->getWhichDetector() << " " << hit->getProbability() << " " << hit->getWeight() << " "
                                           << hit->getAssociated());
        }

        ATH_MSG_VERBOSE("Size event association: " << event_for_association->size());
        for (unsigned int i = 0; i < event_for_association->size(); ++i) {
            const std::shared_ptr<MuonHoughHit> hit = event_for_association->getHit(i);
            ATH_MSG_VERBOSE(hit->getHitx() << " " << hit->getHity() << " " << hit->getHitz() << " " << hit->getMeasuresPhi() << " "
                                           << hit->getWhichDetector() << " " << hit->getProbability() << " " << hit->getWeight() << " "
                                           << hit->getAssociated());
        }

        ATH_MSG_DEBUG("size of event: " << event_for_association->size() << " id_number: " << id_number);
    }
    std::unique_ptr<MuonHoughTransformSteering> houghtransform{whichHoughTransform(id_number)};  // const?

    ATH_MSG_DEBUG("HoughTransform chosen");

    bool test_for_next_level = true;

    for (int level = 0; level < m_maximum_level; level++) {
        if (test_for_next_level) {
            ATH_MSG_DEBUG("Iteration number: " << level);
            
            houghtransform->resetHisto();
            ATH_MSG_DEBUG("fillHistos size hits not in patterns " << event_for_hough->size());
            houghtransform->fill(*event_for_hough);
    
            if (m_use_histos && level == 0 && id_number == MuonHough::hough_curved_at_a_cylinder) {
                const MuonHoughHisto2DContainer& histos = houghtransform->histos();
                TDirectory* dir = gDirectory;
                m_file->cd();
                for (int i = 0; i < histos.size(); ++i) {
                    const std::string hname = "hough_call_" + std::to_string(m_ncalls) + "_hist_" + std::to_string(i);
                    histos.getHisto(i)->bookAndFillRootHistogram(hname)->Write();
                }
                gDirectory = dir;
                ++m_ncalls;
            }

            // hitcontainer for association not updated
            test_for_next_level = analyseHisto(id_number, level, event_for_association, houghtransform, houghpattern);

            if (test_for_next_level) {
                event_for_hough = whichEventHough(id_number, *event_for_hough, weightmdt);
                ATH_MSG_DEBUG("New event size for transform: " << event_for_hough->size());
            }
        } else {
            break;
        }
    }

}  // id_number

StatusCode MuonHoughPatternTool::initialize() {
    if (m_use_histos)  // debug histos
    {
        m_file = std::make_unique<TFile>("HoughPattern.root", "RECREATE");
    }

    ATH_MSG_DEBUG("Use Cosmic Settings: " << m_use_cosmics);

    if (!m_use_cosmics) {
        // change histogram sizes:
        useIPMuons();
    }

    else {
        m_number_of_sectors_xyz = 1;
        m_number_of_sectors_rz = 1;
        // only 1 maximum search (number of sectors ==1):
        m_number_of_maxima = 1;
        // no curved hough for cosmics (muons assumed from ip):
        m_use_curvedhough = false;
    }

    ATH_MSG_VERBOSE("Thresholds for histo: xyz: " << m_thresholdhisto_xyz << " rz: " << m_thresholdhisto_rz);

    ATH_MSG_VERBOSE("Thresholds for pattern: xyz: " << m_thresholdpattern_xyz << " rz: " << m_thresholdpattern_rz);

    ATH_MSG_DEBUG("Number of iterations: " << m_maximum_level << " Maxima per iteration: " << m_number_of_maxima);

    return StatusCode::SUCCESS;
}

void MuonHoughPatternTool::resetAssociation(const MuonHoughHitContainer& event) {
    for (unsigned int i = 0; i < event.size(); ++i) {
        std::shared_ptr<MuonHoughHit> hit = event.getHit(i);
        hit->setAssociated(false);
        hit->setId(-1);  // ugly, to be changed?
    }
}

StatusCode MuonHoughPatternTool::finalize() {
    ATH_MSG_VERBOSE("finalize()");

    if (m_use_histos) {
        m_file->Write();
        m_file.reset();
    }

    return StatusCode::SUCCESS;
}

MuonHoughPatternContainerShip MuonHoughPatternTool::emptyHoughPattern() const {
    MuonHoughPatternContainerShip houghpattern;
    houghpattern.reserve(m_number_of_ids);
    for (int i = 0; i < m_number_of_ids; ++i) {
        MuonHoughPatternContainer which_segment_vector;
        which_segment_vector.reserve(m_maximum_level);
        houghpattern.emplace_back(std::move(which_segment_vector));

        for (int lvl = 0; lvl < m_maximum_level; lvl++) {
            MuonHoughPatternCollection level_vector;
            level_vector.reserve(m_number_of_maxima);
            houghpattern[i].emplace_back(std::move(level_vector));

            for (int maximum_number = 0; maximum_number < m_number_of_maxima; maximum_number++) {
                houghpattern[i][lvl].emplace_back(nullptr);

            }  // maximum_number
        }      // maximum_level
    }          // number_of_ids
    return houghpattern;
}  // emptyHoughPattern

bool MuonHoughPatternTool::analyseHisto(int id_number, int level, const std::unique_ptr<MuonHoughHitContainer>& event_to_analyse,
                                        std::unique_ptr<MuonHoughTransformSteering>& houghtransform,
                                        MuonHoughPatternContainerShip& houghpatterns_all) const {
    ATH_MSG_VERBOSE("analyseHisto MuonHoughPatternTool (start)");

    /** The Analyse-fillHistos loop is at three times ended for each id:
        1- if the maximum_number of hhistos[id] ==0
        2- if the houghpattern.size() < threshold_for_next_houghpattern
        3 - if level == maximum_level
        at all these 3 times an array[number_of_ids] of int levelmax will be houghpattern has to be deleted.. (all in level>0)
        4- if level not gets to 1  // but then no houghpatterns made.. ?
        5- if numberOfHits left ==0 */

    bool test_for_next_level = false;

    const unsigned int threshold_for_next_houghpattern = getThresholdHoughPattern(id_number);
    double numberofmaxima = 0;
    double maximum_residu = m_maximum_residu_mm;
    if (m_use_cosmics) { maximum_residu = m_maximum_residu_mm_cosmics; }
    MuonHoughPatternCollection houghpatterns = houghtransform->constructHoughPatterns(
        *event_to_analyse, maximum_residu, m_maximum_residu_angle, m_number_of_maxima);

    for (unsigned int maximum_number = 0; maximum_number < houghpatterns.size(); ++maximum_number) {
  
        std::unique_ptr<MuonHoughPattern>& houghpattern = houghpatterns[maximum_number];
        if (!houghpattern) { continue; }
        numberofmaxima = houghpattern->getMaximumHistogram();
        ATH_MSG_DEBUG("id_number: " << id_number << " maximum_number: " << maximum_number << " size patternseg: " << houghpattern->size());

        if (houghpattern->empty()) { ATH_MSG_DEBUG("houghpattern==0"); }

        // some print statements
        if (houghpattern->size() < numberofmaxima) {
            ATH_MSG_DEBUG("ERROR: houghpattern smaller than maximum, id: " << id_number << " houghpattern.size(): " << houghpattern->size()
                                                                           << " numberofmaxima: " << numberofmaxima);
        }

       if (m_printlevel >= 4) { houghpattern->printHoughPattern(); }

        // checks for next level / maximum

        if (houghpattern->size() >= threshold_for_next_houghpattern) {
            if (level + 1 >= m_maximum_level) {
                ATH_MSG_DEBUG("possibly more levels");
            } else {
                test_for_next_level = hitsLeft(*event_to_analyse);
            }
        } else if (maximum_number == 0) {
            ATH_MSG_DEBUG("houghpattern too small for next level : " << level << " id: " << id_number);
        }

        // print_of houghpatterns:
        ATH_MSG_DEBUG("Size of HoughPatterns: " << houghpattern->size());
        houghpatterns_all[id_number][level][maximum_number] = std::move(houghpattern);

    }  // maximum_number

    ATH_MSG_DEBUG(" Test for next level: " << test_for_next_level);

    return test_for_next_level;

}  // analyseHisto

bool MuonHoughPatternTool::hitsLeft(const MuonHoughHitContainer& event) {
    int number_of_hits = event.size();
    for (int hitid = 0; hitid < number_of_hits; ++hitid) {
        if (!event.getHit(hitid)->getAssociated()) { return true; }
    }
    return false;
}

int MuonHoughPatternTool::numberOfHits(const MuonHoughHitContainer& event) const {
    int number_of_hits_left = 0;
    int number_of_hits = event.size();

    for (int hitid = 0; hitid < number_of_hits; ++hitid) { number_of_hits_left += !event.getHit(hitid)->getAssociated(); }

    // logically impossible --- if (number_of_hits_left <0) {ATH_MSG_WARNING("number of hits smaller than 0");}

    ATH_MSG_VERBOSE("numberOfHits left: " << number_of_hits_left);
    ATH_MSG_VERBOSE("number_of_hits: " << number_of_hits);
    return number_of_hits_left;
}

bool MuonHoughPatternTool::hitInHoughPattern(const std::shared_ptr<MuonHoughHit>& hit, const MuonHoughPatternContainer& houghpattern) const {
    // checks if hit is already assigned to a houghpattern

    for (unsigned int i = 0; i < houghpattern.size(); ++i) {
        for (unsigned int j = 0; j < houghpattern[i].size(); ++j) {
            if (houghpattern[i][j]) {
                if (houghpattern[i][j]->hitInHoughPattern(hit)) {
                    ATH_MSG_VERBOSE("Hit in hough pattern found level " << i << " max " << j << "hitid: " << hit->getId());
                    return true;
                }
            }
        }
    }
    return false;
}

void MuonHoughPatternTool::weightRescaling(const MuonHoughHitContainer& event, int id_number, int level) const {
    double weight_trigger_hits = 1.;
    double weight_mdt_hits = 1.;

    switch (id_number) {
        case MuonHough::hough_xy:
        case MuonHough::hough_yz:
            weight_trigger_hits = 1 - (level - 1) * (1. / (m_maximum_level + 0.0));  // 1 , 1, 0.8, 0.6 , 0.4
            if (weight_trigger_hits > 1) weight_trigger_hits = 1.;
            weight_mdt_hits = weight_trigger_hits;
            break;
        case MuonHough::hough_rz:
        case MuonHough::hough_rzcosmics:
        case MuonHough::hough_rz_rpc:
        case MuonHough::hough_rz_mdt:
        case MuonHough::hough_curved_at_a_cylinder:
            switch (level) {
                case 0:
                    weight_trigger_hits = 1.;
                    weight_mdt_hits = 1.;
                    break;
                case 1:
                    weight_trigger_hits = 1.;
                    weight_mdt_hits = 1.;
                    break;
                case 2:
                    weight_trigger_hits = 1.;
                    weight_mdt_hits = 0.75;
                    break;
                case 3:
                    weight_trigger_hits = 0.75;
                    weight_mdt_hits = 0.5;
                    break;
                case 4:
                    weight_trigger_hits = 0.5;
                    weight_mdt_hits = 0.25;
                    break;
                default:
                    ATH_MSG_DEBUG("no weight defined for this level");
                    weight_trigger_hits = 0.5;
                    weight_mdt_hits = 0.25;
            }
            break;
        default: ATH_MSG_WARNING("no valid id (id_number)");
    }

    for (unsigned int i = 0; i < event.size(); ++i) {
        std::shared_ptr<MuonHoughHit> hit = event.getHit(i);
        MuonHough::DetectorTechnology technology = hit->getDetectorId();
        switch (technology) {
            case MuonHough::CSC:
            case MuonHough::RPC:
            case MuonHough::TGC: hit->setWeight(hit->getOrigWeight() * weight_trigger_hits); break;
            case MuonHough::MDT: hit->setWeight(hit->getOrigWeight() * weight_mdt_hits); break;
            default: ATH_MSG_WARNING("no valid detector technology");
        }
    }
}

void MuonHoughPatternTool::calculateWeights(const MuonHoughHitContainer& event, double weightmdt) const {
    if (weightmdt < 0.5) return;
    // else do nothing (e.g. cosmics case)
    for (unsigned int i = 0; i < event.size(); ++i) {
        std::shared_ptr<MuonHoughHit> hit = event.getHit(i);
        MuonHough::DetectorTechnology technology = hit->getDetectorId();
        if (technology == MuonHough::MDT) {
            // recalculate weight, especially important for cavern background MDT events
            double p_old = hit->getOrigWeight();
            double p_calc = 0.25 * p_old * (1. - weightmdt);
            double p_new = p_calc / (p_calc + weightmdt * (1 - p_old));
            ATH_MSG_VERBOSE(" MDT probability old " << p_old << " Recalculated " << p_new);
            hit->setWeight(p_new);
        }
    }
}

int MuonHoughPatternTool::overlapHoughPatterns(const MuonHoughPattern& houghpattern1, const MuonHoughPattern& houghpattern2) const {
    // both vectors houghpatterns->m_hitid[] are ordered, asked is the percentage of overlap between both vectors

    int overlap = 0;
    unsigned int j = 0;

    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        while (j < houghpattern2.size()) {
            if (houghpattern1.getHitId(i) == houghpattern2.getHitId(j)) {
                ++overlap;
                ++j;  // this j cant be found again
                break;
            }
            if (houghpattern1.getHitId(i) < houghpattern2.getHitId(j)) { break; }
            ++j;
        }
    }
    double percentage1 = (1.0 * overlap) / houghpattern1.size();  // size() gives double
    double percentage2 = (1.0 * overlap) / houghpattern2.size();

    ATH_MSG_DEBUG("Percentage Overlap: " << percentage1 << " " << percentage2);
    return overlap;
}

std::unique_ptr<MuonHoughHitContainer> MuonHoughPatternTool::whichEventAssociation(int id_number,
                                                                                   const MuonHoughHitContainer& event) const {
    std::unique_ptr<MuonHoughHitContainer> event_to_analyse = std::make_unique<MuonHoughHitContainer>();

    switch (id_number) {
        case MuonHough::hough_xy:

            for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                if (hit->getMeasuresPhi()) {
                    if (m_use_csc_in_pattern || (!m_use_csc_in_pattern && hit->getDetectorId() != MuonHough::CSC)) {
                        event_to_analyse->addHit(hit);
                    }
                }
            }
            break;
        case MuonHough::hough_yz:
            for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                event_to_analyse->addHit(hit);
            }
            break;
        case MuonHough::hough_rz:
        case MuonHough::hough_rzcosmics:
        case MuonHough::hough_curved_at_a_cylinder:
            for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                if (!hit->getMeasuresPhi()) {
                    if (m_use_csc_in_pattern || (!m_use_csc_in_pattern && hit->getDetectorId() != MuonHough::CSC)) {
                        event_to_analyse->addHit(hit);
                    }
                }
            }
            break;
        case MuonHough::hough_rz_rpc:
            if (m_use_rpc_measures_eta == 1) {
                for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                    std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                    if (hit->getDetectorId() == MuonHough::RPC) {
                        if (!hit->getMeasuresPhi()) { event_to_analyse->addHit(hit); }
                    }
                }
            } else {
                for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                    std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                    if (hit->getDetectorId() == MuonHough::RPC) { event_to_analyse->addHit(hit); }
                }
            }
            break;
        case MuonHough::hough_rz_mdt:
            for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = event.getHit(hitid);
                if (hit->getDetectorId() == MuonHough::MDT) { event_to_analyse->addHit(hit); }
            }
            break;
        default: ATH_MSG_WARNING(" no valid id");
    }

    return event_to_analyse;
}

std::unique_ptr<MuonHoughHitContainer> MuonHoughPatternTool::whichEventHough(int id_number, const MuonHoughHitContainer& event,
                                                                             double weightmdt) const {
    ATH_MSG_DEBUG("whichEventHough::size of event: " << event.size());
    std::unique_ptr<MuonHoughHitContainer> hits_not_in_patterns{hitsNotInPattern(event, id_number)};
    ATH_MSG_DEBUG("whichEventHough::hitsNotInPattern: " << hits_not_in_patterns->size());
    std::unique_ptr<MuonHoughHitContainer> event_to_analyse = std::make_unique<MuonHoughHitContainer>();

    switch (id_number) {
        case MuonHough::hough_xy:

            for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                if (hit->getMeasuresPhi() == 1) {
                    if (hitThroughCut(hit, weightmdt)) {
                        if (m_use_csc_in_hough || (!m_use_csc_in_hough && hit->getDetectorId() == MuonHough::CSC)) {
                            event_to_analyse->addHit(hit);
                        }
                    }
                }
            }
            break;
        case MuonHough::hough_yz:
            for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                if (hitThroughCut(hit, weightmdt)) { event_to_analyse->addHit(hit); }
            }
            break;
        case MuonHough::hough_rz:
        case MuonHough::hough_rzcosmics:
        case MuonHough::hough_curved_at_a_cylinder:
            for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                if (hitThroughCut(hit, weightmdt)) {
                    if (hit->getMeasuresPhi() == 0) {
                        if (m_use_csc_in_hough || (!m_use_csc_in_hough && hit->getDetectorId() == MuonHough::CSC)) {
                            event_to_analyse->addHit(hit);
                        }
                    }
                }
            }
            break;
        case MuonHough::hough_rz_rpc:
            if (m_use_rpc_measures_eta == 1) {
                for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                    if (hits_not_in_patterns->getDetectorId(hitid) == MuonHough::RPC) {
                        std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                        if (hit->getMeasuresPhi() == 0) { event_to_analyse->addHit(hit); }
                    }
                }
            } else {
                for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                    if (hits_not_in_patterns->getDetectorId(hitid) == MuonHough::RPC) {
                        std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                        event_to_analyse->addHit(hit);
                    }
                }
            }
            break;
        case MuonHough::hough_rz_mdt:
            for (unsigned int hitid = 0; hitid < hits_not_in_patterns->size(); ++hitid) {
                if (hits_not_in_patterns->getDetectorId(hitid) == MuonHough::MDT) {
                    std::shared_ptr<MuonHoughHit> hit = hits_not_in_patterns->getHit(hitid);
                    if (hitThroughCut(hit, weightmdt)) { event_to_analyse->addHit(hit); }
                }
            }
            break;
        default: ATH_MSG_WARNING(" no valid id");
    }

    return event_to_analyse;
}

std::unique_ptr<MuonHoughTransformSteering> MuonHoughPatternTool::whichHoughTransform(int id_number) const {
    std::unique_ptr<MuonHoughTransformer> houghtransformer;

    int nbins = 0;
    int nbins_angle = 0;
    double detectorsize_angle_xy = m_detectorsize_angle_xyz;
    double stepsize_per_angle_xy = m_stepsize_per_angle_xyz;
    double detectorsize_curved = m_nbins_curved / 2.;
    double stepsize_xy = m_stepsize_xy;
    // additional histograms for overlaps:
    int number_of_histos_rz = 2 * m_number_of_sectors_rz;

    switch (id_number) {
        case MuonHough::hough_xy:
            if (m_use_cosmics) {
                stepsize_xy = m_stepsize_xy_cosmics;
                stepsize_per_angle_xy = m_stepsize_per_angle_xy_cosmics;
                detectorsize_angle_xy = (m_detectorsize_angle_xyz / 2.);  // patterns not split for cosmics
            }
            nbins = static_cast<int>(2 * m_detectorsize_xy / stepsize_xy);
            nbins_angle = static_cast<int>(detectorsize_angle_xy / stepsize_per_angle_xy);
            houghtransformer = std::make_unique<MuonHoughTransformer_xy>(nbins, nbins_angle, m_detectorsize_xy, detectorsize_angle_xy,
                                                                         m_thresholdhisto_xyz, m_number_of_sectors_xyz);
            break;

        case MuonHough::hough_yz:
            nbins = static_cast<int>(2 * m_detectorsize_yz / m_stepsize_yz);
            nbins_angle = static_cast<int>(m_detectorsize_angle_xyz / m_stepsize_per_angle_xyz);
            houghtransformer = std::make_unique<MuonHoughTransformer_yz>(nbins, nbins_angle, m_detectorsize_yz, m_detectorsize_angle_xyz,
                                                                         m_thresholdhisto_xyz, m_number_of_sectors_xyz);
            break;

        case MuonHough::hough_rz:
        case MuonHough::hough_rz_rpc:
        case MuonHough::hough_rz_mdt:
            nbins = static_cast<int>(2 * m_detectorsize_rz / m_stepsize_rz);
            nbins_angle = static_cast<int>(m_detectorsize_angle_rz / m_stepsize_per_angle_rz);
            houghtransformer = std::make_unique<MuonHoughTransformer_rz>(nbins, nbins_angle, m_detectorsize_rz, m_detectorsize_angle_rz,
                                                                         m_thresholdhisto_rz, m_number_of_sectors_rz);
            break;

        case MuonHough::hough_rzcosmics:
            nbins = static_cast<int>(2 * m_detectorsize_rz / m_stepsize_rz_cosmics);
            nbins_angle = static_cast<int>(m_detectorsize_angle_rz / m_stepsize_per_angle_rz_cosmics);
            houghtransformer = std::make_unique<MuonHoughTransformer_rzcosmics>(
                nbins, nbins_angle, m_detectorsize_rz, m_detectorsize_angle_rz, m_thresholdhisto_rz, m_number_of_sectors_rz_cosmics);
            break;

        case MuonHough::hough_curved_at_a_cylinder:
            nbins = m_nbins_curved;
            nbins_angle = static_cast<int>(m_detectorsize_angle_rz / (2 * m_stepsize_per_angle_rz));
            houghtransformer = std::make_unique<MuonHoughTransformer_CurvedAtACylinder>(
                nbins, nbins_angle, detectorsize_curved, m_detectorsize_angle_rz, m_thresholdhisto_rz, number_of_histos_rz);
            break;

        default: ATH_MSG_WARNING("no valid id");
    }

    if (houghtransformer) {
        houghtransformer->useNegativeWeights(m_use_negative_weights);
        houghtransformer->setIP(!m_use_cosmics);
    }

    ATH_MSG_DEBUG("**** histo houghtransformer: ****");
    ATH_MSG_DEBUG("Id number: " << id_number);
    ATH_MSG_DEBUG("NBins: " << nbins << " angle: " << nbins_angle);
    if (m_use_negative_weights) ATH_MSG_DEBUG(" Negative weights are used ");
    ATH_MSG_DEBUG("IP setting: " << !m_use_cosmics);
    ATH_MSG_DEBUG("*********************************");
    return std::make_unique<MuonHoughTransformSteering>(std::move(houghtransformer));
}

std::vector<int> MuonHoughPatternTool::maxLevelHoughPattern(const MuonHoughPatternContainerShip& houghpattern) const  // obsolete?
{
    std::vector<int> maxlevel_houghpattern(m_number_of_ids);

    for (int id_number = 0; id_number < m_number_of_ids; id_number++) {
        maxlevel_houghpattern[id_number] = maxLevelHoughPattern(houghpattern, id_number);
    }  // number_of_ids
    return maxlevel_houghpattern;
}

int MuonHoughPatternTool::maxLevelHoughPattern(const MuonHoughPatternContainerShip& houghpattern, int id_number) const {
    int maxlevel = houghpattern[id_number].size();
    bool continu = true;
    while (maxlevel >= 1 && continu == 1) {
        unsigned int maximum_number = 0;

        while (continu && maximum_number < houghpattern[id_number][maxlevel - 1].size())  // m_number_of_maxima)
        {
            ATH_MSG_DEBUG("maximum_number: " << maximum_number << " "
                                             << "maxlevel_houghpattern: " << maxlevel << " id: " << id_number);
            if (!houghpattern[id_number][maxlevel - 1][maximum_number]->empty()) { continu = false; }

            ++maximum_number;

        }  // number_of_maxima

        if (continu) { maxlevel--; }
    }  // while
    return maxlevel;
}  // maxLevelHoughPattern

void MuonHoughPatternTool::transformCoordsMaximum(std::pair<double, double>& coordsmaximum, double r0_true) const {
    double z_true = coordsmaximum.first;
    double theta_true = coordsmaximum.second;

    //  double theta_cor = - 0.042*(r0_true/4000.)*(r0_true/4000);
    double theta_cor = m_theta_cor_constant * (r0_true / m_theta_cor_constant2) * (r0_true / m_theta_cor_constant2);

    //  double z_cor = - 10000 * (std::cos(theta_true) * r0_true/6000.) * (std::cos(theta_true)*r0_true/6000.);
    double z_cor =
        m_z_cor_constant * (std::cos(theta_true) * r0_true / m_z_cor_constant2) * (std::cos(theta_true) * r0_true / m_z_cor_constant2);

    double z_rec = z_true + z_cor;
    double theta_rec = theta_true + theta_cor;

    if (std::cos(theta_true) < 0) {
        z_rec = z_true - z_cor;
        theta_rec = theta_true - theta_cor;
    }

    coordsmaximum.first = z_rec;
    coordsmaximum.second = theta_rec;
}  // transformCoordsMaximum

std::unique_ptr<MuonPrdPatternCollection> MuonHoughPatternTool::getPhiMuonPatterns(MuonHoughPatternContainerShip& houghpatterns) const {
    std::unique_ptr<MuonPrdPatternCollection> phipatterncollection = std::make_unique<MuonPrdPatternCollection>();
    phipatterncollection->reserve(m_maximum_level * m_number_of_maxima);

    MuonHoughPatternContainer& phipatterns = houghpatterns[MuonHough::hough_xy];

    // Bookkeeping for merged or double phi pattersn

    std::map<MuonHoughPattern*, int> mergedpatterns;
    for (unsigned int i = 0; i < phipatterns.size(); ++i) {
        for (unsigned int j = 0; j < phipatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern = phipatterns[i][j];
            if (!houghpattern) continue;
            mergedpatterns[houghpattern.get()] = 0;
        }
    }

    // Search for identical phi patterns and remove them
    // and search for overlapping phi patterns and merge them for IP constraint (10-1-2008, does merging ever happen? JS)

    for (unsigned int i = 0; i < phipatterns.size(); ++i) {
        for (unsigned int j = 0; j < phipatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern1 = phipatterns[i][j];
            if (!houghpattern1) continue;
            if (phipatterns[i][j]->size() < m_thresholdpattern_xyz) continue;
            ATH_MSG_DEBUG(" patterns size before Merge " << phipatterns[i][j]->size());
            for (unsigned int k = i; k < phipatterns.size(); k++) {
                for (unsigned int l = 0; l < phipatterns[k].size(); l++) {
                    std::unique_ptr<MuonHoughPattern>& houghpattern2 = phipatterns[k][l];
                    if (!houghpattern2) continue;
                    if (phipatterns[k][l]->size() < m_thresholdpattern_xyz) continue;
                    //cppcheck-suppress mismatchingContainers
                    if (houghpattern1.get() == houghpattern2.get()) continue;
                    if (mergedpatterns[houghpattern1.get()] == 1) continue;
                    if (mergedpatterns[houghpattern2.get()] == 1) continue;
                    const double phi1 = houghpattern1->getEPhi();
                    const double phi2 = houghpattern2->getEPhi();
                    CxxUtils::sincos scphi1(phi1);
                    CxxUtils::sincos scphi2(phi2);
                    double dotprod = scphi1.cs * scphi2.cs + scphi1.sn * scphi2.sn;
                    if (dotprod > 1.)
                        dotprod = 1.;
                    else if (dotprod < -1.)
                        dotprod = -1.;
                    double psi = std::acos(dotprod);
                    const double the1 = houghpattern1->getETheta();
                    const double the2 = houghpattern2->getETheta();
                    CxxUtils::sincos scthe1(the1);
                    CxxUtils::sincos scthe2(the2);
                    dotprod = scthe1.cs * scthe2.cs + scthe1.sn * scthe2.sn;
                    if (dotprod > 1.)
                        dotprod = 1.;
                    else if (dotprod < -1.)
                        dotprod = -1.;
                    double chi = std::acos(dotprod);
                    ATH_MSG_DEBUG(" patterns phi1 " << phi1 << " phi2 " << phi2 << " psi " << psi);
                    ATH_MSG_DEBUG(" patterns the1 " << the1 << " the2 " << the2 << " chi " << chi);
                    if (chi < 0.5 || psi < 0.5) {
                        int overlap = overlapHoughPatterns(*houghpattern1, *houghpattern2);
                        ATH_MSG_DEBUG(" Phi Overlap " << overlap << " size1 " << houghpattern1->size() << " size2 "
                                                      << houghpattern2->size());
                        int ns1 = houghpattern1->size();
                        int ns2 = houghpattern2->size();
                        if (overlap <= ns1 && overlap == ns2) {
                            ATH_MSG_DEBUG(" DROP patterns same hits ");
                            mergedpatterns[houghpattern2.get()] = 1;
                            continue;
                        }
                        if (overlap == ns1 && overlap < ns2) {
                            ATH_MSG_DEBUG(" DROP patterns same hits ");
                            mergedpatterns[houghpattern1.get()] = 1;
                            continue;
                        }
                        if (m_use_ip) {
                            // Merge and do cleaning  (IP constraint)
                            std::unique_ptr<Muon::MuonPrdPattern> muonpattern;
                            if ((overlap > 0.8 * ns1 || overlap > 0.8 * ns2) && ns1 >= ns2) {
                                muonpattern = houghPatternsToOnePhiPattern(*phipatterns[i][j], *phipatterns[k][l]);
                            }
                            if ((overlap > 0.8 * ns1 || overlap > 0.8 * ns2) && ns1 < ns2) {
                                // Merge and do cleaning  (IP constraint)
                                muonpattern = houghPatternsToOnePhiPattern(*phipatterns[k][l], *phipatterns[i][j]);
                            }
                            if (muonpattern) {
                                phipatterncollection->push_back(std::move(muonpattern));
                                mergedpatterns[houghpattern1.get()] = 1;
                                mergedpatterns[houghpattern2.get()] = 1;
                                continue;
                            }
                        }  // use IP
                    }      // angular cut
                }
            }  // end k
        }
    }  // end i

    for (unsigned int i = 0; i < phipatterns.size(); ++i) {
        for (unsigned int j = 0; j < phipatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern = phipatterns[i][j];
            if (!houghpattern) { continue; }
            if (mergedpatterns[houghpattern.get()] == 1) continue;

            if (!phipatterns[i][j]->empty()) {
                std::unique_ptr<Muon::MuonPrdPattern> muonpattern;

                if (!m_use_ip) {
                    muonpattern = houghPatternToPhiPattern(*phipatterns[i][j]);
                } else {
                    muonpattern = houghPatternToCleanPhiPattern(*phipatterns[i][j]);
                }

                if (muonpattern) {
                    ATH_MSG_DEBUG(" Lift MuonPhiPattern size " << muonpattern->numberOfContainedPrds());
                    if (msgLvl(MSG::VERBOSE)) { printPattern(muonpattern.get()); }
                    phipatterncollection->push_back(std::move(muonpattern));
                }
            }
        }
    }

    return phipatterncollection;
}

std::unique_ptr<MuonPrdPatternCollection> MuonHoughPatternTool::getEtaMuonPatterns(MuonHoughPatternContainerShip& houghpatterns) const {
    std::unique_ptr<MuonPrdPatternCollection> etapatterncollection = std::make_unique<MuonPrdPatternCollection>();

    int maximum_number_of_patterns = m_maximum_level * m_number_of_maxima;

    if (m_use_curvedhough) maximum_number_of_patterns = 2 * maximum_number_of_patterns;

    etapatterncollection->reserve(maximum_number_of_patterns);

    int id = MuonHough::hough_rz;
    if (m_use_cosmics) {
        id = MuonHough::hough_rzcosmics;
        ATH_MSG_DEBUG(" GetEtaMuonPatterns Use RZ curved hough patterns ");
    } else if (m_use_curvedhough) {
        id = MuonHough::hough_curved_at_a_cylinder;
        ATH_MSG_DEBUG(" GetEtaMuonPatterns Use curved hough patterns ");
    } else {
        ATH_MSG_DEBUG(" GetEtaMuonPatterns Use RZ hough patterns ");
    }

    MuonHoughPatternContainer& etapatterns = houghpatterns[id];

    // Bookkeeping for merged or double eta patterns

    std::map<MuonHoughPattern*, int> mergedpatterns;
    for (unsigned int i = 0; i < etapatterns.size(); ++i) {
        for (unsigned int j = 0; j < etapatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern = etapatterns[i][j];
            if (!houghpattern) continue;
            mergedpatterns[houghpattern.get()] = 0;
        }
    }

    // Search for identical eta patterns and remove them
    // and search for overlapping eta patterns and merge them (10-1-2008, does merging ever happen? JS, yes it does!)

    for (unsigned int i = 0; i < etapatterns.size(); ++i) {
        for (unsigned int j = 0; j < etapatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern1 = etapatterns[i][j];
            if (!houghpattern1) continue;
            if (etapatterns[i][j]->size() < m_thresholdpattern_rz) continue;
            ATH_MSG_DEBUG(" Eta patterns size before Merge " << etapatterns[i][j]->size());
            for (unsigned int k = i; k < etapatterns.size(); k++) {
                for (unsigned int l = 0; l < etapatterns[k].size(); l++) {
                    std::unique_ptr<MuonHoughPattern>& houghpattern2 = etapatterns[k][l];
                    if (!houghpattern2) continue;
                    if (etapatterns[k][l]->size() < m_thresholdpattern_rz) continue;
                    //cppcheck-suppress mismatchingContainers
                    if (houghpattern1.get() == houghpattern2.get()) continue;
                    if (mergedpatterns[houghpattern1.get()] == 1) continue;
                    if (mergedpatterns[houghpattern2.get()] == 1) continue;

                    // calculate if curvatures are compatible, not done for cosmics
                    double alpha = 0.;
                    if (!m_use_cosmics) {
                        double curv1 = houghpattern1->getECurvature();
                        double curv2 = houghpattern2->getECurvature();
                        if (std::abs(curv1) < 1001. || std::abs(curv2) < 1001.) {
                            ATH_MSG_DEBUG("Curvature too small, should not be possible: " << curv1 << " " << curv2);
                            continue;
                        }

                        double angle1 = std::acos((std::abs(curv1) - 1000.) / curv1);  // angle change after 1000 (mm)
                        double angle2 = std::acos((std::abs(curv2) - 1000.) / curv2);
                        alpha = std::abs(std::sin(angle1 - angle2));

                        ATH_MSG_DEBUG(" patterns curv1 " << curv1 << " curv2 " << curv2 << " alpha " << alpha);
                    }

                    double phi1 = houghpattern1->getEPhi();
                    double phi2 = houghpattern2->getEPhi();
                    double dotprod = std::cos(phi1) * std::cos(phi2) + std::sin(phi1) * std::sin(phi2);
                    if (dotprod > 1.)
                        dotprod = 1.;
                    else if (dotprod < -1.)
                        dotprod = -1.;
                    double psi = std::acos(dotprod);
                    double the1 = houghpattern1->getETheta();
                    double the2 = houghpattern2->getETheta();
                    dotprod = std::cos(the1) * std::cos(the2) + std::sin(the1) * std::sin(the2);
                    if (dotprod > 1.)
                        dotprod = 1.;
                    else if (dotprod < -1.)
                        dotprod = -1.;
                    double chi = std::acos(dotprod);

                    ATH_MSG_DEBUG(" patterns phi1 " << phi1 << " phi2 " << phi2 << " psi " << psi);
                    ATH_MSG_DEBUG(" patterns the1 " << the1 << " the2 " << the2 << " chi " << chi);

                    if (chi < 0.5 && psi < 0.5 && alpha < 0.05) {  // 0.05 (rad) corresponds with 3 degrees per m

                        int overlap = overlapHoughPatterns(*houghpattern1, *houghpattern2);
                        const int ns1 = houghpattern1->size();
                        const int ns2 = houghpattern2->size();

                        ATH_MSG_DEBUG(" Eta Overlap " << overlap << " size1 " << ns1 << " size2 " << ns2);

                        if (overlap == ns2 && overlap <= ns1) {
                            ATH_MSG_DEBUG(" DROP patterns overlapping hits ");
                            mergedpatterns[houghpattern2.get()] = 1;
                            continue;
                        }
                        if (overlap == ns1 && overlap < ns2) {
                            ATH_MSG_DEBUG(" DROP patterns overlapping hits ");
                            mergedpatterns[houghpattern1.get()] = 1;
                            continue;
                        }
                        std::unique_ptr<Muon::MuonPrdPattern> muonpattern;
                        // Look for 80% or more overlap
                        if ((overlap > 0.8 * ns1 || overlap > 0.8 * ns2) && ns1 >= ns2) {
                            muonpattern = houghPatternsToOneEtaPattern(*etapatterns[i][j], *etapatterns[k][l]);
                        }
                        if ((overlap > 0.8 * ns1 || overlap > 0.8 * ns2) && ns1 < ns2) {
                            muonpattern = houghPatternsToOneEtaPattern(*etapatterns[k][l], *etapatterns[i][j]);
                        }
                        if (muonpattern) {
                            etapatterncollection->push_back(std::move(muonpattern));
                            mergedpatterns[houghpattern1.get()] = 1;
                            mergedpatterns[houghpattern2.get()] = 1;
                            continue;
                        }
                    }  // end angular cut
                }
            }  // end k
        }
    }  // end i

    for (unsigned int i = 0; i < etapatterns.size(); ++i) {
        for (unsigned int j = 0; j < etapatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern = etapatterns[i][j];
            if (!houghpattern) { continue; }
            if (mergedpatterns[houghpattern.get()] == 1) continue;

            if (!etapatterns[i][j]->empty()) {
                std::unique_ptr<Muon::MuonPrdPattern> muonpattern = houghPatternToEtaPattern(*etapatterns[i][j]);
                etapatterncollection->push_back(std::move(muonpattern));

                ATH_MSG_DEBUG(" Lift MuonEtaPattern size " << etapatterns[i][j]->size());
                if (msgLvl(MSG::VERBOSE)) { printPattern(muonpattern.get()); }
            }
        }
    }

    return etapatterncollection;
}

std::unique_ptr<MuonPrdPatternCollection> MuonHoughPatternTool::getCurvedMuonPatterns(MuonHoughPatternContainerShip& houghpatterns) const {
    std::unique_ptr<MuonPrdPatternCollection> curvedpatterncollection = std::make_unique<MuonPrdPatternCollection>();

    int maximum_number_of_patterns = m_maximum_level * m_number_of_maxima;

    curvedpatterncollection->reserve(maximum_number_of_patterns);

    MuonHoughPatternContainer& curvedpatterns = houghpatterns[MuonHough::hough_curved_at_a_cylinder];
    for (unsigned int i = 0; i < curvedpatterns.size(); ++i) {
        for (unsigned int j = 0; j < curvedpatterns[i].size(); ++j) {
            std::unique_ptr<MuonHoughPattern>& houghpattern = curvedpatterns[i][j];
            if (!houghpattern) { continue; }

            if (!curvedpatterns[i][j]->empty()) {
                std::unique_ptr<Muon::MuonPrdPattern> muonpattern = houghPatternToEtaPattern(*curvedpatterns[i][j]);
                curvedpatterncollection->push_back(std::move(muonpattern));
                ATH_MSG_DEBUG(" Lift MuoncurvedPattern size " << curvedpatterns[i][j]->size());
            }
        }
    }
    return curvedpatterncollection;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonHoughPatternTool::houghPatternToEtaPattern(const MuonHoughPattern& houghpattern) const {
    ATH_MSG_VERBOSE("houghPatternToEtaPattern");

    const Amg::Vector3D position = houghpattern.getEPos();
    const Amg::Vector3D direction = houghpattern.getEDir();

    double curvature = houghpattern.getECurvature();
    double charge = curvature < 0 ? -1 : 1.;
    double pscale = std::abs(curvature);

    const double r0 = m_use_cosmics ? houghpattern.getERPhi() : 0.001;

    double x0 = charge * r0 * std::sin(houghpattern.getEPhi());
    double y0 = -charge * r0 * std::cos(houghpattern.getEPhi());

    const Amg::Vector3D pos = Amg::Vector3D(x0, y0, position[2]);
    const Amg::Vector3D dir = Amg::Vector3D(pscale * direction[0], pscale * direction[1], pscale * direction[2]);

    ATH_MSG_DEBUG("position: " << x0 << " " << y0 << " " << position[2]);
    ATH_MSG_DEBUG("direction: " << direction[0] << " " << direction[1] << " " << direction[2]);

    ATH_MSG_DEBUG(" Lift Eta Hough Pattern with charge " << charge << " Curvature " << pscale);
    std::unique_ptr<Muon::MuonPrdPattern> muonpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);

    for (unsigned int i = 0; i < houghpattern.size(); ++i) { muonpattern->addPrd(houghpattern.getPrd(i)); }

    return muonpattern;
}
std::unique_ptr<Muon::MuonPrdPattern> MuonHoughPatternTool::houghPatternToPhiPattern(const MuonHoughPattern& houghpattern) const {
    ATH_MSG_VERBOSE("houghPatternToPhiPattern");

    const Amg::Vector3D pos = houghpattern.getEPos();
    const Amg::Vector3D dir = houghpattern.getEDir();

    ATH_MSG_DEBUG("position: " << pos[0] << " " << pos[1] << " " << pos[2]);
    ATH_MSG_DEBUG("direction: " << dir[0] << " " << dir[1] << " " << dir[2]);
    std::unique_ptr<Muon::MuonPrdPattern> muonpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);

    for (unsigned int i = 0; i < houghpattern.size(); ++i) {
        muonpattern->addPrd(houghpattern.getPrd(i));

        ATH_MSG_VERBOSE("PrepRawData Added " << houghpattern.getPrd(i));
    }

    return muonpattern;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonHoughPatternTool::houghPatternsToOneEtaPattern(const MuonHoughPattern& houghpattern1,
                                                                                         const MuonHoughPattern& houghpattern2) const {
    ATH_MSG_DEBUG("houghPatternsToOneEtaPattern");

    const int ns1 = houghpattern1.size();
    const int ns2 = houghpattern2.size();

    const double the1 = houghpattern1.getETheta();
    const double the2 = houghpattern2.getETheta();
    const double theta = (ns1 * the1 + ns2 * the2) / (ns1 + ns2);

    const double phi1 = houghpattern1.getEPhi();
    const double phi2 = houghpattern2.getEPhi();
    const double cos_phi = (ns1 * std::cos(phi1) + ns2 * std::cos(phi2)) / (ns1 + ns2);
    const double sin_phi = (ns1 * std::sin(phi1) + ns2 * std::sin(phi2)) / (ns1 + ns2);
    const double phi = std::atan2(sin_phi, cos_phi);

    const double invcur1 = 1. / houghpattern1.getECurvature();
    const double invcur2 = 1. / houghpattern2.getECurvature();

    const Amg::Vector3D position1 = houghpattern1.getEPos();
    const Amg::Vector3D position2 = houghpattern2.getEPos();
    const double z0 = (ns1 * position1[2] + ns2 * position2[2]) / (ns1 + ns2);

    const double invcur = (ns1 * invcur1 + ns2 * invcur2) / (ns1 + ns2);

    ATH_MSG_DEBUG("Start Making one eta pattern theta " << theta << " phi " << phi << " invcur " << invcur);

    ATH_MSG_DEBUG("eta patterns theta1  " << the1 << " theta2 " << the2 << " phi1 " << phi1 << " phi2 " << phi2 << " invcur1 " << invcur1
                                          << " invcur2 " << invcur2 << " ns1 " << ns1 << " ns2 " << ns2);

    ATH_MSG_DEBUG(" z values " << z0 << " z1 " << position1[2] << " z2 " << position2[2]);

    // require at least two eta hits on muon pattern

    if (ns1 + ns2 < 2) return nullptr;

    double invcurvature = invcur;
    double charge = 1.;
    if (invcurvature < 0) charge = -1;
    double pscale = 1.;
    if (invcurvature != 0) pscale = 1. / std::abs(invcurvature);

    double r0 = 0.001;

    if (m_use_cosmics) {  // calculate new r0
        r0 = (ns1 * houghpattern1.getERPhi() + ns2 * houghpattern2.getERPhi()) / (ns1 + ns2);
        ATH_MSG_DEBUG("New r0: " << r0);
    }

    double x0 = charge * r0 * sin_phi;
    double y0 = -charge * r0 * cos_phi;

    ATH_MSG_DEBUG(" Lift one Eta pattern charge " << charge << " curvature  " << pscale);

    const Amg::Vector3D pos = Amg::Vector3D(x0, y0, z0);
    const Amg::Vector3D dir =
        Amg::Vector3D(pscale * std::sin(theta) * cos_phi, pscale * std::sin(theta) * sin_phi, pscale * std::cos(theta));

    std::unique_ptr<Muon::MuonPrdPattern> muonpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);
    int neta = 0;

    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double the = houghpattern1.getTheta(i);
        muonpattern->addPrd(houghpattern1.getPrd(i));
        ++neta;
        ATH_MSG_VERBOSE("PrepRawData Added theta " << the);
    }

    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        bool accept = true;
        double the = houghpattern2.getTheta(i);
        for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
            if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
        }
        if (accept) {
            muonpattern->addPrd(houghpattern2.getPrd(i));
            ++neta;
            ATH_MSG_VERBOSE("PrepRawData Added theta " << the);
        } else {
            ATH_MSG_VERBOSE(" PrepRawData already there " << the);
        }
    }

    ATH_MSG_VERBOSE(" END make One Eta pattern hits " << neta);

    return muonpattern;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonHoughPatternTool::houghPatternsToOnePhiPattern(const MuonHoughPattern& houghpattern1,
                                                                                         const MuonHoughPattern& houghpattern2) const {
    /** IP constraint used, should not be used for cosmics! */

    ATH_MSG_DEBUG("houghPatternsToOnePhiPattern");

    double theta = (houghpattern1.getETheta() + houghpattern2.getETheta()) / 2.;
    double cos_phi{0.}, sin_phi{0.};
    int nphi = 0;
    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double phi = houghpattern1.getPhi(i);
        cos_phi += std::cos(phi);
        sin_phi += std::sin(phi);
        ++nphi;
    }
    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        double phi = houghpattern2.getPhi(i);
        bool accept = true;
        for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
            if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
        }
        if (accept) {
            cos_phi += std::cos(phi);
            sin_phi += std::sin(phi);
            ++nphi;
        }
    }

    ATH_MSG_VERBOSE("Start Merged Phi hits cleaning with " << nphi << " hits ");
    // require at least two phi hits on muon phi pattern

    if (nphi < 2) return nullptr;

    double cphit = cos_phi / nphi;
    double sphit = sin_phi / nphi;

    cos_phi = 0.;
    sin_phi = 0.;
    nphi = 0;

    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double phi = houghpattern1.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.95) {
            cos_phi += std::cos(phi);
            sin_phi += std::sin(phi);
            ++nphi;
        }
    }

    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        double phi = houghpattern2.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.95) {
            bool accept = true;
            for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
                if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
            }
            if (accept) {
                cos_phi += std::cos(phi);
                sin_phi += std::sin(phi);
                ++nphi;
            }
        }
    }

    if (nphi < 2) return nullptr;
    cphit = cos_phi / nphi;
    sphit = sin_phi / nphi;

    cos_phi = 0.;
    sin_phi = 0.;
    nphi = 0;
    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double phi = houghpattern1.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.99) {
            cos_phi += std::cos(phi);
            sin_phi += std::sin(phi);
            ++nphi;
        }
    }
    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        double phi = houghpattern2.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.99) {
            bool accept = true;
            for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
                if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
            }
            if (accept) {
                cos_phi += std::cos(phi);
                sin_phi += std::sin(phi);
                ++nphi;
            }
        }
    }
    if (nphi < 2) return nullptr;
    cphit = cos_phi / nphi;
    sphit = sin_phi / nphi;

    theta = 0;
    cos_phi = 0.;
    sin_phi = 0.;
    nphi = 0;
    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double phi = houghpattern1.getPhi(i);
        double thetah = houghpattern1.getTheta(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.995) {
            cos_phi += std::cos(phi);
            sin_phi += std::sin(phi);
            theta += thetah;
            ++nphi;
        }
    }
    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        double phi = houghpattern2.getPhi(i);
        double thetah = houghpattern2.getTheta(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.995) {
            bool accept = true;
            for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
                if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
            }
            if (accept) {
                cos_phi += std::cos(phi);
                sin_phi += std::sin(phi);
                theta += thetah;
                ++nphi;
            }
        }
    }
    if (nphi < 2) return nullptr;
    cphit = cos_phi / nphi;
    sphit = sin_phi / nphi;
    theta = theta / nphi;

    double r0 = 1.;  // put 1 mm r0 value
    double x0 = r0 * sphit;
    double y0 = -r0 * cphit;
    double z0 = 0.;

    const Amg::Vector3D pos{x0, y0, z0};
    const Amg::Vector3D dir{std::sin(theta) * cphit, std::sin(theta) * sphit, std::cos(theta)};

    std::unique_ptr<Muon::MuonPrdPattern> muonpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);
    nphi = 0;

    for (unsigned int i = 0; i < houghpattern1.size(); ++i) {
        double phi = houghpattern1.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.995) {
            muonpattern->addPrd(houghpattern1.getPrd(i));
            ++nphi;
            ATH_MSG_VERBOSE("PrepRawData Merged Clean Phi Added " << phi);
        } else {
            ATH_MSG_VERBOSE("PrepRawData Merged Phi Dropped " << phi);
        }
    }

    for (unsigned int i = 0; i < houghpattern2.size(); ++i) {
        double phi = houghpattern2.getPhi(i);
        double dotprod = cphit * std::cos(phi) + sphit * std::sin(phi);
        if (dotprod > 0.995) {
            bool accept = true;
            for (unsigned int j = 0; j < houghpattern1.size(); ++j) {
                if (houghpattern2.getPrd(i) == houghpattern1.getPrd(j)) accept = false;
            }
            if (accept) {
                muonpattern->addPrd(houghpattern2.getPrd(i));
                ++nphi;
                ATH_MSG_VERBOSE("PrepRawData Merged Clean Phi Added " << phi);
            } else {
                ATH_MSG_VERBOSE("PrepRawData Merged Phi Dropped " << phi);
            }
        } else {
            ATH_MSG_VERBOSE("PrepRawData Merged Phi Dropped " << phi);
        }
    }

    ATH_MSG_VERBOSE("END Clean Merged Phi hits " << nphi);

    return muonpattern;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonHoughPatternTool::houghPatternToCleanPhiPattern(MuonHoughPattern& houghpattern) const {
    /** IP constraint used, should not be used for cosmics! */

    // TODO: rewrite with removing hits from patterns, instead of building up

    ATH_MSG_DEBUG("houghPatternToCleanPhiPattern");

    if (msgLevel(MSG::VERBOSE)) {
        for (unsigned int i = 0; i < houghpattern.size(); ++i) {
            const std::shared_ptr<MuonHoughHit> hit = houghpattern.getHit(i);
            ATH_MSG_VERBOSE(hit->getHitx() << " " << hit->getHity() << " " << hit->getHitz() << " " << hit->getPhi() << " "
                                           << hit->getMeasuresPhi() << " " << hit->getWhichDetector() << " " << hit->getWeight() << " "
                                           << hit->getAssociated());
        }
    }

    double theta = houghpattern.getETheta();
    unsigned int size = houghpattern.size();
    double phi = houghpattern.getEPhi();
    double r0 = houghpattern.getERPhi();

    ATH_MSG_DEBUG("Start Phi hits cleaning with " << size << " hits "
                                                  << " theta " << theta);
    ATH_MSG_DEBUG("Start Phi: " << phi << " r0: " << r0);
    houghpattern.updateParametersRPhi();
    unsigned int newsize = houghpattern.size();

    ATH_MSG_VERBOSE("size: " << newsize << " r0: " << r0 << " phi: " << phi);

    CxxUtils::sincos scphi(phi);

    const int number_of_iterations = 4;
    static constexpr std::array<double, number_of_iterations> cutvalues{1000., 500., 250., 125.};

    const MuonHoughPattern* newpattern{&houghpattern};
    std::unique_ptr<MuonHoughPattern> pat_owner{};
    for (int it = 0; it < number_of_iterations; it++) {
        bool change = true;
        while (change) {
            ATH_MSG_VERBOSE("size: " << newsize << " r0: " << r0 << " phi: " << phi);

            double max_dist = 0.;
            unsigned int max_i = UINT_MAX;
            for (unsigned int i = 0; i < newpattern->size(); ++i) {
                double dist = newpattern->getHitx(i) * scphi.sn - newpattern->getHity(i) * scphi.cs - r0;
                ATH_MSG_VERBOSE("Dist: " << dist);
                if (dist > max_dist) {
                    max_dist = dist;
                    max_i = i;
                }
            }
            if (max_dist < cutvalues[it]) {
                change = false;
            } else {
                std::unique_ptr<MuonHoughPattern> newpattern2 = std::make_unique<MuonHoughPattern>(MuonHough::hough_xy);
                for (unsigned int i = 0; i < newpattern->size(); ++i) {
                    if (i != max_i) { newpattern2->addHit(newpattern->getHit(i)); }
                }
                newpattern2->updateParametersRPhi();
                phi = newpattern2->getEPhi();
                r0 = newpattern2->getERPhi();
                newsize = newpattern2->size();
                pat_owner = std::move(newpattern2);
                newpattern = pat_owner.get();
                scphi = CxxUtils::sincos(phi);
            }
        }
    }

    ATH_MSG_DEBUG("Final size: " << newsize << " r0: " << r0 << " phi: " << phi);

    double thetanew = 0.;
    for (unsigned int i = 0; i < newpattern->size(); ++i) {
        double thetah = newpattern->getTheta(i);
        thetanew += thetah;
    }

    thetanew = thetanew / (newpattern->size() + 1e-7);

    double r0_new = 1.;  // put 1 mm r0 value
    double x0_new = r0_new * scphi.sn;
    double y0_new = -r0_new * scphi.cs;
    double z0_new = 0.;

    CxxUtils::sincos sctheta(thetanew);

    const Amg::Vector3D pos = Amg::Vector3D(x0_new, y0_new, z0_new);
    const Amg::Vector3D dir = Amg::Vector3D(sctheta.sn * scphi.cs, sctheta.sn * scphi.sn, sctheta.cs);

    std::unique_ptr<Muon::MuonPrdPattern> muonpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);

    for (unsigned int i = 0; i < newpattern->size(); ++i) { muonpattern->addPrd(newpattern->getPrd(i)); }

    ATH_MSG_DEBUG("END Clean Phi hits " << newsize << " theta " << thetanew);

    ATH_MSG_VERBOSE("cleaned pattern: ");
    if (msgLvl(MSG::VERBOSE)) { printPattern(muonpattern.get()); }
    return muonpattern;
}

std::unique_ptr<MuonHoughHitContainer> MuonHoughPatternTool::hitsNotInPattern(const MuonHoughHitContainer& event, int /*id_number*/) {
    std::unique_ptr<MuonHoughHitContainer> hits_not_in_patterns = std::make_unique<MuonHoughHitContainer>();

    for (unsigned int hitid = 0; hitid < event.size(); ++hitid) {
        if (!event.getHit(hitid)->getAssociated()) { hits_not_in_patterns->addHit(event.getHit(hitid)); }
    }
    return hits_not_in_patterns;
}
unsigned int MuonHoughPatternTool::getThresholdHoughPattern(int id_number) const {
    unsigned int thresholdpattern = 0;
    switch (id_number) {
        case MuonHough::hough_xy:
        case MuonHough::hough_yz: thresholdpattern = m_thresholdpattern_xyz; break;
        case MuonHough::hough_rz:
        case MuonHough::hough_rzcosmics:
        case MuonHough::hough_rz_rpc:
        case MuonHough::hough_rz_mdt:
        case MuonHough::hough_curved_at_a_cylinder: thresholdpattern = m_thresholdpattern_rz; break;
        default: ATH_MSG_WARNING("no valid id (id_number)");
    }  // switch
    return thresholdpattern;
}

double MuonHoughPatternTool::getThresholdHisto(int id_number) const {
    double thresholdhisto = 0.;
    switch (id_number) {
        case MuonHough::hough_xy:
        case MuonHough::hough_yz: thresholdhisto = m_thresholdhisto_xyz; break;
        case MuonHough::hough_rz:
        case MuonHough::hough_rzcosmics:
        case MuonHough::hough_rz_rpc:
        case MuonHough::hough_rz_mdt:
        case MuonHough::hough_curved_at_a_cylinder: thresholdhisto = m_thresholdhisto_rz; break;
        default: ATH_MSG_WARNING("no valid id (id_number)");
    }  // switch
    return thresholdhisto;
}

void MuonHoughPatternTool::setWeightMdtCutValue(const MuonHoughHitContainer& event, double& weightmdt) const {
    if (m_use_cosmics) {
        weightmdt = 0.;
        return;
    }
    int mdthits = event.getMDThitno();  // slow function!
    weightmdt = mdthits > 0. ? 1. - 5. / std::sqrt(mdthits) : 0.;
}

bool MuonHoughPatternTool::hitThroughCut(const std::shared_ptr<MuonHoughHit>& hit, double weightmdt) const {
    return (!m_weightcutmdt || hit->getDetectorId() != MuonHough::MDT || hit->getProbability() >= weightmdt) &&
           (!m_weightcut || hit->getProbability() >= m_weight);
}

void MuonHoughPatternTool::printPattern(Muon::MuonPrdPattern* muonpattern) const {
    if (!muonpattern) {
        ATH_MSG_VERBOSE("Printout of Pattern: nullptr");
        return;
    }
    ATH_MSG_VERBOSE("Printout of Pattern: ");
    for (unsigned int k = 0; k < muonpattern->numberOfContainedPrds(); k++) {
        const Trk::PrepRawData* prd = muonpattern->prd(k);
        const Muon::MdtPrepData* mdtprd = dynamic_cast<const Muon::MdtPrepData*>(prd);
        if (mdtprd) {
            const Trk::Surface& surface = mdtprd->detectorElement()->surface(mdtprd->identify());
            const Amg::Vector3D& gpos = surface.center();
            ATH_MSG_VERBOSE("mdt " << k << " x: " << gpos.x() << " y: " << gpos.y() << " z: " << gpos.z());
        } else if (!mdtprd) {
            const Muon::MuonCluster* muoncluster = dynamic_cast<const Muon::MuonCluster*>(prd);
            if (muoncluster) {
                const Amg::Vector3D& gpos = muoncluster->globalPosition();
                ATH_MSG_VERBOSE("cluster " << k << " x: " << gpos.x() << " y: " << gpos.y() << " z: " << gpos.z());
            }
            if (!muoncluster) { ATH_MSG_DEBUG("no muon prd? "); }
        }
    }
}
