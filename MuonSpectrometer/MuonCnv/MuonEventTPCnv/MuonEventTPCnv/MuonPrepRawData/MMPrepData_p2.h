/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMPREPDATA_p2_TRK_H
#define MMPREPDATA_p2_TRK_H

#include "AthenaPoolUtilities/TPObjRef.h"
#include "Identifier/IdentifierHash.h"
#include "EventPrimitives/EventPrimitives.h" // for Amg::MatrixX

namespace Muon
{
  /** 
  We don't write out (from Trk::PrepRawData) 
     * m_indexAndHash (can be recomputed), 
     * m_clusId (can be recomputed - well, it's basically stored in Muon::MdtPRD_Container_p2).
  */
    class MMPrepData_p2
    {
    public:
    MMPrepData_p2() = default;
        
        std::vector< signed char > 	m_rdoList{}; //!< Store offsets
        
        /// @name Data from Trk::PrepRawData
        //@{
        float               m_locX{0.f}; //!< Equivalent to localPosition (locX) in the base class.
        float               m_errorMat{0.f}; //!< 1-d ErrorMatrix in the base class.

	short int           m_time{0};      // for single-strip PRD, that's the time measured
	int                 m_charge{0};    // for single-strip PRD, that's the charge measured
	float               m_driftDist{0.f};    // for single-strip PRD, that's the calibrated drift distance

	float               m_angle{0.f};          ///
	float               m_chisqProb{0.f};      /// these are the parameters of the muTPC reconstruction

	/// information about clusters strips
	std::vector<uint16_t>  m_stripNumbers{};
	std::vector<short int> m_stripTimes{};
	std::vector<int>       m_stripCharges{};

	std::vector<float>        m_stripDriftDist{};
	std::vector<float> m_stripDriftErrors_0_0{};
        std::vector<float> m_stripDriftErrors_1_1{};
	short m_author{0}; // contains the info about which cluster builder tool produced the PRD
	uint8_t m_quality{0};
        
        //@}
    };
}

#endif 
