/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWERRORCALIBDATA_H
#define MUONCONDDATA_NSWERRORCALIBDATA_H


// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "AthenaBaseComps/AthMessaging.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"
#include <functional>
#include <map>

class NswErrorCalibData: public AthMessaging {
    public:
    /**
     * @brief Helper struct to be parsed to the object to
     *        derive the specific error of the cluster
     */
    struct Input {
        /// Identifier of the strip
        Identifier stripId{};
        /// Author of the cluster
        uint8_t clusterAuthor{0};
        /// Direction of the muon in the local coordinate frame
        double locTheta{0.};
        /// Azimuthal angle in the local frame
        double locPhi{0.};
        /// Local position on the surface
        Amg::Vector2D localPos{Amg::Vector2D::Zero()};
        /// Cluster size
        unsigned int clusterSize{0};
    };
    
    using errorParametrizer = std::function<double(const Input& input, 
                                                   const std::vector<double>& pars)>;
    
    static errorParametrizer getParametrizer(const std::string& funcName);
    /**
     *  Helper struct to store different error calibrations for a certain channel range & also 
     *  for seperate ClusterBuilders (ROT &  Prd making stage)
    */
    class ErrorConstants {
        public:
            ErrorConstants(const std::string& funcName,
                           uint8_t author,
                           uint16_t minStrip,
                           uint16_t maxStrip,
                           std::vector<double>&& parameters);
            /// Returns the minimal strip of validitiy
            uint16_t minStrip() const;
            /// Returns the maximal strip of validity
            uint16_t maxStrip() const;
            /// Returns the cluster author flag
            uint8_t author() const;
            /// Calculates the cluster uncertainty
            double clusterUncertainty(const Input& clustInfo) const;

        bool operator<(const ErrorConstants& other) const;
    private:       
        errorParametrizer m_evalFunc;
        /// Author of the cluster to apply the error
        uint8_t m_clusAlgAuthor{0};
        /// Strip range for which the constants are valid
        uint16_t m_stripMin{0};
        uint16_t m_stripMax{0};
        std::vector<double> m_pars{};
    };

    struct ErrorIdentifier{
        uint16_t strip{0};
        uint8_t clusAlgAuthor{0};
    };
    /// Share the same error constants amongst several gasGaps
    using ErrorConstantsSet = std::set<ErrorConstants, std::less<>>;
    
    public:
        NswErrorCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc);
         ~NswErrorCalibData() = default;

        StatusCode storeConstants(const Identifier& gasGapId,
                                  ErrorConstants&& newConstants);
    
        double clusterUncertainty(const Input& clustInfo) const;
        
    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using ErrorMap = std::unordered_map<Identifier, ErrorConstantsSet>; 
        ErrorMap m_database{};
};

bool operator<(const NswErrorCalibData::ErrorConstants& a, const NswErrorCalibData::ErrorIdentifier& b);
bool operator<(const NswErrorCalibData::ErrorIdentifier& a, const NswErrorCalibData::ErrorConstants& b);

CLASS_DEF( NswErrorCalibData , 118696870 , 1 );
CONDCONT_DEF( NswErrorCalibData , 134103948 );

#endif
