///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETEVENTTPCNV_JETCNVPROPERTIES_H 
#define JETEVENTTPCNV_JETCNVPROPERTIES_H 

class JetCnvProperties {
protected:
  /**
   * Write Jet constituents.
   *
   * Used in JetCnv_p5/6. This flag used to be configurable but
   * since it was never set to true, it is now const.
   */
  inline static const bool s_write0constit{false};
  
};

#endif
