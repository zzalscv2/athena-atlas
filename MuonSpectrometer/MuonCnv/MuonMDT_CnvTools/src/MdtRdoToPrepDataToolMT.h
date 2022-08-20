/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMdtRdoToPrepDataToolMT_H
#define MUONMdtRdoToPrepDataToolMT_H

#include "MdtRdoToPrepDataToolCore.h"

namespace Muon {

    /** @class MdtRdoToPrepDataTool

        This is for the Doxygen-Documentation.
        Please delete these lines and fill in information about
        the Algorithm!
        Please precede every member function declaration with a
        short Doxygen comment stating the purpose of this function.

        @author  Edward Moyse <Edward.Moyse@cern.ch>
    */

    class MdtRdoToPrepDataToolMT : public extends<MdtRdoToPrepDataToolCore, IMuonRdoToPrepDataTool> {
    public:
        MdtRdoToPrepDataToolMT(const std::string&, const std::string&, const IInterface*);

        /** default destructor */
        virtual ~MdtRdoToPrepDataToolMT() = default;

        virtual void printPrepData() const override;
    };
}  // namespace Muon

#endif
