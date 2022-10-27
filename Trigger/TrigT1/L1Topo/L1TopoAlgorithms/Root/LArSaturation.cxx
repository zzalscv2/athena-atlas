/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * LArSaturation.cpp
 * Author: Teng Jian Khoo
 *
 * @brief algorithm that computes the multiplicity for a specified list and ET threshold
 * line 1: 0 or 1, line 1 and 2 : 2 or more, uses 2 bits
 *
 * @param NumberLeading MinET

**********************************/


#include <L1TopoAlgorithms/LArSaturation.h>
#include "L1TopoCommon/Exception.h" 
#include "L1TopoInterfaces/Count.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jTETOBArray.h"
#include <cmath>


REGISTER_ALG_TCS(LArSaturation)

TCS::LArSaturation::LArSaturation(const std::string & name) : CountingAlg(name) {

    setNumberOutputBits(12); //To-Do: Make this flexible to adapt to the menu. Each counting requires more than one bit

}


TCS::StatusCode TCS::LArSaturation::initialize(){


    m_threshold = getThreshold();

    // book histograms
    std::string hname_accept = "LArSaturation_accept_counts_"+m_threshold->name();
    bookHistMult(m_histAccept, hname_accept, "Mult_"+m_threshold->name(), "counts", 15, 0, 15);

    return StatusCode::SUCCESS;
    
}

TCS::StatusCode TCS::LArSaturation::processBitCorrect(const TCS::InputTOBArray & input,
					 Count & count){
                        return process(input, count);
}


TCS::StatusCode TCS::LArSaturation::process( const TCS::InputTOBArray & /*input*/,
			       Count & count )
{

    int counting = 0;
    fillHist1D( m_histAccept[0], counting);
    
    // If required, cast input to appropriate type (jTE) and check saturation bit
    // similarly to EnergyThreshold algorithm

    // Pass counting to TCS::Count object - output bits are composed there
    count.setSizeCount(counting);

  return TCS::StatusCode::SUCCESS;
}
