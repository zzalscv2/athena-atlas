/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// ******************************************************************************
// ATLAS Muon Identifier Helpers Package
// ******************************************************************************

#ifndef MUONIDHELPERS_CSCIDHELPER_H
#define MUONIDHELPERS_CSCIDHELPER_H

#include "MuonIdHelpers/MuonIdHelper.h"

// ******************************************************************************
// class CscIdHelper
// ******************************************************************************
//
// Description
// -----------
// This factory class constructs CSC identifiers and ranges and provides access
// to the levels.  ATLAS note ATL-MUON-2001-014 provides a complete description
// of the hierarchical identifier scheme.  CscIdHelper provides an interface to the
// following fields of the identifier.
//
// Field           Range               Notes
// ==============================================================================
// StationName     unsigned integer    maps to "CSS", "CSL", etc.
// StationEta      [-1,1]              -1 for backward, 1 for forward endcap
// StationPhi      [1,8]               increases with Phi
// Technology      [1]                 maps to "CSC"
// ChamberLayer    [1,2]               increases with |Z|
// WireLayer       [1,4]               increases with |Z|
// MeasuresPhi     [0,1]               0 if measures R, 1 if measures Phi
// Strip           [1,n]               increases with R   for MeasuresPhi=0
//                                     increases with Phi for MeasuresPhi=1
// ==============================================================================
//
// Inheritance
// -----------
// Inherits from MuonIdHelpers/MuonIdHelper.
//
// Author
// ------
// Steven Goldfarb <Steven.Goldfarb@cern.ch>
//
// Compact Id implementation by
// Ketevi A. Assamagan <ketevi@bnl.gov>
// BNL, February 27th, 2003
//
// ******************************************************************************

class CscIdHelper : public MuonIdHelper {
public:
    // Constructor

    CscIdHelper();

    // Destructor

    virtual ~CscIdHelper() = default;

    ///////////// compact identifier stuff begins //////////////////////////////////////

    /// Initialization from the identifier dictionary
    int initialize_from_dictionary(const IdDictMgr& dict_mgr) override;

    // need to overwrite get_module_hash and get_detectorElement_hash for Run2 geometries (since they contain both
    // CSC chamberLayer 1 and 2 although only chamberLayer 2 is actually built into ATLAS)
    // function checks whether chamberLayer 1 identifiers are around and in this case returns the correct module hash
    int get_module_hash(const Identifier& id, IdentifierHash& hash_id) const override;
    int get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const override;

    ///////////// compact identifier stuff ends //////////////////////////////////////

    // Identifier builders
    Identifier elementID(int stationName, int stationEta, int stationPhi) const;
    Identifier elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const;

    Identifier elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const;
    Identifier elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const;

    Identifier elementID(const Identifier& channelID) const;

    Identifier channelID(int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer, int measuresPhi,
                         int strip) const;
    Identifier channelID(int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer, int measuresPhi, int strip,
                         bool& isValid) const;

    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                         int measuresPhi, int strip) const;
    Identifier channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                         int measuresPhi, int strip, bool& isValid) const;

    Identifier channelID(const Identifier& id, int chamberLayer, int wireLayer, int measurePhi, int strip) const;
    Identifier channelID(const Identifier& id, int chamberLayer, int wireLayer, int measurePhi, int strip, bool& isValid) const;

    Identifier parentID(const Identifier& id) const;

    // for an Identifier id, get the list of the daughter readout channel ids
    void idChannels(const Identifier& id, std::vector<Identifier>& vect) const;

    // Access to levels: missing field returns 0

    int channel(const Identifier& id) const override;

    int chamberLayer(const Identifier& id) const;
    int wireLayer(const Identifier& id) const;
    bool measuresPhi(const Identifier& id) const override;
    int strip(const Identifier& id) const;
    int gasGap(const Identifier& id) const override;  // Returns chamber Layer here

    int sector(const Identifier& id) const;
    // Access to min and max of level ranges
    // to be remove when we moved to compact ids

    int stationEtaMin() const;
    int stationEtaMax() const;
    int stationPhiMin() const;
    int stationPhiMax() const;
    int chamberLayerMin() const;
    int chamberLayerMax() const;
    int wireLayerMin() const;
    int wireLayerMax() const;
    int measuresPhiMin() const;
    int measuresPhiMax() const;
    int stripMin() const;
    int stripMax() const;

    // Access to min and max of level ranges

    int stationEtaMin(const Identifier& id) const;
    int stationEtaMax(const Identifier& id) const;
    int stationPhiMin(const Identifier& id) const;
    int stationPhiMax(const Identifier& id) const;
    int chamberLayerMin(const Identifier& id) const;
    int chamberLayerMax(const Identifier& id) const;
    int wireLayerMin(const Identifier& id) const;
    int wireLayerMax(const Identifier& id) const;
    int measuresPhiMin(const Identifier& id) const;
    int measuresPhiMax(const Identifier& id) const;
    int stripMin(const Identifier& id) const;
    int stripMax(const Identifier& id) const;

    // Public validation of levels

    bool valid(const Identifier& id) const;
    bool validElement(const Identifier& id) const;

