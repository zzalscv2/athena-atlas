/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonRDO/versions/NSWTPRDOAuxContainer_v1.h"

namespace {
   static const std::string preFixStr {"NSWTP_"};
}
#define TPAUX_VARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);
namespace xAOD {
    NSWTPRDOAuxContainer_v1::NSWTPRDOAuxContainer_v1()
    : AuxContainerBase() { 
      TPAUX_VARIABLE(ROD_L1ID);
      TPAUX_VARIABLE(sectID);
      TPAUX_VARIABLE(EC);
      TPAUX_VARIABLE(BCID);
      TPAUX_VARIABLE(L1ID);
      TPAUX_VARIABLE(window_open_bcid);
      TPAUX_VARIABLE(l1a_request_bcid);
      TPAUX_VARIABLE(window_close_bcid);
      TPAUX_VARIABLE(config_window_open_bcid_offset);
      TPAUX_VARIABLE(config_l1a_request_bcid_offset);
      TPAUX_VARIABLE(config_window_close_bcid_offset);
   

   	 TPAUX_VARIABLE(pad_coincidence_wedge);
		 TPAUX_VARIABLE(pad_candidateNumber);
		 TPAUX_VARIABLE(pad_phiID);
		 TPAUX_VARIABLE(pad_bandID);
		 TPAUX_VARIABLE(pad_BCID);
		 TPAUX_VARIABLE(pad_idleFlag);
    

		 TPAUX_VARIABLE(merge_LUT_choiceSelection);
		 TPAUX_VARIABLE(merge_nsw_segmentSelector);
		 TPAUX_VARIABLE(merge_valid_segmentSelector);
		 TPAUX_VARIABLE(merge_candidateNumber);
		 TPAUX_VARIABLE(merge_segments)
     TPAUX_VARIABLE(merge_BCID_sectorID);

    }
}
#undef TPAUX_VARIABLE