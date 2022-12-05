/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IjFEXFormTOBs.h  -
//                              -------------------
//     begin                : 11 08 2022
//     email                : sergi.rodriguez@cern.ch
//  ***************************************************************************/

#ifndef IjFEXFormTOBs_H
#define IjFEXFormTOBs_H

#include "GaudiKernel/IAlgTool.h"

namespace LVL1 {

/*
Interface definition for eFEXFormTOBs
*/

  static const InterfaceID IID_IjFEXFormTOBs("LVL1::IjFEXFormTOBs", 1, 0);

  class IjFEXFormTOBs : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual uint32_t formTauTOB  (int, int, int, int, int, int, int) = 0;
    virtual uint32_t formSRJetTOB(int, int, int, int, int, int) = 0;
    virtual uint32_t formLRJetTOB(int, int, int, int, int, int) = 0;
    virtual uint32_t formSumETTOB(int, int, int ) = 0;
    virtual uint32_t formMetTOB  (int, int, int ) = 0;

  private:

  };

  inline const InterfaceID& LVL1::IjFEXFormTOBs::interfaceID()
  {
    return IID_IjFEXFormTOBs;
  }

} // end of namespace

#endif

