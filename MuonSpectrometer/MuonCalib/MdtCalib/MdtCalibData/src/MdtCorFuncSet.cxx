/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/MdtCorFuncSet.h"

namespace MuonCalib {

    MdtCorFuncSet::MdtCorFuncSet(std::unique_ptr<IMdtSlewCorFunc>&& s, 
                                 std::unique_ptr<IMdtBFieldCorFunc>&& bf, 
                                 std::unique_ptr<IMdtTempCorFunc>&& t, 
                                 std::unique_ptr<IMdtBackgroundCorFunc>&& bg,
                                 std::unique_ptr<IMdtWireSagCorFunc>&& w) :
    m_slewing(std::move(s)), 
    m_bField(std::move(bf)), 
    m_temperature(std::move(t)), 
    m_background(std::move(bg)), 
    m_wireSag(std::move(w)) {}


    void MdtCorFuncSet::setSlewing(std::unique_ptr<IMdtSlewCorFunc>&& slew) {
        m_slewing = std::move(slew);
    }
    void MdtCorFuncSet::setBField(std::unique_ptr<IMdtBFieldCorFunc>&& bField) {
        m_bField = std::move(bField);
    }
    void MdtCorFuncSet::setTemperature(std::unique_ptr<IMdtTempCorFunc>&& temperature) {
        m_temperature = std::move(temperature);
    }
    void MdtCorFuncSet::background(std::unique_ptr<IMdtBackgroundCorFunc>&& background) {
        m_background = std::move(background);
    }
    void MdtCorFuncSet::wireSag(std::unique_ptr<IMdtWireSagCorFunc>&& wireSag) {
        m_wireSag = std::move(wireSag);
    }
}  // namespace MuonCalib
