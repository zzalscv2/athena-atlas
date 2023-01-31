//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRIGGER_GETQUADRANT_H
#define XAODTRIGGER_GETQUADRANT_H

namespace {
    /// This function is meant to solve one issue with the
    /// jFEX TOBs where the FPGA numbering skim is not going
    /// in increasing order.
    ///
    /// The variable @c fpgaNumber has values of
    /// (U1=0, U2=1, U4=3, U3=2) in increasing phi.
    ///
    inline unsigned int getQuadrant(unsigned int fpgaNumber) {
        switch(fpgaNumber) {
        case 2:
            return 3;
        case 3:
            return 2;
        default:
            return fpgaNumber;
        }
    }
}

#endif // XAODTRIGGER_GETQUADRANT_H
