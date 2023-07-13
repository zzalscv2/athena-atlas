/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWASBUILTDBDATA_H
#define MUONCONDDATA_NSWASBUILTDBDATA_H

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include <memory>
#ifndef SIMULATIONBASE
#include "MuonNSWAsBuilt/StripCalculator.h"
#include "MuonNSWAsBuilt/StgcStripCalculator.h"
#endif

class NswAsBuiltDbData {

public:
#ifndef SIMULATIONBASE
    /// Storage to the micromega as built calculator
    std::unique_ptr<const NswAsBuilt::StripCalculator> microMegaData{};
    /// Storage to the stgc as built calculator
    std::unique_ptr<const NswAsBuilt::StgcStripCalculator> sTgcData{};
#endif
    NswAsBuiltDbData() = default;
    virtual ~NswAsBuiltDbData() = default;

};

CLASS_DEF( NswAsBuiltDbData , 163462850 , 1 )
CLASS_DEF( CondCont<NswAsBuiltDbData> , 20792446 , 1 )

#endif
