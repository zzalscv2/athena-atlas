/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MDTFULLCALIBDATA_H
#define MUONCALIB_MDTFULLCALIBDATA_H

#include <MdtCalibData/MdtCorFuncSet.h>
#include <MdtCalibData/MdtRtRelation.h>
#include <MdtCalibData/MdtTubeCalibContainer.h>
#include <GeoModelUtilities/TransientConstSharedPtr.h>
namespace MuonCalib {

    /** class which holds the full set of calibration constants for a given tube */
    struct MdtFullCalibData {
        using CorrectionPtr = GeoModel::TransientConstSharedPtr<MdtCorFuncSet>;
        using RtRelationPtr = GeoModel::TransientConstSharedPtr<MdtRtRelation>;
        using TubeContainerPtr = GeoModel::TransientConstSharedPtr<MdtTubeCalibContainer>;
        
        CorrectionPtr corrections{nullptr};
        RtRelationPtr rtRelation{nullptr};
        TubeContainerPtr tubeCalib{nullptr};
        /// Returns whether one of the constants is set.
        operator bool () const {
          return corrections || rtRelation || tubeCalib;
        }
    };

}  // namespace MuonCalib

#endif
