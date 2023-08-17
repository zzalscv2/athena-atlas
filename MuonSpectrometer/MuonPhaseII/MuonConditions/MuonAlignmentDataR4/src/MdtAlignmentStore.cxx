/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#include <exception>
#include <sstream>

void MdtAlignmentStore::storeDistortion(const Identifier& detElId, const BLinePar* bline, const MdtAsBuiltPar* asBuilt) {
    if (!bline && !asBuilt) return;
    chamberDistortions& distorts = m_alignMap[detElId];
    if (distorts.bLine || distorts.asBuilt) {
        std::stringstream sstr{};
        sstr<<__FILE__<<":"<<__LINE__<<" The alignment parameter "<<detElId.get_compact()<<" is already cached ";
        throw std::runtime_error(sstr.str());
    }
    distorts.bLine = bline;
    distorts.asBuilt = asBuilt;
}