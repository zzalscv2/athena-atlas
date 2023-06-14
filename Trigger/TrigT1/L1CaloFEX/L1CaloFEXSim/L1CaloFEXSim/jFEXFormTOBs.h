/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************
//                                 jFEXFormTOBs.h
//                                 --------------
//     begin                       : 11 08 2022
//     email                       : sergi.rodriguez@cern.ch
//***********************************************************************

#ifndef jFEXFORMTOBS_H
#define jFEXFORMTOBS_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXFormTOBs.h"

namespace LVL1 {

//Doxygen class description below:
/*
 * The jFEXFormTOBs class provides functions for creating TOBs for jFEX objects
*/

class jFEXFormTOBs : public AthAlgTool, virtual public IjFEXFormTOBs {

    public:
        /** Constructors */
        jFEXFormTOBs(const std::string& type, const std::string& name, const IInterface* parent);

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;
        /** Destructor */
        virtual ~jFEXFormTOBs();

        virtual uint32_t formTauTOB  (int, int, int, int, int, int, int) override;
        virtual uint32_t formSRJetTOB(int, int, int, int, int, int) override;
        virtual uint32_t formLRJetTOB(int, int, int, int, int, int) override;
        virtual uint32_t formSumETTOB(int, int, int ) override;
        virtual uint32_t formMetTOB  (int, int, int ) override;

        /** Internal data */
    private:

        int Get_calibrated_SRj_ET(int, int, int );

};

} // end of namespace

CLASS_DEF( LVL1::jFEXFormTOBs , 186886379 , 1 )

#endif
