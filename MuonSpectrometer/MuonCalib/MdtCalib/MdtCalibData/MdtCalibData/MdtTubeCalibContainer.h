/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MDTTUBECALIBCONTAINER_H
#define MUONCALIB_MDTTUBECALIBCONTAINER_H

#include <iostream>
#include <string>
#include <vector>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonCalib {

    /** Holds single-tube calibration constants of one chamber */
    class MdtTubeCalibContainer {
    public:
        struct SingleTubeCalib {
            //!< relative t0 in chamber (ns)
            float t0{0.f};  
            //!< inverse propagation speed (ns/mm)              
            float inversePropSpeed{0.f};
            //!< multiplicative correction factor for ADC measurement w.r.t. multilayer average (around 1.0)
            float adcCal{0.f};            
            //!< quality flag for the SingleTubeCalib constants: 0 all ok, 1 no hits found, 2 too few hits, 3 bad chi2
            unsigned int statusCode{1};  
            SingleTubeCalib() = default;
                        
            operator bool() const { return inversePropSpeed >0.;}
        };

        /** nMl = number of multilayres, nLayers = number of layers in multilayer (3 or 4); nTubes = number of tubes in one layer */
        MdtTubeCalibContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                              const Identifier& moduleID);

        /** return calibration constants of a single tube */
        const SingleTubeCalib* getCalib(const Identifier& tubeId) const {
            unsigned int idx = vectorIndex(tubeId);
            if (idx >= m_data.size()) return nullptr;
            return &m_data[idx];
        };

        /** set the calibration constants of a single tube */
        bool setCalib(SingleTubeCalib&& val, const Identifier& tubeId, MsgStream& msg);

        /** return container name and dimensions */
        const Identifier& identify() const { return m_moduleID; }
        unsigned int size() const { return m_data.size(); }
        unsigned int numMultilayers() const { return m_nMl; };
        unsigned int numLayers() const { return m_nLayers; };
        unsigned int numTubes() const { return m_nTubes; };

        const Muon::IMuonIdHelperSvc* idHelperSvc() const;

    protected:
        const Identifier m_moduleID{};
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        const MdtIdHelper& m_idHelper{m_idHelperSvc->mdtIdHelper()};
       
        /** calculate postion of tube in vector */
        unsigned int vectorIndex(const Identifier& measID) const {            
            return (m_idHelper.multilayer(measID) - 1) * (m_nLayers * m_nTubes) + 
                   (m_idHelper.tubeLayer(measID) - 1) * m_nTubes  + 
                   (m_idHelper.tube(measID) -1);
        }
        unsigned int m_nMl{0};       //!< number of multilayers in chamber
        unsigned int m_nLayers{0};   //!< number of layer
        unsigned int m_nTubes{0};    //!< number of tubes
        std::vector<SingleTubeCalib> m_data{};
    };

}  // namespace MuonCalib

#endif
