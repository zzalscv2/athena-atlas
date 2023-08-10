/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************
//                            eFEXFillEDM - description
//                              ---------------------
//      begin                 : 22 04 2021
//      email                 : nluongo@uoregon.edu
//***********************************************************************

#ifndef eFEXFillEDM_H
#define eFEXFillEDM_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IeFEXFillEDM.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"

namespace LVL1 {

  //Doxygen class description below:
  /** The eFEXFillEDM class defines how to fill eFEX EDM
  */

  class eFEXFillEDM : public AthAlgTool, virtual public IeFEXFillEDM {

  public:

    /** Constructors */

    eFEXFillEDM(const std::string& type, const std::string& name, const IInterface* parent);

    /** Destructor */

    eFEXFillEDM&& operator= (const eFEXFillEDM& ) = delete;

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode finalize() override;

    virtual StatusCode execute() override;

    /** Create and fill a new fillEmEDM object (corresponding to this window), and return a pointer to it */
    virtual void fillEmEDM(std::unique_ptr<xAOD::eFexEMRoIContainer> &container, uint8_t eFEXNumber, const std::unique_ptr<eFEXegTOB> &tobObject, bool xTOB=false) const override;
    /** Create and fill a new fillTauEDM object (corresponding to this window), and return a pointer to it */
    virtual void fillTauEDM(std::unique_ptr<xAOD::eFexTauRoIContainer> &container, uint8_t eFEXNumber, const std::unique_ptr<eFEXtauTOB> &tobObject, bool xTOB=false) const override;

    /** Internal data */
  private:

  };

} // end of namespace

#endif
