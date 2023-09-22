/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISHIFTMAPTOOLS_H
#define ISHIFTMAPTOOLS_H

#include "GaudiKernel/IAlgTool.h"

class Identifier;

namespace MuonCalib {

    /**
       @class IShiftMapTools
       Interface for the ShiftMapTools inheriting from MdtCalibrationShiftMapBase

       @author der.andi@cern.ch
    */

    class IShiftMapTools : virtual public IAlgTool {
    public:
       
        virtual ~IShiftMapTools() = default;

        /* Creates the InterfaceID and interfaceID() method */
        DeclareInterfaceID(IShiftMapTools, 1, 0);

        /* get shift value */
        virtual float getValue(const Identifier& id) const = 0;
    };

}  // namespace MuonCalib

#endif
