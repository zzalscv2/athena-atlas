/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//  LArSaturation.h
//  TopoCore
//  Currently a dummy alg, to be filled in with logic later if needed
//  Author: Teng Jian Khoo

#ifndef __TopoCore__LArSaturation__
#define __TopoCore__LArSaturation__

#include <vector>
#include <L1TopoInterfaces/CountingAlg.h>
#include "L1TopoEvent/TOBArray.h"
#include "TrigConfData/L1Threshold.h"


class TH2;


namespace TCS {

    class LArSaturation : public CountingAlg {

        public:

            LArSaturation(const std::string & name); 

            virtual ~LArSaturation() = default;

            virtual StatusCode initialize() override; 

            virtual StatusCode processBitCorrect(const TCS::InputTOBArray & input,
                                                Count & count) override final;

            virtual StatusCode process(const TCS::InputTOBArray & input, 
                                                Count & count) override final;

        private:

            TrigConf::L1Threshold const *m_threshold{nullptr};

    };
}

#endif
