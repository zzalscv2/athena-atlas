/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkCaloExtension/CaloExtension.h"

namespace Trk {

CaloExtension::CaloExtension(
  const TrackParameters* caloEntry,
  const TrackParameters* muonEntry,
  std::vector<CurvilinearParameters>&& caloLayers)
  : m_caloEntryLayerIntersection(caloEntry)
  , m_muonEntryLayerIntersection(muonEntry)
  , m_caloLayerIntersections(std::move(caloLayers))
{}

CaloExtension::~CaloExtension()
{
  delete m_caloEntryLayerIntersection;
  delete m_muonEntryLayerIntersection;
}
}
