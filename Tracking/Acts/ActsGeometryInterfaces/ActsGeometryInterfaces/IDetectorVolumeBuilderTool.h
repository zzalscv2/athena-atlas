/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSDETECTORVOLUMEBUILDERTOOL_H
#define ACTSGEOMETRYINTERFACES_IACTSDETECTORVOLUMEBUILDERTOOL_H

#include "GaudiKernel/IAlgTool.h"

#include "Acts/Detector/interface/IDetectorComponentBuilder.hpp"


namespace ActsTrk{
    class IDetectorVolumeBuilderTool : public Acts::Experimental::IDetectorComponentBuilder, virtual public IAlgTool
 {
        public:
        DeclareInterfaceID(IDetectorVolumeBuilderTool, 1, 0);

        virtual ~IDetectorVolumeBuilderTool() = default;
    };
}

#endif