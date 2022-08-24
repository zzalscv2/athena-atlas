/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToPrepDataToolMT.h"

Muon::MdtRdoToPrepDataToolMT::MdtRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p) :
    base_class(t, n, p) {}
void Muon::MdtRdoToPrepDataToolMT::printPrepData() const {
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadHandleKey<Muon::MdtPrepDataContainer> k(m_mdtPrepDataContainerKey.key());
    k.initialize().ignore();
    printPrepDataImpl(SG::makeHandle(k, ctx).get());
}
