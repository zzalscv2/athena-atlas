/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCPMUONOBJ_H
#define MCPMUONOBJ_H

#include "xAODMuon/Muon.h"
#include "MuonMomentumCorrections/EnumDef.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

namespace
{
    static constexpr double GeVtoMeV = 1e+3;
    static constexpr double MeVtoGeV = 1e-3;
} // namespace


namespace MCP {
   /// Basic object to cache all relevant information from the track
   struct TrackCalibObj{
        TrackCalibObj() = default;
        TrackCalibObj(const xAOD::TrackParticle* track, TrackType t, int charge,
                      DataYear year, bool isData)
            : type{t},
              is_valid{track != nullptr},
              uncalib_pt{(track != nullptr) ? track->pt() * MeVtoGeV : 0},
              calib_pt{uncalib_pt},
              eta{(track != nullptr) ? track->eta() : FLT_MAX},
              phi{(track != nullptr) ? track->phi() : FLT_MAX},
              mass{(track != nullptr) ? track->m() : 0},
              uncalib_charge{charge},
              calib_charge{uncalib_charge},
              year{year},
              isData{isData},
              pars{(track != nullptr) ? track->definingParameters()
                                      : AmgVector(5)::Zero()},
              covariance{(track != nullptr)
                             ? track->definingParametersCovMatrix()
                             : AmgSymMatrix(5)::Zero()} {}

        TrackCalibObj(const xAOD::TrackParticle* track, TrackType t, int charge,
                      double eta, double phi, DataYear year, bool isData)
            : type{t},
              is_valid{track != nullptr},
              uncalib_pt{(track != nullptr) ? track->pt() * MeVtoGeV : 0},
              calib_pt{uncalib_pt},
              eta{eta},
              phi{phi},
              mass{(track != nullptr) ? track->m() : 0},
              uncalib_charge{charge},
              calib_charge{uncalib_charge},
              year{year},
              isData{isData},
              pars{(track != nullptr) ? track->definingParameters()
                                      : AmgVector(5)()},
              covariance{(track != nullptr)
                             ? track->definingParametersCovMatrix()
                             : AmgSymMatrix(5)()} {}

        TrackCalibObj(TrackType t, int charge, double pt, double eta, double phi, double mass, AmgVector(5) pars, AmgSymMatrix(5) cov, DataYear year, bool isData):
          type{t},
          is_valid{true},
          uncalib_pt{pt*MeVtoGeV},
          calib_pt{uncalib_pt},
          eta{eta},
          phi{phi},
          mass{mass},
          uncalib_charge{charge},
          calib_charge{uncalib_charge},
          year{year},
          isData{isData},
          pars {pars},
          covariance{cov}
          {}

        TrackCalibObj(TrackType t, double pt, double eta, double phi, DataYear year, bool isData):
          type{t},
          is_valid{true},
          uncalib_pt{pt*MeVtoGeV},
          calib_pt{uncalib_pt},
          eta{eta},
          phi{phi},
          mass{-999},
          uncalib_charge{-999},
          calib_charge{-999},
          year{year},
          isData{isData},
          pars (AmgVector(5)::Zero()),
          covariance (AmgSymMatrix(5)::Zero())
          {}



        /// Flag telling the code whether this is CB/ME/ID
        const TrackType type{};
        /// Flag telling whether the track particle exists at all
        const bool is_valid{false};
        /// Value of the track-pt pre-calibration
        const double uncalib_pt{0.};
        /// Smeared track pt
        double calib_pt{0.};
        /// Value of the track-eta
        const double eta{0.};
        /// Value of the track-phi
        const double phi{0.};
        /// Value of the track-mass
        const double mass{0.};
        /// Value of the track-charge (before calibration)
        const int uncalib_charge{0};
        /// Value of the track-charge (after calibration)
        int calib_charge{0};
        // Data year
        const DataYear year{};
        // isData
        const bool isData{};

        /// Track perigee parameters.
        const AmgVector(5) pars{AmgVector(5)::Zero()};
        /// Full track covariance matrix
        const AmgSymMatrix(5) covariance{AmgSymMatrix(5)::Zero()};
    };


    struct MuonObj 
    {
        MuonObj(const TrackCalibObj& CB, const TrackCalibObj& ID, const TrackCalibObj& ME): ID{ID}, ME{ME}, CB{CB}{}
        
        TrackCalibObj ID{};
        TrackCalibObj ME{};
        TrackCalibObj CB{};

        /// Random numbers helping for the calibration
        double rnd_g0{0.};
        double rnd_g1{0.};
        double rnd_g2{0.};
        double rnd_g3{0.};
        double rnd_g4{0.};
        double rnd_g_highPt{0.};

        
        using ResolutionCategory = CP::IMuonSelectionTool::ResolutionCategory;
        ResolutionCategory raw_mst_category{ResolutionCategory::unclassified};

        // Expected resolution number for statistical combination
        double expectedResID{0.};
        double expectedResME{0.};

        double expectedPercentResID{0.};
        double expectedPercentResME{0.};

        inline double getCalibpt(TrackType type) const
        {
            if(type == MCP::TrackType::CB)      return CB.calib_pt;
            else if(type == MCP::TrackType::ID) return ID.calib_pt;
            else if(type == MCP::TrackType::ME) return ME.calib_pt;
            return 0;
        }  
    };
} 
#endif