private:
    bool isStNameInTech(const std::string& stationName) const override;
    int init_id_to_hashes();
    
    // CSS / CSL
    static constexpr unsigned int s_stDim = 2;
    // -1, 1 eta dimension
    static constexpr unsigned int s_etaDim = 2;
    /// 8 phi stations
    static constexpr unsigned int s_phiDim = 8;
    /// 2 multi layer
    static constexpr unsigned int s_mlDim = 2;

    static constexpr unsigned int s_modHashDim = s_stDim * s_etaDim * s_phiDim;
    static constexpr unsigned int s_detHashDim = s_modHashDim * s_mlDim;
     
    std::array<unsigned int, s_modHashDim> m_module_hashes{};
    std::array<unsigned int, s_detHashDim> m_detectorElement_hashes{};
    
    unsigned int moduleHashIdx(const Identifier& id) const;
    unsigned int detEleHashIdx(const Identifier& id) const;
    /// Minimal station index found
    unsigned int m_stationShift{std::numeric_limits<unsigned int>::max()};
 

    // compact id indices
    size_type m_CHAMBERLAYER_INDEX{0};
    size_type m_WIRELAYER_INDEX{0};
    size_type m_MEASURESPHI_INDEX{0};

    IdDictFieldImplementation m_cla_impl;
    IdDictFieldImplementation m_lay_impl;
    IdDictFieldImplementation m_mea_impl;
    IdDictFieldImplementation m_str_impl;

    // Private validation of levels

    bool validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const;
    bool validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                      int measuresPhi, int strip) const;

    // Utility methods

    int cscTechnology() const;

    // hash offset of strips */
    int strip_hash_offsets();

    // Level indices

    enum CscIndices { ChamberLayerIndex = 5, WireLayerIndex = 6, MeasuresPhiIndex = 7, StripIndex = 8 };

    // Level ranges

    enum CscRanges {
        StationEtaMin = -1,
        StationEtaMax = 1,
        StationPhiMin = 1,
        StationPhiMax = 8,
        ChamberLayerMin = 1,
        ChamberLayerMax = 2,
        WireLayerMin = 1,
        WireLayerMax = 4,
        MeasuresPhiMin = 0,
        MeasuresPhiMax = 1,
        StripMin = 1,
        StripMax = 216  // up to AMDB P
    };
    int m_hashOffset[2][2]{};

    unsigned int m_stripMaxPhi{UINT_MAX};  // maximum number of strips for layer which measuresPhi
    unsigned int m_stripMaxEta{UINT_MAX};  // maximum number of strips for layer which does not measure phi
    bool m_hasChamLay1{false};
};

// For backwards compatibility

typedef CscIdHelper CSC_ID;

CLASS_DEF(CscIdHelper, 4171, 1)

#endif  // MUONIDHELPERS_CSCIDHELPER_H
