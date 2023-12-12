/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MDT_Digitization/RT_Relation_DB_DigiTool.h"

#include <iostream>

#include "MDT_Digitization/MdtDigiToolInput.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

using namespace MuonGM;

RT_Relation_DB_DigiTool::RT_Relation_DB_DigiTool(const std::string &type, const std::string &name, const IInterface *parent) :
    AthAlgTool(type, name, parent)  {
    declareInterface<IMDT_DigitizationTool>(this);
}

StatusCode RT_Relation_DB_DigiTool::initialize() {
    ATH_MSG_INFO("Initializing RT_Relation_DB_DigiTool");

    const MuonGM::MuonDetectorManager* detMgr{nullptr};
    ATH_CHECK(detStore()->retrieve(detMgr));    
    m_maxRadius = detMgr->getGenericMdtDescriptor()->innerRadius;
    ATH_CHECK(m_calibDbKey.initialize());
    return StatusCode::SUCCESS;
}

MdtDigiToolOutput RT_Relation_DB_DigiTool::digitize(const MdtDigiToolInput &input, CLHEP::HepRandomEngine *rndmEngine) {
    ATH_MSG_DEBUG("Digitizing input ");

    if (isTubeEfficient(input.radius(), rndmEngine)) {
        Identifier DigitId = input.getHitID();
        MdtDigiToolOutput output(true, getDriftTime(input.radius(), DigitId, rndmEngine), getAdcResponse(input.radius(), rndmEngine));

        return output;
    }
    MdtDigiToolOutput output(false, 0., 0.);

    return output;
}

double RT_Relation_DB_DigiTool::getDriftTime(double r, Identifier DigitId, CLHEP::HepRandomEngine *rndmEngine) const {
    // Get RT relation from DB
    SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> calibConstants{m_calibDbKey};

    if (!calibConstants.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve calib constants "<<m_calibDbKey.fullKey());
        throw std::runtime_error("No Mdt calibration constants");
    }
    using RtRelationPtr = MuonCalib::MdtCalibDataContainer::RtRelationPtr;
    const RtRelationPtr& data{calibConstants->getCalibData(DigitId, msgStream())->rtRelation};

    double time = 0.0;
    double t = 0.0;
    bool outOfBound = false;

    if (data) {
        // get RT relation and resolution function
        const MuonCalib::IRtRelation *rtRelation = data->rt();
        const MuonCalib::IRtResolution *rtResolution = data->rtRes();

        // get inverse rt and calculate time resolution
        const MuonCalib::TrRelation *trRelation = data->tr();
        time = trRelation->tFromR(std::abs(r), outOfBound);

        if (time < 0.0) {
            time = 0.0;
            ATH_MSG_WARNING("Drift time <0 ! Returning 0.0 as drift time");
            return (time);
        }

        double radiusWidth = rtResolution->resolution(time);
        double velocity = rtRelation->driftvelocity(time);
        // std::cout << "time = " << time << "  drift radius = " << r << "  outOfBound = "<< outOfBound << "  velocity = " << velocity <<
        // std::endl;

        if (velocity <= 0) {
            ATH_MSG_WARNING("Drift velocity <=0 ! Time will not be smeared with resolution but will take the default r-t value");
            return time;
        }
        double timeWidth = radiusWidth / velocity;

        // now smear t according to t resolution
        double tUp = rtRelation->tUpper();
        // double tLow = rtRelation->tLower();

        double tmin = time - 3.4 * timeWidth;
        double tmax = time + 3.4 * timeWidth;

        bool outOfBound2 = false;
        if (tmin < 0.0) tmin = 0.0;
        if (tmax > tUp)
            tmax =
                trRelation->tFromR(m_maxRadius, outOfBound2);  // tmax = tUp+tLow; //means: tmax  = (tmax of rt relation) + (one binwidth )

        double gaussian;
        constexpr double sqrt_one_over_two_pi = 0.39894228;
        double p1r = 0.8480 * std::exp(-0.5879 * r);
        int flag = 0;
        int cutoff = 0;

        do {
            cutoff++;  // avoid eternal loop in case of problems
            t = CLHEP::RandFlat::shoot(rndmEngine, tmin, tmax);

            gaussian = (1 - p1r) * sqrt_one_over_two_pi * exp(-(t - time) * (t - time) / (2.0 * timeWidth * timeWidth));
            if (gaussian >= CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0) || cutoff > 200) { flag = 1; }
        } while (flag == 0);

        // print summary
        ATH_MSG_DEBUG("t from r = " << time << "  t resolution = " << timeWidth << "\nr resolution = " << radiusWidth
                                    << "  driftvelocity = " << velocity << "\nsmeared t = " << t << "  cutoff = " << cutoff);
    } else {
        ATH_MSG_ERROR("Null pointer returned from CalibDBSvc. Returning 0.0 as drift time");
        return 0.0;
    }

    return t;
}

double RT_Relation_DB_DigiTool::getAdcResponse(double radius, CLHEP::HepRandomEngine *rndmEngine) {
    // parametrization of the average adc value with respect to radius
    constexpr double p0 = 57.38141;
    constexpr double p1 = 8.616943;
    constexpr double p2 = 2.497827;
    constexpr double p3 = -1.625900;
    constexpr double p4 = 0.3125281;
    constexpr double p5 = -0.02929554;
    constexpr double p6 = 0.001367115;
    constexpr double p7 = -0.00002541936;

    double adcfunc = p0 + p1 * radius + p2 * std::pow(radius, 2) + p3 * std::pow(radius, 3) + p4 * std::pow(radius, 4) +
                     p5 * std::pow(radius, 5) + p6 * std::pow(radius, 6) + p7 * std::pow(radius, 7);

    // now the resolution function
    constexpr double g0 = 10.27808;
    constexpr double g1 = -0.3774593;
    constexpr double g2 = 0.02751001;
    constexpr double g3 = -0.0005994742;

    double adcWidth = g0 + g1 * radius + g2 * std::pow(radius, 2) + g3 * std::pow(radius, 3);

    // now smear according to adc width
    double adc = CLHEP::RandGaussZiggurat::shoot(rndmEngine, adcfunc, adcWidth);

    return adc;
}
